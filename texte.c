/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "rawMode.c"
#include "textViewer.c"
#include "rawInOut.c"


//prints an error msg and exits
//if any of teh proc returns -1 means they failed and die is called


/**** init ***/ 
void initEditor() {
	if(getWindowSize(&E.screenrows, &E.screencols) == -1 )
		die("getWindowsSize");
}
////
int main() {
	enableRaw();
	initEditor();
	//displays the input as long as q isnot pressed
	while(1)	{
		
		editorRefreshScreen();
		editorProcessKeypress();
		
		/*char c = '\0';
		if(read(STDIN_FILENO, &c,1) == -1 && errno != EAGAIN)
			die("read");
		if(iscntrl(c)) { //checks if c is printable
			printf("%d\r\n",c);
		}
		else {
			printf("%d (%c)\r\n",c,c);
		}
		if (c== CTRL_KEY('q')) 
			break;*/
	}
	return 0;
		
}

