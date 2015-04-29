/*
 * test6.c
 * 
 * DJ (c) 1996
 * 
 * USUAL DISCLAIMER: THIS EXAMPLE IS PROVIDED AS IS 
 * WITH NO WARRANTY FOR ANY PURPOSE.
 */

#if defined(__GNUC__)
#include <libc.h>

#define assert(_x_) _assert((_x_),__FILE__,__LINE__)
extern void _assert(int,const char*,int);
#else
#include <stdio.h>
#include <assert.h>
extern itoa(unsigned short value, char* s);
#endif

#if defined(__GNUC__) && defined(DEBUG)
extern void breakpoint(void);
extern void set_debug_traps(void);
#endif

/*---------------------------------------------------------------*/
void test_arithmetic(void)
/*---------------------------------------------------------------*/
{
	register int i,j;
	
	puts("--- testing arithmetic\n");
	
	i=0;
	j=0;
	assert(i+j==0);
	assert(i-j==0);
	assert(i*j==0);
	assert(i/2==0);

	i=1;
	j=1;
	assert(i+j==2);
	assert(i-j==0);
	assert(i*j==1);
	assert(i/2==0);
	assert(i%2==1);

	i=2;
	j=2;
	assert(i+j==4);
	assert(2*i-j==2);
	assert(i*j==4);
	assert(i/2==1);
	assert(i%2==0);

	i=179;
	j=10;
	assert(i*j==1790);
	assert(i/j==17);
	assert(i%j==9);

	i=1;
	j=-1;
	assert(i+j==0);
	assert(i-j==2);
	assert(i*j==-1);
	assert(j*j==1);
	
	i=30000;
	j=-5;
	assert(i*j==-150000);
	assert(i/j==-6000);
	assert(i%j==0);
	
	i=-179;
	j=10;
	assert(i*j==-1790);
	assert(i/j==-17);
	assert(i%j==-9);	
}


/*---------------------------------------------------------------*/
void test_itoa(void)
/*---------------------------------------------------------------*/
{
	char s[12];
	/* register int i; */
	
	puts("--- testing itoa\n");

	itoa(0,s);
	assert(s[0]=='0');
	assert(s[1]==0);

	itoa(1,s);
	assert(s[0]=='1');
	assert(s[1]==0);
	
	itoa(10,s);
	assert(s[0]=='1');
	assert(s[1]=='0');
	assert(s[2]==0);
}

/*---------------------------------------------------------------*/
void test_array_i(char *out_a)
/*---------------------------------------------------------------*/
{
	out_a[0] = 0;
	out_a[1] = 1;
	out_a[2] = 2;
	out_a[3] = 3;		
}

/*---------------------------------------------------------------*/
void test_array(void)
/*---------------------------------------------------------------*/
{
	static char ca1[12];
	int ia1[12];
	char *cptr;
	int *iptr;
	register int i;
	char ca2[4];
	
	puts("--- testing arrays\n");
	for(i=0;i<12;i++)
	{
		ca1[i]=i;
	}
	cptr=(char*)ca1;
	assert(*cptr++==0x0);
	assert(*cptr++==0x1);
	assert(*cptr++==0x2);
	assert(*cptr++==0x3);
	for(i=0;i<12;i++)
	{
		assert(ca1[i]==i);
	}
	
	assert(sizeof(ia1)==12*sizeof(int));
	for(i=0;i<12;i++)
	{
		ia1[i]=0x1000000*i;
		/*ia1[i]=0xcafe;*/
	}
	iptr=(int*)ia1;
	assert(*iptr++==0);
	assert(*iptr++==0x1000000);
	assert(*iptr++==0x2000000);
	for(i=0;i<12;i++)
	{
		assert(ia1[i]==0x1000000*i);
	}
	
	test_array_i(ca2);
	assert(ca2[0]==0);
	assert(ca2[1]==1);
	assert(ca2[2]==2);
	assert(ca2[3]==3);
	
}

/*---------------------------------------------------------------*/
void test_switch(void)
/*---------------------------------------------------------------*/
{
	int i;
	char c;
			
	puts("--- testing switch\n");

	switch(1)
	{
	 case 1: break;
	 default: assert(0); break;
	}
	
	for(i=0;i<100;i+=10)
	{
		switch(i)
		{
		 case 0: 
			assert(i==0);
			break;
		 case 10: 
			assert(i==10);
			break;
		 case 20: 
			assert(i==20);
			break;
		 case 30: 
			assert(i==30);
			break;
		 case 40: 
			assert(i==40);
			break;
		 case 50: 
			assert(i==50);
			break;
		 case 60: 
			assert(i==60);
			break;
		 case 70: 
			assert(i==70);
			break;
		 case 80: 
			assert(i==80);
			break;
		 case 90: 
			assert(i==90);
			break;
			
		 default:
			assert(0);
			break;
		}
	} /* end of for */

	for(c='a';c<='j';c++)
	{
		switch(c)
		{
		 case 'a': 
			assert(c=='a');
			break;
		 case 'b': 
			assert(c=='b');
			break;
		 case 'c': 
			assert(c=='c');
			break;
		 case 'd':
			assert(c=='d');
			break;
		 case 'e': 
			assert(c=='e');
			break;
		 case 'f': 
			assert(c=='f');
			break;
		 case 'g': 
			assert(c=='g');
			break;
		 case 'h': 
			assert(c=='h');
			break;
		 case 'i': 
			assert(c=='i');
			break;
		 case 'j': 
			assert(c=='j');
			break;
			
		 default:
			puts("switch failed on character: ");
			putchar(c);
			puts("\n");
			assert(0);
			break;
		}
	} /* end of for */

	
}

