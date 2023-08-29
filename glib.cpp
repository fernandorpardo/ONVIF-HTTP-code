/**************************************************************************************************
 * General library
 * (c) Etherjuice 
 * 
 **************************************************************************************************
*/

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

#include "cstr.h"
#include "glib.h"

//#define DEBUGFILE_GLIB "/var/www/ramdisk/debug_glib.log"
//#include "debug.h"

// Generate TIMESTAMP for log files
// Returns:
// 2017-01-20 20:42:56
char *timestamp(char *ts, size_t max_sz)
{
	time_t t = time(NULL); 
	struct tm *tm = localtime(&t);
	snprintf(ts, max_sz, "%04d-%02d-%02d %02d:%02d:%02d", 2000+tm->tm_year-100, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return ts;
}
// Generates timestamp for elapsed time calculation
// Returns:
// yy-mm-dd hh:mm:ss
char *timestamp_record(char *ts, size_t ts_max_sz)
{
	time_t t = time(NULL); 
	struct tm *tm = localtime(&t);
	snprintf(ts, ts_max_sz, "%02d-%02d-%02d %02d:%02d:%02d", 
		
		tm->tm_year-100, 
		tm->tm_mon,
		tm->tm_mday, 

		tm->tm_hour, 
		tm->tm_min, 
		tm->tm_sec);
	return ts;
} // timestamp_record()

// input
//	ts = "yy-mm-dd hh:mm:ss"
char *timestamp_elapsed(char *ts, char* elapsed, size_t max_sz)
{
	elapsed[0]='\0';
	if(strlen(ts) == 17)
	{	
		char ts_now[24];
		char sdate[16], stime[16];
		char sh[4], sm[4], ss[4];
		char a[4], b[4], c[4];
		
		// -----------------------------------------
		// tm1= time now
		timestamp_record(ts_now, sizeof(ts_now));
		sscanf(ts_now, "%s %s", sdate, stime);
		// date
		cstr_replace(sdate, '-', ' ');
		sscanf(sdate, "%s %s %s", a, b, c);
		unsigned long d1= atoi(b) * 30 + atoi(c);	
		// time
		cstr_replace(stime, ':', ' ');
		sscanf(stime, "%s %s %s", sh, sm, ss);
		unsigned long tm1= ((unsigned long)atoi(sh) * 60L  + (unsigned long)atoi(sm)) * 60L + (unsigned long)atoi(ss);
		
		// -----------------------------------------
		// tm0 = time input ts= yy-mm-dd hh:mm:ss
		sscanf(ts, "%s %s", sdate, stime);
		// date
		cstr_replace(sdate, '-', ' ');
		sscanf(sdate, "%s %s %s", a, b, c);
		unsigned long d0=  atoi(b) * 30 + atoi(c);		
		// time
		cstr_replace(stime, ':', ' ');
		sscanf(stime, "%s %s %s", sh, sm, ss);
		unsigned long tm0= ((unsigned long)atoi(sh) * 60L  + (unsigned long)atoi(sm)) * 60L + (unsigned long)atoi(ss);
		unsigned long t= tm1 - tm0;
		if(tm0 > tm1) t= tm1 + 24 * 60L * 60L - tm0;
		
		unsigned long h, m, s;
		s= (unsigned long) (t % 60L);
		m= (unsigned long) ((t / 60L) % 60L);
		h= (unsigned long) (t / (60L * 60L));
		
		char s1[8], s2[8]; s1[0]='\0'; s2[0]='\0';
		// days
		//if( (d1-d0) > 1 ) snprintf(s1, sizeof(s1), "%lu ", (d1-d0-1));
		//else if(h>24) snprintf(s1, sizeof(s1), "%lu ", h / 24);
		if(d1>d0 || h>24)
		{
			unsigned long ddss= d1 * 24L * 3600L + tm1 -  d0 * 24L * 3600L - tm0;
			snprintf(s1, sizeof(s1), "%lu ", ddss / (24L * 3600L) );
		}
		// hours
		if(h) snprintf(s2, sizeof(s2), "%02lu ", h % 24);
		snprintf(elapsed, max_sz, "%s%s%02lu %02lu", s1, s2, m, s);
		fprintf(stdout,"\nelapsed= %s", elapsed);
		
		#ifdef _HADEBUG_H_
		char line[128];
		snprintf(line, sizeof(line),"tm1= %lu tm0= %lu t= %lu d= %lu", tm1, tm0, t, (d1-d0));
		trace2file(DEBUGFILE_GLIB, line);
		fprintf(stdout,"\n%s", line);
		snprintf(line, sizeof(line),"ts_now= %s ts= %s", ts_now, ts);
		trace2file(DEBUGFILE_GLIB, line);
		fprintf(stdout,"\n%s", line);
		#endif		
		
	}	
	return elapsed;
} // timestamp_elapsed()


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Data record
// ------------------------------------------------------------------------------------------------
int logWriteTS(const char *ts, const char *text, char* filename)
{
	if(filename[0]=='\0') return -1;
	
	//char ts[64];
	FILE *fp = fopen(filename, "r+");
	// CREATE - If file does not exist then create it
	if(fp == NULL)
	{
		// create file 
		fp = fopen(filename, "w");
		if(fp == NULL) {
			fprintf(stdout, "[ERROR] logWrite: failed to create file %s\n", filename);
			fflush(stdout);
			return -1;
		}
		file_mode_change(filename, "zx-11a:zx-11a", "666" );
	}
	// File is open for update
	if (fp != NULL) {
		//timestamp(ts, sizeof(ts)-1);
		fseek(fp, 0L, SEEK_END);
		// WRITE <timestamp><text><\n>
		fwrite(ts,  sizeof(char), strlen(ts), fp);
		fwrite(" ",  sizeof(char), 1, fp);
		size_t filelength =  fwrite(text,  sizeof(char), strlen(text), fp);
		fwrite("\n",  sizeof(char), 1, fp);
		fclose(fp);
		return (int) filelength;
	}
	return -1;
}
int fileRead(const char*filename, char*memptr, unsigned int ufsz)
{
	FILE *fp = fopen(filename, "r");	
	if ( fp != NULL ) 
	{	
		int n=0;
		unsigned int i= 0;
		do {
			n= fread (&memptr[i], sizeof(char), ufsz - i, fp);
			if(n>0) i += (size_t) n;
		} while (n>0 && i<ufsz);
		memptr[i]='\0';
		fclose(fp); 
		return (int) i;
	}
	return -1;
}
// ------------------------------------------------------------------------------------------------
// FILE
// ------------------------------------------------------------------------------------------------
// File management and manipulation library

// File functions
int file_size(const char *filename)
{
	int filelength= 0;
	long size;
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		if (fseek(fp, 0L, SEEK_END) == 0) 
			size= ftell(fp);
		// this is the normal flow ...
		if(size<=0) {
			fprintf(stdout, "\nSEEK_END size %ld", size);
			fflush(stdout);			
			fclose(fp);
			return (int) size;
		} 
		// ... but try to read something to double check - the other checks does not work always
		if (fseek(fp, 0L, SEEK_SET) == 0) {
			char buffer[2];
			size_t sz= fread(buffer, sizeof(char), 1, fp);
			if(sz==0) {
				fprintf(stdout, "\nfile_size sz %d", sz);
				fflush(stdout);			
				fclose(fp);
				return -1;
			} 
		}
		else {
			fprintf(stdout, "\nSEEK_SET");
			fflush(stdout);			
			fclose(fp);
			return -1;
		}
		filelength= (int) size;
		fclose(fp);
		return filelength;
	}
	else return -1;
}

