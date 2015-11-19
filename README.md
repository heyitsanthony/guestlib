# guestlib

A library for loading processes as "guests" into a host process. Useful for binary analysis.

This was originally part of another project for binary symbolic execution. I split it out
in hopes of improving the general quality of this component; it's the oldest bit and has
suffered some code rot.

## Some Goals
* Portable representation of processes
* Hide debugger interface
* Attach to processes in debugger style
* Launch new synthetic processes
* Launch snapshotted processes
* Easy control of system call dispatch
* Easy process manipulation