/*---------------------------------------------------------------*/
void test_if(void)
/*---------------------------------------------------------------*/
{
	int i;
			
	puts("--- testing if\n");
	i=0;
	if(i==0)
	{
		i=1;
	}
	else
	{
		assert(0);
	}
	if(i==1)
	{
		i=2;
	}
	else
	{
		assert(0);
	}
	if((i>1)&&(i<3))
	{
		i=3;
	}
	else
	{
		assert(0);
	}
	if((i<4)||(i==99))
	{
		/* second expression not tested. */
		i=4;
	}
	else
	{
		assert(0);
	}
	if((i<0)&&(i==4))
	{
		/* second expression not tested. */
		assert(0);
	}
	else
	{
		i=5;
	}
		
	
}

/*---------------------------------------------------------------*/
void test_bit_operations(void)
/*---------------------------------------------------------------*/
{
	long i;
	unsigned long u, v;
	puts("--- testing bit operations\n");
	
	i=0;
	assert((~i)==-1);
	
	u=0xFF;
	assert((~u)==0xFFFFFF00);
	
	i=-1;
	assert(((unsigned long)i)==0xFFFFFFFF);
	
	u=2;
	assert((u>>1)==1);
	assert((u>>2)==0);
	assert((u<<1)==4);
	assert((u<<2)==8);

	u=0xFF;
	assert((u<<8)==0xFF00);
	
	u=0xFF;
	v=0xFF0;
	assert((u^v)==0xF0F);
	
}

typedef struct
{
	unsigned char one;
	char two;
	unsigned short three;
	short four;
	unsigned long five;
	long six;
	char seven[7];
	short eight[8];
} T_S;

/*---------------------------------------------------------------*/
void test_struct_i1(T_S s)
/*---------------------------------------------------------------*/
{
	assert(s.one==1);
	assert(s.two==-2);
	assert(s.three==0x3333);
	assert(s.four==-4444);
	assert(s.five==0xF555555);
	assert(s.six==-666666);
	assert(s.seven[0]==7);
	assert(s.seven[1]==77);
	assert(s.seven[2]==-7);
	assert(s.eight[0]==8);
	assert(s.eight[1]==88);
	assert(s.eight[2]==888);
	assert(s.eight[3]==8888);		
}

/*---------------------------------------------------------------*/
void test_struct_i2(T_S *p_s)
/*---------------------------------------------------------------*/
{
	assert(p_s->one==1);
	assert(p_s->two==-2);
	assert(p_s->three==0x3333);
	assert(p_s->four==-4444);
	assert(p_s->five==0xF555555);
	assert(p_s->six==-666666);
	assert(p_s->seven[0]==7);
	assert(p_s->seven[1]==77);
	assert(p_s->seven[2]==-7);
	assert(p_s->eight[0]==8);
	assert(p_s->eight[1]==88);
	assert(p_s->eight[2]==888);
	assert(p_s->eight[3]==8888);		
}

/*---------------------------------------------------------------*/
void test_struct_i3(T_S *p_s)
/*---------------------------------------------------------------*/
{
	p_s->one=10;
	p_s->two=20;
	p_s->three=30;
	p_s->four=40;
	p_s->five=50;
	p_s->six=60;
	p_s->seven[0]=70;
	p_s->seven[1]=70;
	p_s->seven[2]=70;
	p_s->eight[0]=80;
	p_s->eight[1]=80;
	p_s->eight[2]=80;
	p_s->eight[3]=80;
}

#if 0
/*---------------------------------------------------------------*/
void test_struct_i4(const T_S *p_s)
/*---------------------------------------------------------------*/
{
	/* test 'const' --> OK */
	p_s->one=10;
}
#endif
	
	
/*---------------------------------------------------------------*/
void test_struct(void)
/*---------------------------------------------------------------*/
{
	T_S s;
	
	puts("--- testing struct\n");

	s.one=1;
	s.two=-2;
	s.three=0x3333;
	s.four=-4444;
	s.five=0xF555555;
	s.six=-666666;
	s.seven[0]=7;
	s.seven[1]=77;
	s.seven[2]=-7;
	s.eight[0]=8;
	s.eight[1]=88;
	s.eight[2]=888;
	s.eight[3]=8888;

	assert(s.one==1);
	assert(s.two==-2);
	assert(s.three==0x3333);
	assert(s.four==-4444);
	assert(s.five==0xF555555);
	assert(s.six==-666666);
	assert(s.seven[0]==7);
	assert(s.seven[1]==77);
	assert(s.seven[2]==-7);
	assert(s.eight[0]==8);
	assert(s.eight[1]==88);
	assert(s.eight[2]==888);
	assert(s.eight[3]==8888);
	
	
	test_struct_i1(s);
	test_struct_i2(&s);
	test_struct_i3(&s);

	assert(s.one==10);
	assert(s.two==20);
	assert(s.three==30);
	assert(s.four==40);
	assert(s.five==50);
	assert(s.six==60);
	assert(s.seven[0]==70);
	assert(s.seven[1]==70);
	assert(s.seven[2]==70);
	assert(s.eight[0]==80);
	assert(s.eight[1]==80);
	assert(s.eight[2]==80);
	assert(s.eight[3]==80);

}

