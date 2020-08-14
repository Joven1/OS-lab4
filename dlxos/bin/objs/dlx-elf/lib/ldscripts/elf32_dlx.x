/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-dlx", "",
	      "")
OUTPUT_ARCH(dlx)
SEARCH_DIR("/home/shay/g/ee469/experimental/NewEE469/objs/dlx-elf/lib");
SECTIONS
{
  . = 0;
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.text)
    etext = ALIGN(1);
  }
  . = ALIGN(1);
  .data :
  {
    *(.data)
    CONSTRUCTORS
    edata  =  .;
  }
  .bss :
  {
   *(.bss)
   *(COMMON)
   end = . ;
  }
}
