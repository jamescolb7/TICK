#include <stdio.h>
#include <windows.h>

typedef struct {
	int height;
	int width;
} ScreenDimensions;

ScreenDimensions screenSize() {
	//Adapted from here https://stackoverflow.com/questions/6812224/getting-terminal-size-in-c-for-windows
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, rows;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	
	ScreenDimensions dims = {
		csbi.srWindow.Right - csbi.srWindow.Left + 1,
		csbi.srWindow.Bottom - csbi.srWindow.Top + 1
	};

	return dims;
}

void clearScreen(ScreenDimensions dims) {
	// for (int i; i < dims.height; i++) {
	// 	printf("\n");
	// }
	printf("\033[2J");
	// print("\\033[u");
}

void drawRoot(ScreenDimensions dims) {
	// printf("\x1b[0;39;41m");

	printf("%d\n", dims.height);
	printf("%d", dims.width);
	// for (int i = 0; i < dims.height; i++) {
	// 	for (int j = 0; j < dims.width; j++) {
	// 		printf(" ");
	// 	}
	// }
}

void clock(ScreenDimensions *initial_dims, int forceFirstDraw) {
	ScreenDimensions dims = screenSize();
	if (!forceFirstDraw && dims.height == initial_dims -> height && dims.width == initial_dims -> width) return clock(initial_dims, 0);
	drawRoot(dims);
	Sleep(60 / 1000);
	clearScreen(dims);
	initial_dims -> width = dims.width;
	initial_dims -> height = dims.height;
	clock(initial_dims, 0);
}

void startDrawing() {
	ScreenDimensions initial_dims = screenSize();

	clock(&initial_dims, 1);
}