#ifndef _HADEBUG_H_
#define _HADEBUG_H_

//#define WS_DEBUG "/var/www/ramdisk/wsdebug.log"
//void filetrace(char *str, bool do_ts);
void trace2file( char const*filename, char *str, bool do_ts=true);
#endif
// END OF FILE