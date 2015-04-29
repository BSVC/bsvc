//
// Handle INCLUDE directives in the source code
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
buildCompleteSourceFile(FILE * currentFile, char *currentFileName,
			FILE * completeFile, int level)
{
	FILE *includeFile;

	char buffer[2048], directive[1024], operand[1024], *error;

	/* Make sure we don't loop forever */
	if (level > 15)
		return ("Too many nested INCLUDEs\n");

	for (;;) {
		fgets(buffer, sizeof(buffer), currentFile);
		if (feof(currentFile))
			break;

		if (sscanf(buffer, "%s %s", directive, operand) != 2) {
			fprintf(completeFile, "%s", buffer);
			continue;
		}
		for (int i = 0; i < strlen(directive); ++i)
			directive[i] = tolower(directive[i]);

		if (strcmp(directive, "include") == 0) {
			includeFile = fopen(operand, "r");
			if (!includeFile) {
				char *tmp = (char *)malloc(256);
				snprintf(tmp, 256,
					 "Could not find INCLUDE file: %s\n  for file: %s\n",
					 operand, currentFileName);
				return tmp;
			}

			error = buildCompleteSourceFile(includeFile,
							operand,
							completeFile,
							level + 1);

			fclose(includeFile);

			if (error) {
				return error;
			}
			fprintf(completeFile, "%s", buffer);
		}
	}

	return NULL;
}
