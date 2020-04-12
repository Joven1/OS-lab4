#!bin/bash

savefile=$1
git add *
git commit -m "Save File: $1"
git push
