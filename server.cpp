/** ************************************************************************************************
 *	Web Server process
 *  Part of the Wilson project (www.iambobot.com)
 *  Powered-by Etherjuice
 *  Fernando R
 *
 *  1.0.0 - May 2023
 *  1.0.1 - read loop based on Content-Length
 *  1.0.2 - Sep 2023 Project ONVIF
 *  - remove debug.h dependencies (trace2file)
 *
 ** ************************************************************************************************
**/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cstr.h"
#include "server.h"

//#include "debug.h"

//bool trace_wserver= true;
void web_server(int portno, int (* f)(char*, int, char*, size_t, int ), int tracelevel);

pid_t WEBSERVERCreate( int (*WEBSERVERCallback)(char* , int , char* , size_t, int), int tracelevel )
{
	pid_t pid_server= 0;	
	// Negative Value: creation of a child process was unsuccessful.
	// Zero: Returned to the newly created child process.
	// Positive value: Returned to parent or caller. The value contains process ID of newly created child process.
	if((pid_server = fork())< 0) 
	{
		perror("Fork failed");	
		return 0;
	}
	// FORKED PROCESS
	if (pid_server == 0)
	{	
		web_server(WEBSERVER_PORT, WEBSERVERCallback, tracelevel);
		fprintf(stdout, "\n[FATAL ERROR] ----------------  process WEB SERVER ended");
		fflush(stdout);
		return 0;
	} // (END) FORKED PROCESS
	return pid_server;	
} // WEBSERVERCreate

/* ----------------------------------------------------------------------------------------------------- */
/* --------------------------------  WEB LIB (candidates) ---------------------------------------------- */

