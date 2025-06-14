# 04/15/2025

I had a hard time trying to understand things together, searching sollutions on internet and stuff, but i realized what's wrong.
I'm using **Visual Studio Code** while Casey and most of users i found are using **Visual Studio**.

My biggest strugle was how to set up stuff. In VS (Visual Studio) you have your workspaces treated as a project, and have properties setup, in VSC (Visual Studio Code) you have a workspace just like it is.

i'm using VSC to just write the code, i compile it and run it on the terminal, i don't know how to debug on terminal.

# 04/22/2025

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

# 05/08/2025

## Summary on creating a window

### Windows entry point

`WinMain()` WinMain is a WINAPI (Windows API) functions that creates a Windows thread for your program.
It receives a Handle instance, command-line arguments as a character string and the flag of the window (maximized, minimized or shown normally)

### Creating and registering a class

WNDCLASS is a class for a window management it receives a pointer to the window procedure, the handle instance which the class will be managed and the name of the window class.

`RegisterClass()` is a function that registers the class with the operating system it receives a pointer to the class instead of the class itself.

`CreateWindowEx()` is a function that creates an instance of an window, it receives window styles, window name, window class, position and size, parent window, menu setup, instance handle and additional data.

### Handling window messages

a window can send a lot of messages about its status, in `LRESULT CALLBACK WindowProc()` LRESULT is an integer value that the program returns to Windows, it's a response to a message, CALLBACK is a name convention and `WindowProc()` is a window procedure function. It usually contains a `switch` to handle each necessary message, usually also contains the `DefWindowProc()` function which gives a default procedure for a given message, it's usually used to handle not-so important messages. Both function receives the instance handle, the message code and some additional data depending on the message.

### Translating and Dispatching messages 

the functions `TranslateMessage()` and `DispatchMessage()` are contained within a loop while `GetMessage()` does return zero.
`GetMessage()` pulls a message the operating system message queue, if there are none the function blocks (not necessarily meaning froozing the program but it can happen, to avoid it it's good to create other threads).
`TranslateMessage()` is related to the keyboard, key strokes and etc. how it functions is not important for now.
`DispatchMessage()` tell the operating system to call the function of window procedure of such particular message.
All these three function receives a `MSG` struct pointer as a parameter.

### Showing the window

`ShowWindow()` is a function that shows the window on screen, that's simple, it receives the window handle and a value that tells how the window should be shown.

# 05/15/2025

## Painting a window

`COLORREF` is a data type of variable designed to hold color info, it's a 32-bit number where the 24 lower bits describes the RGB info.
`HBRUSH` is a handle to a brush tool used by Windows GDI (Graphics Device Interface) used to paint polygons.

We can paint our window with `BeginPaint()` and `EndPaint()` functions, they receive the window handle of the desired window to paint and a pointer to a `PAINTSTRUCT` struct.

`FillRect()` is a function to paint a rectangular area of our window, it receives a device context variable, a `RECT` struct (the `PAINTSTRUCT` has one) and a `HBRUSH` to be used.

`PAINTSTRUCT` only marks invalid areas to paint (areas to be painted), the whole window is invalid when it first launches but then only new area by resizing are invalid and will be painted.

To repaint previously painted area we need to invalidate them, we can use `InvalidateRect()` function to do that, it receives the window handle, a pointer to a `RECT` structure to mark the area to be invalidated (if `NULL` the whole window will be marked) and a boolean to set if the area must be erased (true) or not (false).
*//I've seen no difference with the `FillRect()` maybe it happen on more complex paintings.*

We can also revalidate an area with `ValidateRect()` function (this means, mark a rectangular area that should not be redrawn), it receives a window handle and a pointer to a `RECT` struct.

# 06/07/2025

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

# 06/14/2025

## Rendering method

I gonna use a buffer to hold a bitmap for our graphics.

A buffer is simply a temporary space of memory used to transfer info.

## DIBs

I'm going to use DIB section to talk to Windows about our buffer, and bitmap.
DIB stands for Device Indepedent Bitmap.

I understood nothing what i've done, whoever today's session is over.