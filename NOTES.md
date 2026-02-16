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

# 30/12/2025

## Input Withdrawal Methods

### Interrupt

Interrupt based devices send a interrupt signal to the CPU when a important change happened (a button pressed, sensor wigle, etc.), this is an old way and it got rare.

### Poll

Poll based devices store its own data waiting for the CPU to request them, then the data is processed at the program's designed pace.

Or the data can be pulled by the CPU into a buffer, where it'll be processed with a queue timing.

## XInput

XInput is the Windows API for handling controller inputs.
The main work here is that we're going to cycle through all the controllers available, get theirs state and handle whatever data we received.
```cpp
for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) {
      XINPUT_STATE ControllerState;
      if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
        // Controller connected.
      }else {
        // Controller not connected or error.
      }
    }
```

The `XINPUT_STATE` is a struct where it flags if the state of the controller changed and it contains the struct with the state.
Its parameters are:
`dwPacketNumber` the number of the current "state index" of the controller, if the controller state changes this number changes as well, if it didn't changed that means the controller state hasn't changed.
`Gamepad` is a `XINPUT_GAMEPAD` struct that contains details of the controller state, such as which buttons are pressed, triggers' pressure and joysticks direction. More details [here](https://learn.microsoft.com/en-us/windows/win32/api/XInput/ns-xinput-xinput_gamepad#members).

### Note on XInput unresolved symbol

It seems that not all people would have the adequate system for XInput linkage. I assume i wouldn't get any problems since Microsoft's official documents signs older version of windows, and my program still is working for now.

# 19/01/2026

## DirectSound API

DirectSound API is currently deprecated and i'll switch to XAudio2 later

### DirectSound procedure

- **Load the library:**

This API will have the same symbol problem as XInput so i'll first load the `dsound.dll` library and then direct a pointer to the desired function.

- **Create DirectSound object and set priority:**

We need to create a DS (DirectSound) object and then from that object set the priority type with the `SetCooperativeLevel()` method within the DS object.

`SetCooperativeLevel()` will receive a handle to the target window and the priority type to set the program.

The primary buffer won't be used for the sound output and it serves only to receive a handle to set the format of the waveform-audio data (which i'll refer to just as "audio data").

- **Initialize required structures:**

To create a sound buffer we first initialize a pointer to the buffer struct `LPDIRECSOUNDBUFFER`, a struct of the buffer description `DSBUFFERDESC` and a `WAVEFORMATEX` structure to set the format of the audio data.

The buffer description will contain data about the desired buffer such as the size of itself, the flags, the size of the new buffer, the waveformat and the guid.
In the case of theK primary buffer, the only flag will be `DSBCAPS_PRIMARYBUFFER`, buffer size and waveformat must be 0. GUID is only used on 3D audio which won't be our case, so it must receive `GUID_NULL` which is NOT zero.

- **Create primary sound buffer and set format:**

The buffer is finally created with the DS object method `CreateSoundBuffer()` which will receive pointers for the buffer description and the buffer itself.

Then we can set the audio data format with the buffer's `SetFormat()` method, which receives only a pointer to a WAVEFORMATEX structure.

- **Create secondary sound buffer:**

The secondary buffer is the buffer we will actually store our audio on.
It is created just like the primary buffer but its `DSBUFFERDESC` will receive the size and waveformat info as well to not having the `DSBCAPS_PRIMARYBUFFER` flag.

You don't need a different DS object to create the secondary buffer.

# 20/01/2026

## Playing sound with DirectSound

After all the object, buffer creation and structure filling shenanigans, it is time to play something.

### Getting sound cursors

First we need to know where we are on the buffer to write properly.

The sound buffer works in a cycle manner (goes to the beginning when it ends) and it has two cursors: **play** and **write**.

The play cursor is where the sound card is outputting the sound and the write cursor is ahead of the play cursor in order to avoid audio errors and weird noises.

The DS buffer method `GetCurrentPosition()` is what is used to get such pointers, it receives the address of variables to store the pointers.

First we need to know what region is availabe to write withou problems, there are two cases for such, when the write and play cursors are on the same loop and when the write cursor is on the next loop thus being behind the play cursor.

When both cursors are on the same loop we need to write the bytes after the write cursor and before the play cursor leaving the space in between untouched.

