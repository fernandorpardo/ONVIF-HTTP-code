/** ************************************************************************************************
 *	ONVIF library  
 *
 *  This is an iambobot.com project
 *  Fernando R
 *
 * 	1.0.0 - May 2023
 *		- Device information and capabilities
 * 		- Events 
 *			this implementation comprises the Basic Notification Interface (WS-BaseNotification) 
 *			and the notification stream management
 *			Real-time Pull-Point Notification Interface is not implemented
 *		- PTZ
 *
 *
 ** ************************************************************************************************
**/
#include <stdlib.h>
#include <iostream>			// time / localtime
#include <string.h>			// strlen
#include <sys/time.h>		// gettimeofday
#include <openssl/sha.h> 	// needed for OpenSSL SHA1

#include "onvif.h"

char DEVICE_IP[64];
char mdata[4096];
char UsernameToken[512];


char *timenowISO8601(char *ts, size_t sz_max);
char *GenerateUsernameToken(void);


ONVIFclient::ONVIFclient(void)
{
	fprintf(stdout, "\nONVIFclient created"); 
	fflush(stdout);	
}
ONVIFclient::~ONVIFclient(void)
{
	fprintf(stdout, "\nOBJECT ONVIFClient destroyed"); 
	fflush(stdout);
}


// ------------------------------------------------------------------------------------------------
// --------------- HTTP  HEADER   -----------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
char* AddHTTPHeader(const char * service)
{
	char header[512];
	snprintf(header, sizeof(header),
		"POST /%s HTTP/1.1\r\n"			// service
		"Host: %s\r\n"					// DEVICE_IP 
		"User-Agent: %s\r\n"			// USER_AGENT
//		"Accept-Encoding: gzip, deflate\r\n"
		"Content-Type: application/soap+xml; charset=utf-8\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		, service
		, DEVICE_IP
		, USER_AGENT
		, strlen(mdata)
		);
	size_t szpayload=  strlen(mdata);
	if( (strlen(header) + szpayload) < sizeof(mdata) )
	{
		// shift payload to leave room to the header
		for(size_t i=0; i<=szpayload; i++) mdata[strlen(header) + szpayload - 1 - i] = mdata[szpayload - 1- i];
		// copy header
		for(size_t i=0; i<strlen(header); i++) mdata[i]= header[i];
		mdata[strlen(header) + szpayload]= '\0';
	}
	else mdata[0]= '\0';
	return mdata;
} // AddHTTPHeader


/**
	DOCUMENTATION REFERENCES:

	[1] ONVIF Core Specification
		https://www.onvif.org/specs/core/ONVIF-Core-Specification.pdf
	[2] ONVIF™ Event Handling Test Specification
		https://www.onvif.org/wp-content/uploads/2018/07/ONVIF_Event_Handling_Test_Specification_18.06.pdf
	[3] ONVIF™  Application Programmer's Guide
		https://www.onvif.org/wp-content/uploads/2016/12/ONVIF_WG-APG-Application_Programmers_Guide-1.pdf
	[4] OASIS Web Services Base Notification
		http://docs.oasis-open.org/wsn/wsn-ws_base_notification-1.3-spec-os.pdf
	[5] Web Services Base Notification
		http://xml.coverpages.org/WS-BaseNotification.pdf
**/

/**
	-----------------------------------------------------------------------------------------------
	---------------  First Actions After Discovery   ----------------------------------------------
	-----------------------------------------------------------------------------------------------

	[3] ONVIF™  Application Programmer's Guide
		https://www.onvif.org/wp-content/uploads/2016/12/ONVIF_WG-APG-Application_Programmers_Guide-1.pdf
	
	5.1 First Actions After Discovery
		After ONVIF devices are discovered using WS-Discovery, you would typically access a device
		using the supplied XAddrs to test where it is reachable. Use device.GetSystemDateAndTime
		to accomplish this because it should not require authentication. You can also consider calling
		device.GetDeviceInformation and device.GetCapabilities.
**/

// status: ok
char* GetDeviceInformation(void)
{
	snprintf(mdata, sizeof(mdata),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<s:Header>"
				"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
			"</s:Header>"
			"<s:Body><tds:GetDeviceInformation/></s:Body>"
		"</s:Envelope>"
		, GenerateUsernameToken()
		);
	return mdata;
} // GetDeviceInformation

