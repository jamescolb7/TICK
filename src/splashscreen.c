/*
FILENAME:   splashscreen.c
COURSE:     MREN 178

~ Group 1 ~

Beric Dengler            STUDENT ID: 20515669
James Colbourne          STUDENT ID: 20523893

*/

#include <stdio.h>
#include "interface.h"
#include "colours.h"

#define SPLASH_HEIGHT 26
#define SPLASH_WIDTH 63

char TICK_SPLASH[SPLASH_HEIGHT][SPLASH_WIDTH] = 
                        {
                        "                                                               ",
                        "---------------------------------------------------------------",
                        "                                                               ",
                        "|##########|     |##########|      /##########     \\##/    /###",
                        "\\##########/     \\##########/     /##########/     |##    /###/",
                        "    |##|             \\##/         |###/            |##   /##/  ",
                        "    |##|             |##|         /##/             |##\\ /##/   ",
                        "    |##|             |##|         ###              |######/    ",
                        "    |##|             |##|         ###              |#####<     ",
                        "    |##|             |##|         ###              |######\\    ",
                        "    |##|             |##|         \\##\\             |##/ \\##\\   ",
                        "    |##|             /##\\         |###\\            |##   \\##\\  ",
                        "    /##\\         /##########\\     \\##########\\     |##    \\###\\",
                        "   /####\\        |##########|      \\##########     /##\\    \\###",
                        "                                                               ",
                        "---------------------------------------------------------------",
                        "                                                               ",
                        "              /---------------------------------\\              ",
                        "              |Terminal Interface Chatting Kiosk|              ",
                        "              \\---------------------------------/              ",
                        "                                                               ",
                        "           Made By James, Nic, Lukas & Beric Dengler           ",
                        "                                                               ",
                        "                                                               ",
                        "                                                               ",
                        "                 Press Any Key to Continue ...                 ",
                        };

void splashscreen(ScreenDimensions dims) {
	for (int i = 0; i < SPLASH_HEIGHT; i++) {
		int lineSize = SPLASH_WIDTH;
		int startIndex = dims.width / 2 - lineSize / 2;

		for (int j = 0; j < dims.width; j++) {
			if (j >= startIndex && j < startIndex + lineSize) {
				printf("%c", TICK_SPLASH[i][j - startIndex]);
        if(i>24)
          setColour(COMMAND_BLINK);
        else if(i>20)
          setColour(NOTICE);
        else if(i>16)
          setColour(SUB_TITLE);
        else if(i>14)
          setColour(DARK_GRAY);
        else if(i>1)
          setColour(TITLE);
        else
          setColour(DARK_GRAY);
			} else {
				printf(" ");
			}
		}
    
		printf("\n");
	}
}