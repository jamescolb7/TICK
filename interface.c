#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>

#include "splashscreen.h"
#include "colours.h"

static volatile int clockRunning = 0;
static volatile Screens screen = SPLASHSCREEN;
static volatile Inputs selectedInput = MESSAGE;

//Simple interface utils
void clearScreen() {
	printf("\033c");
}

void charPrint(int n) {
	printf("%c", n);
}

void setColour(char colour[]){
	printf("\033[0;39;49m");
	printf("%s",colour);
}

void setCursorPos(int x, int y) {
  printf("\033[%d;%dH", y, x);
}

//A more complicated clearscreen that can clear a specific amount of lines
void clearLines(int lines, ScreenDimensions dims) {
    setCursorPos(1, dims.height - lines + 1);
    printf("\033[0J");
}

//Nicely handles cleaning the screen after the SIGTERM is detected
void interfaceCleanup() {
	clockRunning = 0;
	clearScreen();
}

//Retrieve terminal dimensions from the Windows API
ScreenDimensions screenSize() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	
	ScreenDimensions dims = {
		csbi.srWindow.Bottom - csbi.srWindow.Top + 1,
		csbi.srWindow.Right - csbi.srWindow.Left + 1
	};

	return dims;
}

void drawBox(int x, int y, struct BoxOptions options) {
	Coordinate start = options.start;
	Coordinate end = options.end;
	char *title = options.title;

	int singleThick[6] = {218, 191, 192, 217, 196, 179};
	int doubleThick[6] = {201, 187, 200, 188, 205, 186};

	int *asciiChars = options.doubleThick == 0 ? singleThick : doubleThick;

	int titleLength = strlen(title);

	if (y == start.y && x == start.x) {
		charPrint(asciiChars[0]);
	} else if (y == end.y && x == start.x) {
		charPrint(asciiChars[2]);
	} else if (y == start.y && x == end.x) {
		charPrint(asciiChars[1]);
	} else if (y == end.y && x == end.x) {
		charPrint(asciiChars[3]);
	} else if (y == start.y && x > start.x && x < start.x + 1 + titleLength) {
		printf("%c", title[x - start.x - 1]);
	} else if (x == start.x || x == end.x) {
		charPrint(asciiChars[5]);
	} else if (y == start.y || y == end.y) {
		charPrint(asciiChars[4]);
	} else {
		printf(" ");
	}
}

void drawRoot(ScreenDimensions dims, int startLine, char **inputsList) {
	char appName[] = "TICK";
	int appNameSize = strlen(appName);

	setColour(BACKGROUND);

	int startHeight = startLine != 0 ? dims.height - startLine - 1 : 0;

	setCursorPos(1, startHeight);

	for (int y = startHeight; y < dims.height; y++) {
		for (int x = 0; x < dims.width; x++) {
			//First header row
			char channelName[] = "#general";
			int channelNameSize = strlen(channelName);

			if (y == 0) {
				if (x > 10 && x < 11 + appNameSize) {
					int adj = x - 10;
					setColour(BOLD_HEADER);
					charPrint(appName[adj - 1]);
					setColour(BACKGROUND);
				} else if (x >= 27 && x < 27 + channelNameSize) {
					printf("%c", channelName[x - 27]);
				} else if (x == 10) {
					charPrint(181);
				} else if (x == 10 + appNameSize + 1) {
					charPrint(198);
				} else if (x == 0) {
					charPrint(201);
				} else if (x == dims.width - 1) {
					charPrint(187);
				} else if (x == 25) {
					charPrint(203);
				} else {
					charPrint(205);
				}
			} else if (y == dims.height - 1) {
				//Bottom row
				setColour(BACKGROUND);
				if (x == 0) {
					charPrint(200);
				} else if (x == dims.width - 1) {
					charPrint(188);
				} else if (x == 25) {
					charPrint(202);
				} else {
					charPrint(205);
				}
			} else if (x > 24 && y == dims.height - 3) {
				//Messaging box
				char boxTitle[] = "Message";
				int boxTitleSize = strlen(boxTitle);

				if (x == 25) {
					charPrint(204);
				} else if (x >= 27 && x < 27 + boxTitleSize) {
					printf("%c", boxTitle[x - 27]);
				} else if (x == dims.width - 1) {
					charPrint(185); 
				} else {
					charPrint(205);
				}
			} else if (x > 25 && x < dims.width && y == dims.height - 2) {
				// printf("EXISTS");
				setColour(TEXT);
				char *messageText = inputsList[MESSAGE];
				int messageLength = strlen(messageText);
				// printf("%d", messageLength);
				// if (messageText != NULL && x - 24 < messageLength) {
				// 	if (inputsList[MESSAGE][x - 25]) 
				// 	printf("%c", messageText[x - 24]);
				// } else {
					printf(" ");
				// }
				// if (text != 0) {
				// 	printf("%c", text);
				// } else {
					// printf(" ");
				// }
				// int textState = inputStates[MESSAGE];
				// int textLength = strnlen(textState, dims.width - 26);
				// for (int i = 0; i < textLength; i++) {
				// 	printf("%c", textState[MESSAGE][i]);
				// }

			} else if (y >= 1 && y <= 15 && x >= 1 && x <= 24) {
				// Channels Box
				Coordinate start = {1, 1};
				Coordinate end = {24, 15};

				setColour(CHANNELS);
				
				char title[] = "Channels";
				struct BoxOptions opts = {start, end, 0, title};

				drawBox(x, y, opts);

				setColour(BACKGROUND);
			} else if (y >= 1 && y <= 30 && x >= 1 && x <= 24) {
				// Channels Box
				Coordinate start = {1, 16};
				Coordinate end = {24, 30};

				setColour(CHANNELS);
				
				char title[] = "Users";
				struct BoxOptions opts = {start, end, 0, title};

				drawBox(x, y, opts);

				setColour(BACKGROUND);
			} else {
				//All additional left borders
				setColour(BACKGROUND);
				if (x == 0 || x == 25 || x == dims.width - 1) {
					charPrint(186);
				} else {
					printf(" ");
				}
			}
		}
	}

	// setCursorPos(28, dims.height - 1);
	// printf("\033[4h");
}