When the write cursor is one loop ahead, the bytes we should write to is just those in between the two pointers.

_On the diagrams below, the '#' signals bytes free to write._

_play and write on the same loop._
```
X###|------|#####X
    ^      ^
  play   write
```
K
_Write one loop ahead._
```
X--|##########|--X
   ^          ^
 write      play
```

### Locking write regions

After determining the regions to write, we must pass these on the DS buffer method `Lock()` where it will return us two pointers and two lengths.

The second pointer and length are non-zero if the wrap around case occurs.

After the lock we are free to modify the sound buffer and then we must release the buffer with the `Unlock()` method to notify you're done, not doing so will break the audio or just be silent.

### Play the audio

Now we must call the DS buffer `play()` method to effectively play the audio data on the buffer.

# 22/01/2026

## How computers deals with fractions

### Fixed point numbers

On the old days where common computer hardware couldn't deal with floating point arithmetic, fraction numbers were a part of a byte integer.

Let's say that a fraction variablLe is 32-bits long, the lower 8-bits of this variable were reserved to hold the fraction part as a number.

So the decimal number is the integer part plus the division of the fraction representation by the size of the fraction bits.

```
n = whole number.
i = integer part.
f = fraction part.

32-bit fixed point:
X------------|----X
      ^        ^
   Integer  Fraction

n = i + (f / 256) 
in case of 8-bit fraction.
```

It's called **fixed point** because the precision and decimal points don't vary.

### Floating point numbers

Often just defined as computer fraction, floating point numbers use a certain technique that allow to vary the decimal depth of a number by using significant digits (mantissa) and an exponent to move it around.

In this method the number is define as the mantissa times 2 to power of exponent.

```
N = whole number.
M = mantissa.
E = exponent.

32-bit floating point:
X----|------------X
  ^       ^
Exponent Mantissa

N = M * 2^E
```

The exponent defines where the decimal point is, that's why it's called **floating point**.

The exponent can be negative, indicating a lower decimal depth (e.g. 0.00000000000\[mantissa]) just like how scientific notation work, but in this case within binary.

# 27/01/2026

## DirectSound doesn't work anymore.

So i've hit a wall which i think it is time to switch for XAudio2. So basically what's happening is that the program is passing _"invalid"_ arguments to the `Lock()` method, so we can't write on the buffer leaving a unsync loop of the first buffer written.

I'll start the studies on XAudio2 soon.

# 03/02/2026

## XAudio2 Interface

**IXAudio2** is the interface for the XAudio2 API, it's capable to enumerate devices, configure global properties, create **voices** and monitor performance.

Multiple instances can be created on a single client application, each object has its own audio thread with only debug settings shared, but you should not pass information between their **graphs**.

## Voices

Voices are the XAudio2 objects, they're the thing used to process, manipulate and play audio data.
There are three types of voices:
- **Source Voice:** The entry point for the audio processing workflow, it receives the audio data passed by the client, it can output to one or more submix or mastering voices.
- **Submix Voice:** An intermediate step for performance improvement or effect processing, it can't receive data directly and the audio is only heard if submitted to a mastering voice.
- **Mastering Voice:** Represents the output audio device, it can't receive data directly but from another voice type. This voice is the one that allows audio to be heard.

All voices can:
- Set the overall volume.
- Apply digital signaling effects (f.e.: reverb).
- Set per-channel volume.
- Separate a mix to be sent to a destination voice or output device (via mastering voice), and change the number of channels if needed.

## Audio Graph

Audio Graph is a set of voices connected in a pipeline, it always start with a source voice and ends with a mastering voice, the simplest audio graph being `source -> mastering`.
The graph can be changed dynamically in runtime altering its state.

Any function call that affects the objects in a graph also changes the graph state, such as: Creating, destroying, starting, stopping and changing the destination of voices; enabling, disabling, modifying chains of effects; setting parameters on effects or on built-in SRCs, filters, volumes and mixers.

XAudio2 can handle any sample rate or channel conversion with the following limitations:
- Destination voices for a particular voice must have the same sample rate.
- Effects can change the number of channels but not the sample rate.
- A voice receiving from an effect chain must have the same channel count as the effect output.
- Changes on dynamic graphs can't break the rules above.

On input, source voices can read any format supported by XAudio2.

