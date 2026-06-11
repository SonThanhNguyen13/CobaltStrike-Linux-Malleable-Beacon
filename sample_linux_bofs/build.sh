#!/bin/sh

x86_64-linux-gnu-gcc -c -FPIC id.c -o id.x64.o || echo "[!] x86_64 compiler not found, skipping"
x86_64-linux-gnu-gcc -c -FPIC cat.c -o cat.x64.o || echo "[!] x86_64 compiler not found, skipping"