//Renders views selectively
void renderView(ScreenDimensions dims, int lines, char **inputsList) {
	switch (screen){
		case SPLASHSCREEN:
			splashscreen(dims);
			break;
		case MAIN:
			drawRoot(dims, lines, inputsList);
			break;
	}
}

int writeToInput(int c, Inputs input, char **inputsList) {
    char *str = inputsList[input];
    if (str == NULL) return 0;

    int currentLength = strlen(str);
    char *newStr = realloc(str, (currentLength + 2) * sizeof(char));
    if (newStr == NULL) return 0;

    newStr[currentLength] = (char)c;
		//Need the null terminated pointer here
    newStr[currentLength + 1] = '\0';

    inputsList[input] = newStr;

		ScreenDimensions dims = screenSize();

		// clearScreen();
		// renderView(dims, inputsList);
		// clearLines(2, dims);

		Coordinate displayStart = {23, dims.height - 3};

		renderView(dims, 2, inputsList);

		// printf("%c", c);

    return 1;
}

void handleInput(int c, char **inputsList) {
	switch (screen){
		case SPLASHSCREEN:
			screen = MAIN;
			break;
		case MAIN:
			writeToInput(c, MESSAGE, inputsList);
			break;
	}
}

void clock(ScreenDimensions *initial_dims, char **inputsList) {
	//Keep the function running, the sleep will throttle it to a good frame rate
	Screens curscreen = SPLASHSCREEN;

	while (clockRunning) {
		ScreenDimensions dims = screenSize();

		if (_kbhit()) {
			// setCursorPos(dims.width, dims.height);
			handleInput(getch(), inputsList);
		}

		//Dimensions haven't changed and theres no reason to redraw
		if (dims.height == initial_dims -> height && dims.width == initial_dims -> width && curscreen == screen) {
			Sleep(5);
		} else {
			clearScreen();
			renderView(dims, 0, inputsList);

			Sleep(5);
			initial_dims -> width = dims.width;
			initial_dims -> height = dims.height;
			curscreen = screen;
		}
	}
}

void initializeDisplay() {
	clockRunning = 1;

	clearScreen();

	ScreenDimensions initial_dims = screenSize();

	//Malloc double pointer (array of strings) for all inputs to be temporarily used in the program
	char **inputsList = malloc(TOTAL_INPUTS * sizeof(char*));
	for (int i = 0; i < TOTAL_INPUTS; i++) {
		inputsList[i] = (char *)malloc(sizeof(char));
	}

	renderView(initial_dims, 0, inputsList);
	clock(&initial_dims, inputsList);
}