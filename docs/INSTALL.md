# TICK Installation

This assumes that you already have the GCC compiler installed to your Windows device. The specific version the team is using is the 64-bit GCC 14.2.0 with POSIX threads compiled by winlibs.com at [this link](https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-12.0.0-ucrt-r3/winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.7-mingw-w64ucrt-12.0.0-r3.7z).

To compile the binaries for the application, you must be in the src folder, which can be navigated to with the following:

```bash
cd src
```

> [!IMPORTANT]  
> Please note that this application only works on Windows as it uses binaries such as winsock2.h and windows.h. Please use a Windows device or virtual machine, as compatibility with tools like Wine has not been tested yet.

### Server

The server should be compiled and running before any clients are able to connect.

To compile the server binary, run:
```bash
gcc server.c DSAFunctions.c networkHelpers.c -o ./server -lwsock32 -lWs2_32
```

This can then be executed using:
```bash
./server
```

### Client

The client will then be used as a separate process to connect to the server. This will have to be done in a separate terminal.

To compile the client/interface binary, run:
```bash
gcc main.c interface.c splashscreen.c client.c DSAFunctions.c networkHelpers.c -o ./client -lwsock32 -lWs2_32
```

Then start it using:
```bash
./client
```

## How to Use

Once the server is started and a client is running, you should be placed on a splash screen. Press any key to go to proceed to the application.

Type in the IP Address of your server. If running on the same device, you may be able to use `127.0.0.1` or `localhost` for example. However, localhost seems to make the app perform a bit slower due to the Windows networking stack. It is preferred to use `127.0.0.1`. Press enter to continue to the next page. You may also use another device to connect if there is LAN access between them and port `3000` on the server has been correctly allowed through the firewall (The campus Wi-Fi has historically allowed this if on the same subnet, but may be unstable).

You should then be prompted to enter a username. This can be anything that is less than 18 characters and does not use any special characters. Press enter to continue again.

Now, you should be on the main chat interface. You are free to type messages and add other members to the same server, adding them through the same process.

A set of predefined channels is available on the left side, use `Tab` to navigate through the channels.

When other members send messages, you will not see them automatically. This system does not have realtime capabilities. Press the `~` (tilde without shift, or backtick) key to reload new messages and fetch more from the server.

The window is able to be resized and auto redraw based on the screen dimensions, keeping the app responsive.

This is the extent of the app behaviour.