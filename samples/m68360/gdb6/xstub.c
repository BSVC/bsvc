/*
 * Provide the functions needed by gdb stub
 * 
 * DJ, July 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#define GSOCK_ADDRESS 0xe0000

#include "xstub.h"
#include "ustub.h"

/*-----------------------------------------------------*/
void (*exceptionHook)() = 0;

/*-----------------------------------------------------*/
int getDebugChar(void)
/*-----------------------------------------------------*/
{
	char c;
	/* wait until a character is available */
	while( ((*((char*)GSOCK_ADDRESS))&11) != 11 );
	/* read character */
	c=*((char*)GSOCK_ADDRESS+2);
	return c;
}

/*-----------------------------------------------------*/
void putDebugChar(int c)
/*-----------------------------------------------------*/
{
	/* wait until we can send */
	while( ((*((char*)GSOCK_ADDRESS))&0x7) != 0x7 );
	*((char*)(GSOCK_ADDRESS+3)) = c;
}

/*-----------------------------------------------------*/
void exceptionHandler(int num, void *addr)
/*-----------------------------------------------------*/
{
	/* see boot2.s for explanations */
	extern unsigned char exceptionTable[];
	int index=6*(num-2);
	*((unsigned char**)(&(exceptionTable[index])+2)) = (unsigned char*)addr;
}

/*-----------------------------------------------------*/
void flush_i_cache()
/*-----------------------------------------------------*/
{
}


/*-----------------------------------------------------*/
void getpacket(char *buffer)
/*-----------------------------------------------------*/
{
	/* scan for the sequence $<data>#<checksum> */
	unsigned char checksum;
	unsigned char xmitcsum;
	int  i;
	int  count;
	char ch, ch1;
	
	/* char dj_tmp[10]; */
	
	do 
	{
		/* wait around for the start character, ignore all other characters */
		while ((ch = (getDebugChar() & 0x7f)) != '$'); 
		checksum = 0;
		xmitcsum = -1;
		
		count = 0;
		
		/* now, read until a # or end of buffer is found */
		while (count < BUFMAX) 
		{
			ch = getDebugChar() & 0x7f;
			if (ch == '#') break;
			checksum = checksum + ch;
			buffer[count++] = ch;
		}
		buffer[count] = 0;
		if (ch == '#') 
		{
			/* puts("received checksum: "); */
			ch1=getDebugChar() & 0x7f;
			xmitcsum = hex(ch1) << 4;
			/* putchar(ch1); */
			ch1=getDebugChar() & 0x7f;
			xmitcsum += hex(ch1);
			/* putchar(ch1); */
			/* putchar('\n'); */
      
			if (checksum != xmitcsum)
			{
				putDebugChar('-');  /* failed checksum */
			}
			else 
			{
				putDebugChar('+');  /* successful transfer */
				/* if a sequence char is present, reply the sequence ID */
				if (buffer[2] == ':') 
				{
					putDebugChar( buffer[0] );
					putDebugChar( buffer[1] );
					/* remove sequence chars from buffer */
					count = strlen(buffer);
					for (i=3; i <= count; i++) buffer[i-3] = buffer[i];
				}				
			} 
		} 
	} 
	while (checksum != xmitcsum);  
}



/*-----------------------------------------------------*/
void putpacket(char *buffer)
/*-----------------------------------------------------*/
{
	/* send the packet in buffer.  The host get''s one chance to read it.  
     * This routine does not wait for a positive acknowledge.  
	 */
	unsigned char checksum;
	int  count;
	char ch;

	/*  $<packet info>#<checksum>. */
	putDebugChar('$');
	checksum = 0;
	count    = 0;
	
	while ((ch=buffer[count])) 
	{
		putDebugChar(ch&0x7f);
		checksum += ch&0x7f;
		count += 1;
	}
	
	putDebugChar('#');
	putDebugChar(hexchars[checksum >> 4]);
	putDebugChar(hexchars[checksum % 16]);
}

					
