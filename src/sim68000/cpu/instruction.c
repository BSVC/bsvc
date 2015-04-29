/*
 * instruction.c
 *
 * This program reads in an instruction definition file and builds the
 * m68000DecodeTable.hxx file.  It checks the input to make sure that all
 * instruction entries are distinguishable (if not an error is reported).
 *
 * Usage: instruction
 *
 * Sim68000 "Motorola 68000 Simulator"
 * Copyright (c) 1993
 * By: Bradford W. Mott
 * November 3,1993
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INFILE "instruction.list"
#define OUTFILE "m68000DecodeTable.hxx"

typedef struct {
  int mask;
  int signature;
  char *name;
  char *gen;
} Entry;

Entry table[4096];


int Compare(const void *a,const void *b)
{
  /* return( strcmp( ((Entry*)a)->name, ((Entry*)b)->name ) ); */
  return( ((Entry*)a)->signature - ((Entry*)b)->signature );
}

unsigned int BinaryToInt(char *binary)
{
  int t,v;

  v=0;
  for(t=0;t<strlen(binary);++t)
  {
    v=v<<1;
    if(binary[t]=='1')
      v=v|1;
  }
  return(v);
}

int IsNotDistinct(char *a,char *b)
{
  int t,len;
  char ta[80],tb[80];

  len=strlen(a);
  if(strlen(b)<len)
    len=strlen(b);

  strcpy(ta,a);
  strcpy(tb,b);
  for(t=0;t<len;++t)
  {
    if((ta[t]!='0') && (ta[t]!='1'))
      tb[t]=ta[t];
    else if((tb[t]!='0') && (tb[t]!='1'))
      ta[t]=tb[t];
  }

  for(t=0;t<len;++t)
    if(ta[t]!=tb[t])
      return(0);

  return(1);
}

main()
{
  FILE *fp;
  int num_of_entries;
  char input[320],name[80],gen[80];
  char mask[80],signature[80];
  int t,s;

  fp=fopen(INFILE,"r");

  /* Read in table of instruction generators and names */
  printf("\nReading '%s' file...\n",INFILE);
  num_of_entries=0;
  while(fgets(input,319,fp)!=NULL)
  {
     t=sscanf(input,"%s %s",gen,name);
     if(t==2)
     {
       table[num_of_entries].name=(char*)malloc(strlen(name)+1);
       table[num_of_entries].gen=(char*)malloc(strlen(gen)+1);
       strcpy(table[num_of_entries].name,name);
       strcpy(table[num_of_entries].gen,gen);
       ++num_of_entries;
     }
     else if(t>0)
     {
       printf("Input Error: %d (%s)\n",t,input);
       exit(-1);
     }
  }
  fclose(fp);

  /* Compute the mask and signature values from the generator string */
  printf("Computing mask and signature values...\n");
  for(t=0;t<num_of_entries;++t)
  {
    for(s=0;s<strlen(table[t].gen);++s)
    {
      if((table[t].gen[s]=='0') || (table[t].gen[s]=='1'))
      {
        mask[s]='1';
        signature[s]=table[t].gen[s];
      }
      else
      {
        mask[s]='0';
        signature[s]='0';
      }
    }
    mask[s]='\0';
    signature[s]='\0';

    table[t].mask = BinaryToInt(mask);
    table[t].signature = BinaryToInt(signature);
  }
  
  /* Make sure all entries are distinct */
  printf("Checking distinguishability...\n");
  for(t=0;t<num_of_entries;++t)
  {
    for(s=0;s<num_of_entries;++s)
    {
      if(s!=t)
      {
        if(IsNotDistinct(table[t].gen,table[s].gen))
        {
          printf("ERROR: Entry ( %s , %s ) is not distinguishable\n",
                 table[t].gen,table[t].name);
          printf("       from entry ( %s , %s )!\n",table[s].gen,table[s].name);
          exit(-1);
        }
      }
    }
  }

  printf("Sorting list...\n");
  qsort((void*)table,num_of_entries,sizeof(Entry),Compare);

  /* Generate the instruction decode table */
  printf("Writing '%s' file...\n\n",OUTFILE);
  fp=fopen(OUTFILE,"w");
  for(t=0;t<num_of_entries;++t)
    if(t!=num_of_entries-1)
      fprintf(fp,"  { 0x%04x, 0x%04x, &m68000::Execute%s },\n",
             table[t].mask, table[t].signature, table[t].name);
    else
      fprintf(fp,"  { 0x%04x, 0x%04x, &m68000::Execute%s }\n",
             table[t].mask, table[t].signature, table[t].name);

  fclose(fp);
  exit(0);
}
