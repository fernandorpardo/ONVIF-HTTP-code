/** ************************************************************************************************
 *	HA - Home Assistant RESP API client 
 *  Part of the Wilson project (www.iambobot.com)
 *  Fernando R
 *
 *  see version.cpp for description
 *
 ** ************************************************************************************************
**/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/wait.h>		// kill
#include <unistd.h>
#include <string.h>
using namespace std;
#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/socket.h>
//#include "../zlib-1.2.13/zlib.h"	// ZLIB_VERSION

#include "local.h"
#include "cstr.h"
#include "glib.h"
#include "cl.h"
//#include "systemlib.h"
#include "HTTPlib.h"
#include "onvif.h"
#include "webserver.h"
#include "tapo.h"

char *version(char *str, size_t max_sz);
const char *prompt;
const char prompt_regular[]= "tapo> ";
const char prompt_ptz[]= "-- ptz> ";
bool ptzmode= false;

/**
---------------------------------------------------------------------------------------------------
		
								   WEBSERVER process

---------------------------------------------------------------------------------------------------
**/
// This code is running in WEBSERVER process

//#define WEBSERVER_KEY	"cacaculo"

// HTTP POST messages received from the browser
// "http_data" contains the full HTTP message including the header and payload
// payload starts after \r\n\r\n and is a json message
// OUTPUT:
//  In "response" buffer the payload to be attached to the HTTP response message 

int WEBSERVERCallback(char* httpdata, int n, char* response, size_t max_sz, int tracelevel)
{	
	response[0]='\0';
	int i= cstr_find(httpdata, "\r\n\r\n", 0, 0);
	if(i<0 )
	{
		fprintf(stdout, "\n[WEBSERVERCallback] No HTML");
		return -1;
	}
	XML_Dump(&httpdata[i]);
/*
	char *payload= &http_data[i+4];
	int m= strlen(payload);
	fprintf(stdout, "\n");
	cstr_dump(payload, m);
	fflush(stdout);
*/
	 // Default response
	//snprintf(response, max_sz, "");
	return strlen(response); 
} // WEBSERVERCallback
/**
---------------------------------------------------------------------------------------------------
									(END) WEBSERVER process
---------------------------------------------------------------------------------------------------
**/


// ------------------------------------------------------------------------------------------------
//										CLI
// ------------------------------------------------------------------------------------------------


char storedresponse[8*1024];
size_t storedresponse_length;

void ManageResponse(int n)
{
	if(n<0)
	{
		fprintf(stdout,"\nERROR HTTPrequest %d. The camera at %s is not connected", n, DEVICE_IP);
		return;
	}		
	storedresponse_length= 0;
	memset(storedresponse, 0, sizeof(storedresponse));
	// Response is in JSON file. Dump file content
	char json_file[128];
	snprintf(json_file, sizeof(json_file), "%s%s", HTTPFILESTORAGE, HTTP_OUTPUT_FILE_EXT);
	int fsz= file_size(json_file);
	unsigned int ufsz= fsz>=0? (unsigned int) fsz : 0;
	char *memptr= (char *) malloc(sizeof(char) * (ufsz + 1024));
	if(memptr)
	{	
		int n;
		if( (n=fileRead(json_file, memptr, ufsz)) > 0)
		{
			//fprintf(stdout,"\nfileRead n= %d / sz= %d", n, ufsz);
			//cstr_dump(memptr, fsz);
			fprintf(stdout,"\n");
			for(unsigned int i=0; i<ufsz; i++) fprintf(stdout, "%c", memptr[i]);
			fprintf(stdout,"\n\n");
			storedresponse_length= (ufsz >= (int)sizeof(storedresponse))? sizeof(storedresponse)-1 : ufsz;
			memcpy(storedresponse, memptr, storedresponse_length);
			storedresponse[storedresponse_length]='\0';
			fflush(stdout);
		}
		free(memptr);
	}
}

