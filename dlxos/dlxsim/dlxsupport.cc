//
//	dlxsupport.cc
//
//	Support routines for the DLX simulator.  Hopefully, these are
//	the only routines that need to be modified to port the simulator
//	to different platforms.  This also includes the "main" routine
//	for the simulator.
//
//	Copyright (c) 1999 by Ethan L. Miller
//	University of Maryland Baltimore County
//

static char rcsid[] = "$Id: dlxsupport.cc,v 1.2 2000/09/20 22:16:34 elm Exp elm $";

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dlx.h"

static char	*pn;



//----------------------------------------------------------------------
//
//	SetDebug
//
//----------------------------------------------------------------------
void
SetDebug (char *dbs)
{
  printf("setting debug flag %s\n", dbs);

  if (dbs == NULL) {
    strcpy (debug, "");
  } else {
    strncpy (debug, dbs, 90);
  }
}

//----------------------------------------------------------------------
//
//	Cpu::SetupRawIo
//
//	Set up stdin and stdout to be raw terminals.  This is necessary
//	so that the simulator can get stuff character by character, and
//	so that the terminal doesn't buffer output (for echoing).
//
//----------------------------------------------------------------------
void
Cpu::SetupRawIo ()
{
  struct termios tio;

  // Save the old terminal info
  tcgetattr (0, &term);
  // Make stdin a raw device.
  tcgetattr (0, &tio);
#ifdef	SOLARIS
  tio.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
  tio.c_oflag &= ~OPOST;
  tio.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  tio.c_cflag &= ~(CSIZE|PARENB);
  tio.c_cflag |= CS8;
  tio.c_cc[VMIN] = 0;
  tio.c_cc[VTIME] = 0;
#else
  cfmakeraw (&tio);
#endif
  tio.c_oflag |= OPOST;
  tcsetattr (0, TCSANOW, &tio);
  // Make stdout flush on all prints.
  tcgetattr (1, &tio);
  tio.c_oflag |= FLUSHO;
  tcsetattr (1, TCSANOW, &tio);
  fcntl (0, F_SETFL, O_NONBLOCK);
}

//----------------------------------------------------------------------
//
//	Cpu::GetChar
//
//	Get the next character and put it into the keyboard buffer.
//	If the system has requested it, send an interrupt.
//
// NOTE:If the character is a CTRL-C, the simulator will exit.  This
//	is to allow users to kill the program.  If you want to have an
//	interrupt character for your OS, please pick another one.
//
//----------------------------------------------------------------------
int
Cpu::GetCharIfAvail ()
{
  char	buf[20];
  int	n = 0;
  int	i;

  if ((n = read (0, buf, sizeof (buf))) > 0) {
    // We got at least one character!
    for (i = 0; i < n; i++) {
      if (buf[i] == '\003') {
	printf ("Got a control-C: exiting!\n");
	// Reset the terminal to its original state
	tcsetattr (0, TCSANOW, &term);
	exit (1);
      }
      // Drop the character on the floor if the buffer is full.
      if (kbdbufferedchars < DLX_KBD_BUFFER_SIZE) {
	kbdbufferedchars++;
	kbdbuffer[kbdwpos++] = buf[i];
	kbdwpos %= DLX_KBD_BUFFER_SIZE;
      }
    }
  }
  return ((n > 0) && ((flags & DLX_FLAG_KBD_INTERRUPT) != 0));
}

//----------------------------------------------------------------------
//
//	Cpu::KbdPutChar
//
//	Output a single character to the screen.
//
//----------------------------------------------------------------------
void
Cpu::KbdPutChar (uint32 v)
{
  putchar (v);
}

//----------------------------------------------------------------------
//
//	usageAndExit
//
//	Print correct usage and then exit.
//
//----------------------------------------------------------------------
static
void
usageAndExit ()
{
    static char *usages[] = {
	"-x <execfile>", "execute this file",
	"-l <loadfile>", "load the data from this file, but don't run it",
	"-s <startaddr>", "specify the address at which to run",
	"-m <memorysize>", "specify the size of the simulated memory",
	"-k <stackaddr>", "specify the initial value for the stack pointer",
	"-t <time in us>", "specify the simulated time to execute one instruction",
	"-a", "pass the remaining arguments to the DLX program",
	"-e", "don't handle the exit trap in the simulator",
	"-I", "turn on instruction tracing",
	"-M", "turn on memory tracing",
	"-F", "specify trace file (default is stdout)",
#ifdef	DEBUG
	"-D <debugstr>", "specify simulator debugging options",
#endif	// DEBUG
    };
    int		i;

    printf ("Valid options for %s are:\n", pn);
    for (i = 0; i < (sizeof (usages) / sizeof (char *)); i += 2) {
	printf ("\t%-16s: %s\n", usages[i], usages[i+1]);
    }
    exit (1);
}

