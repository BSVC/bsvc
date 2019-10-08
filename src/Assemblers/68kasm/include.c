//
// Handle INCLUDE directives in the source code
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
buildCompleteSourceFile(FILE *currentFile, char *currentFileName,
			FILE *completeFile, int level)
{
	char buffer[2048], directive[2048], operand[2048], *error;

	/* Make sure we don't loop forever */
	if (level > 15)
		return ("Too many nested INCLUDEs\n");

	while (fgets(buffer, sizeof(buffer), currentFile) != NULL) {
		if (sscanf(buffer, "%s %s", directive, operand) != 2) {
			fprintf(completeFile, "%s", buffer);
			continue;
		}
		for (int i = 0; i < strlen(directive); ++i)
			directive[i] = tolower(directive[i]);
		if (strcmp(directive, "include") == 0) {
			FILE *includeFile = fopen(operand, "r");
			if (!includeFile) {
				const char *fmt =
				    "Could not find INCLUDE file: %s\n"
				    "  for file: %s\n";
				size_t len = strlen(operand) +
				    strlen(currentFileName) +
				    strlen(fmt) + 1;
				char *tmp = malloc(len);
				snprintf(tmp, len, fmt,
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
			continue;
		}
		fprintf(completeFile, "%s", buffer);
	}

	return NULL;
}
