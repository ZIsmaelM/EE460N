/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Ismael Marquez 
    Name 2: Full name of the second partner
    UTEID 1: im6549
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  //argv[1] = "test0.txt";
  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

// Struct for holding information about the current instruction
typedef struct Instruction_Data_Struct{

  char binaryString[17];
  int instrReg,
      opcode,
      operand1,
      operand2,
      operand3;

} Instruction_Data;

Instruction_Data INSTR;
int MAR;
int MDR;
int BEN;

// converts an int into a string of binary characters (prints as inverse of value)
void intToBinary(int val, char* string) {
  int valDiv;
  int valMod;

  for (int i = 0; i <= 15; i++) {
    valDiv = val >> 1;
    valMod = val % 2;

    if (valMod != 0)
      string[i] = '1';
    else
      string[i] = '0';

    val = valDiv;
  }
  string[16] = '\0';
}

// converts a string of binary characters into an int
int binaryToInt(char* string) {
  int numDigits = strlen(string);
  int val = 1;
  int sum = 0;
  int isNeg = string[numDigits-1] - '0';

  for (int i = 0; i < numDigits; i++) {
    if (isNeg) {
    	if (string[i] == '0')
    		sum += val;
    } else {
    	if (string[i] == '1')
    		sum += val;
    }
    val *= 2;
  }

  if (isNeg) {
  	sum += 1;
  	sum = -sum;
  }

  return sum;
}

void getRegString(Instruction_Data INSTR, char * reg, int regIndex) {
  int regSize = 3;

  for (int i = 0; i < regSize; i++) {
    reg[i] = INSTR.binaryString[regIndex];
    regIndex--;
  }
  reg[3] = '\0';
}

void setCC(int result) {
  if (result > 0) {
    NEXT_LATCHES.P = 1;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
  }
  else if (result == 0) {
    NEXT_LATCHES.P = 0;
    NEXT_LATCHES.N = 1;
    NEXT_LATCHES.Z = 0;
  }
  else if (result < 0) {
    NEXT_LATCHES.P = 0;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 1;
  }
}
void opADD(Instruction_Data INSTR) {

  // get DR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get SR1
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get imm5 or SR2
  int op3 = (INSTR.instrReg & 0x0007);
  if (INSTR.binaryString[5] == '0')
    op3 = CURRENT_LATCHES.REGS[op3];
  else
    op3 = (INSTR.instrReg & 0x001F);

  int result = CURRENT_LATCHES.REGS[INSTR.operand2] + op3;
  setCC(result);

  NEXT_LATCHES.REGS[INSTR.operand1] = result;
}

void opAND(Instruction_Data INSTR) {

  // get DR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get SR1
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get imm5 or SR2
  int op3 = (INSTR.instrReg & 0x0007);
  if (INSTR.binaryString[5] == '0')
    op3 = CURRENT_LATCHES.REGS[op3];
  else
    op3 = (INSTR.instrReg & 0x001F);

  int result = INSTR.operand2 & op3;
  setCC(result);
  NEXT_LATCHES.REGS[INSTR.operand1] = result;
}

void opXORNOT(Instruction_Data INSTR) {
  // get DR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get SR1
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get imm5 or SR2
  int result;
  int op3 = (INSTR.instrReg & 0x0007);
  if (INSTR.binaryString[5] == '0') {
    op3 = CURRENT_LATCHES.REGS[op3];
    result = CURRENT_LATCHES.REGS[INSTR.operand2] + op3;
  }
  else {
    result = ~CURRENT_LATCHES.REGS[INSTR.operand2];
  }

  setCC(result);
  NEXT_LATCHES.REGS[INSTR.operand1] = result;
}

