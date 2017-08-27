CMatoduino is a pure C simulation of the connectome implemented in
the Nematoduino robotics projects. It is used for monte carlo and
plotting purposes.

It can be compiled with:
```
gcc -I. -o c_matoduino main.c muscles.c neural_rom.c
```

Nematoduino attempts to emulate the C. Elegans nervous system
on an Arduino Uno (AtMega328). It is based heavily upon the file
'connectome.py' from this repository:

https://github.com/Connectome/GoPiGo

This should still be considered beta software.

CMatoduino is covered under the GNU Public License v2.
