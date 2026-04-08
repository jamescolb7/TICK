#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <conio.h>

#include "splashscreen.h"
#include "colours.h"
#include "DSAFunctions.h"
#include "client.h"

static volatile int clockRunning = 0;
static volatile Screens screen = SPLASHSCREEN;
static volatile Inputs selectedInput = MESSAGE;
int selectedChannel = 0;

char *serverIP = "";

#pragma comment(lib, "ws2_32")

SOCKET sock;
User currentUser = {0, "Undefined"};
// Channel currentChannel = {"Undefined",}

///depercated implementation
//Message msgs[20] = {{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0},{"", "", 0}};

int messageArrayCount = 0;

ScreenChannel channelsList[] = {
	{GENERAL, "#general"}, {CHILL, "#chill"}, {MEMES, "#memes"}, {QUOTES, "#quotes"}, {NEWS, "#news"}, {OFF_TOPIC, "#off_topic"} 
};

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
void interfaceCleanup(int sig) {
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

//Draws a box to the screen
void drawBox(int x, int y, struct BoxOptions options, void (*f)(int x, int y)) {
	Coordinate start = options.start;
	Coordinate end = options.end;
	char *title = options.title;

	//Defines the ASCII characters for the double and single thick borders
	int singleThick[6] = {218, 191, 192, 217, 196, 179};
	int doubleThick[6] = {201, 187, 200, 188, 205, 186};

	//Selects appropriate character array to use
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
		//Title printing
		printf("%c", title[x - start.x - 1]);
	} else if (x == start.x || x == end.x) {
		charPrint(asciiChars[5]);
	} else if (y == start.y || y == end.y) {
		charPrint(asciiChars[4]);
	} else {
		(*f)(x, y);
	}
}

void drawChannels(int x, int y) {
	if (y >= 3 && y <= 13 && x >= 3 && x <= 22) {
		if (y % 2 == 1) {
			int index = (y - 3) / 2;
			if (index >= 0 && index < sizeof(channelsList) / sizeof(channelsList[0])) {
				const char *str = channelsList[index].channel_name; // needs const or we get a warning... im not sure why
				int stringIndex = x - 3;
				if (stringIndex < strlen(str) && strlen(str) != 0) {
					printf("%c", str[stringIndex]);
				} else {
					if (index == selectedChannel) {
						printf("<");
					} else {
						printf(" ");
					}
				}
			} else {
				printf(" ");
			}
		} else {
			printf(" ");
		}
	} else {
		printf(" ");
	}
};

//Handles drawing of actual messages to the screen
// int drawMessages(int x, int y, ScreenMessage messages[], int messagesCount, ScreenDimensions dims) {
// 	//Find what message this would correspond to based on the y value
// 	int messageIndex = ((dims.height - 5) - y) / 3;
	
// 	//Find the position of the header line
// 	int headerLinePos = dims.height - 7 - messageIndex * 3;
	
// 	//Making sure that the entire message can be rendered in the space, so that no content is cutoff
// 	if (headerLinePos < 2) {
// 		printf(" ");
// 		return 0;
// 	}
	
// 	//Making sure the index is valid and won't cause any memory issues
// 	if (messageIndex < 0 || messageIndex >= messagesCount) {
// 		printf(" ");
// 		return 0;
// 	}
	
// 	ScreenMessage message = messages[messageIndex];
// 	int lineType = y - headerLinePos;
// 	int stringIndex = x - 27;
	
// 	if (lineType == 0) {
// 		//Header
// 		setColour(HEADER);
// 		int userLen = strlen(message.username);
// 		if (stringIndex >= 0 && stringIndex < userLen) {
// 			printf("%c", message.username[stringIndex]);
// 		} else {
// 			printf(" ");
// 		}
// 		setColour(BACKGROUND);
// 	} else if (lineType == 1) {
// 		//Content
// 		setColour(TEXT);
// 		int contentLen = strlen(message.message);
// 		if (stringIndex >= 0 && stringIndex < contentLen) {
// 			printf("%c", message.message[stringIndex]);
// 		} else {
// 			printf(" ");
// 		}
// 		setColour(BACKGROUND);
// 	} else {
// 		//Handle any other line type values
// 		printf(" ");
// 	}
	
