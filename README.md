The `c_matoduino` project is a pure C simulation of the connectome implemented in
the `nematoduino` robotics project. It is used for monte carlo and plotting purposes.

It can be compiled with:

```
gcc -I. -o c_matoduino main.c muscles.c neural_rom.c
```

Nematoduino attempts to emulate the C. Elegans nervous system
on an Arduino Uno (AtMega328). It is based heavily upon the file
'connectome.py' from this repository:

https://github.com/Connectome/GoPiGo

This software is left whatever state was most useful to me in the development
of `nematoduino`, so it needs some care and attention to be generally useful
to others.

TO DO:
======

* Clean up python plotting scripts so they're less ad-hoc
* Same for simulation code (ideally outputs should be separable from core simulation)

`c_matoduino` is covered under the GNU Public License v2.
