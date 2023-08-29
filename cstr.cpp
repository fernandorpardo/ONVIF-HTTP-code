/** ************************************************************************************************
 *	C-strings management & parsers library
 *  Part of the Wilson project (www.iambobot.com)
 *  Fernando R
 *
 *  C strings management
 *  XML parser
 *  JSON parser
 *
 *  1.0.0 - May 2021
 *  1.1.0 - October 2022
 *  1.2.0 - November 2022
 * - jsonParseObject deprecated
 *  1.2.1 - January 2023
 * - XML parser clean-up
 *  1.2.2 - June 2023
 * - XML_Dump()
 *
 ** ************************************************************************************************
**/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//#include <sys/types.h>
//#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
using namespace std;
#include "cstr.h"

/** 
* -------------------------------------------------------------------------------------------------
*
*      CSTR - Character string management functions
*
* -------------------------------------------------------------------------------------------------
**/
// search 'needle' into 'str' and
// retuns
//    the postion in 'needle' if found
//    -1 if not found
int cstr_find(char* str, char const* needle, int pos, int max_sz)
{
	int i= 0;
	int j= 0;
	if(str==0 || needle==0 || needle[0]=='\0') return -1;
	if(pos>0) while(str[i]!='\0' && i<pos) i++;
	if(str[i]=='\0') return -1;
	while(str[i]!='\0' && (max_sz==0 || i<max_sz))
	{
		char *p=&str[i], c= needle[0];
		for(; *p!=c && (max_sz==0 || i<max_sz) && str[i]!='\0'; p++, i++);
		if(*p==c)
		{
			j=0;
			char *p= &str[i+j], *pn= (char*)&needle[j];
			for(; *p==*pn && (max_sz==0 || (i+j)<max_sz) && *p!='\0'; p++, pn++, j++);
			if(needle[j]=='\0' && j>0) 
				return i;
			i++;
		}
	}
	return -1;
} // cstr_find

void cstr_dump(char * bf, int nbytes)
{
	//fprintf(stdout,"\nn= %d\n", nbytes);
	char str[32];
	str[0]='\0';
	int j=0;
	for(int i=0; i<nbytes; i++)
	{
		char c= bf[i];
		fprintf(stdout, "%02X ", bf[i]);
		if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>=0x20 && c<=0x7e)) str[j++]= c;
		else str[j++]='.';
		str[j]='\0';
		if(j== 16)
		{
			fprintf(stdout,"   %s\n", str);
			j=0;
		}
	}
	if(j)
	{
		while(j++<16) fprintf(stdout,"   ");
		fprintf(stdout,"   %s\n", str);
	}
	//fprintf(stdout,"\n");
}

void cstr_fdump(FILE *fp, char * bf, int nbytes)
{
	char str[32];
	str[0]='\0';
	int j=0;
	char sbf[64];
	for(int i=0; i<nbytes; i++)
	{
		char c= bf[i];
		snprintf(sbf, sizeof(sbf), "%02X ", bf[i]);
		fwrite(sbf,  strlen(sbf), 1, fp);
		if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>=0x20 && c<=0x7e)) str[j++]= c;
		else str[j++]='.';
		str[j]='\0';
		if(j== 16)
		{
			snprintf(sbf, sizeof(sbf),"   %s\n", str);
			fwrite(sbf,  strlen(sbf), 1, fp);
			j=0;
		}
	}
	if(j)
	{
		while(j++<16) {
			snprintf(sbf, sizeof(sbf),"   ");
			fwrite(sbf,  strlen(sbf), 1, fp);
		}
		
		snprintf(sbf, sizeof(sbf),"   %s\n", str);
		fwrite(sbf,  strlen(sbf), 1, fp);
	}
}