// 	return 1;
// }

int drawMessages(int x, int y, ScreenMessage messages[], int messageCount, ScreenDimensions dims) {
	// return 1;
	//The y value with the header spacing removed
	int offsetY = y - 2;

	//Line type 0 is the title bar (username) line, 1 is the content, 2 is a space
	int lineType = offsetY % 3;

	//Finding what index in the array a message would correspond to
	int messageIndex = (int)(offsetY / 3);

	//With this logic, the message index should never be out of bounds of the messages array
	if (messages == NULL || messageIndex >= messageCount) return 1;

	ScreenMessage message = messages[messageIndex];

	//Offset x that will be used as a index in printing out the string
	int offsetX = x - 27;

	// return 0;

	switch (lineType){
		case 0:
			//header
			setColour(HEADER);

			int usernameLen = strlen(message.username);
			if (offsetX >= 0 && offsetX < usernameLen) {
				printf("%c", message.username[offsetX]);
			} else {
				printf(" ");
			}

			setColour(BACKGROUND);
			break;
		case 1:
			//Content
			setColour(TEXT);

			int contentLen = strlen(message.message);
			if (offsetX >= 0 && offsetX < contentLen) {
				printf("%c", message.message[offsetX]);
			} else {
				printf(" ");
			}

			// printf("C");
			setColour(BACKGROUND);
			break;
		case 2:
			//Extra space
			printf(" ");
			break;
		default:
			//This shouldn't be needed but keep it anyway
			printf(" ");
			break;
	}

	// printf("%d", offsetY);
}

