# 15/04/2025

I had a hard time trying to understand things together, searching sollutions on internet and stuff, but i realized what's wrong.
I'm using **Visual Studio Code** while Casey and most of users i found are using **Visual Studio**.

My biggest strugle was how to set up stuff. In VS (Visual Studio) you have your workspaces treated as a project, and have properties setup, in VSC (Visual Studio Code) you have a workspace just like it is.

i'm using VSC to just write the code, i compile it and run it on the terminal, i don't know how to debug on terminal.

# 22/04/2025

`typedef` is a way to making a *"new"* type of data to be called be its new name, mostly used in old C programs, for example:

```
// in C

//creating a struct.

struct playerStats {
    int health = 100;
    int mana = 20;
    int stamina = 50;
}

//calling that struct

struct playerStats playerStats;
```

```
// in C

//creating a struct with typedef.

typedef struct playerStats { //the name of the struct is now actually irrelevant.
    int health = 100;
    int mana = 20;
    int stamina = 50;
} PlayerStats; // this will be what is going to call this specific struct.

//calling that struct

PlayerStats playerStats;
```

# 08/05/2025

## Summary on creating a window

### Windows entry point

`WinMain()` is a **WINAPI** (Windows API) function that creates a Windows thread for your program.
It receives a Handle instance, command-line arguments as a character string and the flag of the window (maximized, minimized or shown normally)

### Creating and registering a class

**WNDCLASS** is a class for a window management it receives a pointer to the window procedure, the handle instance which the class will be managed and the name of the window class.

`RegisterClass()` is a function that registers the class with the operating system it receives a pointer to the class instead of the class itself.

`CreateWindowEx()` is a function that creates an instance of an window, it receives window styles, window name, window class, position and size, parent window, menu setup, instance handle and additional data.

### Handling window messages

a window can send a lot of messages about its status, in `LRESULT CALLBACK WindowProc()` LRESULT is an integer value that the program returns to Windows, it's a response to a message, CALLBACK is a name convention and `WindowProc()` is a window procedure function. It usually contains a `switch` to handle each necessary message, usually also contains the `DefWindowProc()` function which gives a default procedure for a given message, it's usually used to handle not-so important messages. Both function receives the instance handle, the message code and some additional data depending on the message.

### Translating and Dispatching messages 

