#define TOTAL_INPUTS 3

void drawRoot();

void clearScreen();

void initializeDisplay();

void interfaceCleanup();

void setColour(char colour[]);

typedef struct {
	int height;
	int width;
} ScreenDimensions;

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

typedef enum {
	MESSAGE,
	SERVER_IP_INPUT,
	USERNAME_INPUT
} Inputs;

typedef enum {
	SPLASHSCREEN,
	HOST_PAGE,
	USERNAME_PAGE,
	MAIN
} Screens;