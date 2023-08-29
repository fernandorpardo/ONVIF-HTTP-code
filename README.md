# ONVIF-HTTP-code
C++ implementation of the ONVIF protocol at HTTP level

## About
This is a C++ client partially implementing the ONVIF protocol at plain HTTP level. This is not a comprehensive implementation of the protocol, but some working code tested with some TP-Link TAPO camera variants (C100, C320 and C500, so far).

The behaviours described here are the one observed with the devices and versions tested. 

The code is written for Raspberry PI.

## Intro
ONVIF organization defines a set of interfaces for security products such as surveillance cameras. When in a commercial brochure it is said that a camera is ONVIF compliant it means that the camera implements the interfaces defined by the ONVIF specification that you can use to get access to the camera functionality.  

When I tried  ONVIF to manage my cameras I realized that most of the examples I found used libraries (e.g. gSOAP) to manage ONVIF complexity and of its companions specifications, to the point you actually don’t benefit of the openness and simplicity of the web services nature.

What comes next tries to show what happens underneath.

## Some basics
ONVIF is a SOAP protocol. That means that the message exchange is performed using HTTP carrying an XML formatted message as payload. Although these terms may sound complex all what is going on between the client and the camera is a message exchange in plain text format over a TCP connection. That message is made of an HTTP header and the XML structure as defined by ONVIF, something that looks like this:

```xml
POST /onvif/device_service HTTP/1.1
Host: 192.168.1.10
Content-Type: application/json; charset=UTF-8
Content-Length: … payload length …

<?xml version="1.0" encoding="UTF-8"?>
<? … here the SOAP message … >
```

In Linux a TCP connection is stablished creating a socket, and the data is sent and received using the write() and read() functions.

You can take the kind of text string above and send it over to the camera using the language and OS of your choice and it will work the same. The complexity is in the generation of the XML part; all the rest is a no-brainer.

## A simple case
The simplest case is the **GetSystemDateAndTime** that does not require authentication.

Let assume your camera’s IP address is 192.168.1.127

TAPO camera ONVIF server is listening at PORT 2020.

To retrieve date and time information of the camera, you need to:

(1) open a TCP connection to 192.168.1.127:2020

(2) Over the open connection, send the following text
```xml
POST /onvif/service HTTP/1.1
Host: 192.168.1.127
User-Agent: ONVIFClient
Content-Type: application/soap+xml; charset=utf-8
Accept: */*
Connection: close
Content-Length: 229

<?xml version="1.0" encoding="UTF-8"?>
<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:tds="http://www.onvif.org/ver10/device/wsdl">"
<s:Header>
</s:Header>
  <s:Body>
    <tds:GetSystemDateAndTime/>
  </s:Body>
</s:Envelope>
```

(3) Wait for the response (this what I get from the C500)
```xml
HTTP/1.1 200 OK
Connection: close
Content-Type: application/soap+xml; charset=utf-8
Content-Length: 2461

<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope" xmlns:SOAP-ENC="http://www.w3.org/2003/05/soap-encoding" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing" xmlns:wsdd="http://schemas.xmlsoap.org/ws/2005/04/discovery" xmlns:chan="http://schemas.microsoft.com/ws/2005/02/duplex" xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" xmlns:wsa5="http://www.w3.org/2005/08/addressing" xmlns:xmime="http://tempuri.org/xmime.xsd" xmlns:xop="http://www.w3.org/2004/08/xop/include" xmlns:wsrfbf="http://docs.oasis-open.org/wsrf/bf-2" xmlns:wstop="http://docs.oasis-open.org/wsn/t-1" xmlns:wsrfr="http://docs.oasis-open.org/wsrf/r-2" xmlns:wsnt="http://docs.oasis-open.org/wsn/b-2" xmlns:tt="http://www.onvif.org/ver10/schema" xmlns:ter="http://www.onvif.org/ver10/error" xmlns:tns1="http://www.onvif.org/ver10/topics" xmlns:tds="http://www.onvif.org/ver10/device/wsdl" xmlns:tmd="http://www.onvif.org/ver10/deviceIO/wsdl" xmlns:trt="http://www.onvif.org/ver10/media/wsdl" xmlns:tev="http://www.onvif.org/ver10/events/wsdl" xmlns:tdn="http://www.onvif.org/ver10/network/wsdl" xmlns:timg="http://www.onvif.org/ver20/imaging/wsdl" xmlns:trp="http://www.onvif.org/ver10/replay/wsdl" xmlns:tan="http://www.onvif.org/ver20/analytics/wsdl" xmlns:tptz="http://www.onvif.org/ver20/ptz/wsdl" xmlns:hikwsd="http://www.onvifext.com/onvif/ext/ver10/wsdl" xmlns:hikxsd="http://www.onvifext.com/onvif/ext/ver10/schema"><SOAP-ENV:Header></SOAP-ENV:Header
><SOAP-ENV:Body>
<tds:GetSystemDateAndTimeResponse>
  <tds:SystemDateAndTime>
    <tt:DateTimeType>NTP</tt:DateTimeType>
    <tt:DaylightSavings>false</tt:DaylightSavings>
    <tt:TimeZone>
      <tt:TZ>UTC+01:00</tt:TZ>
    </tt:TimeZone>
    <tt:UTCDateTime>
      <tt:Time>
        <tt:Hour>7</tt:Hour>
        <tt:Minute>44</tt:Minute>
        <tt:Second>59</tt:Second>
      </tt:Time>
      <tt:Date>
        <tt:Year>2023</tt:Year>
        <tt:Month>8</tt:Month>
        <tt:Day>25</tt:Day>
      </tt:Date>
    </tt:UTCDateTime>
    <tt:LocalDateTime>
      <tt:Time>
        <tt:Hour>9</tt:Hour>
        <tt:Minute>44</tt:Minute>
        <tt:Second>59</tt:Second>
      </tt:Time>
      <tt:Date>
        <tt:Year>2023</tt:Year>
        <tt:Month>8</tt:Month>
        <tt:Day>25</tt:Day>
      </tt:Date>
    </tt:LocalDateTime>
  </tds:SystemDateAndTime>
</tds:GetSystemDateAndTimeResponse>
</SOAP-ENV:Body>
</SOAP-ENV:Envelope>
```