void cstr_replace (char *str, char a, char b)
{
	if(str==0 || str[0]=='\0') return;
	for(int i=0; str[i]!='\0'; i++) 
	if(str[i] == a) 
	{
		// remove
		if(b=='\0') for(int j=i; str[j]!='\0'; j++) str[j]= str[j+1];
		// replace
		else str[i]=b;
	}
}
char *cstr_sub (char *str, char *sub, int p0, int p1)
{
	int i=0;
	for(; (p0+i)<=p1 && str[p0+i]!='\0'; i++) sub[i]= str[p0+i];
	sub[i]='\0';
	return sub;
} // cstr_sub()

/*
int str_find(char* str, char const* needle , int max)
{
	//if(max==0) { fprintf(stdout,"\n-------------------------"); return -1; }
	int i= 0, j;
	if(str==0 || needle==0 || needle[0]=='\0') return -1;
	while(str[i]!='\0' && i<max)
	{
		char *p=&str[i], c= needle[0];
		for(; *p!=c && i<max && str[i]!='\0'; p++, i++);
		if(*p==c)
		{
			j=0;
			char *p= &str[i+j], *pn= (char*)&needle[j];
			for(; *p==*pn && (i+j)<max && *p!='\0'; p++, pn++, j++);
			if(needle[j]=='\0' && j>0) return i;
			i++;
		}
	}
	return -1;
}
*/
// to be used instead of snprintf to prevent destitation size warning
char* cstr_copy(char* dst, char* org , size_t dst_max_sz)
{
	int i=0;
	int sz= (int) dst_max_sz;
	for(; org[i]!='\0' && i<(sz - 1); i++) dst[i]=org[i];
	dst[i]= '\0';
	return dst;
} // cstr_copy

/** 
* -------------------------------------------------------------------------------------------------
*
*      HTTP PARSER
*
* -------------------------------------------------------------------------------------------------
**/

// say whether there is an HTTP HEADER start
// return 0..n > 0 with the position
// return -1 if it is not
int HTTPHeader_begin(char *htmlcode, size_t sz)
{
	// Check 1 - HTTP start
	return cstr_find(htmlcode, "HTTP/", 0, sz);
}
// say whether HTTP HEADER END is found
bool HTTPHeader_end(char *htmlcode, size_t sz)
{
	// Check 2 - HTTP end
	return (cstr_find(htmlcode, "\r\n\r\n", 0, sz)>0);
}

// HTTP/1.1 200 OK
// HTTP/1.1 404 Not Found
unsigned int HTTPHeader_status_code(char *htmlcode)
{
	char v[16];
	char str[16];
	char code[16];
	if(!strlen(htmlcode)) return 0;
	sscanf(htmlcode,"HTTP/%s %s %s", v, code, str);
	return atoi(code);
}

void HTTPHeader_status_str(char *htmlcode, char *str)
{
	char v[16];
	char code[16];
	char line[32];
	int l, i, e;
	str[0]='\0';
	if(!strlen(htmlcode)) return;
	sscanf(htmlcode,"HTTP/%s %s %s", v, code, str);
	for(e=0; htmlcode[e]!='\n' && htmlcode[e]!='\r' && e<31; e++);
	for(l=0; l<e; l++) line[l]=htmlcode[l];
	line[l]= '\0';
	for(i=0; line[i]!=' ' && i<l; i++);
	i++;
	for(;  i<l && line[i]!=' '; i++);
	i++;
	if(i<l) strcpy(str, &line[i]);
//	write(STDOUT_FILENO, line, strlen(line));
}

// RETURNS
// value = enty string if "entity" not found
//         string with the value of the "entity" if "entity" found
char* HTTPHeader_Entity(const char *entity, char *htmlcode, int max, char *value)
{
	char str[128];
	value[0]='\0';
	int i= cstr_find(htmlcode, entity, 0, max);
	snprintf(str, sizeof(str),"%s: %%s", entity);
	if(i>=0) sscanf(&htmlcode[i], str, value);
	return value;
}



