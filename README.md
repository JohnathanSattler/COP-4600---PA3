COP 4600 - PA3
======

This module will allow the user to input multiple characters into a buffer at a time, and will allow the user to read the whole buffer when needed. 

###

Module:
------

Build Module: `make`

Install Input Module: `sudo insmod pa3in.ko`

Install Output Module: `sudo insmod pa3out.ko`

Remove Output Module: `sudo rmmod pa3out`

Remove Input Module: `sudo rmmod pa3in`

View Kernal Messages: `dmesg`

###

Testing:
------

Compile: `gcc -o test test.c`

Run: `sudo ./test`
