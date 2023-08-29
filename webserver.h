#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#define WEBSERVER_VERSION	"1.0.1"
#define DEBUGFILE_WEBSERVER "/var/www/ramdisk/debug_onvif.log"
#define WEBSERVER_PORT 		8002
#define WEBSERVER_NAME		"ONVIFWS/1.0" // Server: Apache/2.4.7 (Ubuntu)
pid_t WEBSERVERCreate( int (*f)(char* , int , char* , size_t , int ), int tracelevel);

#endif
/* END OF FILE */