// Sun, 17 Jan 2016 18:03:58 GMT
const char* strwday[]={"Sun", "Mon","Tue","Wed","Thu","Fri","Sat"}; 
const char* strmonth[]= {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char *time_record(char *ts, size_t max)
{
	time_t t = time(NULL); 
	struct tm *tm = localtime(&t);
	snprintf(ts, max, "%s, %d %s %d %02d:%02d:%02d", 
		(size_t)tm->tm_wday<(sizeof(strwday)/sizeof(strwday[0])) ? strwday[tm->tm_wday] : "error", 
		tm->tm_mday, 
		(size_t)tm->tm_mon<(sizeof(strmonth)/sizeof(strmonth[0]))? strmonth[tm->tm_mon] : "error", 
		2000+tm->tm_year-100, 
		tm->tm_hour, 
		tm->tm_min, 
		tm->tm_sec);
	return ts;
} // time_record()


// ------------------------------------------------------------------------------------------------
// --------------- SERVER -------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Test server
// 	curl -X POST -H "Content-Type: application/json" -d '{"type":"data_request","key":"cacaculo"}' http://192.168.1.100:8001
	
void web_server(int portno, int (* f)(char*, int, char*, size_t, int), int tracelevel)
{
	char httpdata[4096];
	char response[512];
	char line[128];
	char str[128];
	int sockfd= -1;

	struct sockaddr_in servaddr, client_addr;
	int n;

	fprintf(stdout, "\nSERVER ver. %s process running. port %d", WEBSERVER_VERSION, WEBSERVER_PORT);
	fflush(stdout);	
	
	// PROCESS LOOP
	while (1)
	{
		// SOCKET
		while(sockfd<0)
		{
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) {
				snprintf(line, sizeof(line), "\n[WEB SERVER] *** ERROR *** web_server(): socket() (%d) %s", errno, strerror(errno));
				fprintf(stdout,"%s", line);
				//trace2file(DEBUGFILE_WEBSERVER, line);
				perror("web_server:socket() failed");
				fflush(stdout);
			}
			else 
			{
				int enable = 1;
				if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
				{
					snprintf(line, sizeof(line), "\n[WEB SERVER] *** ERROR *** web_server(): setsockopt() (%d) %s", errno, strerror(errno));
					fprintf(stdout,"%s", line);
					//trace2file(DEBUGFILE_WEBSERVER, line);
					perror("web_server:setsockopt() failed");
					fflush(stdout);
					close(sockfd);
					sockfd= -1;
				}
				else
				{
					bzero((char *) &servaddr, sizeof(servaddr));
					servaddr.sin_family = AF_INET;
					servaddr.sin_addr.s_addr = INADDR_ANY;
					servaddr.sin_port = htons(portno);
					if (bind(sockfd, (struct sockaddr *) &servaddr,  sizeof(servaddr)) < 0) 
					{
						snprintf(line, sizeof(line), "\n[WEB SERVER] *** ERROR *** web_server(): bind() (%d) %s", errno, strerror(errno));
						fprintf(stdout,"%s", line);
						//trace2file(DEBUGFILE_WEBSERVER, line);
						perror("web_server:bind() failed");
						fflush(stdout);
						close(sockfd);
						sockfd= -1;
					}	
				}
			}
			if(sockfd<0) 
			{
				snprintf(line, sizeof(line), "\n[WEB SERVER] waiting sockect (%d) %s", errno, strerror(errno));
				fprintf(stdout,"%s", line);
				//trace2file(DEBUGFILE_WEBSERVER, line);
				fflush(stdout);
				sleep(5);
			}
		}
		// (END) SOCKET
		
		// ACCEPT LOOP
		while(1)
		{
			// marks the socket referred to by sockfd as a passive socket, that is, as a socket that will be used to accept incoming
			// connection requests using.
			// The backlog argument (=5) defines the maximum length to which the queue of pending connections for sockfd may grow
			if( listen(sockfd, 5) < 0)
			{
				snprintf(line, sizeof(line), "\n[WEB SERVER] *** WARNING *** listen() (%d) %s", errno, strerror(errno));
				fprintf(stdout,"%s", line);
				//trace2file(DEBUGFILE_WEBSERVER, line);
				fflush(stdout);
				// ... warning and continue
			} 
			
			// (1) Wait connection request from client
			socklen_t clilen = sizeof(client_addr);
			int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clilen);
			if (newsockfd < 0) 
			{
				snprintf(line, sizeof(line), "\n[WEB SERVER] ERROR accept()\n(%d) %s", errno, strerror(errno));
				fprintf(stdout,"%s", line);
				fflush(stdout);
				//trace2file(DEBUGFILE_WEBSERVER, line);
				close(sockfd);
				sockfd= -1;
				break;
			}
			fprintf(stdout, "\n[WEB SERVER] Connection to %s: ", inet_ntoa(client_addr.sin_addr));
			fflush(stdout);	
			
			// Set time-out before connect
			// RCV time-out 2 seconds
			struct timeval timeout;      
			timeout.tv_sec = 2; 	// 2 seconds
			timeout.tv_usec = 0;
			if (setsockopt (newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			{
				fprintf(stdout, "\n[WEB SERVER] ERROR web_server() setting timeout"); 
				perror("web_server() setsockopt() failed\n");
				fflush(stdout);
				//close(sockfd);
				//return -1;	
			}
			/*
			// SND time-out 3 seconds
			timeout.tv_sec = 3; 
			timeout.tv_usec = 0;
			if (setsockopt (newsockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			{
				fprintf(stdout, "\n[ERROR] wsSocketCreate() ");	
				perror("setsockopt() failed\n");
				fflush(stdout);
				close(sockfd);
				return -1;	
			}	
			*/
			fprintf(stdout, "\n[WEB SERVER] Awaiting messsage ... ");
			fflush(stdout);				
			// (2) RECEIVE
			bzero(httpdata, sizeof(httpdata));		
			if ((n = read(newsockfd, httpdata, sizeof(httpdata)-1)) <= 0) 
			{
				snprintf(line, sizeof(line), "\n[WEB SERVER] *** WARNING *** ERROR reading from socket\n(%d) %s", errno, strerror(errno));
				fprintf(stdout,"%s", line);
				fflush(stdout);
				//trace2file(DEBUGFILE_WEBSERVER, line);
			}
			// Show message
			else 
			{
				size_t Content_Length= atoi(HTTPHeader_Entity("Content-Length", httpdata, sizeof(httpdata), str));
				fprintf(stdout, "\n[WEB SERVER] RECEIVED Content-Length= %d bytes", Content_Length);
				//fprintf(stdout, "\n%s\n", httpdata);
				/*
				#ifdef _HADEBUG_H_
				snprintf(line, sizeof(line),"%s (%d bytes)", inet_ntoa(client_addr.sin_addr), n);
				trace2file(DEBUGFILE_WEBSERVER, line);
				#endif		
				*/				
				//-------------------------------------------------------
				// First packet
				// Process response
				int ixpl= cstr_find(httpdata, "\r\n\r\n", 0, n);
				if(ixpl)
				{
					int left= (Content_Length + ixpl + 4) -n;
					if(left>0)
					{
						// Check if there are packets left
						// get packets until getting Content_Length bytes of the payload
						int nloop=0;
						do
						{
							fprintf(stdout, "\nleft %d bytes", left);
							nloop= read(newsockfd, &httpdata[n], left);
							fprintf(stdout, "\nreceived %d bytes", nloop);
							if(nloop>0)
							{
								n += nloop;
								left -= nloop;
							}
						} while(nloop>0 && left>0);
						if(nloop<=0) fprintf(stdout, "\n[WEB SERVER] WARNING read. left= %d bytes, should be zero left", nloop);
						if(left==0) fprintf(stdout, "\n[WEB SERVER] Ok - no left to receive");
					}
				}
				httpdata[n]='\0';
				// Trace
				fprintf(stdout, "\n[WEB SERVER] RECEIVED (%d bytes):", n);
				//fprintf(stdout, "\n%s\n", httpdata);
				//cstr_dump(httpdata, n);
				fflush(stdout);

				
				// callback
				// -1 no response. dump raw request
				if( f(httpdata, (int)n, response, sizeof(response), tracelevel)<0) 
				{
					fprintf(stdout, "\n\n[WEB SERVER] NO RESPONSE\n\n");
					cstr_dump(httpdata, n);			
					fflush(stdout);		
				}
				else
				{	
					char trecord[128];		
					snprintf(httpdata, sizeof(httpdata),
					"HTTP/1.1 200 OK\r\n"
					"Date: %s GMT\r\n"					// timerecord
					"Server: %s\r\n"					// WEBSERVER_NAME
					//"X-Powered-By: PHP/5.5.9-1ubuntu4.7\r\n"
					//"Cache-Control: no-cache\r\n"
					//"Cache-Control: no-store\r\n"
					//"Access-Control-Allow-Origin: *\r\n"
					//"Vary: Accept-Encoding\r\n"
					//"Content-Type: application/json\r\n"
					"Content-Type: application/soap+xml; charset=utf-8\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n"
					"%s", 
					time_record(trecord, sizeof(trecord)), 
					WEBSERVER_NAME,
					strlen(response), 
					response);
					
					// (2) RESPONSE
					if ( (n = write(newsockfd, httpdata, strlen(httpdata))) < 0) 
					{
						snprintf(line, sizeof(line), "\n[*** WARNING ***] WEBSERVER ERROR writing to socket\n(%d) %s", errno, strerror(errno));
						fprintf(stdout,"%s", line);
						fflush(stdout);
						//trace2file(DEBUGFILE_WEBSERVER, line);						
					}
					// Show message
					if(tracelevel)
						printf("\n[WEB SERVER] RESPONSE:\n%s\n\n", httpdata);
					close(newsockfd);
				}
			}
		}
	} // While PROCESS
	close(sockfd);
} // web_server()


// END OF FILE