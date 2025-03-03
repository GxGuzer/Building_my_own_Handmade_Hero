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
toX = &x // & indicates the memory location of x
```

### Endianness

It refers to how the order of overflowed values are stored in memory.

It has two main types, Big-endian and Little-endian.

When a number is higher than a byte (higher than 8 bits, higher than one memory cell), it obivously needs to use more bytes to store that number.
The endianness comes in how to store these numbers.

let's take the number 500 for example:

500 in binary is 111110100 which has more than 8 bits so needs another cell to store it.

**Big-endian** is the mode that considers the bigger values at first.

the number 500 in memory would look like this:

| M1 | M2 |
| :---: | :---: |
| 00000001 | 11110100 |

the higher value comes first just like in our decimal numerical system,
when in number 12 for example, the digit '1' have more value than the digit '2'.

**Little-endian** is the other way around, the higher values comes at last.

keeping 500 as example, in memory, it would look like this:

| M1 | M2 |
| :---: | :---: |
| 11110100 | 00000001 |

if this mode were applied to our decimal number, the number '21' would have the value of 12.

### Structure (struct)

Create a type of object that can hold multiple values of multiples types.

if an ```int``` were a structure, it would be a structure of only one value of only one type.

The values of a structure are called members

in C++ we set structures like this:
```
struct {                // structure declaration
    int value;          // structure member
    char text;          // structure member
} structVariableNames;  // structure variable
```

let's make a struct of a small player data:
```
struct {
    string username;
    int maxHP
    int totalXP;
    int level;
} Player;

// Assigning values of the structure
Player.username = guy1234;
Player.maxHP = 100;
Player.totalXP = 1024;
Player.level = 3;

// the structure Player is ready to use
```

Structure can be named to be treated like a Data Type.
```
struct strucutureName{
    // members goes here
};
```

A structure is essentially a list of different types of values.

in C++ structures supports function and inheritance, C does not.
in C structures are always treated like a Data Type.
