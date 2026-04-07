#include "interface.h"
#include <stdio.h>
#include <signal.h>

int main() {
	signal(SIGINT, interfaceCleanup);

	initializeDisplay();

	return 0;
}