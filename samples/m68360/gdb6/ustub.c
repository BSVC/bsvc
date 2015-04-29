/*
 * ustub.c
 * 
 * Utilities functions for gdb stub
 *
 * DJ, July 96.
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#include "ustub.h"


/*=====================================================================*/
const char hexchars[]="0123456789abcdef";
/*=====================================================================*/

/*=====================================================================*/
int hex(char ch)
/*=====================================================================*/
{
  if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
  if ((ch >= '0') && (ch <= '9')) return (ch-'0');
  if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
  return (-1);
}


/*=====================================================================*/
int hexToInt(char **ptr, int *intValue)
/*=====================================================================*/
{
    int numChars = 0;
    int hexValue;
    
    *intValue = 0;

    while (**ptr)
    {
        hexValue = hex(**ptr);
        if (hexValue >=0)
        {
            *intValue = (*intValue <<4) | hexValue;
            numChars ++;
        }
        else
            break;
        
        (*ptr)++;
    }

    return (numChars);
}

/*=====================================================================*/
char* mem2hex(char* mem, char* buf, int count)
/*=====================================================================*/
{
	/* convert the memory pointed to by mem into hex, placing result in buf
	 * return a pointer to the last char put in buf (null) 
	 */
	int i;
	unsigned char ch;
	
	for (i=0;i<count;i++) 
	{
		ch = *mem++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch % 16];
	}
	*buf = 0; 
	return(buf);
}

/*=====================================================================*/
char* hex2mem(char *buf, char *mem, int count)
/*=====================================================================*/
{
	/* convert the hex array pointed to by buf into binary to be placed 
	 * in mem. return a pointer to the character AFTER the last byte 
	 * written.
	 */
	int i;
	unsigned char ch;
	
	for (i=0;i<count;i++) 
	{
		ch = hex(*buf++) << 4;
		ch = ch + hex(*buf++);
		*mem++ = ch;
	}
	return(mem);
}