int file_getcontent(const char *filename, char *buffer, size_t max)
{
	int filelength= 0;
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		/* Go to the end of the file. */
		if (fseek(fp, 0L, SEEK_END) == 0)  {
			if ((filelength = ftell(fp)) > 0) {
				/* Go back to the start of the file. */
				if (fseek(fp, 0L, SEEK_SET) == 0) {
					/* Read the entire file into memory. */
					size_t newLen = fread(buffer, sizeof(char), filelength> (int)max? max : filelength, fp);
					filelength= newLen;
				}
			}
		}
		fclose(fp);
	}
	else return -1;
	return filelength;	
}
bool file_is(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		fclose(fp);
		return true;
	}
	return false;
}

bool isNumber(char *s)
{
	unsigned int i=0;
	if(s[i]!='\0' && (s[i]=='-' || s[i]=='+')) i++;
	for (; i<strlen(s) && isdigit(s[i]); i++);
	return !(i<strlen(s));
}

int file_mode_change(const char *filename, const char *user, const char *mode)
{
	if(fork() == 0) {
		execl("/bin/chown", "chown", user, filename, (char *)0);
		exit(EXIT_SUCCESS);
	} else {
		wait(NULL);
		if(fork() == 0) {
			execl("/bin/chmod", "chmod", mode, filename, (char *)0);
			exit(EXIT_SUCCESS);
		} else {
			wait(NULL);
			return 0;
		}
	}
	return -1;
}
int file_copy(const char *filename, const char *filename_copy)
{
	if(fork() == 0) {
		execl("/bin/cp", "cp", filename, filename_copy, (char *)0);
		exit(EXIT_SUCCESS);
	} else {
		wait(NULL);
		return 0;
	}
	return -1;
}