void opLDB(Instruction_Data INSTR) {
  // DR = SEXT(mem[BaseR + SEXT(boffset6)]);

  // get DR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get BR
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get offset6
  INSTR.operand3 = (INSTR.instrReg & 0x003F);

  printf("Op3: 0x%.4X\n", INSTR.operand3);
  if (INSTR.operand3 > 31)
    INSTR.operand3 = Low16bits(0xFFC0 + INSTR.operand3);

  int memIndex = Low16bits(Low16bits(CURRENT_LATCHES.REGS[INSTR.operand2])
                  + Low16bits(INSTR.operand3));

  printf("Op3: 0x%.4X\n", memIndex);
  int memContents;
  if (memIndex % 2 == 0)
    memContents = MEMORY[memIndex>>1][0]; // lower 8 bits of address contents
  else
    memContents = MEMORY[memIndex>>1][1]; // upper 8 bits of address contents

  int result = memContents;
  if (memContents >= 128)
    result += 0xFF00 + memContents;

  setCC(result);
  NEXT_LATCHES.REGS[INSTR.operand1] = result;//INSTR.operand3;
  // printf("Op1: %d\n", INSTR.operand1);
  // printf("Op2: %d\n", CURRENT_LATCHES.REGS[INSTR.operand2]);
  // printf("Op3: %d\n", op3);
  // printf("result: %d\n", NEXT_LATCHES.REGS[INSTR.operand1]);
}

void opLDW(Instruction_Data INSTR) {
  // DR = SEXT(mem[BaseR + SEXT(boffset6)]);

  // get DR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get BR
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get offset6
  INSTR.operand3 = (INSTR.instrReg & 0x003F);

  if (INSTR.operand3 > 31)
    INSTR.operand3 = Low16bits(0xFFC0 + INSTR.operand3);

  INSTR.operand3 = INSTR.operand3 << 1;

  int memIndex = Low16bits(Low16bits(CURRENT_LATCHES.REGS[INSTR.operand2])
                  + Low16bits(INSTR.operand3));

  int memContents;
  if (memIndex % 2 == 0)
    memContents = MEMORY[memIndex>>1][0]; // lower 8 bits of address contents
  else
    memContents = MEMORY[memIndex>>1][1]; // upper 8 bits of address contents

  int result = memContents;
  if (memContents >= 128)
    result += 0xFF00 + memContents;

  setCC(result);
  NEXT_LATCHES.REGS[INSTR.operand1] = result;
}

void opSTB(Instruction_Data INSTR) {
  // mem[BaseR + SEXT(boffset6)] = SR[7:0];

  // get SR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get BR
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get offset6
  INSTR.operand3 = (INSTR.instrReg & 0x003F);

  if (INSTR.operand3 > 31)
    INSTR.operand3 = Low16bits(0xFFC0 + INSTR.operand3);

  int memIndex = Low16bits(Low16bits(CURRENT_LATCHES.REGS[INSTR.operand2])
              + Low16bits(INSTR.operand3));

  if (memIndex % 2 == 0)
    MEMORY[memIndex >> 1][0] = CURRENT_LATCHES.REGS[INSTR.operand1];
  else
    MEMORY[memIndex >> 1][1] = CURRENT_LATCHES.REGS[INSTR.operand1];
}

void opSTW(Instruction_Data INSTR) {
  // MEM[BaseR + LSHF(SEXT(offset6), 1)] = SR;

  // get SR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get BR
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get offset6
  INSTR.operand3 = (INSTR.instrReg & 0x003F);

  if (INSTR.operand3 > 31)
    INSTR.operand3 = Low16bits(0xFFC0 + INSTR.operand3);

  INSTR.operand3 = INSTR.operand3 << 1;
  int memIndex = Low16bits(Low16bits(CURRENT_LATCHES.REGS[INSTR.operand2])
              + Low16bits(INSTR.operand3));
  
  if (memIndex % 2 == 0)
    MEMORY[memIndex >> 1][0] = CURRENT_LATCHES.REGS[INSTR.operand1];
  else
    MEMORY[memIndex >> 1][1] = CURRENT_LATCHES.REGS[INSTR.operand1];
}