the functions `TranslateMessage()` and `DispatchMessage()` are contained within a loop while `GetMessage()` does return zero.
`GetMessage()` pulls a message from the operating system message queue, if there are none the function blocks (not necessarily meaning freezing the program but it can happen, to avoid it it's good to create other threads).
`TranslateMessage()` is related to the keyboard, key strokes and etc. how it functions is not important for now.
`DispatchMessage()` tell the operating system to call the function of window procedure of such particular message.
All these three function receives a **MSG** struct pointer as a parameter.

### Showing the window

`ShowWindow()` is a function that shows the window on screen, that's simple, it receives the window handle and a value that tells how the window should be shown.

# 15/05/2025

## Painting a window

**COLORREF** is a data type of variable designed to hold color info, it's a 32-bit number where the 24 lower bits describes the RGB info.
**HBRUSH** is a handle to a brush tool used by Windows GDI (Graphics Device Interface) used to paint polygons.

We can paint our window with `BeginPaint()` and `EndPaint()` functions, they receive the window handle of the desired window to paint and a pointer to a **PAINTSTRUCT** struct.

`FillRect()` is a function to paint a rectangular area of our window, it receives a device context variable, a **RECT** struct (the **PAINTSTRUCT** has one) and a **HBRUSH** to be used.

**PAINTSTRUCT** only marks invalid areas to paint (areas to be painted), the whole window is invalid when it first launches but then only new area by resizing are invalid and will be painted.

To repaint previously painted area we need to invalidate them, we can use `InvalidateRect()` function to do that, it receives the window handle, a pointer to a **RECT** structure to mark the area to be invalidated (if **NULL** the whole window will be marked) and a boolean to set if the area must be erased (true) or not (false).
*//I've seen no difference with the `FillRect()` maybe it happen on more complex paintings.*

We can also revalidate an area with `ValidateRect()` function (this means, mark a rectangular area that should not be redrawn), it receives a window handle and a pointer to a **RECT** struct.

# 07/06/2025

## Resource Handling tips

When doing an application people like to have full control of the creation and destruction of resources individualy, even when it's not neccessary.

Think on these resources as chunks of resources instead of each individual resource.

About window, window handlers and etc. there's no need to clean that up because, first: we want at least one window through the entire execution, second: Windows automatically cleans all that up when the application is terminated.

## Definitions of `static`

From what i understand `static` when used in a variable means persitency, that is, such variable will keep the last defined value for such variable (skipping the declaration).
Example:
```
static int color = WHITE; // value of WHITE and BLACK is defined elsewhere.
if(color == WHITE) {
    color = BLACK;
}
if(color == BLACK) {
    color = WHITE;
}
```
This code will switch the `color` variable between `WHITE` and `BLACK` without redefining it to `WHITE` at the declaration.

When `static` is used on a function it means that such function cannot be called by other files.
Example:
```
// On operations.cpp
...

static int integerMultiply(term1, term2) {
    if(!isInteger(term1) || !isInteger(term2)) {
        // error handle
    }
    int result;
    result = term1 * term2;
    return result;
}
```

```
// On calculator.cpp
...

if(multiplyMode) {
    cout << integerMultiply(x1, x2) << endl; // a error will occur.
}
```

# 14/06/2025

## Rendering method

I gonna use a buffer to hold a bitmap for our graphics.

A buffer is simply a temporary space of memory used to transfer info.

## DIBs

I'm going to use DIB section to talk to Windows about our buffer, and bitmap.
DIB stands for Device Indepedent Bitmap.

I understood nothing what i've done, whoever today's session is over.

# 29/06/2025

## Rendering method

We're gonna use a backbuffer for pixels, and then allocate it to the graphics card to draw.

## Closing window process

`WM_CLOSE` is a message sent when the users attempts to close the window, if this message isn't handled Windows will just proceed to close the window, this message is a opportunity to the program intervene and ask for a confirmation or/and send a warning about losing progress.

`WM_DESTROY` is a message sent when the window has been closed on screen but still exists on the system, this happens after a `WM_CLOSE` message, at this stage the program has the chance to process unsaved info, saving or deleting them, and apply the last changes and checks to whatever it's handling. This message is sent to the window being destroyed and then to the child windows if there are any.

At this stage a `PostQuitMessage()`function should be called for the proper termination of the program.

`WM_QUIT` is the message which indicates to Windows that the program must be terminated, it is usually sented as a response for the `WM_DESTROY` message, but `PostQuitMessage()` can be called at any place by design.

## Device-Independent Bitmaps

Device-Independent Bitmaps (DIBs) are Bitmaps that can be easily stored and loaded across different devices.

It stores an array of color in red, green and blue (RGB) triplets of pixels of the image.
It also contains more info about its source such like, the color format of the device which created it, the palette of the device which created it and the resolution of the device which created it.

## Getting application area and building a buffer

### Rendering procedure summary

We're gonna to create a buffer, store whatever image we want to draw in that buffer, then we're going to resize it to fit the window and paint the window with that buffer when necessary.

We will also force a paint in order to do animation because usually Windows only repaints when a part of the window gets offscreen or is occupied by another window. (probably with `InvalidateRect()` function which i already used).

### Getting client area

`GetClientRect()` is the function which retrieves the coordinates about a window's client area, this area is the area which can be drawn it is not the border, menus or buttons of a window, these parts are handled by Windows.

It receives the handle for the target window and a pointer to a **RECT** structure which the coordinates will be stored, the coordinates are the upper-left corner and the bottom-right corner right outside the area. (it seems that the upper-left corner is always 0,0).

# 13/07/2025

## Raster operation code

It's the type of bit-wise operations that is done at the destination buffer.

Essentially it says what to do with the color data of the source at the destination using a specified pattern (brush). For example: an inverted colored image, or green filtered image, etc.

## BITMAPINFO

**BITMAPINFO** is a structure which defines the dimension and scale of a DIB.

### BITMAPINFOHEADER

**BITMAPINFOHEADER** Is a structure which holds information about the DIB, such as size, dimensions, color table, compression and etc.

## Bitmap deletion

It seems that Windows will accumulate memory when recreating bitmap handles so we need to free this space of memory ourselves.

# 18/09/2025

## StretchDIBits/BitBlt confusion

Casey always used `BitBlt()` to render on screen, it needs a source Device Context (DC) to keep the target bitmap send it to a destination DC that would be the Windows display buffer.

But the function `StretchDIBits()` can perform a bitmap transference without the DC conversion, using DIBs.

`BitBlt()` used to be faster than `StretchDIBits()` but nowadays both functions make no relevant difference.

## Soliciting memory with a padding

In x86 architecture there's some kind of penalty of unaligned memory bits.

Memory alignment is about using the spacing of bits for different information, being on a base-2 size variable (8 bits, 16 bits, 32 bits and 64 bits.)

For example a RGB pixel could have one byte (8 bits) for each color channel:
```
  Red     Green     Blue
|------| |------| |-------|
00000000 00000000 000000000
```
As you can see, this variable uses 24 bits, which is bigger than a 16 bit variable, so it needs a padding to fill a next size variable, in the end it being:
```
  Red     Green     Blue   Padding
|------| |------| |------| |------|
00000000 00000000 00000000 00000000
```

Alignment is about ending a variable with a base-2 byte size.

# 04/10/2025

## Requesting memory with VirtualAlloc()

`VirtualAlloc()` is a function which allows you to request memory and windows will deliver you entire pages of a memory.
If you request memory smaller than a page size Windows still delivers one entire page and the remaining memory will just be wasted if your program doesn't use it.

It reserves, commits or changes the state of a range of pages based on the parameters:
`lpAddress`: The starting address of the desired request, if the address is being reserved or commited this value will be rounded to the next address available or to the next page boundary respectfully.
`dwSize`: The size of the region being requested if `lpAddress`is **NULL** then the size will be rounded to the next page boundary, otherwise it will include all pages within range and if it pass one byte of the next page it will contain that entire page aswell.
`flAllocationType`: It's the type of allocation (reserve, commit, reset, etc.) it must contain one valid value listed in [Microsoft's memoryapi.h documentation](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc)
`flProtect`: is the type of memory protection (if it can be read, write or executed by other programs).

