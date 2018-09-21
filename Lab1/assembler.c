#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>

FILE *infile = NULL;
FILE *outfile = NULL;

int main(int argc, char* argv[]) {
	printf("---START---\n");

	// parse the command line arguments
	char *prgName = NULL;
	char *iFileName = NULL;
	char *oFileName = NULL;

	prgName = argv[0];
	iFileName = argv[1];
	oFileName = argv[2];

	printf("program name = '%s'\n", prgName);
	printf("input file name = '%s'\n", iFileName);
	printf("output file name = '%s'\n", oFileName);

	// open the input and output files
	infile = fopen(argv[1], "r");
	outfile = fopen(argv[2], "w");
	
	if (!infile) {
		printf("Error: Cannot open file %s\n", argv[1]);
		exit(4);
	}
	if (!outfile) {
		printf("Error: Cannot open file %s\n", argv[2]);
		exit(4);
	}

	/* TODO: Translate the input file and write to the output file */

	fclose(infile);
	fclose(outfile);

	return 0;
}

/* check if given string is a valid opcode
	Valid Opcodes: ADD, AND, BR(all variations), HALT, JMP, JSRR, LDB, LDW, LEA,
				 NOP, NOT, RET, LSHF, RSHFL, RSHFA, RTI, STB, STW, TRAP, XOR
*/

bool isOpcode( const char * lPtr ) {

	// lPtr[] always lowercase
	if ( strcmp(lPtr, "add"))
		return true;
	if ( strcmp(lPtr, "and"))
		return true;

	// TODO: add BR codes

	if ( strcmp(lPtr, "halt"))
		return true;
	if ( strcmp(lPtr, "jmp"))
		return true;
	if ( strcmp(lPtr, "jsrr"))
		return true;
	if ( strcmp(lPtr, "ldb"))
		return true;
	if ( strcmp(lPtr, "ldw"))
		return true;
	if ( strcmp(lPtr, "lea"))
		return true;
	if ( strcmp(lPtr, "nop"))
		return true;
	if ( strcmp(lPtr, "not"))
		return true;
	if ( strcmp(lPtr, "ret"))
		return true;
	if ( strcmp(lPtr, "lshf"))
		return true;
	if ( strcmp(lPtr, "rshfl"))
		return true;
	if ( strcmp(lPtr, "rshfa"))
		return true;
	if ( strcmp(lPtr, "rti"))
		return true;
	if ( strcmp(lPtr, "stb"))
		return true;
	if ( strcmp(lPtr, "stw"))
		return true;
	if ( strcmp(lPtr, "trap"))
		return true;
	if ( strcmp(lPtr, "xor"))
		return true;


	return false;
}

/* Converts a string into a hex or decimal number */
int toNum( char * pStr ) {
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
		pStr++;
		if( *pStr == '-' )				/* dec is negative */
		{
			lNeg = 1;
			pStr++;
		}

		t_ptr = pStr;
		t_length = strlen(t_ptr);
		for(k=0;k < t_length;k++)
		{
			if (!isdigit(*t_ptr))
			{
				printf("Error: invalid decimal operand, %s\n",orig_pStr);
				exit(4);
			}
			t_ptr++;
		}

		lNum = atoi(pStr);
		if (lNeg)
			lNum = -lNum;

		return lNum;
   }
   else if( *pStr == 'x' )	/* hex     */
   {
		pStr++;
		if( *pStr == '-' )				/* hex is negative */
		{
			lNeg = 1;
			pStr++;
		}

		t_ptr = pStr;
		t_length = strlen(t_ptr);
		for(k=0;k < t_length;k++)
		{
			if (!isxdigit(*t_ptr))
			{
				printf("Error: invalid hex operand, %s\n",orig_pStr);
				exit(4);
			}
			t_ptr++;
		}

		lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
		lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
		if( lNeg )
			lNum = -lNum;

		return lNum;
   }
   else
   {
		printf( "Error: invalid operand, %s\n", orig_pStr);
		exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

/* Take a line from the input file and parse it into its corresponding section
	(i.e. LABEL, OPCODE, OPERANDS, COMMENTS)
	*/
#define MAX_LINE_LENGTH 255
enum
{
	DONE, OK, EMPTY_LINE
};

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
)
{
	char * lRet, * lPtr;
	int i;
	if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
		return( DONE );
	for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = tolower( pLine[i] );

	/* convert entire line to lowercase */
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

	/* ignore the comments */
	lPtr = pLine;

	while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' ) 
		lPtr++;

	*lPtr = '\0';
	if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
		return( EMPTY_LINE );

	if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
	{
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	}

	*pOpcode = lPtr;
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg1 = lPtr;
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg2 = lPtr;
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg3 = lPtr;
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg4 = lPtr;

	return( OK );
}
/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */

