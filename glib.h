#ifndef GLIB_HEADER_FILLE_H
#define GLIB_HEADER_FILLE_H


char *timestamp(char *ts, size_t max);
char *timestamp_record(char *ts, size_t ts_max_sz);
char *timestamp_elapsed(char *ts0, char* elapsed, size_t max_sz);

int logWriteTS(const char *ts, const char *text, char* filename);
int fileRead(const char*filename, char*memptr, unsigned int ufsz);

// log
extern char defaultfilename[];
int logInit(const char *filename, bool create=false);
int logWrite(const char *text, char* filename=defaultfilename);

// file
int file_copy(const char *filename, const char *filename_copy);
int file_size(const char *);
int file_getcontent(const char *, char *, size_t );
int file_rotate(char *filename);
int file_mode_change(const char *filename, const char *user, const char *mode);
int file_delete(const char *filename);

bool isNumber(char *);

#endif
/* END OF FILE */