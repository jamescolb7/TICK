/*
FILENAME:   main.c
COURSE:     MREN 178

~ Group 1 ~

James Colbourne          STUDENT ID: 20523893

*/

#include "interface.h"
#include <stdio.h>
#include <signal.h>

int main() {
	signal(SIGINT, interfaceCleanup);

	initializeDisplay();

	return 0;
}