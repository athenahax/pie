#!/bin/bash

gcc enc.c -o enc -lm
gcc compile/compiler.c -o compiler
mkdir release
mv enc release; mv compiler release
cp compile/stub.c release
cp evil.asm release