// status: ok
char* GetCapabilities(void)
{
	snprintf(mdata, sizeof(mdata),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<s:Header>"
//				"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
//				"%s" //"<UsernameToken>"
//				"</Security>"
			"</s:Header>"		
			"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
				"<GetCapabilities xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
					"<Category>All</Category>"
				"</GetCapabilities>"
			"</s:Body>"
		"</s:Envelope>"
//		, GenerateUsernameToken()
		);
	return mdata;
} // GetCapabilities

// status: ok
char* GetSystemDateAndTime(void)
{
	snprintf(mdata, sizeof(mdata),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<s:Header>"
//				"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
//				"%s" //"<UsernameToken>"
//				"</Security>"
			"</s:Header>"		
			"<s:Body>"
				"<tds:GetSystemDateAndTime/>"
			"</s:Body>"
		"</s:Envelope>"
//		, GenerateUsernameToken()
		);
	return mdata;
} // GetSystemDateAndTime

// Function for generic messages
// Example: GetCapabilities GetServiceCapabilities
char* GetSometing(char *message)
{
	snprintf(mdata, sizeof(mdata),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<s:Header>"
				"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
			"</s:Header>"
			"<s:Body><tds:%s/></s:Body>"
		"</s:Envelope>"
		, GenerateUsernameToken()
		, message
		);
	return mdata;
} // GetSometing



/**
	-----------------------------------------------------------------------------------------------
	---------------  EVENTS  ----------------------------------------------------------------------
	-----------------------------------------------------------------------------------------------
	
	[3] ONVIF™  Application Programmer's Guide
		https://www.onvif.org/wp-content/uploads/2016/12/ONVIF_WG-APG-Application_Programmers_Guide-1.pdf
		
	9 Eventing
	
	The ONVIF specification includes three different types of event notifications: 
	- Real-time Pull-Point Notification Interface 
	- Basic Notification Interface (WS-BaseNotification) 
	- Notification Streaming Interface (metadata streaming)
	
	The GetEventProperties action, which is a way of finding out what notifications 
	a device supports and what information they contain.
	
**/


//	GetEventProperties
//	C100 and C320WS response: Method 'tds:GetEventProperties' not implemented: method name or namespace not recognized
//	C500 is ok
char* GetEventProperties(void)
{
	snprintf(mdata, sizeof(mdata),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<SOAP-ENV:Envelope "
					"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
					"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
					//"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
					"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
					//"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\""
					">"
			"<SOAP-ENV:Header>"
				"<wsa5:Action SOAP-ENV:shallUnderstand=\"true\">"
					"https://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"
				"</wsa5:Action>"
				"<Security SOAP-ENV:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
			"</SOAP-ENV:Header>"
			"<SOAP-ENV:Body>"
				"<tds:GetEventProperties/>"
			"</SOAP-ENV:Body>"
		"</SOAP-ENV:Envelope>"
		, GenerateUsernameToken()
		);
	return mdata;
} // GetEventProperties


/**
	-----------------------------------------------------------------------------------------------
	---------------  Basic Notification Interface  [WS-BaseNotification] specification  -----------
	-----------------------------------------------------------------------------------------------	
		
	[1] ONVIF Core Specification
		https://www.onvif.org/specs/core/ONVIF-Core-Specification.pdf
	
	9.3 Basic Notification Interface
	
	The Base Subscription Manager Interface of the [WS-BaseNotification] is mandatory. The specification 
	consists of the Renew and Unsubscribe operations (methods).
	The implementation of the Pull-Point Interface of the [WS-BaseNotification] on a device is 
	optional (9.3.2 Requirements).


	Subscribe request instantiates a Subscription Manager (the Subscription Manager is created at the device (CAMERA) side).
	Subscription Manager is terminated  either by calling unsubscribe or through a timeout.

	This SW	<--------	|	--------------------> The camera 
	
	------------            -----------------
	| Client   | 			| Event Service |
    -----------				----------------
		|							|
		|	SubscriptionRequest		|
		| ------------------------> |          Instantiate
		|							| ------------------------>	 -----------------------
		|	SubscriptionResponse	|							| Subscription Manager |
		| <------------------------ |							 -----------------------
		|							|										|
		|		Notify				|										|
		| <------------------------ |										|
		|							 										|
		|	                    RenewRequest								|		
		| --------------------------------------------------------------->	|
		|	                    RenewResponse								|					
		| <---------------------------------------------------------------	|
		|							 										|
		|	                    UnsubscribeRequest							|
		| --------------------------------------------------------------->	|
		|	                    UnsubscribeResponse							|
		| <---------------------------------------------------------------	|
	
**/	