## Authentication
All except **GetSystemDateAndTime** and **GetCapabilities** requests require client authentication (this is the observed behaviour with the TAPO cameras listed).

Authentication is defined in [“ONVIF Application Programmer's Guide – 6. Security”](https://www.onvif.org/wp-content/uploads/2016/12/ONVIF_WG-APG-Application_Programmers_Guide-1.pdf).

To authenticate the client, you need to include a UsernameToken in the header as follows:
```xml
<s:Header>
<UsernameToken>
<Username>{username}</Username>
<Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest">{digest}</Password>
<Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary">{nonce}</Nonce>
<Created xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">{created}</Created>
</UsernameToken>
</s:Header>
```

You should have created a camera account in your camera via the Tapo app (if you don’t know how, then check [“How to view Tapo camera on PC through RTSP stream?”](https://www.tapo.com/en/faq/34/). 

Edit the file onvif.h and modify 'DEVICE_ACCOUNT_USERNAME' and 'DEVICE_ACCOUNT_PASSWORD' with the username/password of your camera. The client will take the values from there to generate the **UsernameToken**.

The **UsernameToken** has 4 parameters:

   {username} – is the username of the camera account.  
   {nonce} - A random, unique number generated by a client.  
   {created} - The UtcTime when the request is made.  
   {digest} – a hash of the password.

To better understand these parameters, you can see how to generate them in Python that works well as pseudocode:
```python
	created = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%S.000Z")

	raw_nonce = os.urandom(20)
	nonce = base64.b64encode(raw_nonce)

	sha1 = hashlib.sha1()
	sha1.update(raw_nonce + created.encode('utf8') + password.encode('utf8'))
	raw_digest = sha1.digest()
	digest = base64.b64encode(raw_digest)	
```

The C code for this job is in function `char *GenerateUsernameToken(void)` in file onvif.cpp.
Note that you need to install a SHA library in your Linux environment such as OpenSSL.

To install OpenSSL libraries in a Raspberry PI, run:
```
sudo apt-get install libssl-dev
```

## Events
Events is the way to get notifications when something happens at the camera side, such as a movement detection.

In general, there are three kind of Event service interfaces [“ONVIF Event Handling Test Specification - 1.1.1. Events”](https://www.onvif.org/wp-content/uploads/2018/07/ONVIF_Event_Handling_Test_Specification_18.06.pd).

   •	Basic Notification interface  
   •	Real time Pull Point Notification interface  
   •	Seek
   
The one I tried is the **Basic Notification Interface** ["ONVIF Core Specification - 9.3 Basic Notification Interface"](https://www.onvif.org/specs/core/ONVIF-Core-Specification.pdf), that defines 4 operations: Subscribe, Notify, Renew and Unsubscribe. 


The way it works is that the client sends a **Subscribe** request to the camera such as this:
```xml
POST /onvif/service HTTP/1.1
Host: 192.168.1.127
User-Agent: ONVIFClient
Content-Type: application/soap+xml; charset=utf-8
Accept: */*
Connection: close
Content-Length: 1584

<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope" xmlns:tds="http://www.onvif.org/ver10/device/wsdl" xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing" xmlns:wsa5="http://www.w3.org/2005/08/addressing" xmlns:wsnt="http://docs.oasis-open.org/wsn/b-2"><SOAP-ENV:Header><wsa5:Action SOAP-ENV:shallUnderstand="true">http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeRequest</wsa5:Action><wsa5:ReplyTo><wsa5::Address>http://www.w3.org/2005/08/addressing/anonymous</wsa5:Address></wsa5:ReplyTo><Security SOAP-ENV:mustUnderstand="1" xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
<UsernameToken>
<Username>iambobot</Username>
<Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest">71EEct0VNVY1LoLokPRUCeBGEqU=</Password>
<Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary">G+jnjXZaLmMzn8maZjINtzFYo1o=</Nonce>
<Created xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">2023-08-26T08:54:46.000Z</Created>
</UsernameToken>
</Security>
<wsa5:To SOAP-ENV:shallUnderstand="1">http://192.168.1.127/onvif/service</wsa5:To> </SOAP-ENV:Header>
<SOAP-ENV:Body>
  <wsnt:Subscribe>
    <wsnt:ConsumerReference>
      <wsa5:Address>http://192.168.1.100:8002</wsa5:Address>
    </wsnt:ConsumerReference>
    <wsnt:InitialTerminationTime>2023-08-26T09:24:46Z</wsnt:InitialTerminationTime>
  </wsnt:Subscribe>
</SOAP-ENV:Body>
</SOAP-ENV:Envelope>
```

In the Subscribe request we are telling where to send the notifications:
```xml
<wsa5:Address>http://192.168.1.100:8002</wsa5:Address>
```

In this case we have a process listening at port 8002 that is part of the client running in a Raspberry PI which IP address is 192.168.1.100

Besides, we are setting an expiration time so that the camera will stop sending notifications pass that time
```xml
<wsnt:InitialTerminationTime>2023-08-26T09:24:46Z</wsnt:InitialTerminationTime>
```

A **Notify** message is received whenever the camera has something to notify.

The other two operations (Renew and Unsubscribe) I couldn’t make them work.


## PTZ
TAPO C500 supports pan&tilt.

The client includes a PTZ mode that allows you to move the camera using the arrow keys of the keyboard. This functionality relies on the **RelativeMove** request.

You can move the camera to predefined spots using the **AbsoluteMove** request.

With these two operations you can actually get all what you need for PTZ with the C500, although some other operations are implemented and supported.

The **RelativeMove** and **AbsoluteMove** have x,y coordenates as parameters. For **RelativeMove** x,y is a delta to the current position while for **AbsoluteMove** x,y is an absolute position, e.g. 
```xml
<AbsoluteMove xmlns="http://www.onvif.org/ver20/ptz/wsdl">
     <ProfileToken>profile_2</ProfileToken>
     <Position>
        <PanTilt x="0.500000" y="0.500000" space="http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace"/>
     </Position>
     <Speed>0.500000</Speed>
</AbsoluteMove
```

In both cases **Speed** parameter sets the rotation speed.

The range of the x,y coordinates is -1,0 to 1,0, and speed goes from 0,0 to 1,0. These values are obtained in the response to **GetNodes**:
```xml
<tt:SupportedPTZSpaces>
   <tt:AbsolutePanTiltPositionSpace>
   <tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace</tt:URI>
   <tt:XRange>
      <tt:Min>-1</tt:Min>
      <tt:Max>1</tt:Max>
   </tt:XRange>
   <tt:YRange>
      <tt:Min>-1</tt:Min>
      <tt:Max>1</tt:Max>
   </tt:YRange>
</tt:AbsolutePanTiltPositionSpace>
   <tt:RelativePanTiltTranslationSpace>
   <tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace</tt:URI>
   <tt:XRange>
      <tt:Min>-1</tt:Min>
      <tt:Max>1</tt:Max>
   </tt:XRange>
   <tt:YRange>
      <tt:Min>-1</tt:Min>
      <tt:Max>1</tt:Max>
   </tt:YRange>
</tt:RelativePanTiltTranslationSpace>
<tt:PanTiltSpeedSpace>
   <tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace</tt:URI>
   <tt:XRange>
      <tt:Min>0</tt:Min>
      <tt:Max>1</tt:Max>
   </tt:XRange>
</tt:PanTiltSpeedSpace>
```

 
## System dependencies
The client uses a temporal file storage for the HTTP responses. You need to set the path `LOCAL_FILE_STORAGE` in file local.h and make sure right access is given to the program.