/** 
* -------------------------------------------------------------------------------------------------
*
*      XML PARSER
*
* -------------------------------------------------------------------------------------------------
**/
// check that the XML contains the ending tag </tag> after <?xml, 
// that should be enough to validate that the XML data is complete
bool XML_CodeComplete(char* xmlcode, const char* tag)
{
	if(!strlen(xmlcode)) return false;
	//string str(htmlcode);
	//int pos = str.find("<?xml");
	int pos0= cstr_find(xmlcode, "<?xml", 0, strlen(xmlcode));
	if(pos0<0) return false;
	//pos = str.find("</data>");
	char endingstag[256];
	snprintf(endingstag, sizeof(endingstag),"</%s>", tag);
	int pos= cstr_find(xmlcode, endingstag, pos0, strlen(xmlcode));
	if(pos<0) return false;
	return true;
} // XML_CodeComplete

// in HTML reponse gets the postion of the XML data
// returns: pointer to "<?xml"
char *XML_CodePtr(char* htmlcode)
{
//	string str(htmlcode);
//	int pos = str.find("<?xml");
	int pos = cstr_find(htmlcode, "<?xml", 0, strlen(htmlcode));
	if(pos<0) return (char *) 0;
	return &htmlcode[pos];
} // XML_CodePtr

// simple parser
// search a tag value into xmlcode string
// 1 - (true) found
// 0 - (false) not found
int XML_Parser(char* xmlcode, char const* tag, char *value, size_t sz_value)
{
	char stag[256];
	value[0]= '\0';
	if(xmlcode == 0 || xmlcode[0]=='\0') return 0;
	
	//int p0= str_find0(xmlcode, "<?xml"); 
	//int pos1= cstr_find(xmlcode, "<?xml", 0, strlen(xmlcode));
	if(cstr_find(xmlcode, "<?xml", 0, strlen(xmlcode))<0) return 0;
	
	snprintf(stag, sizeof(stag), "<%s>", tag);
//	pos= str_find0(&xmlcode[p0], stag);
	int pos1= cstr_find(xmlcode, stag, 0, strlen(xmlcode));
	if(pos1<0) return 0;
	int lengtag= strlen(stag);
	
	snprintf(stag, sizeof(stag),"</%s>", tag);
//	int pos2= str_find0(&xmlcode[p0], stag);
	int pos2= cstr_find(xmlcode, stag, 0, strlen(xmlcode));
	if(pos2<0) return 0;
	
	int ln= pos2 - pos1 - lengtag;
	if(ln<0) return 0;
	
	int i;
	for(i=0; i<ln && i<(int)(sz_value-1); i++)
		value[i]= xmlcode[pos1 + lengtag + i];
	value[i]= '\0';
	return 1;
}

// a - offset al COMIENZO relativo a ptr[0]
// b - offset al final del </tag> relativo a ptr[0]
bool getInnerTag(char *ptr, char *tag, int &a, int &b, int max)
{
	char tag_begin0[32];
	char tag_begin1[32];
	char tag_end[32];
	int i, j, i0, i1, taglength;
	a=0;
	b=0;
	snprintf(tag_begin0, sizeof(tag_begin0), "<%s ", tag);
	snprintf(tag_begin1, sizeof(tag_begin1), "<%s>", tag);
	snprintf(tag_end, sizeof(tag_end), "</%s>", tag);
	
	i0=cstr_find(ptr, tag_begin0, 0, max);
	i1=cstr_find(ptr, tag_begin1, 0, max);
	bool tag0_type= (i1<0 || (i0>=0 && i0<i1));
	i= tag0_type ? i0 : i1;
	taglength= tag0_type ? strlen(tag_begin0) : strlen(tag_begin1);
	if(i>=0)
	{
		i+= taglength;
		if(tag0_type)
		{
			for(; ptr[i]!='>' && i<max; i++);
			if(max!=0 && !(i<max)) 
			{
				return false;
			}
			i++;
		}
		if((j=cstr_find(&ptr[i], tag_end, 0, max-i))>0)
		{
			a= i;
			b= i+j;
			return true;
		}
	}
	return false;
}

