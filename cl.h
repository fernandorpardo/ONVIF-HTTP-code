/* ----------------------------------------------------------------------------------------------------- */
/* ------------------------------------------  CommandLineBuff ----------------------------------------- */
#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

#define COMMNAD_QUEUE_LENGTH 5
class CommandLineBuff
{
	public:
		CommandLineBuff(char *command_buffer, string prompt);
		void Up();
		void Down();
		int Last();
		void Store();
	private:
		string command_queue[COMMNAD_QUEUE_LENGTH];
		int command_b_read;
		int command_b_write;
		int command_b_length;
		int command_b_last;
		char *command_buffer;
		string prompt;
};

// terminal
int termios_init();
int termios_restore();
int kbhit(void);

// command line parser
size_t cParser(char* inputline, char argv[8][32]);
#endif
// END OF FILE