void drawRoot(ScreenDimensions dims, int startLine, char **inputsList) {
	char appName[] = "TICK";
	int appNameSize = strlen(appName);

	setColour(BACKGROUND);

	int startHeight = startLine != 0 ? dims.height - startLine : 0;

	setCursorPos(1, startHeight + 1);

	for (int y = startHeight; y < dims.height; y++) {
		for (int x = 0; x < dims.width; x++) {
			//First header row
			const char *channelName = channelsList[selectedChannel].channel_name; //also requires const... not sure
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
			} else if (x > 25 && x < dims.width - 1 && y == dims.height - 2) {
				//Message box text content
				setColour(TEXT);

				char *messageText = inputsList[MESSAGE];
				int messageLength = strlen(messageText);

				if (messageText != NULL && x - 27 < messageLength && x - 27 >= 0) {
					printf("%c", messageText[x - 27]);
				} else {
					printf(" ");
				}
			} else if (y >= 1 && y <= 15 && x >= 1 && x <= 24) {
				// Channels Box
				Coordinate start = {1, 1};
				Coordinate end = {24, 15};

				setColour(CHANNELS);
				
				char title[] = "Channels (Tab)";
				struct BoxOptions opts = {start, end, 0, title};

				drawBox(x, y, opts, drawChannels);

				setColour(BACKGROUND);
			} else if (x > 26 && x < dims.width - 2 && y > 1 && y < dims.height - 4) {
				//Draw the channel's messages to the screen
				drawMessages(x, y, msgs, 10, dims);
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

	setCursorPos(28, dims.height - 1);
}

void drawHostInput(ScreenDimensions dims, char **inputsList) {
	char header[18] = "Enter a Server IP:";

	char continueStr[18] = "Then press enter.";

	char *boxText = inputsList[SERVER_IP_INPUT];
	int boxTextLen = strlen(boxText);

	for (int i = 0; i < 6; i++) {
		int lineSize = 18;
		int startIndex = dims.width / 2 - lineSize / 2;

		for (int j = 0; j < dims.width; j++) {
			if (j >= startIndex && j < startIndex + lineSize) {
				if (i == 2) {
					setColour(HEADER);
					printf("%c", header[j - startIndex]);
				} else if (i == 3) {
					setColour(TEXT);
					int stringIndex = j - startIndex;
					if (stringIndex >= 0 && stringIndex < boxTextLen) {
						printf("%c", boxText[stringIndex]);
					} else {
						printf(" ");
					}
				} else if (i == 5) {
					setColour(GREEN_BLINK);
					printf("%c", continueStr[j - startIndex]);
				}
			} else {
				printf(" ");
			}
		}
    
		printf("\n");
	}
}

void drawUsernameInput(ScreenDimensions dims, char **inputsList) {
	char header[18] = "Enter a Username:";

	char continueStr[18] = "Then press enter.";

	char *boxText = inputsList[USERNAME_INPUT];
	int boxTextLen = strlen(boxText);

	for (int i = 0; i < 6; i++) {
		int lineSize = 18;
		int startIndex = dims.width / 2 - lineSize / 2;

		for (int j = 0; j < dims.width; j++) {
			if (j >= startIndex && j < startIndex + lineSize) {
				if (i == 2) {
					setColour(HEADER);
					printf("%c", header[j - startIndex]);
				} else if (i == 3) {
					setColour(TEXT);
					int stringIndex = j - startIndex;
					if (stringIndex >= 0 && stringIndex < boxTextLen) {
						printf("%c", boxText[stringIndex]);
					} else {
						printf(" ");
					}
				} else if (i == 5) {
					setColour(GREEN_BLINK);
					printf("%c", continueStr[j - startIndex]);
				}
			} else {
				printf(" ");
			}
		}
    
		printf("\n");
	}
}

//Renders views selectively
void renderView(ScreenDimensions dims, int lines, char **inputsList) {
	switch (screen){
		case SPLASHSCREEN:
			splashscreen(dims);
			break;
		case HOST_PAGE:
			drawHostInput(dims, inputsList);
			break;
		case USERNAME_PAGE:
			drawUsernameInput(dims, inputsList);
			break;
		case MAIN:
			drawRoot(dims, lines, inputsList);
			break;
	}
}

int writeToInput(int c, Inputs input, char **inputsList) {
		//Make sure the char is something valid, (a-z, A-Z, symbols, etc, not things like delete)
		if ((c < 33 || c > 126) && c != 32 && c != 8) return 0;
	
    char *str = inputsList[input];
    if (str == NULL) return 0;

    int currentLength = strlen(str);
    if (c == 8) {
			//Backspace handling
			if (currentLength < 1) return 0;
			char *newStr = realloc(str, (currentLength == 1 ? 1 : (currentLength - 1)) * sizeof(char));
			if (newStr == NULL) return 0;

			if (currentLength == 1) {
				//Case where there would be no characters in the string, only the null character
				newStr[0] = '\0';
			} else {
				newStr[currentLength - 1] = '\0';
			}
			inputsList[input] = newStr;
		} else {
			//Adding a new character
			char *newStr = realloc(str, (currentLength + 1) * sizeof(char));
			if (newStr == NULL) return 0;

			newStr[currentLength] = (char)c;
			//Need the null terminated character here
			newStr[currentLength + 1] = '\0';
			inputsList[input] = newStr;
		}

		ScreenDimensions dims = screenSize();

		//Extra screen logic for the message, clearing the last two lines to reprint content
		if (input == MESSAGE) {
			clearLines(2, dims);
			renderView(dims, 2, inputsList);
		} else if (input == SERVER_IP_INPUT || input == USERNAME_INPUT) {
			clearScreen();
			renderView(dims, 0, inputsList);
		} 

    return 1;
}

//Basic methods needed to open and close sockets quickly after every request is sent
int createSocketInterface() {
	if (sock == INVALID_SOCKET || sock == 0) {
		return initializeClient(&sock, serverIP);
	} else {
		return 1;
	}
}

void shutdownSocket() {
	if (sock != INVALID_SOCKET) {
		sockShutdown(sock);
		sock = INVALID_SOCKET;
	}
}

void fetchMessages(char **inputsList) {
	if (sock == INVALID_SOCKET) createSocketInterface();
	// Message tempmessages[10];
	recieveMsgLatest(&sock, selectedChannel, msgs);

	sockShutdown(sock);

	clearScreen();

	ScreenDimensions dims = screenSize();

	renderView(dims, 0, inputsList);

	sock = INVALID_SOCKET;
}

//Basic screen input handling
void handleInput(int c, char **inputsList) {
	// printf("%d", c);
	switch (screen){
		case SPLASHSCREEN:
			screen = HOST_PAGE;
			break;
		case HOST_PAGE:
			if (c == 13) {
				//Enter pressed, move to next page, make sure something is at least typed
				if (strlen(inputsList[SERVER_IP_INPUT]) != 0) screen = USERNAME_PAGE;
				serverIP = inputsList[SERVER_IP_INPUT];
				if (createSocketInterface()) break;
				shutdownSocket();
			} else {
				writeToInput(c, SERVER_IP_INPUT, inputsList);
			}
			break;
		case USERNAME_PAGE:
			if (c == 13) {
				//Enter pressed, move to next page, make sure something is at least typed
				if (strlen(inputsList[USERNAME_INPUT]) != 0) screen = MAIN;
				if (createSocketInterface()) break;
				int userId = genUser(&sock, inputsList[USERNAME_INPUT]);

				User u = {
					userId,
					inputsList[USERNAME_INPUT]
				};

				currentUser.UUID = u.UUID;
				currentUser.name = u.name;

				shutdownSocket();
			} else {
				writeToInput(c, USERNAME_INPUT, inputsList);
			}
			break;
		case MAIN:
			ScreenDimensions dims = screenSize();

			if (c == 13) {
				//Handle logic to send the message to the server
				
				// printf("%d", strlen(inputsList[MESSAGE]));
				if (strlen(inputsList[MESSAGE]) == 0) break;
				
				// clearLines(2, dims);
				// renderView(dims, 2, inputsList);
				
				if (createSocketInterface()) break;

				MessageDeque *blankDeque = (MessageDeque *)malloc(sizeof(MessageDeque));
				if (blankDeque != NULL) {
					blankDeque->head = NULL;
					blankDeque->tail = NULL;
				}

				Channel channelObj = {
					channelsList[selectedChannel].channel_name,
					blankDeque,
					channelsList[selectedChannel].id
				};

				Message *m = initMessage(inputsList[MESSAGE], &currentUser, 0, &channelObj);

				if (m != NULL && sock != INVALID_SOCKET) {
					int send_err = sendMessage(&sock, m);
				}

				char *newMessageText = (char *)malloc(sizeof(char));
				if (newMessageText != NULL) {
					newMessageText[0] = '\0';
					char *temp = inputsList[MESSAGE];
					free(temp);
					inputsList[MESSAGE] = newMessageText;
				}

				if (blankDeque != NULL) {
					free(blankDeque);
				}

				shutdownSocket();
			} else if (c == 96) {
				fetchMessages(inputsList);
			} else if (c == 9) {
				//Handle tab to loop through selected channels
				if (selectedChannel >= 5) {
					selectedChannel = 0;
				} else {
					selectedChannel++;
				}
				clearScreen();
				renderView(dims, 0, inputsList);
			} else {
				writeToInput(c, MESSAGE, inputsList);
			}
			break;
	}
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
			// fetchMessages();
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
		//Important to initialize these strings with a null character
		inputsList[i][0] = '\0';
	}

	//Render the first view and start the display clock
	renderView(initial_dims, 0, inputsList);
	clock(&initial_dims, inputsList);
}