/** ************************************************************************************************
 *	HTTP library 
 *  Part of the Wilson project (www.iambobot.com)
 *  Fernando R
 *
 *  HTTP function for GET/POST
 *
 * 1.0.0 - May 2021
 * 1.1.0 - August 2022
 *		hhtpGET - is DEPRECATED
 *		HTTPrequest - function to manage HTTP (no SSL) GET & POST
 * 1.2.0 - 01 Sep 2023 Commons version from tapo.d
 *
 * take a look to
 * https://www.geeksforgeeks.org/socket-programming-cc/
 *
 ** ************************************************************************************************
**/

#include "local.h"					// local dependences
#include <stdio.h>      			// Input/Output - FILE *
#include <string.h>					// strlen
#include <errno.h>      			// Errors - errno
#include <stdlib.h>    			 	// General Utilities - malloc(), atoi()
#include <netdb.h>
#include <sys/time.h> 				// gettimeofday() - elapsed time
#include <assert.h>					// assert()
#include <unistd.h>
#include "cstr.h"					// HTTPHeader_ functions
#include "HTTPlib.h"

#ifdef ZLIB_SUPPORT
#include "../zlib-1.2.13/zlib.h"	// inflate
void payload_inflate(const char *src, const char *dst, int mwindowBit);
#endif

#ifndef ERROR_STATUS
#define ERROR_STATUS (int)-1
#endif



// ------------------------------------------------------------------------------------------------
// --------------- HTTP        --------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*
	HTTPrequest()
	Send HTTP request to server:port and stores the response in a file (storage + HTTP_OUTPUT_FILE_EXT)