char *XML_CodeGetParam(char* xmlcode, char const* param, char *value)
{
	value[0]= '\0';
	if(strlen(xmlcode)>0) {
		string str(xmlcode);
		int pos = str.find("<?xml");
		if(pos>=0) {
			string xml = str.substr(pos);
			string strparam(param);
			string s= "<" + strparam + ">";
			int p1 = xml.find(s); 
			int p2 = xml.find("</" + strparam + ">");
			if(p1>=0 && p2>=0) {
				int l= p2-p1-s.length();
				if(l>0) {
					string strvalue= xml.substr(p1+s.length(), l);
					strcpy(value, strvalue.c_str());
				}
			}
		}
	}
	return value;
}



// -1 - not found
// >=0 length of value string
int XML_GetNParam(char* xmlcode, char const* param, char *value, int N)
{
	value[0]= '\0';
	int count= 0;
	if(strlen(xmlcode)>0 && N>0) 
	{
		string str(xmlcode);
		string::size_type  pos = str.find("<?xml");
		if(pos != string::npos) 
		{	
			str = str.substr(pos);
			string strparam(param);
			size_t sl= ("<" + strparam + ">").length();					
			string::size_type  pstart, pend;
			do {						
				pstart = str.find("<" + strparam + ">"); 
				pend =   str.find("</" + strparam + ">");
				if(pstart == string::npos || pend == string::npos) return -1;
				count ++;
				//fprintf(stdout, "\n%d %d %d", N, pstart,pend );
				//fflush(stdout);				
				if(count == N) break;
				str = str.substr(pend + ("</" + strparam + ">").length());
			} while(count < N);			
			int l= pend - pstart - sl;
			if(l>0) {
				string strvalue= str.substr(pstart + sl, l);
				strcpy(value, strvalue.c_str());
				return (int) sl;
			}
		}
	}
	return -1;
}

// Dump XML data in a tabulated structure format
// if n==0 data is assumed to be a zero-ended char string
void XML_Dump(const char *xml, unsigned int n)
{
	int tab=0;
	bool withinbegintag= false;
	bool withinheader= false;
	unsigned int m= (n==0) ? strlen(xml) : n;
	for(unsigned int i=0; i<m; i++)
	{
		char c= xml[i];
		if(withinheader)
		{
			fprintf(stdout, "%c", c);
			if(c=='>' && xml[i-1]=='?')  withinheader= false;
		}
		else if(c=='<' && (i+1)<m && xml[i+1]=='?') 
		{
			fprintf(stdout, "\r\n");
			fprintf(stdout, "%c", c);
			withinheader= true;
		}
		else if(c=='<' && (i+1)<m && xml[i+1]=='/') 
		{ 
			fprintf(stdout, "\r\n");
			if(tab) tab--; 
			for(int j=0; j<tab; j++) fprintf(stdout, "    ");
			fprintf(stdout, "%c", c);
		}
		else if(c=='<') 
		{  
			withinbegintag= true;
			fprintf(stdout, "\r\n");
			for(int j=0; j<tab; j++) fprintf(stdout, "    ");
			tab++;
			fprintf(stdout, "%c", c);
		}
		else if(c=='>') 
		{  
			fprintf(stdout, "%c", c);
			if(xml[i-1]=='/' && tab) tab--;
			if(withinbegintag && (i+1)<m && xml[i+1]!='<') 
			{
				fprintf(stdout, "\r\n");
				for(int j=0; j<tab+1; j++) fprintf(stdout, "    ");
			}
			withinbegintag= false;
		}
		else if(c!='\r' && c!='\n') fprintf(stdout, "%c", c);
	}
} // XML_Dump()

