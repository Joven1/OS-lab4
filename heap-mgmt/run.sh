#!/bin/bash
cd os 
make clean; make
cd ..
cd apps/example/
make clean; make; make run
#cd apps/example/make_procs
#make clean; make; make run
