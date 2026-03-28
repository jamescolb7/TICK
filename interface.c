#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>

#include "splashscreen.h"
#include "colours.h"

typedef enum {
	SPLASHSCREEN,
	MAIN
} Screens;

#define TOTAL_INPUTS 1

typedef enum {
	MESSAGE
} Inputs;

static volatile int clockRunning = 0;
static volatile Screens screen = SPLASHSCREEN;
static volatile Inputs selectedInput = MESSAGE;

ScreenDimensions screenSize() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	
	ScreenDimensions dims = {
		csbi.srWindow.Bottom - csbi.srWindow.Top + 1,
		csbi.srWindow.Right - csbi.srWindow.Left + 1
	};

	return dims;
}

void clearScreen() {
	printf("\033c");
}

typedef struct {
	int sidebarWidth;
} DisplayBounds;

typedef struct {
	int x;
	int y;
} Coordinate;

struct BoxOptions {
	Coordinate start;
	Coordinate end;
	int doubleThick;
	char *title;
};

void charPrint(int n) {
	printf("%c", n);
}

void drawBox(int i, int j, struct BoxOptions options) {
	Coordinate start = options.start;
	Coordinate end = options.end;
	char *title = options.title;

	int singleThick[6] = {218, 191, 192, 217, 196, 179};
	int doubleThick[6] = {201, 187, 200, 188, 205, 186};

	int *asciiChars = options.doubleThick == 0 ? singleThick : doubleThick;

	int titleLength = strlen(title);

	if (j == start.y && i == start.x) {
		charPrint(asciiChars[0]);
	} else if (j == end.y && i == start.x) {
		charPrint(asciiChars[1]);
	} else if (j == start.y && i == end.x) {
		charPrint(asciiChars[2]);
	} else if (j == end.y && i == end.x) {
		charPrint(asciiChars[3]);
	} else if (i == start.x) {
		if (j > start.x + 1 && j < start.x + 1 + titleLength) {
			printf("%c", title[j + 1 - start.x]);
		} else {
			charPrint(asciiChars[4]);
		}
	} else if (i == end.x) {
		charPrint(asciiChars[4]);
	} else if (j == start.y || j == end.y) {
		charPrint(asciiChars[5]);
	} else {
		printf(" ");
	}
}

// void drawRoot(ScreenDimensions dims) {
// 	for (int j = 0; j < dims.height; j++) {
// 		for (int i = 0; i < dims.width; i++) {
// 			if (j == 0 || i == 0 || j == dims.height - 1 || i == dims.width - 1) {
// 				Coordinates start = {
// 					0, 0
// 				};
			
// 				Coordinates end = {
// 					dims.height - 1, dims.width - 1
// 				};

// 				return drawBox(start, end, 1);
// 			}
// 		}
// 	}
// }

void setColour(char colour[]){
	printf("\033[0;39;49m");
	printf("%s",colour);
}

void drawRoot(ScreenDimensions dims, char **inputsList) {
	char appName[] = "TICK";
	int appNameSize = strnlen(appName, 20);

	setColour(BACKGROUND);
	for (int j = 0; j < dims.height; j++) {
		for (int i = 0; i < dims.width; i++) {
			//First header row
			char channelName[] = "#general";
			int channelNameSize = strnlen(channelName, 20);

			if (j == 0) {
				if (i > 10 && i < 11 + appNameSize) {
					int adj = i - 10;
					setColour(BOLD_HEADER);
					charPrint(appName[adj - 1]);
					setColour(BACKGROUND);
				} else if (i >= 27 && i < 27 + channelNameSize) {
					printf("%c", channelName[i - 27]);
				} else if (i == 10) {
					charPrint(181);
				} else if (i == 10 + appNameSize + 1) {
					charPrint(198);
				} else if (i == 0) {
					charPrint(201);
				} else if (i == dims.width - 1) {
					charPrint(187);
				} else if (i == 25) {
					charPrint(203);
				} else {
					charPrint(205);
				}
			} else if (j == dims.height - 1) {
				//Bottom row
				setColour(BACKGROUND);
				if (i == 0) {
					charPrint(200);
				} else if (i == dims.width - 1) {
					charPrint(188);
				} else if (i == 25) {
					charPrint(202);
				} else {
					charPrint(205);
				}
			} else if (i > 24 && j == dims.height - 3) {
				//Messaging box

				char boxTitle[] = "Message";
				int boxTitleSize = strlen(boxTitle);

				if (i == 25) {
					charPrint(204);
				} else if (i >= 27 && i < 27 + boxTitleSize) {
					printf("%c", boxTitle[i - 27]);
				} else if (i == dims.width - 1) {
					charPrint(185); 
				} else {
					charPrint(205);
				}
			} else if (i > 25 && i < dims.width && j == dims.height - 2) {
				setColour(TEXT);
				char *messageText = inputsList[MESSAGE];
				int messageLength = strlen(messageText);
				// printf("%d", messageLength);
				// if (messageText != NULL && i - 24 < messageLength) {
					// if (inputsList[MESSAGE][i - 25]) 
					// printf("%c", messageText[i - 24]);
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

			// } else if (j >= 1 && j <= 15 && i >= 1 && i <= 30) {
			// 	// Channels Box

			// 	Coordinate start = {1, 1};
			// 	Coordinate end = {15, 30};

			// 	setColour(CHANNELS);
				
			// 	char title[] = "test";
			// 	struct BoxOptions opts = {start, end, 0, title};

			// 	drawBox(j, i, opts);

			// 	setColour(BACKGROUND);
			} else {
				//All additional left borders
				setColour(BACKGROUND);
				if (i == 0 || i == 25 || i == dims.width - 1) {
					charPrint(186);
				} else {
					printf(" ");
				}
			}
		}
	}
}

//Nicely handles cleaning the screen after the SIGTERM is detected
void interfaceCleanup() {
	clockRunning = 0;
	clearScreen();
}

//Renders views selectively
void renderView(ScreenDimensions dims, char **inputsList) {
	switch (screen){
		case SPLASHSCREEN:
			splashscreen(dims);
			break;
		case MAIN:
			drawRoot(dims, inputsList);
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

		clearScreen();
		renderView(dims, inputsList);
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
	// if (screen == SPLASHSCREEN) return screen = MAIN;

	// if (screen == MAIN) writeToInput(c, MESSAGE, inputsList);
}


void fetchTextBuffer(Inputs input, char **inputsList) {
	char *str = inputsList[input];
	// int stringLength = strnlen(str)
	// for (int i = 0; i < )
}

void clock(ScreenDimensions *initial_dims, char **inputsList) {
	//Keep the function running, the sleep will throttle it to a good frame rate
	Screens curscreen = SPLASHSCREEN;

	while (clockRunning) {
		ScreenDimensions dims = screenSize();

		if (_kbhit()) {
			handleInput(getch(), inputsList);
		}

		//Dimensions haven't changed and theres no reason to redraw
		if (dims.height == initial_dims -> height && dims.width == initial_dims -> width && curscreen == screen) {
			Sleep(5);
		} else {
			clearScreen();
			renderView(dims, inputsList);

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

	renderView(initial_dims, inputsList);
	clock(&initial_dims, inputsList);
}