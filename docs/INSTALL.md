# TICK Installation

To compile the binaries for the application, you must be in the src folder, which can be navigated to with the following:

```bash
cd src
```

> [!IMPORTANT]  
> Please note that this application only works on Windows as it uses binaries such as winsock2.h and windows.h. Please use a Windows device or virtual machine, as compatibility with tools like Wine has not been tested yet.

### Server

To compile the server binary, run:
```bash
gcc server.c DSAFunctions.c networkHelpers.c -o ./server -lwsock32 -lWs2_32
```

This can then be executed using:
```bash
./server
```

### Client

To compile the client/interface binary, run:
```bash
gcc main.c interface.c splashscreen.c client.c DSAFunctions.c networkHelpers.c -o ./client -lwsock32 -lWs2_32
```

Then start it using:
```bash
./client
```