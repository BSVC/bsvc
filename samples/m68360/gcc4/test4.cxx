/*
 * test4.cxx
 * 
 * DJ (c) 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

extern "C"
{	
	extern void DUART_INIT(void);
	extern void DUART_PUTC(char c);
	extern char DUART_GETC(void);
}


/*
 * Function: puts (print a string)
 */
void puts(const char*s)
{
	while(*s) DUART_PUTC(*s++);
}

/*
 * Class CHello
 */
class CHello
{
 public:	
	CHello(char* who);
	void Print() const;
	
 protected:
	char*m_who;
};

/*
 * Constructor CHello
 */
CHello::CHello(char *who)
{
	DUART_INIT();
	m_who=who;
}

/*
 * Method CHello::Print
 */
void CHello::Print() const
{
	puts("Hello world and ");
	puts(m_who);
	puts("\n");   	     
}

/*
 * Operator new (if not overloaded, need 'malloc')
 */
#if 1
void *operator new(int s)
{
	// one shot operator!
	static zone[4];
	
	return (void*)zone;
}
#endif

/*
 * Function main
 */
int main(void)
{
	CHello c("C++ classes");
	c.Print();

#if 1
	// to see what is needed for 'new' operator
	CHello *pc;
	
	pc=new CHello("dynamic");
	pc->Print();
#endif	
	
	while(1);
	return 0;
}
