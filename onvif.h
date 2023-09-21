#ifndef _ONVIF_H_
#define _ONVIF_H_

#define DEVICE_ACCOUNT_USERNAME	"your_username"
#define	DEVICE_ACCOUNT_PASSWORD "your_password"

#define ONVIF_SERVICE 			"onvif/service"
#define DEFAULT_DEVICE_IP 		"192.168.1.127"
#define ONVIF_DEVICE_PORT 		"2020" 							// HTTP 80 and 8080 returns "Connection refused"
#define USER_AGENT				"ONVIFClient"
#define NOTIFICATIONS_ENDPOINT	"http://192.168.1.100:8002"		// Webserver process



class ONVIFclient
{
	public:
		ONVIFclient(void);
		~ONVIFclient(void);
};

extern char mdata[4096];
extern char DEVICE_IP[64];

//int GenerateHTML(char *html, size_t size_of_html, char *data);
//int AddHTTPHeader2(char *html, size_t size_of_html, const char * service, char *data);
char* AddHTTPHeader(const char * service);
char *timeISO8601(char *ts, size_t sz_max, char *offset);
//char *GenerateUsernameToken(char *str, size_t sz_max);



// ------------------------------------------------------------------------------------------------
// ONVIF methods
// ------------------------------------------------------------------------------------------------
// First Actions After Discovery
char* GetDeviceInformation(void);
char* GetSystemDateAndTime(void);
char* GetCapabilities(void);
// ------------------------------------------------------------------------------------------------
// Events
char* GetEventProperties(void);
// Basic Notification Interface
char* SubscribeRequest(void);
char* RenewRequest(void);
char* UnSubscribeRequest(void);
char* GetCurrentMessage(void);
// --------------- PTZ     ---------------------------------------------------------------------
// PTZ Service Specification - https://www.onvif.org/specs/srv/ptz/ONVIF-PTZ-Service-Spec-v1712.pdf
// 5.1 PTZ Node
char* PTZ_GetNodes(void);
char* PTZ_GetNode(const char*);
// 5.2 PTZ Configuration 
char* PTZ_GetConfigurations(void);
// 5.3 Move Operations
char* PTZ_AbsoluteMove(float x, float y, float speed=0.5);
char* PTZ_RelativeMove(float x, float y);
char* PTZ_ContinuousMove(int x, int y);
char* PTZ_Stop(void);
char* PTZ_GetStatus(void);
// 	5.4 Preset operations 
char* PTZ_GetPresets(void);
char* PTZ_GotoPreset(int preset);
// 5.5 Home Position operations 
char* PTZ_GotoHomePosition(void);
char* PTZ_SetHomePosition(void);
// 5.10
char* PTZ_GetServiceCapabilities(void);

// Aux
char* PTZ_SendAuxiliaryCommand(char *command, char *value);

// others
char* GetSometing(char *message);

#endif