## Releasing memory with VirtualFree()

`VirtualFree()` is a function that can decommit and release memory pages.
It has three parameters:
`lpAddress`: It's the address of the base of the region to be release or decommited.
`dwSize`: It's the size of the region to be freed, if the region is going to be full release (not reserved anymore) it must be the entire region delivered by `VirtualAlloc()` (in this case the parameter must be 0), if the region is going to be decommited (addresses still reserved) then this parameter should specify the size to be decommited, just like allocating the size will be rounded to catch entire pages.
`dwFreeType`: Defines the free operation the function executes, a full release (`MEM_RELEASE`) or a decommit (`MEM_DECOMMIT`).

## "use after free" bug prevention

`VirtualProtect()` is a function that changes the protection type of a commited memory region.

It is suggested, on debugging, to change the protection of a memory region to `PAGE_NOACCESS` instead of freeing it in order to catch pointers that would aim for a freed memory, therefore preventing "use after free" bugs and stale reads. 

# 17/10/2025

## Stride

Stride is how a 2D or 3D image is read through a 1D buffer (memory addresses) and identifying this method is essential for drawing the image on the right way.

`StrecthDIBits()` use the sign of the height to determine the direction Y (bottom-up or top-down), if positive it's a bottom-up, if negative it's a top-down.

# 18/10/2025

## Conclusion on BGR

Regarding [file] so it happens that Windows is a little endian architecture, so the memory withdrawal is swapped, but to make RGB doesn't look BGR, the *designers* inverted the procedure.
so when you look into memory you'll see the RGB pattern, but when the system pulls the bytes they pull the Blue byte first, so on our code, the values must be inverted.

# 13/11/2025

## PeekMessage

`PeekMessage()` is a function that handle messages without letting us stuck with 'em, the downside is it gets out of loop (returns zero) when there are no messages.

# 11/12/2025

## Aliasing

Aliasing is when two pointers could point to the same thing and the compiler doesn't know about it, forcing it to go for memory twice or more without necessity.
```
char *A = smwhereInMemory;
char *B = smwhereInMemory;

int x = *A;
*B = 5;
int y = *A;
```
Compiler could set `y = x`, considering both are declared with the same pointer,
but the pointer could be altered before reaching to that code, let's say `A = B` then `y` would be different than `x`.

If `y = x` is desired then the pointer should not be used on `y`'s declaration.

## Stack memory

The stack memory is a region of memory designed to be used on runtime to store local variables of functions.

Whenever a function is called its local variables are stored onto stack and when it's returned these variables are thrown out and that space will be used by other function if there are one.

The location of a variable doesn't change major behaviors, only attention is required when refering variables because local variables cannot be referred to outside their region.

### Classes constructors

With classes this behavior is different since classes can have **constructors** and **destructors**.

**Constructor** is essentially a function that is called whenever a object is created.

**Destructors** is essentially a function that is called whenever a object cease existence.

Locality in this case matters because whenever you put a declaration of a class, it will call those functions and if you don't want those functions to be called at a certain region you shall not put a object there.
Constructors are called as soon on declaration and Destructors are called as soon as it's get out of scope or is destroyed by code.