On output, mastering voices can only produce PCM data, this data will always satisfy the restrictions above.

## Callbacks

Callbacks are function calls provided by XAudio2 to signal events happening on the audio thread.
These calls interrupts the audio processing, so they must return as quickly as possible and shall not access permanent storage, make expensive API calls, synchronize with client code or require significant CPU usage.

There are two types of callbacks: global and specific.

Global callbacks return events on the global XAudio2 engine, these calls are implemented by means of a XAudio2 interface pointer provided by `RegisterForCallback()` method.

Specific callbacks return events of an specific source voice, these calls are implemented by means of a source voice interface pointer provided by `CreateSourceVoice()` method.

Both global and specific callbacks returns `void` instead of `HRESULT`.

Weird, but i'll know how to use them later.

# 09/02/2026

## XAudio2 Audio Effect

Audio Effects are objects that receive audio data and perform some operation (like adding reverb) before passing it on.

### Effect Chain

Any **Voice** can have a chain of audio effects, using an array of `XAUDIO2_EFFECT_DESCRIPTOR` structures to specify effect chains, each descriptor point to an effect object.
Such objects must implement the Audio Processing Object (APO). XAudio2 uses the **XAPO** model.

Effect chains can be modified dynamically: enabled, disabled, or have parameters change, without interrupting the audio.
Whenever the effect graph changes, XAudio2 optimizes it to avoid unnecessary processing.

After an effect is attach to a voice, XAudio2 handles it and no further calls should be done to it.

An effect given to a voice's effect chain must operate at the same sample rate as the voice, the only aspect an effect can change is the number of channels. The field `OutputChannels` can be used to specify the channel count of the output.
An effect chain will fail if any effects can't fulfill the requirements or if a effect receive a channel count it can't handle. Any effect calls that makes the chain to stop fulfilling the requirements will fail.

XAudio2 has only 2 built-in effects, those being a (reverb)[https://learn.microsoft.com/en-us/windows/win32/api/xaudio2fx/nf-xaudio2fx-xaudio2createreverb] and a (volume meter)[https://learn.microsoft.com/en-us/windows/win32/api/xaudio2fx/nf-xaudio2fx-xaudio2createvolumemeter].

## Custom effects

Custom effects can be created with the **XAPO** API.

## Streaming Audio Data

Streaming is the process of keeping only a portion of audio data instead of loading an entire file.
Streaming is accomplished through assynchronously reading data on a queue of disk buffer and then submitted to a voice.
When the voice stop using that data the buffer is free to be read again. Looping through this buffer allows a large file to be played while only a portion of its data is loaded in.

The code for streaming should be put on separate thread, where it can sleep while waiting for the long reading process.
A callback is used to wake the thread when the audio is finished.

## Operation Set

An operation set is a set of methods grouped to be called at once. This means that you can put methods on a "queue" and apply all of them at the same time by calling the `CommitChanges()` method with the set identifier of a group (several XAudio2 receives an identifier as a parameter).
The operation set is guaranteed to be sample-accurate, which means changes can occur in sync, like start multiple voices.

If a method receives `XAUDIO2_COMMIT_NOW` instead of an identifier, the change is applied immediately. If the `CommitChanges()` receives `XAUDIO2_COMMIT_ALL` parameter, every pending set is applied.

Some methods take effect immediately after a call, while others only take effect on the next audio pass even if they receive `XAUDIO2_COMMIT_NOW` or its group is committed, because of that method calls may not happen in the same order they've been called.

All pending operation are committed when `StopEngine()` is called. When restarted XAudio2 returns to asynchronous mode.

Operation sets are useful on those examples:
- Starting multiple sounds simultaneosly.
- Submitting a buffer to. setting parameters of and starting a voice.
- Making large-scale changes to a graph.

## Debugging Facilities

You can set the level of debug logging at runtime in the `XAUDIO2_DEBUG_CONFIGURATION` structure and passing it to the `SetDebugConfiguration()` method.

The debug information is retrieved through the Event Tracing of Windows.
According to the documentation.

# 13/02/2026

I might have found out the problem with filling the buffer, but now with no errors the sound is _choking_.

XAudio2 will wait a little longer.

# 16/02/2026

My sound is choking, so instead of trying to fill the buffer every frame, i will fill half by half, with at start the full buffer is filled.