struct
{
	char name[32];
	char desscription[128];
} commanlist[]= {
		  {"usage/help/?", 		"this help"}
		, {"version", 			"display SW versions"}
		, {"time", 				"current time UTC"}
		, {"ip", 				"show/change theIP address of target camera"}
		, {"something", 		"send a 'something' request message (for testing)"}
		, {"dump", 				"dump payload of latest received message"}

		// Device info
		, {"info", 				"GetDeviceInformation"}
		, {"systemdateandtime", "GetSystemDateAndTime"}
		, {"capabilities", 		"GetCapabilities"}
		
		// EVENTS
		, {"eventproperties", 	"GetEventProperties"}
		// Basic Notification Interface
		, {"subscribe", 		"SubscribeRequest"}
		, {"renew", 			"RenewRequest"}
		, {"unsubscribe", 		"UnSubscribeRequest"}
		, {"currentmessage", 	"GetCurrentMessage"}
		
		// PTZ
		, {"ptz", 				"toggle PTZ mode, keyboard arrow keys perform PAN & TILT"}
		, {"nodes", 			"GetNodes"}
		, {"node", 				"GetNode"}
		, {"config", 			"GetConfigurations"}
		, {"amove", 			"AbsoluteMove"}
		, {"move", 				"ContinuousMove"}
		, {"stop", 				"Stop"}
		, {"status", 			"GetStatus"}
		// Preset operations 
		, {"getpresets", 		"GetPresets"}
		, {"preset", 			"GotoPreset go to preset position"}
		//	Home Position operations 
		, {"gohome", 			"GotoHomePosition"}
		, {"sethome", 			"SetHomePosition"}
		// Capabilities
		, {"servicecap", 		"GetServiceCapabilities"}
		// Auxiliary Operations
		, {"aux", 				"SendAuxiliaryCommand"}
		, {"", ""}
	};

void CLI_usage()
{
    fprintf(stdout, "\n\nUsage:\n");
	for(int i=0; strlen(commanlist[i].name); i++)
	{
		for(int j=0; j<4; j++) fprintf(stdout, " ");
		fprintf(stdout, "%s", commanlist[i].name);
		for(int j=strlen(commanlist[i].name); j<20; j++) fprintf(stdout, " ");
		fprintf(stdout, "- %s\n", commanlist[i].desscription);
	}
}

