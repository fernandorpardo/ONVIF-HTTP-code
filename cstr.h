/** ************************************************************************************************
 *	char string & parser library header
 *
 ** ************************************************************************************************
**/
#define CSTRLIB_VERSION	"1.2.0"

// Char * str functions
int cstr_find(char* str, char const* needle, int pos=0, int max=0);
void cstr_replace (char *str, char a, char b);
char *cstr_sub (char *str, char *sub, int p0, int p1);
void cstr_dump(char * bf, int nbytes);
void cstr_fdump(FILE *fp, char * bf, int nbytes);
char* cstr_copy(char* dst, char* org , size_t dst_max_sz);

// ------------------------------------------------------------------------------------------------
// --------------- HTTP HEADER --------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
int HTTPHeader_begin(char *htmlcode, size_t sz);
bool HTTPHeader_end(char *htmlcode, size_t sz);
unsigned int HTTPHeader_status_code(char *htmlcode);
void HTTPHeader_status_str(char *htmlcode, char *str);
char* HTTPHeader_Entity(const char *entity, char *htmlcode, int max, char *value);

// ------------------------------------------------------------------------------------------------
// --------------- XML PARSER  --------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// XML parser
bool XML_CodeComplete(char* xmlcode, const char* tag);
bool getInnerTag(char *ptr, char *tag, int &a, int &b, int max);
char *XML_CodeGetParam(char* xmlcode, char const* param, char *value);
char *XML_CodePtr(char* htmlcode);
int XML_GetNParam(char* xmlcode, char const* param, char *value, int N=1);
int XML_Parser(char* xmlcode, char const* tag, char *value, size_t sz_value);
void XML_Dump(const char *xml, unsigned int n=0);

// ------------------------------------------------------------------------------------------------
// --------------- JSON PARSER --------------------------------------------------------------------						
// ------------------------------------------------------------------------------------------------
//int jsonParseObject(char*ptr, unsigned int pos0, unsigned int len);
//int jsonScan(char*ptr, unsigned int len, int q);
int jsonObj(char*ptr, unsigned int len, int index, int *ix, int *isz);
//int jsonGetObj(char*ptr, unsigned int len, char*buffer, unsigned int szmax, int iobj);
//int jsonSearchObj(char*ptr, unsigned int len, unsigned int &a, unsigned int &b);
//char* jsonParseName(const char *pname, char*ptr, unsigned int pos0, unsigned int len,  char*pvalue, size_t max);
char* jsonParseValue(const char *pname, char*ptr, unsigned int pos0, unsigned int len,  char*pvalue, size_t sz_max);
void jsonDump(const char *str, unsigned int n=0);

// END OF FILE