/**
 * ------------------------------------------------------------------------------------------------
 *										JSON PARSER library
 *
 * https://javaee.github.io/tutorial/jsonp001.html
 *
 * JSON defines only two data structures: objects and arrays. An object is a set of name-value pairs, 
 * and an array is a list of values. JSON defines seven value types: 
 * string, number, object, array, true, false, and null.
 *
 * JSON has the following syntax.
 * -  Objects are enclosed in braces ({}), their name-value pairs are separated by a comma (,), and 
 *    the name and value in a pair are separated by a colon (:). Names in an object are strings, whereas 
 *    values may be of any of the seven value types, including another object or an array.
 * -  Arrays are enclosed in brackets ([]), and their values are separated by a comma (,). Each value 
 *    in an array may be of a different type, including another array or an object.
 * -  When objects and arrays contain other objects or arrays, the data has a tree-like structure.
 * 
**/

/*
// Parse an object in buffer ptr 
// at position <a> of length <len>, excluding brackets {}
// {"cap":1619244096331,"volume":144507306347,"liquidity":2731199984,"btcDominance":0.4385740958260267}
// Return:
// Nada, ¿para que la hice? ¿para testing?
// candidate para deprecate
int jsonParseObject(char*ptr, unsigned int pos0, unsigned int len)
{	
	// PARSE JSON
	unsigned int a= pos0, b=0;
	char c;
	while(a < (pos0 + len))
	{
		b= a;
		for(c= ptr[b]; b<(pos0 + len)  && c!=','; c=ptr[++b]);
		if(b > a)
		{
			// item -> name:value
			char name[512];
			char value[512];
			unsigned int i, j;
			// name
			for(i= a, j=0; ptr[i]!=':' && i<b && j<(sizeof(name)-1); i++, j++) name[j]= ptr[i];
			name[(i<b)? j : 0]= '\0';
			i++;
			// value
			for(j=0; i<b && j<(sizeof(value)-1); i++, j++) value[j]= ptr[i];
			value[j]= '\0';
			cstr_replace(name,'"','\0');
			//fprintf(stdout, "--- %s = %s\n", name, value);
		}
		// next
		a= b;
		for(c= ptr[a]; a<(pos0 + len) && c!=','; c=ptr[++a]);
		a++;
	} 
	return 1;
}
*/
/* For shell command and testing
*/
/*
int jsonScan(char*ptr, unsigned int len, int q)
{
	unsigned int a= 0;
	unsigned int b;
	int count= 0;

	// look for first {
	char c= ptr[a];
	for(; a<len && c!='{'; c=ptr[++a]);
	if(a<len) 
	{
		a++;
		do
		{
			c= ptr[a];
			for(; a<len && c!='{'; c=ptr[++a]);
			if(a < len)
			{
				int brackets=0;
				a++;
				b= a;
				c= ptr[b];
				for(; b<len; )
				{
					if(c=='{') brackets ++;
					if(c=='}')
					{
						if(brackets) brackets --;
						else break;
					}
					c= ptr[++b];
				}
				if(b < len)
				{
					b--;
					fprintf(stdout, "\nObject %d len = %d\n", count, b-a+1);
					for(unsigned int i=a; i<=b; i++) fprintf(stdout, "%c", ptr[i]);
					fprintf(stdout, "\n--------------------------------");
					count ++;
					a= b+1;
					if(q>0) { q--; if(q==0) break; }
				}
			}
		} while(a<len && b<len);
	}
	return count;
}
*/
/*
	From a nexted list of objects [ {object 0}, {object 1}, {object 2}, ... ]
	gets object at position "index", including brackets
	index= 0 is first
	returns:
	ix - prt index pointing to the open bracket
	isz - is object length
	function returns Object length / -1 if index is beyond the list
*/
int jsonObj(char*ptr, unsigned int len, int index, int *ix, int *isz)
{
	unsigned int a= 0;
	unsigned int b;
	int count= 0;
	char c;
	*ix= 0; *isz= 0;
	if(index<0 || len==0 || ptr==0 || ix==0 || isz==0) return -1;
	do
	{
		for(c= ptr[a]; a<len && c!='{'; c= ptr[++a]);
		if(a < len)
		{
			int brackets=0;
			b= a + 1;
			c= ptr[b];
			for( ; b<len ; )
			{
				if(c=='{') brackets ++;
				if(c=='}')
				{
					if(brackets) brackets --;
					else break;
				}
				c= ptr[++b];
			}
			if(b < len)
			{
				if(index == count)
				{
					*ix= a; *isz= (b-a+1);
					return (b-a+1);
				}
				count ++;
				a= b + 1;
			}
		}
	} while(a<len && b<len);
	return -1;
} // jsonObj()