typedef struct
{
	unsigned bit0:1;
	unsigned bit1:1;
	unsigned bit2:1;
	unsigned bit3_4:2;
	unsigned bit5_7:3;
	unsigned bit8_15:8;
	unsigned bit16_31:16;
} T_BF;

/*---------------------------------------------------------------*/
void test_bit_field(void)
/*---------------------------------------------------------------*/
{
	T_BF data;
	
	puts("--- testing bit fields\n");

	data.bit0=0;
	data.bit1=0;
	data.bit2=0;
	data.bit3_4=0;
	data.bit5_7=0;
	data.bit8_15=0;
	data.bit16_31=0;

	assert(data.bit0==0);
	assert(data.bit1==0);
	assert(data.bit2==0);
	assert(data.bit3_4==0);
	assert(data.bit5_7==0);
	assert(data.bit8_15==0);
	assert(data.bit16_31==0);
	
	data.bit0=1;
	assert(data.bit0==1);
	assert(data.bit1==0);

	data.bit1=1;
	assert(data.bit0==1);
	assert(data.bit1==1);

	data.bit0=0;
	data.bit1=0;
	assert(data.bit0==0);
	assert(data.bit1==0);

	data.bit3_4=2;
	assert(data.bit0==0);
	assert(data.bit1==0);
	assert(data.bit2==0);
	assert(data.bit3_4==2);
	assert(data.bit5_7==0);

	data.bit3_4=3;
	assert(data.bit0==0);
	assert(data.bit1==0);
	assert(data.bit2==0);
	assert(data.bit3_4==3);
	assert(data.bit5_7==0);

	data.bit16_31=0xabcd;
	assert(data.bit0==0);
	assert(data.bit1==0);
	assert(data.bit2==0);
	assert(data.bit3_4==3);
	assert(data.bit5_7==0);
	assert(data.bit8_15==0);
	assert(data.bit16_31==0xabcd);

}

jmp_buf context;
/*--------------------------------------------------*/
void j1()
/*--------------------------------------------------*/
{
	/* puts("jumping..."); */
	longjmp(context,1);
}
/*--------------------------------------------------*/
void j2()
/*--------------------------------------------------*/
{
	/* puts("jumping..."); */
	longjmp(context,0x12000);
}

/*---------------------------------------------------------------*/
void test_jump()
/*---------------------------------------------------------------*/
{
	int v,w;
	int a=0;
	int b=12;
	register unsigned long r1=0xC0CAC0CA;
	register unsigned short r2=0xCAFE;

	puts("--- testing jump\n");
	
	v=setjmp(context);
	if(v==0)
	{
		/* puts("normal path <1> OK\n"); */
		j1();
	}
	else if(v==1)
	{
		/* puts(" jump <1> OK\n"); */
		assert(a==0);
		assert(b==12);
		assert(r1==0xC0CAC0CA);
		assert(r2==0xCAFE);
		
		/* jumping again */
		w=setjmp(context);
		if(w==0)
		{
			/* puts("normal path <2> OK\n"); */
			j2();
		}		
		else if(w==0x12000)
		{
			/* puts(" jump <2> OK\n"); */
			assert(a==0);
			assert(b==12);
			assert(r1==0xC0CAC0CA);
			assert(r2==0xCAFE);
		}
		else
		{
			puts("ERROR: bad 'longjmp' <2> value\n");
			assert(0);
		}
	}
	else
	{
		puts("ERROR: bad 'longjmp' <1> value\n");
		assert(0);
	}

}

/*---------------------------------------------------------------*/
void main()
/*---------------------------------------------------------------*/
{
	extern void DUART_INIT(void);
	DUART_INIT();
	puts("WELCOME in BSVC-M68K\n");

#if defined(__GNUC__) && defined(DEBUG)
	set_debug_traps();
	breakpoint();
#endif
	
   	test_arithmetic();
   	test_itoa();
	test_array();
	test_switch();
	test_if();
	test_bit_operations();
	test_struct();
	test_bit_field();
	test_jump();
	
	puts("--- ALL TESTS OK\n");
#if defined(__GNUC__)
	while(1);
#endif	
}

#if defined(__GNUC__)
/*---------------------------------------------------------------*/
void _assert(int expr, const char *the_file, int the_line)
/*---------------------------------------------------------------*/
{
	char buf[16];
	if(!expr)
	{
		puts("assert failed in ");
		puts(the_file);
		puts(" ");
		itoa(the_line,buf);
		puts((const char*)buf);
		puts("\n");
		while(1);
	}
}
#endif