// status: ok (it works)
char* SubscribeRequest(void)
{
	char ts[32];
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"		
		"<SOAP-ENV:Envelope "
			"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
			"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
			"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
			"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
			"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\""
			">"
			"<SOAP-ENV:Header>"
				"<wsa5:Action SOAP-ENV:shallUnderstand=\"true\">"
					"http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeRequest"
				"</wsa5:Action>"
				//"<MessageID xmlns=\"http://www.w3.org/2005/08/addressing\">urn:uuid:9f2a12de-3a76-461b-a421-e472517bcc7e</MessageID>"
				"<wsa5:ReplyTo>"
					"<wsa5::Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
				"</wsa5:ReplyTo>"
				"<Security SOAP-ENV:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
				"<wsa5:To SOAP-ENV:shallUnderstand=\"1\">http://%s/onvif/service</wsa5:To>" // DEVICE_IP
			"</SOAP-ENV:Header>"
			"<SOAP-ENV:Body>"
				"<wsnt:Subscribe>"
					"<wsnt:ConsumerReference>"
						"<wsa5:Address>%s</wsa5:Address>"	// NOTIFICATIONS_ENDPOINT
					"</wsnt:ConsumerReference>"
					"<wsnt:InitialTerminationTime>"
						"%s" // timeISO8601 ( funciona, 2023-05-15T20:07:22Z) creo que falla con .000Z
					"</wsnt:InitialTerminationTime>" 
				"</wsnt:Subscribe>"
			"</SOAP-ENV:Body>"
		"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()	
	, DEVICE_IP
	, NOTIFICATIONS_ENDPOINT
	, timeISO8601(ts, sizeof(ts), (char*)"00:00:30:00") // subscribe for 30 minutes
 	);
	return mdata;
} // SubscribeRequest()

// status: does not work
char* RenewRequest(void)
{
	//char ts[32];
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"		
		"<SOAP-ENV:Envelope "
					"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
					//"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
					"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
					"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
					//"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
					">"
			"<SOAP-ENV:Header>"
				"<Security SOAP-ENV:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
				"<wsa5:Action SOAP-ENV:shallUnderstand=\"true\">http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewRequest</wsa5:Action>"
				"<wsa5:MessageID>urn:uuid:5fb7e924-4429-493b-88d2-b2b124debbb3</wsa5:MessageID>"
				"<wsa5:ReplyTo>"
					"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
				"</wsa5:ReplyTo>"
				"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>" // DEVICE_IP
			
			"</SOAP-ENV:Header>"
			"<SOAP-ENV:Body xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" >"
				"<Renew>"
					"<TerminationTime>P1D</TerminationTime>"
					//"<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
				"</Renew>"
			"</SOAP-ENV:Body>"
		"</SOAP-ENV:Envelope>"	
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	//, NOTIFICATIONS_ENDPOINT
	//, timeISO8601(ts, sizeof(ts), (char*)"00:00:30:00") // subscribe fro 30 minutes
 	);
	return mdata;	
} // RenewRequest()

// Unsubscribe
// status: does not work
char* UnSubscribeRequest(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"		
		"<SOAP-ENV:Envelope "
					"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
					"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
					"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
					"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
					"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\""
					">"
			"<SOAP-ENV:Header>"
				"<wsa5:Action SOAP-ENV:shallUnderstand=\"1\">"
					"http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeRequest"
//					"http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/UnsubscribeRequest"
				"</wsa5:Action>"
				"<Security SOAP-ENV:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
				//"<wsa:To>http://192.168.1.100:8002</wsa:To>"
				"<wsa5:To SOAP-ENV:shallUnderstand=\"1\">http://%s/onvif/service</wsa5:To>" // DEVICE_IP
			"</SOAP-ENV:Header>"
			"<SOAP-ENV:Body>"
				"<wsnt:Unsubscribe>"
				"</wsnt:Unsubscribe>"
			"</SOAP-ENV:Body>"
		"</SOAP-ENV:Envelope>"	
	, GenerateUsernameToken()
	, DEVICE_IP
	//, NOTIFICATIONS_ENDPOINT
 	);
	return mdata;
} // UnSubscribeRequest()

