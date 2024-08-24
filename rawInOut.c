/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


//make ctrl q exit the program like q does
//ctrl_key macro 00011111 but is specified in hex and sets the firts digits to 0 mirroring what the ctrl does
#define CTRL_KEY(k) ((k) & 0x1f)
#define TEXTED_VERSION "0.0.1a"

enum editorKey {
	//now well have to represent arrow key to not conflict w/ WASD
	//we give large value out of char range to not conflict
	ARROW_LEFT = 1000,
	ARROW_RIGHT ,
	ARROW_UP ,
	ARROW_DOWN
};

/**append buffer**/
//to write one big insted of small ones
struct abuf {
	char *b; 
	int len; //length
};
#define ABUF_INIT {NULL, 0} //refe to a pointer to our buffer in mem and a length
//this is an empty buffer ans serves as a {constructor} for abuf type


//basically the new write
void abAppend(struct abuf *ab, const char *s, int len) { //constructor
	char *new = realloc(ab->b, ab->len + len);
	//reallocates space just enough fo rhte string
	if(new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;//ads the len, othrwise no output generated
}

void abFree(struct abuf *ab) { //destructor
	free(ab -> b);
}

//funtion for low level key-press read 
/*** input ***/

//reads the keyinput and returns it
char editorReadKey() {
	
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1) != 1)) {
		if (nread == -1 && errno != EAGAIN)
			die("read");
	}
	//for the keys to function to move
	if(c=='\x1b'){
		char seq[3];
		if(read(STDIN_FILENO, &seq[0], 1) != 1)
			return '\x1b';
		if(read(STDIN_FILENO, &seq[1], 1) != 1)
			return '\x1b';
		
		if(seq[0] == '['){ //[ is called when an arrow is pressed
		//if time out we assume it was just ESC and do nothing
		//if it returns an actual arrow press, it returns its corresponding thingy
			switch (seq[1]) {
				case 'A':
					return ARROW_UP; //instead of mapping to the char it maps to an special char in an enum
				case 'B':
					return ARROW_DOWN;
				case 'C':
					return ARROW_RIGHT;
				case 'D':
					return ARROW_LEFT;
			}
		}
		else
			return c;
	}
	
	return c;
}

//this is now where it will output what we write, basically the new main
int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;
	
	if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) //n gives us status on terminal 6 for cursor pos
		return -1;
	
	while(i < sizeof(buf) - 1) {
		if(read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if(buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';
	if(buf[0] != '\x1b' || buf[1] != '[')
		return -1;
	
	//sscanf from stdio

	if(sscanf(&buf[2], "%d;%d", rows, cols) != 2)
		return -1;
	
	return 0;
	//printf("\r\n&buf[1]: '%s'\r\n", &buf[1]);
	
	/*printf("\r\n");
	char c;
	
	while (read(STDIN_FILENO, &c, 1) ==1) {
		if(iscntrl(c)) 
			printf("%d\r\n", c);
		else 
			printf("%d (%c)\r\n", c,c);
	}
	*/
	editorReadKey(); //temp measure
	return -1;
	
}

//Termional Input Output Control Get WINdows SiZe
int getWindowSize(int *rows, int *cols) {
	struct winsize ws; //from ioctl.h
	
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col ==0){
		if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) //moves the cursor C, down B a lot of lines 999
			return -1;
		//since we always return -1 editorReadKey was called to give ius time before die comes and ruins the fun
		return getCursorPosition(rows,cols);
	}
	else {
		*cols = ws.ws_col;
		*rows = ws.ws_row; 
		//on sucess it sends throught the pointer the val, this is good for sending multiple int values
		return 0;
	}
}

void editorMoveCursor(char key) {
	switch( key) {
		//calls for the values in enum to get the arrow pressed.
		case ARROW_UP:
			E.cy--;
			break;
		case ARROW_LEFT:
			E.cx--;
			break;
		case ARROW_DOWN:
			E.cy++;
			break;
		case ARROW_RIGHT:
			E.cx++;
			break;
	}
}

/*** terminal ***/
//waits for a keypress and handles it, if its a q it exits
//deals with low level termin input
void editorProcessKeypress() {
	char c = editorReadKey();
	
	//to move with WASD or exit with CTRL Q
	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
		
		case ARROW_UP:
		case ARROW_LEFT:
		case ARROW_DOWN:
		case ARROW_RIGHT:
		//if any of the direction are pressed, it calls for move cursor, else it doesnt
			editorMoveCursor(c);
			break;
	}
	
}

//drawas the lil ~ like in vim
void editorDrawRows(struct abuf *ab) {
	int y;
	for (y=0; y < E.screenrows; y++) {
		
		//welcome msg
		if(y == E.screenrows /2) { //for pos
			char welcome[80]; //interpolate the buffer into the welcome msg
			int welcomelen = snprintf(welcome, sizeof(welcome), "TXT editor -- version %s", TEXTED_VERSION);
			if(welcomelen > E.screencols)	welcomelen =E.screencols; //truncate if msg too long
			
			int padding = (E.screencols - welcomelen) /2; //center the text
			//to center
			if (padding) {
				abAppend(ab, "~",1);
				padding--;
			}
			while(padding--) 
				abAppend(ab, " ", 1);
			
			abAppend(ab,welcome, welcomelen);
		}
		else {
			abAppend(ab, "~", 1); //this is basically the new write
		}
			
		abAppend(ab, "\x1b[K", 3); //clears one line at a time instead of all with K comm
		//to write the ~ on the last line
		if(y < E.screenrows - 1) 
			abAppend(ab, "\r\n", 2);
	}
}


/*** output ***/
void editorRefreshScreen() {
	struct abuf ab = ABUF_INIT;
	
	abAppend(&ab, "\x1b[?25l", 6);
	//to hide the cursor while 
	
	//abAppend(&ab, "\x1b[2J", 4); //4 bytes into terminal
	//x1b is the escape char or 27, the other 3 butes are [2j
	//thia is a escape seq, which always start with 27 followed by [ 
	//esc seq take commands, and args. we use J comn to clear and 2 for entire, 1 only where cursor is , 0 for up the cursor
	//no arg is cursor to end, well mostly use VT100 esq seq. llook the docu
	abAppend(&ab, "\x1b[H", 3);
	//3 bytes long, comm H. no args means top left
	//for center ud type [12;40H on a 80x24 screen, args are separated with ;
	
	editorDrawRows(&ab);
	
	char buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1); //terminal starts at 1, not 0
	abAppend(&ab, buf, strlen(buf));
	
	
	abAppend(&ab, "\x1b[?25h", 6);
	
	write(STDOUT_FILENO, ab.b,ab.len);
	abFree(&ab);
	
}

 //the following with draw ~ on each row like in vim

	/*First we make sure it responded with an escape sequence. 
	Then we pass a pointer to the third character of buf to sscanf(), skipping the '\x1b' and '[' characters. 
	So we are passing a string of the form 24;80 to sscanf(). 
	We are also passing it the string %d;%d which tells it to parse two integers separated by a ;, 
	and put the values into the rows and cols variables.
	*/