int CLI_Interpreter(char argv[8][32], size_t argn)
{
	if(argn<1) return -1;
	if ((strcmp(argv[0], "usage")==0) || (strcmp(argv[0], "help")==0) || (strcmp(argv[0], "?")==0))
	{
		CLI_usage();
	}
	// version
	else if (strcmp(argv[0], "version")==0)
	{
		char str[128];	
		fprintf(stdout,"\n%s\nversion %s", APPNAME, version(str, sizeof(str))); 
		fprintf(stdout,"\nHTTPlib %s", HTTPLIB_VERSION ); 
		fprintf(stdout,"\nZLIB %s", ZLIBversion() ); 
		fprintf(stdout,"\n");
	}
	else if (strcmp(argv[0], "time")==0)
	{	
		char ts[32];
		if(argn>1) timeISO8601(ts, sizeof(ts), (char*)argv[1]);
		else timeISO8601(ts, sizeof(ts), (char*)"");
		fprintf(stdout,"\n%s\n", ts); 	
	}
	
	else if (strcmp(argv[0], "ip")==0)
	{	
		if(argn>1) 
		{
			strcpy(DEVICE_IP, argv[1]);
			fprintf(stdout,"\nNEW DEVICE_IP %s", DEVICE_IP ); 
		}
		else fprintf(stdout,"\nDEVICE_IP %s", DEVICE_IP ); 
	}
	else if (strcmp(argv[0], "dump")==0 || strcmp(argv[0], "d")==0 )
	{	
		XML_Dump(storedresponse);
	}
	else if (strcmp(argv[0], "something")==0)
	{
		if(argn>1) 
		{
			GetSometing(argv[1]);
			AddHTTPHeader("onvif/service");
			fprintf(stdout,"\n%s\n", mdata);
			int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
			ManageResponse(n);
		}
	}

	// Device info
	else if (strcmp(argv[0], "info")==0)
	{
		GetDeviceInformation();
		AddHTTPHeader("onvif/service");
//		fprintf(stdout,"\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}		
	else if (strcmp(argv[0], "systemdateandtime")==0)
	{
		GetSystemDateAndTime();
		fprintf(stdout,"\nxml\n%s\n", mdata);
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n+header\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);
		ManageResponse(n);
	}	
	else if (strcmp(argv[0], "capabilities")==0)
	{
		GetCapabilities();
		fprintf(stdout,"\nxml\n%s\n", mdata);
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n+header\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}		
	
	// ----------------------------------------------------
	// EVENTS
	else if (strcmp(argv[0], "eventproperties")==0)
	{
		GetEventProperties();
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	// Basic Notification Interface
	else if (strcmp(argv[0], "subscribe")==0)
	{
		SubscribeRequest();
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	else if (strcmp(argv[0], "renew")==0)
	{
		RenewRequest();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);
		ManageResponse(n);
	}	
	else if (strcmp(argv[0], "unsubscribe")==0)
	{
		UnSubscribeRequest();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	else if (strcmp(argv[0], "currentmessage")==0)
	{
		GetCurrentMessage();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}		
	
	// ----------------------------------------------------
	// PTZ
	else if (strcmp(argv[0], "ptz")==0)
	{
		ptzmode = ! ptzmode;
		fprintf(stdout, "\nPTZ mode is %s", ptzmode?"ON":"OFF");
		prompt= ptzmode ? prompt_ptz : prompt_regular;
	}
	else if (strcmp(argv[0], "nodes")==0)
	{
		PTZ_GetNodes();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	else if (strcmp(argv[0], "node")==0)
	{
		PTZ_GetNode("PTZNODETOKEN");
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	else if (strcmp(argv[0], "config")==0)
	{
		PTZ_GetConfigurations();
		AddHTTPHeader("onvif/service");
		fprintf(stdout,"\n%s\n", mdata);
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	
	// ----------------------------------------------------
	// 5.3 Move Operations
	// 5.3.1 AbsoluteMove
	else if (strcmp(argv[0], "amove")==0)
	{		
		if(argn>2) 
		{
			float x= atof(argv[1]);
			float y= atof(argv[2]);

			if(argn>3) PTZ_AbsoluteMove(x, y, atof(argv[3]));
			else PTZ_AbsoluteMove(x, y);
			AddHTTPHeader("onvif/service");
			int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
			ManageResponse(n);
		}		
	}	
	// 5.3.3 ContinuousMove
	else if (strcmp(argv[0], "move")==0)
	{
		PTZ_ContinuousMove(-1, 0);
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	//	5.3.5 Stop
	else if (strcmp(argv[0], "stop")==0)
	{
		PTZ_Stop();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	// 5.3.6 GetStatus 
	else if (strcmp(argv[0], "status")==0)
	{
		PTZ_GetStatus();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	
	// ----------------------------------------------------
	// 	5.4 Preset operations 
	else if (strcmp(argv[0], "getpresets")==0)
	{
		PTZ_GetPresets();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}	
	else if (strcmp(argv[0], "preset")==0)
	{
		PTZ_GotoPreset((argn>1) ? atoi(argv[1]) : 1);
		AddHTTPHeader("onvif/service");
		HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);		
	}
	// ----------------------------------------------------
	// 5.5 Home Position operations
	// 5.5.1 GotoHomePosition
	else if (strcmp(argv[0], "gohome")==0)
	{
		PTZ_GotoHomePosition();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	// 5.5.2 SetHomePosition 
	else if (strcmp(argv[0], "sethome")==0)
	{
		PTZ_SetHomePosition();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	// 5.10 GetServiceCapabilities
	else if (strcmp(argv[0], "servicecap")==0)
	{
		PTZ_GetServiceCapabilities();
		AddHTTPHeader("onvif/service");
		int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
		ManageResponse(n);
	}
	// Auxiliary Operations
	else if (strcmp(argv[0], "aux")==0)
	{
		if(argn>2) 
		{		
			PTZ_SendAuxiliaryCommand(argv[1], argv[2]);
			AddHTTPHeader("onvif/service");
			int n= HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 1);	
			ManageResponse(n);
		}
	}		
	
	else {
		return -1;
	}
	return 0;
} // CLI_Interpreter()




/* ----------------------------------------------------------------------------------------------------- */
/* -----------------------------------------------  MAIN  ---------------------------------------------- */

void SHELL_usage()
{
    fprintf(stdout, "\nUsage:\n");
	fprintf(stdout, "    -?       - this help\n"); 
	fprintf(stdout, "    -h       - this help\n"); 
	fprintf(stdout, "    --help   - this help\n"); 
	fprintf(stdout, "    --info   - control module versions\n"); 
	fprintf(stdout, "    -a       - agent mode: no termio\n"); 
	fprintf(stdout, "    web      - agent mode: no termio\n"); 
}

int main(int argc, char *argv[])
{
	bool option_process= false;
	size_t n_numbers= 0;
	int numbers[10];
	char str[128]; // de uso general
	ptzmode= false;

	fprintf(stdout,"\n%s\nversion %s", APPNAME, version(str, sizeof(str))); 
	fprintf(stdout,"\nHTTPlib %s", HTTPLIB_VERSION ); 
	fprintf(stdout,"\nZLIB %s", ZLIBversion() ); 
	fprintf(stdout,"\n"); 
	
	prompt= prompt_regular;

	// (1) Command line options
	if (argc>1)
	for(int i=1; i<argc; i++)
	{
		if(argv[i][0]=='-') 
		{
			// ha -[switch]
			if(argv[i][1]!='-')
			switch(argv[i][1])
			{
				case '?':
				case 'h':
						SHELL_usage();
						exit(EXIT_SUCCESS);
					break;
				case 'a':
						option_process= true;
						fprintf(stdout,"\noption: agent"); 
						fflush(stdout);
					break;
				default:
					break;				
			}
			else
			{
				if(strcmp(&argv[i][2], "info")==0) 
				{

				}
			}
		}
		// ha [command]
		else
		{
			if(strcmp(argv[i], "web")==0) 
			{
				option_process= true;
				fprintf(stdout,"\noption: agent");  
				fflush(stdout);
			}
			
		}
	}	
	
	if(argc > 1) 
	{
		for(int i=1; i<argc; i++)
		{	
			if(isNumber((char*)argv[i]))
			{
				if(n_numbers < (sizeof(numbers)/sizeof(size_t))) 
				numbers[n_numbers++]= atoi(argv[i]);
			}
		}
	}
	
	
	if(option_process) {
		sleep(10);
	}

	// MAC address
	//	fprintf(stdout, "\nMAC WLAN0= %s", get_MAC(str, sizeof(str))); 
	// IP address
//	char IP[32];
//	fprintf(stdout, "\nIP %s", get_IP(IP, sizeof(IP))); 	
//	fflush(stdout);
	




	// loop init
//	loop_init();
	

	strcpy(DEVICE_IP, DEFAULT_DEVICE_IP);
	pid_t pid_server= 0;
	
	if((pid_server=WEBSERVERCreate(WEBSERVERCallback, 1)) == 0)
	{
		fprintf(stdout, "\n[ERROR] WEB SERVER creation failed");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	else
	{
		// ---- Command line
		char line[200] = {0};
		int ic=0;
		char c= 0;

		// Repeat command
		CommandLineBuff* CLB = new CommandLineBuff(line, prompt);
		if(!option_process) 
		{
			termios_init();
		}

		int terminate= false;
		char aaa[8][32]; // 8 strings of 32 chars

		while (!terminate)
		{
			while(option_process || !kbhit()) 
			{
				//loop(shd);
			}
					
			switch(c = getchar()) 
			{
				case '\n': 
				{
					size_t argn= cParser(line, aaa);
					if(argn>0) 
					{
						if(CLI_Interpreter(aaa, argn) < 0)
						{
							fprintf(stdout,"\nUnknown command %s", aaa[0]);
						}
						// Store command
						CLB->Store();
					}
					ic=0;
					line[0]=0;						
					fprintf(stdout, "\n%s", prompt);
					fflush(stdout);
				}
				break;
				case '\b':
				case 127:
				{
					if(ic>0) 
					{
						printf("\b \b"); // Backspace
						line[--ic]= 0;
					}
				}
				break;
				// ESCAPE
				case 27:
				{
					// "throw away" next two characters which specify escape sequence
					char c1=0;
					char c2=0;
					if(kbhit()) 
					{
						c1 = getchar();
						if(kbhit()) c2 = getchar();
					}
					switch(c2) 
					{
						// Simple ESC key
						case 0:
							// Terminate
							printf("\nPROGRAM TERMINATED BY USER!\n");
							termios_restore();
							terminate= true;
						break;
						// UP
						case 65:
						// DOWN
						case 66:
							if(ptzmode)
							{
								PTZ_RelativeMove(0.0, c2==65? 0.05: -0.05);
								AddHTTPHeader("onvif/service");
								HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 0);
							}
							else
							{
								ic= CLB->Last();
								// UP
								if(c2==65)
								{
									CLB->Up();
								}
								// DOWN
								if(c2==66)
								{
									CLB->Down();
								}
							}							
						break;
						case 67:
							if(ptzmode)
							{
								PTZ_RelativeMove(0.05, 0.0);
								AddHTTPHeader("onvif/service");
								HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 0);
							}
							else
							{
								cout << "RIGHT" << endl; // key right
								printf (prompt);
							}
						break;

						case 68:
							if(ptzmode)
							{
								PTZ_RelativeMove(-0.05, 0);
								AddHTTPHeader("onvif/service");
								HTTPrequest(HTTPFILESTORAGE, DEVICE_IP, ONVIF_DEVICE_PORT, mdata, 0);		
							}
							else
							{
								cout << "LEFT" << endl; // key left
								printf (prompt);
							}							
						break;
						default: 
							printf("DEFAULT %d\n", (int)c1); 
							printf (prompt);
					}
				}
				break;			
				default: 
				{
					printf("%c", c); // Echo
					line[ic++]= c;
					line[ic]= 0;
				}
			}
		} // while
	}
	
	// TERMINATE
	kill(pid_server, SIGKILL);
	fprintf(stdout,"\nDONE");
	fflush(stdout);
	return EXIT_SUCCESS;
}

// END OF FILE