// status: no error / no message in the response
char* GetCurrentMessage(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"		
		"<SOAP-ENV:Envelope "
					"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
					"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
					"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
					"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
					"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\""
					">"
			"<SOAP-ENV:Header>"
				"<wsa5:Action SOAP-ENV:shallUnderstand=\"1\">"
					"http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/GetCurrentMessage"
				"</wsa5:Action>"
				"<Security SOAP-ENV:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
				"</Security>"
				"<wsa5:To SOAP-ENV:shallUnderstand=\"1\">http://%s/onvif/service</wsa5:To>" // DEVICE_IP
			"</SOAP-ENV:Header>"
			"<SOAP-ENV:Body>"
				"<wsnt:GetCurrentMessage>"
					"<wsnt:ConsumerReference>"
						"<wsa5:Address>%s</wsa5:Address>"	// NOTIFICATIONS_ENDPOINT
					"</wsnt:ConsumerReference>"
				"</wsnt:GetCurrentMessage>"
			"</SOAP-ENV:Body>"
		"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, NOTIFICATIONS_ENDPOINT
 	);
	return mdata;
} // GetCurrentMessage()

/**
	-----------------------------------------------------------------------------------------------
	---------------    PTZ     --------------------------------------------------------------------
	-----------------------------------------------------------------------------------------------
	
	PTZ Service Specification
	https://www.onvif.org/specs/srv/ptz/ONVIF-PTZ-Service-Spec-v1712.pdf
	
	5 Service
	5.1 PTZ Node
		5.1.2 GetNodes 	....................	ok
		5.1.3 GetNode							NOK
	5.2 PTZ Configuration 
		5.2.2 GetConfigurations  ...........	ok 
		5.2.3 GetConfiguration
		5.2.4 GetConfigurationOptions 
		5.2.5 SetConfiguration 
		5.2.6 GetCompatibleConfigurations 
	5.3 Move Operations
		5.3.1 AbsoluteMove  ................	ok
		5.3.2 RelativeMove  ................	ok
		5.3.3 ContinuousMove  ..............	ok
		5.3.4 GeoMove 
		5.3.5 Stop  ........................	ok
		5.3.6 GetStatus 						NOK
	5.4 Preset operations 
		5.4.1 SetPreset 
		5.4.2 GetPresets  ..................	ok
		5.4.3 GotoPreset  ..................	ok
		5.4.4 RemovePres et
	5.5 Home Position operations 
		5.5.1 GotoHomePosition   				NOK
		5.5.2 SetHomePosition 					NOK
	5.6 Auxiliary operations
	5.7 Pre defined PTZ Spaces
	5.8 Preset Tour Operations
	5.9 PT Control Direction Configuration
	5.10 GetServiceCapabilities  ...........	ok
	 
	 
	Also
	 // https://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl

**/

//	--------------- 5.1 PTZ Node
// GetNodes
// status: OK
char* PTZ_GetNodes(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetNodes</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:0d4f802b-4b36-4923-8c9f-b0b7c27371ee</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body>"
			"<GetNodes/>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_GetNodes()

// GetNode
// Description: Get a specific PTZ Node identified by a reference token or a name. 
// status: 	Fails
// 			No such PTZNode on the device
char* PTZ_GetNode(const char *node)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
//			"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetNodes</wsa5:Action>"
//			"<wsa5:MessageID>urn:uuid:0d4f802b-4b36-4923-8c9f-b0b7c27371ee</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body>"
			"<GetNode xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ReferenceToken>%s</ReferenceToken>"
			"</GetNode>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	, node
 	);
	return mdata;
} // PTZ_GetNode()

