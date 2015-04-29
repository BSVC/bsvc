/*
 * libc.c
 * 
 * DJ (c) 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#include "libc.h"

/* from duart.s */
extern void DUART_INIT(void);
extern void DUART_PUTC(char c);
extern char DUART_GETC(void);

/*-----------------------------------------------------*/
void putchar(char c)
/*-----------------------------------------------------*/
{
	DUART_PUTC(c);
}

/*-----------------------------------------------------*/
void puts(const char *s)
/*-----------------------------------------------------*/
{
	while(*s)
	  DUART_PUTC(*s++);
}

/*-----------------------------------------------------*/
void gets(char *s)
/*-----------------------------------------------------*/
{
	char c;
	do
	{
		c=DUART_GETC();
		DUART_PUTC(c);
		*s++ = c;
	}
	while(c != '\n');
	*--s = 0;
}

/*-----------------------------------------------------*/
void *memset(void* addr, int value, int length)
/*-----------------------------------------------------*/
{
	int i;
	for(i=0;i<length;i++) 
	  ((char*)addr)[i] = (char)value;
	return addr;
}

/*-----------------------------------------------------*/
void bcopy(char *b1, char*b2, int length)
/*-----------------------------------------------------*/
{
	int i;
	for(i=0;i<length;i++)
	{
		b2[i]=b1[i];
	}		
}

/*-----------------------------------------------------*/
char *strcpy(char* s1, const char *s2)
/*-----------------------------------------------------*/
{
	char *s = s1;
	while(*s2) *s1++=*s2++;
	*s1=0;
	return s;
}

/*-----------------------------------------------------*/
int strlen(const char *s)
/*-----------------------------------------------------*/
{
	int l;
	l=0;
	while(*s++) l++;
	return l;
}

/*-----------------------------------------------------*/
int strcmp(const char *s1, const char *s2)
/*-----------------------------------------------------*/
{
	while((*s1) && (*s2) && (*s1==*s2))
	{
		s1++;
		s2++;
	}
	return (*s1!=*s2);
}

/*-----------------------------------------------------*/
int strncmp(const char *s1, const char *s2, int count)
/*-----------------------------------------------------*/
{
	int i=0;
	while((*s1) && (*s2) && (*s1==*s2) && (i<count))
	{
		s1++;
		s2++;
		i++;
	}
	if(i==count)
	  return 0;
	return 1;
}

/*-----------------------------------------------------*/
int atoi(const char *s)
/*-----------------------------------------------------*/
{
	int result=0;
	int sign=1;
	
	while((*s) && ((*s==' ')||(*s=='\t')))
	{
		s++;
	}
	if((*s)&&(*s=='+'))
	{
		s++;
	}
	if((*s)&&(*s=='-'))
	{
		s++;
		sign=-1;
	}
	
	if((*s)&&(*s=='$'))
	{
		s++;
		while((*s) && (((*s>='0') && (*s<='9')) || ((*s>='a') && (*s<='f')) ||
		  ((*s>='A') && (*s<='F'))))
		{
			if((*s>='0') && (*s<='9'))
			  result=16*result+(*s-'0');
			else if((*s>='a') && (*s<='f'))
			  result=16*result+(*s-'a'+10);
			else if((*s>='A') && (*s<='F'))
			  result=16*result+(*s-'A'+10);
			s++;
		}	
	}
	else
	{
		while((*s) && (*s>='0') && (*s<='9'))
		{
			result=10*result+(*s-'0');
			s++;
		}	
	}

#ifdef mc68000	
	/* lacking correct '___mulsi3'... functions */	
	if(sign==-1)
	  result = -result;
	return result;
#else
	return sign*result;
#endif	
}

/*-----------------------------------------------------*/
void itoa(unsigned short value, char* s)
/*-----------------------------------------------------*/
{
	char tmp[16];
	unsigned short index=0;
	unsigned short max;
	
	if(value==0)
	{
		s[0]='0';
		s[1]=0;
		return;
	}
	
	while(value!=0)
	{
		tmp[index++]='0'+value%10;
		value/=10;		
	}
	max=index;
	while(index)
	{
		s[max-index]=tmp[index-1];
		index--;
	}		
	s[max]=0;	
}

/*-----------------------------------------------------*/
void itox(unsigned short value, char* s)
/*-----------------------------------------------------*/
{
	char tmp[16];
	unsigned short index=0;
	unsigned short max;
	unsigned short v;
	
	if(value==0)
	{
		s[0]='0';
		s[1]=0;
		return;
	}
	
	while(value!=0)
	{
		v=value%16;
		if(v<10)
		  tmp[index++]='0'+v;
		else
		  tmp[index++]='a'+(v-10);		  
		value/=16;		
	}
	max=index;
	while(index)
	{
		s[max-index]=tmp[index-1];
		index--;
	}		
	s[max]=0;	
}


/*
 * End of file
 */