*/
int HTTPrequest(const char* storage, const char *servername, const char* serverport, const char *request, int tracelevel)
{
	char htmlcode[1024];
	char str[128];
	char output_file[128];
	char gzip_file[128];
	snprintf(output_file, sizeof(output_file), "%s%s", storage, HTTP_OUTPUT_FILE_EXT);
	snprintf(gzip_file, sizeof(gzip_file), "%s%s", storage, INTERMEDIATE_GZIP_FILE_EXT);
	
	// empty file
	FILE *fp = fopen(output_file, "w");
	if (fp != NULL) 
	{
		fclose(fp);
	}
	
	if(tracelevel)
	{
		fprintf(stdout, "\nHTTP request %s:%s", servername, serverport);
		fflush(stdout);
	}
 
	/*
	struct sockaddr_in servaddr;
	char **pptr;
	struct hostent *hptr;
	if((hptr = gethostbyname(servername)) == NULL)
		printf("\nERROR gethostbyname\n");

	fprintf(stdout, "\nhostname: %s", hptr->h_name);
	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL) 
	{
		fprintf(stdout, "\naddress: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
	} 
	else {
		fprintf(stderr, "\n[ERROR]Error call inet_ntop");
		fflush(stderr);
	}
	fflush(stdout);
	*/
	
	// 1. CREATE SOCKET
    struct addrinfo hints = {0}, *addrs;
    hints.ai_family = AF_UNSPEC;	// The address family is unspecified
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(servername, serverport, &hints, &addrs);
    if (status != 0)
    {
        fprintf(stdout, "[HTTPrequest] ERROR %s: %s\n", servername, gai_strerror(status));
		return -1;
    }

    int sockfd, err;
    for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next)
    {
		// On success, a file descriptor for the new socket is returned.  On
		// error, -1 is returned, and errno is set to indicate the error.
        sockfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sockfd == ERROR_STATUS)
        {
            err = errno;
            continue;
        }
		// Set time-out before connect
		struct timeval timeout;      
		timeout.tv_sec = 9; // 9 second timeout
		timeout.tv_usec = 0;
		if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			fprintf(stdout, "[HTTPrequest] ERROR "); 
			fflush(stdout); 
			perror("setsockopt failed\n");
			return -1;
		}
		if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			fprintf(stdout, "[HTTPrequest] ERROR ");
			fflush(stdout); 
			perror("setsockopt failed\n");
			return -1;	
		}
        if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) == 0)
        {
			break;
        }
        err = errno;
        sockfd = ERROR_STATUS;
        close(sockfd);
    }	
    freeaddrinfo(addrs);

    if (sockfd == ERROR_STATUS)
    {
		fprintf(stdout, "\nERROR connect(): (%d) %s", err, strerror(err));
		fflush(stdout);
		return -1;
    }		
	
	if(tracelevel)
	{
		fprintf(stdout, "\n--> SENT ----------------------");
		fprintf(stdout, "\nSEND:\n%s\n", request);
	}
	
	// elapsed time T0
	timeval t0;
	gettimeofday(&t0, NULL);
	
	// 2. SEND REQUEST
	ssize_t n= write(sockfd, request, strlen(request));

	// 3. RECEIVE RESPONSE
	ssize_t sum= 0;
	n = read(sockfd, htmlcode, sizeof(htmlcode)-1);
	if(n>0)
	{
		sum= n;
		//-------------------------------------------------------
		// First packet
		// Process response
		size_t Content_Length= atoi(HTTPHeader_Entity("Content-Length", htmlcode, sizeof(htmlcode), str));
		HTTPHeader_Entity("Content-Encoding", htmlcode, sizeof(htmlcode), str);
		bool is_gzip= (strcmp(str, "gzip")==0);	
		if(tracelevel)
		{		
			fprintf(stdout, "\n<--- RESPONSE ----------------------");
			fprintf(stdout, "\nstatus code: %d", HTTPHeader_status_code(htmlcode));
			fprintf(stdout, "\nContent-Length: %d", Content_Length);
			fprintf(stdout, "\nContent-Encoding value= %s", str);	
			fprintf(stdout, "\nContent-Type value= %s", HTTPHeader_Entity("Content-Type", htmlcode, sizeof(htmlcode), str));
		}
		
		int ixpl= cstr_find(htmlcode, "\r\n\r\n", 0, sizeof(htmlcode));
		if(Content_Length)
		{
			char *pm= (char*)malloc(Content_Length + 1024);
			if (pm==NULL) return -1;
			size_t im= 0;
			if(ixpl>0) {
				// ixpl poin to the end of header limiter /r/n/r/n, then add 4 to poin to the payload star
				im= n - ixpl - 4;
				memcpy(pm, &htmlcode[ixpl+4], im);
			}
			// Check if there are packets left
			// get packets until getting Content_Length bytes of the payload
			do
			{
				n=read(sockfd, &pm[im], Content_Length-im);
				if(n>0)
				{
					sum +=n;
					im +=n;
				}
			} while(n>=0 && (Content_Length-im)>0);
			
			// Trace
			htmlcode[ixpl]='\0';
			if(tracelevel)
			{
				fprintf(stdout, "\n------------ HEADER (%d)\n%s", strlen(htmlcode), htmlcode);
				fprintf(stdout, "\n------------ PAYLOAD (%d)\n", im);
			}
			if(tracelevel>1) cstr_dump(pm, im);
			fflush(stdout);
			
			// pm holds the PAYLOAD received that can we plain text or GZIPed
			// if Content-Encoding is GZIP then the payload needs to be inflated
			if(is_gzip)
			{
#ifdef ZLIB_SUPPORT
				fprintf(stdout, "\n------------ GZIP (%d)", im-10-8);
				// (1) we firts store the GZIP formated PAYLOAD into a file
				FILE *fp_out = fopen(gzip_file, "w");
				if (fp_out != NULL) 
				{
					fwrite (pm, sizeof(char), im, fp_out);
					fclose(fp_out);
				} else {
					fprintf(stdout, "\nERROR fopen %s", gzip_file); 
				}
				// (2) then we take the store GZIP formated file and generate the plain text output_file
				payload_inflate(gzip_file, output_file, 32 + MAX_WBITS);
#else
				fprintf(stdout, "\n\n[ERROR] GZIP not supported. Enable ZLIB_SUPPORT in local.h\n\n");
#endif
			}
			else
			{
				// Store response into output_file
				FILE *fp_out = fopen(output_file, "w");
				if (fp_out != NULL) 
				{
					fwrite (pm, sizeof(char), im, fp_out);
					fclose(fp_out);
				} else {
					fprintf(stdout, "\nERROR fopen %s", output_file); 
				}
			}
			// at this point the received PAYLOAD is in output_file
			// we do not need the allocted memory anylonger
			free(pm);
		}
		// elapsed time - measure the timelapsed between the request is sent and the response is received
		// this an optional feature
		timeval now;
		gettimeofday(&now, NULL);
		double t1= t0.tv_sec * 1000000 + t0.tv_usec;
		double t2= now.tv_sec * 1000000 + now.tv_usec;
		fprintf(stdout, "\nelapsed time %.2f ms", (t2-t1)/1000); fflush(stdout); // microsecs
	}
	// n==0 normally means the receive time out (SO_RCVTIMEO) has expired without response
	else if(n==0) {
		htmlcode[0]='\0';
		fprintf(stdout,"\n--- ZERO BYTES read");
		fflush(stdout);
	}
	else // n<0 
	{
		htmlcode[0]='\0';
		fprintf(stdout, "-- errno= %d -- %s", errno, strerror(errno));
		fflush(stdout);
	}
	// (END) RECEIVE

	close(sockfd); 	
	return 0;
} // HTTPrequest()


