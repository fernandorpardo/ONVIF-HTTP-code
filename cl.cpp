/** ************************************************************************************************
 *	CL - Command Line functions
 *  Part of the Wilson project (www.iambobot.com)
 *  Fernando R
 *
 *
 ** ************************************************************************************************
**/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

#include "cl.h"

// File management and manipulation library
// return: argv count
size_t cParser(char* inputline, char a[8][32])
{
	int ls= strlen(inputline);
	int i=0; 	// index inputline[i]
	size_t j=0;	// index argv[j]
	int k=0; 	// index argv[j][k]
	
	//fprintf(stdout, "\ncParser %d %s", ls, inputline); fflush(stdout);
	//return ls;
	
	a[j][k]= '\0';
	// get i pointing to the first non space char
	while(i<ls && inputline[i]!=0 && inputline[i]==' ') i++;
	while(i<ls && inputline[i]!=0 && j<8)
	{
		if(inputline[i]==' ' || inputline[i]=='\n') 
		{
			// END OF WORD
			// Remove spaces & CR
			while(i<ls && inputline[i]!=0 && (inputline[i]==' ' || inputline[i]=='\n')) i++;
			//start new word
			if(i<ls && inputline[i]!=0) 
			{
				//fprintf(stdout, "\ncParser j= %d", j); fflush(stdout);
				k=0;
				j++;
			    //fprintf(stdout, "\n %lu %d %d %d", (unsigned long) &a[j][k], i, j, k); fflush(stdout);
				a[j][k]='\0';
			}
		}
		else
		{
			//fprintf(stdout, "\n %lu %d %d %d", (unsigned long) &a[j][k], i, j, k); fflush(stdout);
			a[j][k++]= inputline[i++];
			a[j][k]= '\0';
		}
	}
	// count last word
	if(k!=0) j++;
	// done
	return j;
}
// ------------------------------------------------------------------------------------------------
// -------------------------- ---- ----------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/* ----------------------------------------------------------------------------------------------------- */
/* -------------------------------------------  KEYBOARD  ---------------------------------------------- */
// restore values
struct termios term_flags;
int term_ctrl;
int termios_init()
{
	/* get the original state */
	tcgetattr(STDIN_FILENO, &term_flags);
	term_ctrl = fcntl(STDIN_FILENO, F_GETFL, 0);
	return 0;
}
int termios_restore()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &term_flags);
	fcntl(STDIN_FILENO, F_SETFL, term_ctrl);
	return 0;
}
int kbhit(void)
{
	struct termios newtio, oldtio;
	int oldf;

    if (tcgetattr(STDIN_FILENO, &oldtio) < 0) /* get the original state */
        return -1;
    newtio = oldtio;
	/* echo off, canonical mode off */
    newtio.c_lflag &= ~(ECHO | ICANON );  
	tcsetattr(STDIN_FILENO, TCSANOW, &newtio);
 	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	int ch = getchar();
 	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
 	tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	return 0;
}

/* ----------------------------------------------------------------------------------------------------- */
/* ------------------------------------------  CommandLineBuff ----------------------------------------- */
CommandLineBuff::CommandLineBuff(char *command_buffer, string prompt)
{
	this->command_b_read= 0;
	this->command_b_write= 0;
	this->command_b_length= 0;
	this->command_b_last= 0;
	this->prompt= prompt;
	this->command_buffer= command_buffer;
}
void CommandLineBuff::Up()
{
	this->command_b_read --;
	if(this->command_b_read<0) this->command_b_read= this->command_b_length - 1;
//    return 0;
}
void CommandLineBuff::Down()
{
	this->command_b_read ++;
	if(this->command_b_read>=COMMNAD_QUEUE_LENGTH || 
		(this->command_b_length<COMMNAD_QUEUE_LENGTH && this->command_b_read>=this->command_b_length))
		this->command_b_read= 0;	
//    return 0;
}
int CommandLineBuff::Last()
{
	int ic=0;
	if(this->command_b_length > 0)
	{
		// Delete last
		ic= this->command_queue[command_b_last].length();
		printf("\r");
		printf(this->prompt.c_str());
		for (int i=0; i<ic; i++) printf(" ");						
		printf("\r");
		// Print prompt
		printf(this->prompt.c_str());
		// take b_read
		strcpy(this->command_buffer, this->command_queue[command_b_read].c_str());
		printf("%s", this->command_buffer);
		this->command_b_last= this->command_b_read;
		ic= this->command_queue[command_b_read].length();
	}
	return ic;
}
void CommandLineBuff::Store()
{
	string command_line(this->command_buffer);
	this->command_queue[this->command_b_write]= command_line;
	this->command_b_read= this->command_b_write;
	this->command_b_last= this->command_b_read;
	this->command_b_write = (this->command_b_write + 1) % COMMNAD_QUEUE_LENGTH;
	if(this->command_b_length < COMMNAD_QUEUE_LENGTH) this->command_b_length++;
//	return 0;
}

// END OF FILE