//	--------------- 5.2 PTZ Configuration 
// GetConfigurations
// status: OK
char* PTZ_GetConfigurations(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove</wsa5:Action>"
			"<wsa5:MessageID>urn:uuid:0d4f802b-4b36-4923-8c9f-b0b7c27371ee</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body>"
			"<GetConfigurations/>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_GetConfigurations()


//	--------------- 5.3 Move Operations
// 5.3.1 AbsoluteMove
// status: Ok
char* PTZ_AbsoluteMove(float x, float y, float speed)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "	
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"	
		"<SOAP-ENV:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
	//		"<wsa5:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/AbsoluteMove</wsa5:Action>"
	//		"<wsa5:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<AbsoluteMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<Position>"
					"<PanTilt x=\"%f\" y=\"%f\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace\"/>"
				"</Position>"
				"<Speed>%f</Speed>"
			"</AbsoluteMove>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	,x , y
	, speed
 	);
	return mdata;
} // PTZ_AbsoluteMove()

// 5.3.2 RelativeMove
// status: Ok
char* PTZ_RelativeMove(float x, float y)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "	
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"	
		"<SOAP-ENV:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/RelativeMove</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<RelativeMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<Translation>"
					"<PanTilt x=\"%f\" y=\"%f\" xmlns=\"http://www.onvif.org/ver10/schema\"/>"
				"</Translation>"
				"<Speed>.01</Speed>"
			"</RelativeMove>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	,x , y
 	);
	return mdata;
} // PTZ_RelativeMove()

// 5.3.3 ContinuousMove
// status: Ok
char* PTZ_ContinuousMove(int x, int y)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove</wsa5:Action>"
			"<wsa5:MessageID>urn:uuid:0d4f802b-4b36-4923-8c9f-b0b7c27371ee</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<ContinuousMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<Velocity>"
					"<PanTilt x=\"%d\" y=\"%d\" xmlns=\"http://www.onvif.org/ver10/schema\"/>"
				"</Velocity>"
				"<Timeout>PT1S</Timeout>"
			"</ContinuousMove>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	, x
	, y
 	);
	return mdata;
} // PTZ_ContinuousMove()

//	5.3.5 Stop 
// status: Ok
char* PTZ_Stop(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/Stop</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:5fb7e924-4429-493b-88d2-b2b124debbb3</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<Stop xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<PanTilt>true</PanTilt>"
				"<Zoom>true</Zoom>"
			"</Stop>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_Stop()

// 5.3.6 GetStatus 
// status: NOK, does not work
char* PTZ_GetStatus(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetStatus</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:5fb7e924-4429-493b-88d2-b2b124debbb3</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<GetStatus xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
			"</GetStatus>"
			//"<GetStatus/>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_GetStatus()


//	--------------- 5.4 Preset operations 
// 5.4.2 GetPresets 
// status: Ok
char* PTZ_GetPresets(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">"
		"<s:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
//			"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetPresets</a:Action>"
//			"<a:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</a:MessageID>"
			"<a:ReplyTo>"
				"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"
			"</a:ReplyTo>"
			"<a:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</a:To>"
		"</s:Header>"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<GetPresets xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
			"</GetPresets>"
		"</s:Body>"
	"</s:Envelope>"
	, GenerateUsernameToken()	
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;;
} // PTZ_GetPresets()

// 5.4.3 GotoPreset 
// status: OK
char* PTZ_GotoPreset(int preset)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "	
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"	
		"<SOAP-ENV:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
//			"<wsa5:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GotoPreset</wsa5:Action>"
//			"<wsa5:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<GotoPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<PresetToken>%d</PresetToken>"
			"</GotoPreset>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	, preset
 	);
	return mdata;
} // PTZ_GotoPreset()	


//	--------------- 5.5 Home Position operations 
// 5.5.1 GotoHomePosition 
// status: NOT SUPPORTED
// HTTP/1.1 500 Internal Server Error
// ter:ActionNotSupported
char* PTZ_GotoHomePosition(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">"
		"<s:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
//			"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GotoHomePosition</a:Action>"
//			"<a:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</a:MessageID>"
			"<a:ReplyTo>"
				"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"
			"</a:ReplyTo>"
			"<a:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</a:To>"
		"</s:Header>"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<GotoHomePosition xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<Speed>1</Speed>"
			"</GotoHomePosition>"
		"</s:Body>"
	"</s:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_GotoHomePosition()

// 5.5.2 SetHomePosition 
// status: NOT SUPPORTED
// HTTP/1.1 500 Internal Server Error
char* PTZ_SetHomePosition(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">"
		"<s:Header>"
			"<Security s:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
//			"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GotoHomePosition</a:Action>"
//			"<a:MessageID>urn:uuid:791c1b60-a696-4b82-8b6f-5005b172cf83</a:MessageID>"
			"<a:ReplyTo>"
				"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"
			"</a:ReplyTo>"
			"<a:To s:mustUnderstand=\"1\">http://%s:%s/onvif/service</a:To>"
		"</s:Header>"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<SetHomePosition xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
			"</SetHomePosition>"
		"</s:Body>"
	"</s:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_SetHomePosition()		


// 5.10 GetServiceCapabilities 
// Description: 
//	Returns the capabilities of the PTZ service. The result is returned in a typed answer.
// 	Indicates that the PTZStatus includes Position information. (actually id does not)
// https://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl
// status: ok
char* PTZ_GetServiceCapabilities(void)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetServiceCapabilities</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:5fb7e924-4429-493b-88d2-b2b124debbb3</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<GetServiceCapabilities/>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()	
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
 	);
	return mdata;
} // PTZ_GetServiceCapabilities()

