/******************************************************************************
 *
 * Handle INCLUDE directives in the source code
 *
 ******************************************************************************
 * $Id: include.c,v 1.1 1996/08/02 14:42:45 bwmott Exp $
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char* buildCompleteSourceFile(FILE* currentFile, char* currentFileName, 
    FILE* completeFile, int level)
{
  FILE* includeFile;

  char buffer[2048], directive[1024], operand[1024], *error;

  /* Make sure we don't loop forever */
  if(level > 15)
    return("Too many nested INCLUDEs\n");

  while(1)
  {
    fgets(buffer,sizeof(buffer),currentFile);
    if(feof(currentFile))
      break;
   
    if(sscanf(buffer,"%s %s", directive, operand) == 2)
    {
      int i;

      for(i = 0; i < strlen(directive); ++i)
        directive[i] = tolower(directive[i]);

      if(strcmp(directive,"include") == 0)
      {
        includeFile = fopen(operand,"r");
        if(!includeFile)
        {
          char* tmp = (char*)malloc(256);
          sprintf(tmp,"Could not find INCLUDE file: %s\n  for file: %s\n",
              operand, currentFileName);
          return(tmp);
        }
        
        error = buildCompleteSourceFile(includeFile, operand, completeFile,
            level + 1);

        fclose(includeFile);

        if(error)
          return(error);
      }
      else
      {
        fprintf(completeFile,"%s", buffer);
      }
    }
    else
    {
        fprintf(completeFile,"%s", buffer);
    }
  }
  return(NULL);
}


