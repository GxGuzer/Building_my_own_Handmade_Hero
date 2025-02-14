# Notes

These are my notes along the development of the project.

### Linking Stage

Linking handles the reference of multiple files.

when files references each other, requests resources from each other, is the linker who organizes the compiled files to get the program working because a program might be thousands of lines of code which is commonly separated in multiple files.

### WinMain

The machine never know the linkage of your program unless you're building the OS itself, this function makes sure that your program will be linked to Windows to run it properly, most of high-level compilers keep this hidden as it's just a default configuration (sort of).