// Auxiliary Operations
// SendAuxiliaryCommand
// https://www.onvif.org/specs/srv/ptz/ONVIF-PTZ-Service-Spec-v1712.pdf
// 5.6 Auxiliary operations
// operations to manage auxiliary commands of a PTZ node, such as an infrared (IR) lamp, a heater or a wiper
// Auxiliary Command commands hall conform to the syntax specified in Section 8.6 Auxiliary operation of ONVIF Core Specification
char* PTZ_SendAuxiliaryCommand(char *command, char *value)
{
	snprintf(mdata, sizeof(mdata),
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<SOAP-ENV:Envelope "
		"xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		">"
		"<SOAP-ENV:Header>"
			"<Security SOAP-ENV:mustUnderstand=\"0\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
				"%s" //"<UsernameToken>"
			"</Security>"
			//"<wsa5:Action SOAP-ENV:mustUnderstand=\"1\">http://www.onvif.org/ver20/ptz/wsdl/GetServiceCapabilities</wsa5:Action>"
			//"<wsa5:MessageID>urn:uuid:5fb7e924-4429-493b-88d2-b2b124debbb3</wsa5:MessageID>"
			"<wsa5:ReplyTo>"
				"<wsa5:Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address>"
			"</wsa5:ReplyTo>"
			"<wsa5:To SOAP-ENV:mustUnderstand=\"1\">http://%s:%s/onvif/service</wsa5:To>"
		"</SOAP-ENV:Header>"
		"<SOAP-ENV:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			"<SendAuxiliaryCommand xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
				"<ProfileToken>profile_2</ProfileToken>"
				"<%s/>"
				//"<%s>%s</%s>"
			"</SendAuxiliaryCommand>"
		"</SOAP-ENV:Body>"
	"</SOAP-ENV:Envelope>"
	, GenerateUsernameToken()
	, DEVICE_IP
	, ONVIF_DEVICE_PORT
	, command
	//, value , command
 	);
	return mdata;
} // PTZ_SendAuxiliaryCommand()





/**
	-----------------------------------------------------------------------------------------------
	---------------  Autehntication  -----------
	-----------------------------------------------------------------------------------------------	
		
	[1] ONVIF Core Specification
		https://www.onvif.org/specs/core/ONVIF-Core-Specification.pdf
	
	5.9.1 Authentication
**/
/**
Generates the UsernameToken strcuture from the username and password
returns a c-string containing the following

	<UsernameToken>
		<Username>{username}</Username>
		<Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest">{digest}</Password>
		<Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary">{nonce}</Nonce>
		<Created xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">{created}</Created>
	</UsernameToken>
	
Python equivalent code (that I took as reference for the C version below):
	created = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%S.000Z")

	raw_nonce = os.urandom(20)
	nonce = base64.b64encode(raw_nonce)

	sha1 = hashlib.sha1()
	sha1.update(raw_nonce + created.encode('utf8') + password.encode('utf8'))
	raw_digest = sha1.digest()
	digest = base64.b64encode(raw_digest)	
**/	


uint8_t *urandom(uint8_t [], size_t );
char* base64Encoder(uint8_t [], size_t , char *);
//char *timeISO8601(char *, size_t , char *);