// ------------------------------------------------------------------------------------------------
// --------------- ZLIB        --------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

#ifndef ZLIB_SUPPORT
 const char *ZLIB_VERSION= "ZLIB support not included";
#endif

char *ZLIBversion(void)
{
	return (char*)ZLIB_VERSION;
}
#ifdef ZLIB_SUPPORT
// MANUAL
// https://zlib.net/manual.html
// CODE
// code from https://www.zlib.net/zlib_how.html - > https://www.zlib.net/zpipe.c

#define CHUNK 16384
/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */


int inf2(FILE *source, FILE *dest, int mwindowBits)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
	
	// inflate
	ret= inflateInit2(&strm, mwindowBits);
	if (ret != Z_OK)
        return ret;

	fprintf(stdout,"\n1");
    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
		fprintf(stdout,".2");

        /* run inflate() on input until output buffer not full */
        do {
			fprintf(stdout,".3");
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); 
            switch (ret) 
			{
				case Z_NEED_DICT:
					fprintf(stdout,"\n-3 case Z_NEED_DICT");
					ret = Z_DATA_ERROR; 
				case Z_DATA_ERROR:
					fprintf(stdout,"\n-3 case Z_DATA_ERROR  %s",  strm.msg);
				case Z_MEM_ERROR:
					fprintf(stdout,"\n-3 case Z_MEM_ERROR");
					(void)inflateEnd(&strm);
					return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				fprintf(stdout,"\n-3 ERROR 3");
				(void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
		fprintf(stdout,".4");

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
	fprintf(stdout,".DONE");

    // clean up and return 
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

// Inflate (decompress) the GZIP (src) file into a decompressed plain-text PAYLOAD (dst) file
void payload_inflate(const char *src, const char *dst, int mwindowBit)
{
	FILE *fp_zipin = fopen(src, "r");
	FILE *fp_de = fopen(dst, "w+");
	if (fp_zipin != NULL && fp_de != NULL) 
	{
		int ret = inf2(fp_zipin, fp_de, mwindowBit);
		if (ret != Z_OK) fprintf(stdout, "\nERROR inf error= %d", ret); 
		else fprintf(stdout, "\nDONE %s", dst); 
		fclose(fp_de);
		fclose(fp_zipin);
		
	} else {
		fprintf(stdout, "\nERROR fopen either %s OR %s", src, dst); 
	}	
}
#endif

// END OF FILE