//----------------------------------------------------------------------
//
//	main
//
//	Main routine for the simulator.  It first parses the simulator
//	arguments and sets things up.  It then reads in the program whose
//	name is passed, and sets up the stack so the program is passed
//	the remaining arguments from the command line.
//
//	Once the program (usually an OS) has been started, main() simply
//	loops and executes instructions one at a time.
//
//----------------------------------------------------------------------
int
main (int argc, char *argv[])
{
    int		i, j;
    Cpu		*cpu = NULL;
    int		memSize = 1024 * 1024 * 2;	// default is 2 MB memory
    const int	maxMemSize = 1024 * 1024 * 16;	// Max is 16 MB memory
    const int	minMemSize = 1024 * 64;		// Min is 64 KB memory
    int		startAddr = 0;
    uint32	nStartAddr;
    uint32	stackAddr = memSize - 4;
    double	usPerInst = 1.0;
    int		argcount = 0;
    int		arglen = 0;
    int		ignoreExit = 1;
    char	*debugStr = NULL;
    uint32	argbase;
    int		tracing = 0;
    char	*tracefile = NULL;

    pn = argv[0];
    // Parse the arguments passed to the simulator
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    printf ("%s: option %s unrecognized!\n", pn, argv[i]);
	    usageAndExit ();
	} else {
	    switch (argv[i][1]) {
	    case 'h':
		usageAndExit ();
		break;
	    case 'a':
		// remainder of arguments are passed to program
		++i;
		while (i < argc) {
		    arglen += (strlen (argv[i]) + 4) & ~0x3;
		    argcount += 1;
		    i++;
		}
		break;
	    case 'e':
		ignoreExit = 0;
		break;
	    case 's':
		// specify start address
		startAddr = strtol (argv[++i], NULL, 0);
		break;
	    case 'k':
		// specify stack address
		stackAddr = strtol (argv[++i], NULL, 0);
		break;
	    case 'm':
		// specify memory size
		memSize = strtol (argv[++i], NULL, 0);
		if (memSize < minMemSize) {
		    fprintf (stderr, "%s: memory size 0x%x too small!\n",
			     pn, memSize);
		    usageAndExit ();
		} else if (memSize > maxMemSize) {
		    fprintf (stderr,
			     "%s: memory size 0x%x too big (max 0x%x)!\n",
			     pn, memSize, maxMemSize);
		    usageAndExit ();
		}
		break;
	    case 't':
		// specify instruction execution time (in us)
		usPerInst = strtod (argv[++i], NULL);
		if (usPerInst < 0.01) {
		    fprintf(stderr,"%s: min instruction time is 0.01 us!\n",pn);
		    usageAndExit ();
		} else if (usPerInst > 100.0) {
		    fprintf(stderr,"%s: max instruction time is 100 us!\n",pn);
		    usageAndExit ();
		}
		break;
	    case 'x':
		// Set the starting address automagically from the
		// file loaded in
		startAddr = -1;
		// Fall through to 'l' case
	    case 'l':
		// load data file
		if (cpu == NULL) {
		    cpu = new Cpu(memSize);
		}
		if (! (cpu->LoadMemory (argv[++i], nStartAddr))) {
		    fprintf (stderr, "%s: couldn't load %s!\n", pn,
			     argv[i]);
		    exit (1);
		}
		if (startAddr == -1) {
		    startAddr = nStartAddr;
		}
		break;
	    case 'I':
		tracing |= DLX_TRACE_INSTRUCTIONS;
		break;
	    case 'M':
		tracing |= DLX_TRACE_MEMORY;
		break;
	    case 'F':
		tracefile = argv[++i];
		break;
#ifdef	DEBUG
	    case 'D':
		debugStr = argv[++i];
		SetDebug (debugStr);
		break;
#endif	// DEBUG
	    default:
		fprintf (stderr, "%s: option '%s' unknown!\n", pn, argv[i]);
		usageAndExit ();
		break;
	    }
	}
    }
    if (cpu == NULL) {
	usageAndExit ();
    }
    if (stackAddr >= memSize) {
	printf ("%s: stack address (0x%x) must be less than memory size (0x%x)\n",
		pn, stackAddr, memSize);
	usageAndExit ();
    }
    // Place arguments on the stack as text
    stackAddr -= arglen + (argcount + 3) * sizeof (int);
    j = stackAddr;
    cpu->SetMemory(j, argcount);
    j += sizeof (int);
    cpu->SetMemory(j, j+sizeof(int));
    j += sizeof (int);
    argbase = j + (argcount * sizeof (int));
    for (i = argc - argcount; i<argc; i++, j += sizeof (int)) {
	cpu->SetMemory(j, (uint32)argbase);
	strcpy (argbase + (char *)(cpu->MemoryBase ()), argv[i]);
	argbase += (strlen (argv[i]) + 4) & ~0x3;
    }
#ifdef	DEBUG
    printf ("Stack=0x%x, pc starting at 0x%x\n", stackAddr, startAddr);
#endif	// DEBUG
    cpu->Tracing (tracing);
    if (! cpu->TraceFile (tracefile)) {
	printf ("%s: error opening %s as a trace file!\n", pn, tracefile);
    }
    cpu->IgnoreExit(ignoreExit);
    cpu->SetInstrTime (usPerInst);
    cpu->InitPC (startAddr);
    cpu->PutIreg (29, stackAddr);
    // Run the simulator
    while (1) {
	cpu->ExecOne ();
    }
}