char *GenerateUsernameToken(void)
{
	char *str= UsernameToken;
	size_t sz_max= sizeof(UsernameToken);
	
	char created[32];	
	timenowISO8601(created, sizeof(created));
//	fprintf(stdout,"\ncreated= %s", created ); 

	uint8_t raw_nonce[20];
	char nonce[32];
	urandom(raw_nonce, 20);
	base64Encoder(raw_nonce, 20, nonce);

//	fprintf(stdout,"\nnonce  (%d)= %s", strlen(nonce), nonce ); 

	//  raw_nonce, created, DEVICE_ACCOUNT_PASSWORD
	// bytestr= nonce + timenowISO8601 + DEVICE_ACCOUNT_PASSWORD
	uint8_t bytestr[128];
	size_t sz=0;
	size_t i=0;
	for(i=0; i<20; i++) bytestr[sz + i]= raw_nonce[i];
	sz += i;
	for(i=0; i<strlen(created); i++) bytestr[sz + i]= created[i];
	sz += i;
	for(i=0; i<strlen(DEVICE_ACCOUNT_PASSWORD); i++) bytestr[sz + i]= DEVICE_ACCOUNT_PASSWORD[i];
	sz += i;

	// 2.  Get the SHA-1 hash (160 bits) from the concatenated string (nonce + timenowISO8601 + DEVICE_ACCOUNT_PASSWORD)
	// OpenSSL SHA1 
	// sha1_digest must have space for SHA_DIGEST_LENGTH == 20 bytes of output
	unsigned char sha1_digest[SHA_DIGEST_LENGTH];
	SHA_CTX context;
	if(!SHA1_Init(&context)) fprintf(stdout,"\nchungo 1");
	else if(!SHA1_Update(&context, bytestr, sz)) fprintf(stdout,"\nchungo 2");
	else if(!SHA1_Final(sha1_digest, &context)) fprintf(stdout,"\nchungo 3");

	// 3. base64Encoder(digest)
	char digest[64];
	base64Encoder((uint8_t*)sha1_digest, SHA_DIGEST_LENGTH, digest);		
		
//	fprintf(stdout,"\ndigest (%d)= %s", strlen(digest), digest ); 

	snprintf(str, sz_max,
		"<UsernameToken>"
		"<Username>%s</Username>"
		"<Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%s</Password>"
		"<Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">%s</Nonce>"
		"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"
		"</UsernameToken>",
		DEVICE_ACCOUNT_USERNAME,
		digest,
		nonce,
		created);
	return str;
} // GenerateUsernameToken

/*
char *GenerateUsernameToken2(void)
{
	return GenerateUsernameToken(UsernameToken, sizeof(UsernameToken));
}
*/






// ------------------------------------------------------------------------------------------------
// --------------- AUXILIARY     ------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------



// int GenerateHTML(char *html, size_t size_of_html, char *data)
// {
//	char url[]= ONVIF_SERVICE;
	// snprintf(html, size_of_html,
		// "POST /%s HTTP/1.1\r\n"			// ONVIF_SERVICE
		// "Host: %s\r\n"					// DEVICE_IP 
		// "User-Agent: %s\r\n"			// USER_AGENT
		//"Accept:application/json\r\n"
		// "Accept-Encoding: gzip, deflate\r\n"
		// "Content-Type: application/soap+xml; charset=utf-8\r\n"
		// "Accept: */*\r\n"
		// "Connection: close\r\n"
		// "Content-Length: %d\r\n"
		// "\r\n"
		// "%s",
			// ONVIF_SERVICE,
			// DEVICE_IP,
			// USER_AGENT,
			// strlen(data),
			// data
		// );
		// return 0;
// }

// int AddHTTPHeader2(char *html, size_t size_of_html, const char * service, char *data)
// {
//	char url[]= ONVIF_SERVICE;
	// snprintf(html, size_of_html,
		// "POST /%s HTTP/1.1\r\n"			// service
		// "Host: %s\r\n"					// DEVICE_IP 
		// "User-Agent: %s\r\n"			// USER_AGENT
		// "Accept-Encoding: gzip, deflate\r\n"
		// "Content-Type: application/soap+xml; charset=utf-8\r\n"
		// "Accept: */*\r\n"
		// "Connection: close\r\n"
		// "Content-Length: %d\r\n"
		// "\r\n"
		// "%s",
			// service,
			// DEVICE_IP,
			// USER_AGENT,
			// strlen(data),
			// data
		// );
		// return 0;
// }



