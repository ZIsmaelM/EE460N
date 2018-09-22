#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>

#define MAX_LINE_LENGTH 255
enum
{
	DONE, OK, EMPTY_LINE
};

bool isEmpty( char * stringPtr )
{
	if(stringPtr[0] == '\0')
		return true;

	return false;
}

int getOpcodeInt(char * opcode)
{
	// opcode[] always lowercase
	if ( strcmp(opcode, "add"))
		return 1;	// ??????
	if ( strcmp(opcode, "and"))
		return 5;	// ??????

	// TODO: add BR codes

	if ( strcmp(opcode, "halt"))
		return 15;
	if ( strcmp(opcode, "jmp"))
		return 12;
	if ( strcmp(opcode, "jsrr"))
		return 8;
	if ( strcmp(opcode, "ldb"))
		return 2;
	if ( strcmp(opcode, "ldw"))
		return 6;
	if ( strcmp(opcode, "lea"))
		return 14;
	if ( strcmp(opcode, "nop"))
		return 0;
	if ( strcmp(opcode, "not"))
		return 9;
	if ( strcmp(opcode, "ret"))
		return 12;
	if ( strcmp(opcode, "lshf"))
		return 13;
	if ( strcmp(opcode, "rshfl"))
		return 13;
	if ( strcmp(opcode, "rshfa"))
		return 13;
	if ( strcmp(opcode, "rti"))
		return 8;
	if ( strcmp(opcode, "stb"))
		return 3;
	if ( strcmp(opcode, "stw"))
		return 7;
	if ( strcmp(opcode, "trap"))
		return 15;
	if ( strcmp(opcode, "xor"))
		return 9;

	return -1;
}

/* check if given string is a valid opcode
	Valid Opcodes: ADD, AND, BR(all variations), HALT, JMP, JSRR, LDB, LDW, LEA,
				 NOP, NOT, RET, LSHF, RSHFL, RSHFA, RTI, STB, STW, TRAP, XOR */
bool isOpcode( char * lPtr ) {

	if (getOpcodeInt(lPtr) != -1)
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
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
) {
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

	/* return if label */
	if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' )
	{
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	}

	/* return if opcode */
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

char* getArgType(char * pArg)
{
	if ( pArg[0] == 'r' && isdigit(pArg[1]))
		return "reg";
	if ( pArg[0] == '#' || pArg[0] == 'x')
		return "imm";
}

int getArgInt(char * pArg)
{
	/* return the value of the register */
	if ( pArg[0] == 'r' && isdigit(pArg[1])) {
		int num = pArg[1] - '0';
		return num;
	}
	/* return the value of the immediate */
	if ( pArg[0] == '#' || pArg[0] == 'x')
		return toNum(pArg);

	return 0;
}

void shiftUp(int * value, int shiftNum)
{
	*value = *value << shiftNum;
}
int instCount = 0;

int assembleInstruction(char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
)
{
	int opcodeInt = getOpcodeInt(*pOpcode);
	shiftUp(&opcodeInt, 12);

	int arg1Int = 0;
	if( !isEmpty(*pArg1) ) {
		arg1Int = getArgInt(*pArg1) << 9;
		//shiftUp(&arg1Int, 9);
	}

	int arg2Int = 0;
	if( !isEmpty(*pArg2) ) {
		arg2Int = getArgInt(*pArg2) << 6;
		//shiftUp(&arg1Int, 6);
	}

	int arg3Int = 0;
	if( !isEmpty(*pArg3) ) {
		arg3Int = getArgInt(*pArg3);
	}

	int arg4Int = 0;
	if(!isEmpty(*pArg4)) {
		arg4Int = getArgInt(*pArg4);
	}

	int bit5 = 0;
	if( getArgType(*pArg3) == "imm")
		bit5 = 32;

	printf("instr %d: opcode: %d\t operand: %d\t operand: %d\t operand: %d\t operand: %d\t \n"
				, instCount, opcodeInt, arg1Int, arg2Int, arg3Int, arg4Int);

	return opcodeInt + arg1Int + arg2Int + bit5 + arg3Int;

}



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

	char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
	        *lArg2, *lArg3, *lArg4;

	int lRet;
	int instr = 0;
	do
	{
		lRet = readAndParse( infile, lLine, &lLabel,
			&lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE ) // DONE == 0 | OK == 1 | EMPTY_LINE == 2
		{
			instr = assembleInstruction( &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		}

		if( lOpcode[0] != '.') {
			printf("instr %d: opcode: %s\t operand: %s\t operand: %s\t operand: %s\t operand: %s\t \n"
				, instCount, lOpcode, lArg1, lArg2, lArg3, lArg4);
			fprintf( outfile, "0x%.4X\n", instr);
			instCount++;
		}
	} while( lRet != DONE );


	/* TODO: Translate the input file and write to the output file */

	fclose(infile);
	fclose(outfile);

	return 0;
}