/*
// Search json object {}
// return   a= first position after {
//			b= last position before }
int jsonSearchObj(char*ptr, unsigned int len, unsigned int &a, unsigned int &b)
{
	do
	{
		char c= ptr[a];
		for(c= ptr[a]; a<len && c!='{'; c=ptr[++a]);
		if(a < len)
		{
			a++;
			b= a;
			for(c= ptr[b]; b<len &&  c!='}'; c=ptr[++b]);
			if(b < len)
			{
				b--;
				return 1;
			}
		}
	} while(a<len && b<len);
	return 0;
}
*/

/*
	From a nexted list of objects { {object 0}, {object 1}, {object 2}, ...}
	gets inner brackets information (first byte after { and last byte before }) of object at position iobj
	first is 0
	returns:
	\0 ended string in strObj
	function returns lenght of strObj (0 if not found)
*/
/*
int jsonGetObj(char*ptr, unsigned int len, char*strObj, unsigned int szmax, int iobj)
{
	unsigned int a= 0;
	unsigned int b;
	int count= 0;
	
	strObj[0]= '\0';
	// look for first {
	char c= ptr[a];
	for(; a<len && c!='{'; c=ptr[++a]);
	if(a<len) 
	{
		a++;
		do
		{
			c= ptr[a];
			for(; a<len && c!='{'; c=ptr[++a]);
			if(a < len)
			{
				int brackets=0;
				a++;
				b= a;
				c= ptr[b];
				for(; b<len; )
				{
					if(c=='{') brackets ++;
					if(c=='}')
					{
						if(brackets) brackets --;
						else break;
					}
					c= ptr[++b];
				}
				if(b < len)
				{
					b--;
					//fprintf(stdout, "\nObject len = %d\n", b-a+1);
					//for(unsigned int i=a; i<=b; i++) fprintf(stdout, "%c", ptr[i]);
					//fprintf(stdout, "\n--------------------------------");
					if(count == iobj)
					{
						unsigned int i=0;
						for(; i<(szmax-1) && i<(unsigned int)(b-a+1); i++)
							strObj[i]= ptr[a + i];
						strObj[i]= '\0';
						return (b - a + 1);
					}
					count ++;
					a= b + 1;
				}
			}
		} while(a<len && b<len);
	}
	return 0;
}
*/


/* 
	deprecated ????
	Replace by jsonParseValue  ????
*/
/*
char* jsonParseName(const char *pname, char*ptr, unsigned int pos0, unsigned int len,  char*pvalue, size_t max)
{	
	// PARSE JSON
	unsigned int a= pos0, b=0;
	char c;
	pvalue[0]= '\0';
	while(a < (pos0 + len))
	{
		b= a;
		for(c= ptr[b]; b<(pos0 + len)  && c!=','; c=ptr[++b]);
		if(b > a)
		{
			// item -> name:value
			char name[512];
			char value[512];
			unsigned int i, j;
			// name
			for(i= a, j=0; ptr[i]!=':' && i<b && j<(sizeof(name)-1); i++, j++) name[j]= ptr[i];
			name[(i<b)? j : 0]= '\0';
			i++;
			// value
			for(j=0; i<b && j<(sizeof(value)-1); i++, j++) value[j]= ptr[i];
			value[j]= '\0';
			cstr_replace(name,'"','\0');
			//
			//fprintf(stdout, "--- %s = %s\n", name, value);
			//
			if (strcmp(name, pname)==0 )
			{
				snprintf(pvalue, max,"%s",value);
				return pvalue;
			}
		}
		// next
		a= b;
		for(c= ptr[a]; a<(pos0 + len) && c!=','; c=ptr[++a]);
		a++;
	} 
	return pvalue;
}
*/