// Return a random bytestring of size bytes suitable for cryptographic use.
uint8_t *urandom(uint8_t bytestring[], size_t size)
{
	for(size_t i=0; i < size; i++) bytestring[i]= (uint8_t) (rand() & 0x000000FF);
	return bytestring;
}


// Takes string to be encoded as input
// and its length and returns encoded string
char* base64Encoder(uint8_t input_str[], size_t len_str, char *res_str)
{
	const char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	int index, no_of_bits = 0,  val = 0, count = 0, temp;
	unsigned padding = 0;
	unsigned int i, j, k = 0;
	
	// Loop takes 3 characters at a time from
	// input_str and stores it in val
	for (i = 0; i < len_str; i += 3)
	{
		val = 0, count = 0, no_of_bits = 0;

		for (j = i; j < len_str && j <= i + 2; j++)
		{
			// binary data of input_str is stored in val
			val = val << 8;
			// (A + 0 = A) stores character in val
			val = val | input_str[j];
			// calculates how many time loop
			// ran if "MEN" -> 3 otherwise "ON" -> 2
			count++;
		}

		no_of_bits = count * 8;

		// calculates how many "=" to append after res_str.
		padding = no_of_bits % 3;

		// extracts all bits from val (6 at a time)
		// and find the value of each block
		while (no_of_bits != 0)
		{
			// retrieve the value of each block
			if (no_of_bits >= 6)
			{
				temp = no_of_bits - 6;
				// binary of 63 is (111111) f
				index = (val >> temp) & 63;
				no_of_bits -= 6;		
			}
			else
			{
				temp = 6 - no_of_bits;
				// append zeros to right if bits are less than 6
				index = (val << temp) & 63;
				no_of_bits = 0;
			}
			res_str[k++] = char_set[index];
		}
	}
	// padding is done here
	for (i = 1; i <= padding; i++)
	{
		res_str[k++] = '=';
	}
	res_str[k] = '\0';
	return res_str;
} // base64Encoder()



/* ----------------------------------------------------------------------------------------------------- */
/* -------------------------------- UsernameToken  ---------------------------------------------- */
// global standardised ISO 8601 date format that is “YYYY-MM-DDTHH:mm:ssZ”
char *timenowISO8601(char *ts, size_t sz_max)
{
	time_t t = time(NULL); 
	struct tm *tm = localtime(&t);
	// .000 is the fraction of a second and Z indicates UTC timezone.
	// 2023-05-06T16:40:32.000Z
	snprintf(ts, sz_max, "%d-%02d-%02dT%02d:%02d:%02d.000Z", 
		2000+tm->tm_year-100,
		tm->tm_mon + 1,
		tm->tm_mday,	 
		tm->tm_hour, 
		tm->tm_min, 
		tm->tm_sec);
	return ts;
} // timenowISO8601()

// return currernt time in global standardised ISO 8601 date format that is “YYYY-MM-DDTHH:mm:ssZ”
// if offset is provided then timeISO8601() returns current time + offser in ISO8601 format
// offset is a c-string formatted as “dd:hh:mm:ss”
// dd (days) can be any value >=0 and greater than 30 days (1 month)
char *timeISO8601(char *ts, size_t sz_max, char *offset)
{
	int d,h,m,s;
	d=h=m=s=0;
	if(strlen(offset))
	{
		sscanf(offset, "%d:%d:%d:%d", &d,&h,&m,&s);
	}
	timeval tv;
	// gettimeofday gives the number of seconds and microseconds since the Epoch
	// Epoch time is the number of seconds that have elapsed since January 1, 1970 (midnight UTC/GMT), not counting leap seconds (in ISO 8601: 1970-01-01T00:00:00Z)
	gettimeofday(&tv, NULL);
	time_t t= tv.tv_sec + d*24*3600 + h*3600 + m*60 + s;
	struct tm *tm = localtime(&t);
//	snprintf(ts, sz_max, "%d-%02d-%02dT%02d:%02d:%02d.000Z", 
	snprintf(ts, sz_max, "%d-%02d-%02dT%02d:%02d:%02dZ", 
		2000+tm->tm_year-100,
		tm->tm_mon + 1,
		tm->tm_mday,	 
		tm->tm_hour, 
		tm->tm_min, 
		tm->tm_sec);	
	return ts;
} // timeISO8601()


// END OF FILE