void opLEA(Instruction_Data INSTR) {
  // MEM[BaseR + LSHF(SEXT(offset6), 1)] = SR;

  // get SR
  INSTR.operand1 = (INSTR.instrReg & 0x0E00) >> 9;
  // get BR
  INSTR.operand2 = (INSTR.instrReg & 0x01C0) >> 6;
  // get offset6
  INSTR.operand3 = (INSTR.instrReg & 0x003F);

  if (INSTR.operand3 > 31)
    INSTR.operand3 = Low16bits(0xFFC0 + INSTR.operand3);

  INSTR.operand3 = INSTR.operand3 << 1;
  int memIndex = Low16bits(Low16bits(CURRENT_LATCHES.REGS[INSTR.operand2])
              + Low16bits(INSTR.operand3));
  
  printf("Index: %d\n", memIndex);
  if (memIndex % 2 == 0)
    MEMORY[memIndex >> 1][0] = CURRENT_LATCHES.REGS[INSTR.operand1];
  else
    MEMORY[memIndex >> 1][1] = CURRENT_LATCHES.REGS[INSTR.operand1];
}

// get the instruction from memory
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/
int fetch() {
  MAR = CURRENT_LATCHES.PC;
  int instrPtr = CURRENT_LATCHES.PC; 
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

  // TODO: check ready bit
  MDR = MAR;
  int lower8 = MEMORY[instrPtr>>1][0];
  int upper8 = MEMORY[instrPtr>>1][1];
  int instr = (upper8 << 8) + lower8;

  // printf("0x%.4X:		0x%.4X\n", instrPtr, instr);
  // printf("[7-0]: %d\n", lower8);
  // printf("[15-8]: %d\n", upper8);

  return instr;
}

int decode(Instruction_Data INSTR) {

  // Set BEN signal
  if ((INSTR.binaryString[11] - '0' & CURRENT_LATCHES.N)
    || (INSTR.binaryString[10] - '0' & CURRENT_LATCHES.Z)
    || (INSTR.binaryString[9] - '0' & CURRENT_LATCHES.P))
    BEN = 1;
  else
    BEN = 0;

  return (INSTR.instrReg & 0xF000) >> 12;
}

int execute(Instruction_Data INSTR, int state)
{
	// ADD
	if ( state == 1 ) {
		opADD(INSTR);
	}
	// AND
	if ( state == 5 ) {
    opAND(INSTR);
	}
	// NOT, XOR
	if ( state == 9 ) {
    opXORNOT(INSTR);
  }
	// LDB 
	if ( state == 2 ) {
    opLDB(INSTR);
  }
	// LDW 
	if ( state == 6 ) {
    opLDW(INSTR);
  }
	// STB
	if ( state == 3 ) {
    opSTB(INSTR);
  } 
	// STW
	if ( state == 7 ) {
    opSTW(INSTR);
  } 
	// BR
	if ( state == 0 ) {} 
	// JSR, JSRR
	if ( state == 4 ) {} 
	// JMP, RET
	if ( state == 12 ) {} 
	// LSHF, RSHFL, RSHFA
	if ( state == 13 ) {} 
	// TRAP, HALT
	if ( state == 15 ) {} 
	// LEA
	if ( state == 14 ) {
    opLEA(INSTR);
  } 

	return -1;
}

void printDebug(Instruction_Data INSTR) {
  printf("PC: 0x%.4X\n", CURRENT_LATCHES.PC);
	printf("Instr Int: %d\n", INSTR.instrReg);
	printf("Instr Hex: 0x%.4X\n", INSTR.instrReg);
	
	printf("Instr Binary: ");
	for(int i = 15; i >= 0; i--) {
		printf("%c", INSTR.binaryString[i]);
		if(i%4 == 0)
			printf(" ");
	}
	printf("\n");
}

void process_instruction(){

  Instruction_Data INSTR;
  INSTR.instrReg = fetch();  
  intToBinary(INSTR.instrReg, INSTR.binaryString);
  printDebug(INSTR);
  INSTR.opcode = decode(INSTR);
  int foo = execute(INSTR, INSTR.opcode);
  printf("%d\n", INSTR.opcode);

  if (INSTR.instrReg == 0xF025)
    NEXT_LATCHES.PC = 0x0000;
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     

}