/*

e.g. (1)
	"context":{"id":"01GE6KATDN0S5DQ90PMFKP7D77","parent_id":null,"user_id":null}
	for input name= context
	return a c-string in "value" with the 
	value between brackets including the open&close brackets,i.e.,:
	{"id":"01GE6KATDN0S5DQ90PMFKP7D77","parent_id":null,"user_id":null}

e.g. (2)
	"entity_id":"sun.sun"
	for input name= entiry_id
	return a c-string in variable <value> with the value "sun.sun" including the quotation marks
*/
char* jsonParseValue(const char *name, char*ptr, unsigned int pos0, unsigned int len,  char* value, size_t value_sz_max)
{	
	// PARSE JSON
	unsigned int a= pos0, b=0;
	char c;
	value[0]= '\0';
	
	// move after first { or spaces
	for(c= ptr[a]; a<(pos0 + len) && (c== ' ' || c == '{'); c=ptr[++a]);	
	unsigned int i, j;
	char str[512];
	while(a < (pos0 + len))
	{
		//b= pos0 + len;
		// get next name, look for colon symbol <:>	
		for(i= a, j=0; ptr[i]!=':' && i<(pos0 + len) && j<(sizeof(str)-1); i++, j++) str[j]= ptr[i];
		str[(i<(pos0 + len))? j : 0]= '\0';
		
		i++; 
		// <i> is pointing for the character after colon symbol <:>
		// remove quotation marks "
		cstr_replace(str,'"','\0');
		//fprintf(stdout, "\n--- %s  %c", str, ptr[i]);
		
		// is the name I am searching?
		if (strcmp(str, name)==0 )
		{
			
			b= i;
			// position b at the end
			int brackets= 0;
			for(c= ptr[b]; b<(pos0 + len) ; c=ptr[++b])
			{
				if( c == '{') brackets ++;
				else if( c == '}') 
				{	
					if(brackets == 1) {b++; break;}			
					if(brackets == 0) {break;}
					brackets--;
				}
				else if( c == ',' && brackets== 0) break;
			}
			// b points to the end: to , or }		
			//b++;			
			

			//fprintf(stdout, " --- %d / %d", i, b);
				
			// value
			for(j=0; i<b && j<value_sz_max; i++, j++) value[j]= ptr[i];
			value[j]= '\0';			
			
			//fprintf(stdout, " --- %s", value);		
			
			return value;
		}
		
		// next
		for(c= ptr[i]; i<(pos0 + len) && c!=',' && c!='{'; c=ptr[++i]);
		i++;
		a= i;
	} 
	return value;
} // jsonParseValue

// Dump JSON data in a tabulated structure format
// if n==0 data is assumed to be a zero-ended char string
void jsonDump(const char *json, unsigned int n)
{
	int tab=0;
	size_t m= (n==0) ? strlen(json) : n;
	for(size_t i=0; i<m; i++)
	{
		char c= json[i];
		if(c=='}') { if(tab) tab--; fprintf(stdout, "\r\n");for(int j=0; j<tab; j++) fprintf(stdout, "   ");fprintf(stdout, "%c", c);}
		else if(c=='{') {  
			fprintf(stdout, "\r\n");for(int j=0; j<tab; j++) fprintf(stdout, "   ");
			fprintf(stdout, "%c", c);
			tab++;
			fprintf(stdout, "\r\n");for(int j=0; j<tab; j++) fprintf(stdout, "   ");
		}
		else fprintf(stdout, "%c", c);
		if(c==',') { fprintf(stdout, "\r\n");}
		if(c==',') {
			for(int j=0; j<tab; j++) fprintf(stdout, "   ");
		}
	}
} // jsonDump()

// END OF FILE