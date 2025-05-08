**04/15/2025**

    I had a hard time trying to understand things together, searching sollutions on internet and stuff, but i realized what's wrong.
    I'm using **Visual Studio Code** while Casey and most of users i found are using **Visual Studio**.

    My biggest strugle was how to set up stuff. In VS (Visual Studio) you have your workspaces treated as a project, and have properties setup, in VSC (Visual Studio Code) you have a workspace just like it is.

    i'm using VSC to just write the code, i compile it and run it on the terminal, i don't know how to debug on terminal.

**04/22/2025**

    **typedef** is a way to making a *"new"* type of data to be called be its new name, mostly used in old C programs, for example:

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
