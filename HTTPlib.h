#ifndef _HTTPLIB_H_
#define _HTTPLIB_H_

#define HTTPLIB_VERSION	"1.2.0"

// Temporal files
#define INTERMEDIATE_GZIP_FILE_EXT 	".gzip"	// is an intermediate file to store the data in GZIP, is the data source for payload_inflate()
// Output file
#define HTTP_OUTPUT_FILE_EXT 		".txt"	// is the final destination of the PAYLOAD (wwithout HTTP header) in uncompressed format


int HTTPrequest(const char* storage, const char *servername, const char* serverport, const char *request, int trace= 0);
char *ZLIBversion(void);
#endif
// END OF FILE