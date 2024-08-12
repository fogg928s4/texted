/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>



struct editorConfig {
	struct termios orig_termios; //a struct type termios
	int screenrows, screencols;
};

struct editorConfig E;

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s); //from stdio prints the error
	exit(1);
}

void disableRaw() {
	//sets the attribute for the terminal to the orig form
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");

}

void enableRaw() {
	//gets the current termios and saves it in orig
	//if fail returns -1 and exit
	//to fail, input a fail in terminal
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr");
	
	//from stdlib calls the func disableRaw when exit
	atexit(disableRaw);

	struct termios raw = E.orig_termios;

	//flags that determine what is outpute or input
	//&= for and, |= for not

	//input flag
	raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
	
	//output flags
	raw.c_oflag &= ~(OPOST);
	
	//local or misc flags
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	 //not a flag but a but nask that sets the char size to 8 bytes
	raw.c_lflag |= (CS8);		

	//from this point read will wait for input indefilnetly but something can be shown while it waits
	//these come from termios and aare ctrl chars that control settings
	//VTIME is the max time to wait before read() retrns whatever it read and is in tenth of secs
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	//makes the terminal take the attributes given
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}