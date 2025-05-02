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

## Creating a window

    

