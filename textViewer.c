/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


//datatype for storing a row of text
typedef struct erow {
	int size;
	char *chars;
} erow;

struct ediorConfig {
	int cx,cy;
	int screenrows, screencols;
	int numrows;
	erow row;
};