// filename -  full file name including path
int file_rotate(char *filename)
{
	fprintf(stdout, "\n");
	
	char path[32];
	path[0]='\0';
	char name[32];
	name[0]='\0';
	char ext[32];
	ext[0]='\0';
	
	int pa= 0, pb;
	while( (pb=cstr_find((char*)filename, (char*)"/", pa)) >= 0) pa= pb+1;
	if(pa>0) cstr_sub( (char*)filename, path, 0, pa-1);
//	fprintf(stdout, "path  %s\n", path);
	int i= cstr_find( (char*)&filename[pa], (char*)".");
	if(i>0) cstr_sub( (char*)&filename[pa], name, 0, i-1);
//	fprintf(stdout, "name  %s\n", name);
	if(i>0) strcpy( ext, (char*)&filename[pa+i+1]);
//	fprintf(stdout, "ext  %s\n", ext);
	
	char sdate[32];
	timestamp(sdate, sizeof(sdate));
//	fprintf(stdout, "i= %d  %s\n", i, timestamp(sdate, sizeof(sdate)) );
	cstr_replace(sdate, ' ', '_');
	cstr_replace(sdate, '-', '\0');
	cstr_replace(sdate, ':', '\0');
//	fprintf(stdout, "%s\n", sdate );
	
	char str[128];
	snprintf(str, sizeof(str), "%s%s_%s.%s", path, name, sdate, ext);
	fprintf(stdout, "FILE ROTATE %s\n", str );
	
	if(file_copy(filename, str) <0) return -1;
	
	FILE *fp = fopen(filename, "w");
	if (fp != NULL) {
		fclose(fp);
		return 0;
	}
	return -1;
}

int file_delete(const char *filename)
{
	/*
	if(fork() == 0) {
		execl("/bin/rm", "rm", filename, (char *)0);
		exit(EXIT_SUCCESS);
	} else {
		wait(NULL);
		return 0;
	}
	return -1;
	*/

	char str[128];
	str[0]='\0';
	FILE *in;
	snprintf(str, sizeof(str), "sudo rm %s", filename);
	if(!(in = popen(str, "r"))){
		
		return -1;
	}
	else {
		
		while(fgets(str, sizeof(str), in)!=NULL); 
		//str[n]='\0';
		pclose(in);
	}
	
	fprintf(stdout, str);
	return 0;
}

// ------------------------------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------------------------------




// ------------------------------------------------------------------------------------------------
// -------------------------- LOG -----------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

char defaultfilename[128] = {0};

int logInit(const char *filename, bool create)
{
	defaultfilename[0]='\0';
	snprintf(defaultfilename, sizeof(defaultfilename),"%s", filename);
	
	if(filename[0]=='\0') return -1;

	if(create)
	{
		FILE *fp = fopen(filename, "w");
		if(fp == NULL)
		{
			fprintf(stdout, "[ERROR] logWrite: failed to create file %s\n", filename);
			fflush(stdout);
			return -1;
		}
		else
		{
			fclose(fp);
			file_mode_change(filename, "zx-11a:zx-11a", "666" );
		}
	}
	return 0;
}

int logWrite(const char *text, char* filename)
{
	if(filename[0]=='\0') return -1;
	
	char ts[64];
	FILE *fp = fopen(filename, "r+");
	// CREATE - If file does not exist then create it
	if(fp == NULL)
	{
		// create file 
		fp = fopen(filename, "w");
		if(fp == NULL) {
			fprintf(stdout, "[ERROR] logWrite: failed to create file %s\n", filename);
			fflush(stdout);
			return -1;
		}
		file_mode_change(filename, "zx-11a:zx-11a", "666" );
	}
	// File is open for update
	if (fp != NULL) {
		timestamp(ts, sizeof(ts)-1);
		fseek(fp, 0L, SEEK_END);
		// WRITE <timestamp><text><\n>
		fwrite(ts,  sizeof(char), strlen(ts), fp);
		fwrite(" ",  sizeof(char), 1, fp);
		size_t filelength =  fwrite(text,  sizeof(char), strlen(text), fp);
		fwrite("\n",  sizeof(char), 1, fp);
		fclose(fp);
		return (int) filelength;
	}
	return -1;
}


/* END OF FILE */
