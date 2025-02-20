# Notes

These are my notes along the development of the project.

### Linking Stage

Linking handles the reference of multiple files.

when files references each other, requests resources from each other, is the linker who organizes the compiled files to get the program working because a program might be thousands of lines of code which is commonly separated in multiple files.

### WinMain

The machine never know the linkage of your program unless you're building the OS itself, this function makes sure that your program will be linked to Windows to run it properly, most of high-level compilers keep this hidden as it's just a default configuration (sort of).

whatever is in WinMain doesn't run, because it's an int i guess, now i really doesn't know how it works.

### #include

```#include```
works like a import and receive an directory between 'less than' and 'greater than' (<>).


 \- Computer are able to pull only parts of register making no neecesity to extra work for lower base numbers.

### Pointer

Pointers are basically pointing to where a variable is located in memory, just a "locator", it guards only the address of whatever it's pointing.

an asterisk (*) makes a variable in a pointer and an ampersand (&) indicates the memory location of an variable.

```
int x = 10; // any variable
int *toX; // * indicates a pointer
toX = &x // & indicates the memorey location of x
```
