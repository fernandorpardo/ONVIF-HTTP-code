/** ************************************************************************************************
 *	Denug - Debug functions 
 *  Part of the Wilson project (www.iambobot.com)
 *  Fernando R
 *
 *
 ** ************************************************************************************************
**/

#include <stdio.h>	// FILE
#include <string.h>	// strlen

#include "glib.h"
#include "debug.h"



/* ----------------------------------------------------------------------------------------------------- */
/*
	output "str" to file and stdout
	t (true/false) generate time stamp
*/
/*
void filetrace(char *str, bool do_ts)
{
	char filename[]= WS_DEBUG;
	FILE *fp = fopen(filename, "r+");
	// CREATE - If file does not exist then create it
	if(fp == NULL)
	{
		// create file 
		fp = fopen(filename, "w");
		if(fp == NULL) {
			fprintf(stdout, "[ERROR] logWrite: failed to create file %s\n", filename);
			fflush(stdout);
		}
		file_mode_change(filename, "pi:pi", "666" );
	}
	// File is open for update
	if (fp != NULL) {
		fseek(fp, 0L, SEEK_END);
		// with timestamp
		if (do_ts) 
		{
			char ts[32];
			char line[128];			
			snprintf(line, sizeof(line),"[%s] %s", timestamp(ts, sizeof(ts)), str);
			fwrite(line, sizeof(char), strlen(line), fp);
			fprintf(stdout, "\n%s", line);
		}
		// no timestamp
		else {
			fwrite(str, sizeof(char), strlen(str), fp);
			fprintf(stdout, "\n%s", str);
		}
		fwrite("\n", sizeof(char), 1, fp);
		fclose(fp);
		fflush(stdout);	
	}
} // filetrace
*/

void trace2file(char const*filename, char *str, bool do_ts)
{
	FILE *fp = fopen(filename, "r+");
	// CREATE - If file does not exist then create it
	if(fp == NULL)
	{
		// create file 
		fp = fopen(filename, "w");
		if(fp == NULL) {
			fprintf(stdout, "[ERROR] logWrite: failed to create file %s\n", filename);
			fflush(stdout);
		}
		file_mode_change(filename, "zx-11a:zx-11a", "666" );
	}
	// File is open for update
	if (fp != NULL) {
		fseek(fp, 0L, SEEK_END);
		// with timestamp
		if (do_ts) 
		{
			char ts[32];
			char line[128];			
			snprintf(line, sizeof(line),"[%s] %s", timestamp(ts, sizeof(ts)), str);
			fwrite(line, sizeof(char), strlen(line), fp);
			//fprintf(stdout, "\n%s", line);
		}
		// no timestamp
		else {
			fwrite(str, sizeof(char), strlen(str), fp);
			//fprintf(stdout, "\n%s", str);
		}
		fwrite("\n", sizeof(char), 1, fp);
		fclose(fp);
		//fflush(stdout);	
	}
} // trace2file

// END OF FILE