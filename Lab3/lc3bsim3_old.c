/*
    Name 1: Ismael Marquez
    UTEID 1: im6549
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

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

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

// converts an int into a string of binary characters (prints as inverse of value)
void intToBinary(int val, char* string) {
  int valDiv;
  int valMod;
  int strLen = strlen(string);

  for (int i = 0; i <= strLen; i++) {
    valDiv = val >> 1;
    valMod = val % 2;

    if (valMod != 0)
      string[i] = '1';
    else
      string[i] = '0';

    val = valDiv;
  }
  string[strLen+1] = '\0';
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

#define Low8bits(x) ((x) & 0xFF)

void eval_micro_sequencer() {
  
  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

	printf("STATE: %d\n", CURRENT_LATCHES.STATE_NUMBER);
	int j0 = CURRENT_LATCHES.MICROINSTRUCTION[J0];
	int j1 = CURRENT_LATCHES.MICROINSTRUCTION[J1];
	int j2 = CURRENT_LATCHES.MICROINSTRUCTION[J2];
	int j3 = CURRENT_LATCHES.MICROINSTRUCTION[J3];
	int j4 = CURRENT_LATCHES.MICROINSTRUCTION[J4];
	int j5 = CURRENT_LATCHES.MICROINSTRUCTION[J5];

	int cond0 = CURRENT_LATCHES.MICROINSTRUCTION[COND0];
	int cond1 = CURRENT_LATCHES.MICROINSTRUCTION[COND1];

	int ir11 = (CURRENT_LATCHES.IR & 0x0800) >> 11;

	if (CURRENT_LATCHES.BEN && cond0 == 0 && cond1 == 1)
		j2 = 1;
	if (CURRENT_LATCHES.READY && cond0 == 1 && cond1 == 0)
		j1 = 1;
	if (ir11 && cond0 == 1 && cond1 == 1)
		j0 = 1;

	if (CURRENT_LATCHES.MICROINSTRUCTION[IRD])
		NEXT_LATCHES.STATE_NUMBER = ((CURRENT_LATCHES.IR) & 0xF000) >> 12;
	else
		NEXT_LATCHES.STATE_NUMBER = j5*32 + j4*16 + j3*8 + j2*4 + j1*2 + j0*1;

	for(int i = 0; i < CONTROL_STORE_BITS; i++)
		NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
}

int memCycles = 0;
void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
	if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
		memCycles++;
        if (CURRENT_LATCHES.STATE_NUMBER == 29)
            printf("LoopCount: %d\n", memCycles);
		if (memCycles == 4)
            NEXT_LATCHES.READY = 1;
		if (CURRENT_LATCHES.READY) {
            int hiByteRead = MEMORY[CURRENT_LATCHES.MAR/2][1] << 8;
			int lowByteRead = MEMORY[CURRENT_LATCHES.MAR/2][0];
			int hiByteWrite = (CURRENT_LATCHES.MDR & 0xFF00) >> 8;
			int lowByteWrite = CURRENT_LATCHES.MDR & 0x00FF;
			
            if (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION))
			{
				// word
				if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
					MEMORY[CURRENT_LATCHES.MAR/2][1] = hiByteWrite;
					MEMORY[CURRENT_LATCHES.MAR/2][0] = lowByteWrite;
				}
				// byte
				else if (CURRENT_LATCHES.MAR % 2)
					MEMORY[CURRENT_LATCHES.MAR/2][1] = hiByteWrite;
				else
					MEMORY[CURRENT_LATCHES.MAR/2][0] = lowByteWrite;	

			}
            printf("ByteRead: 0x%.4X\n",Low16bits(*MEMORY[CURRENT_LATCHES.MAR/2]));
            printf("ByteWrite: 0x%.4X\n", CURRENT_LATCHES.MDR);
			// else if ((lowByteRead & 0x0080) != 0)
   //              NEXT_LATCHES.MDR = lowByteRead & 0x00FF;
   //          else
   //              NEXT_LATCHES.MDR = lowByteRead & 0xFF00;
				NEXT_LATCHES.MDR = hiByteRead + lowByteRead;
			//printf("Instruction: 0x%.4X\n", NEXT_LATCHES.MDR);
			//printf("CYCLE COUNT: %d\n", CYCLE_COUNT);
		}
        printf("MDR: 0x%.4X\n", NEXT_LATCHES.MDR);
	} else {
		memCycles = 0;
		NEXT_LATCHES.READY = 0;
	}
}


int gatePCVal, gateMDRVal, gateALUVal, gateMARMUXVal, gateSHFVal;
void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */

	// State: 18, 19, 20, 21, 28
	if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION))
		gatePCVal = CURRENT_LATCHES.PC;

	// State: 27, 30, 31, 35
	//if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
		// word
		if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
			gateMDRVal = CURRENT_LATCHES.MDR;
		
		// byte
		else {
			int low8 = CURRENT_LATCHES.MDR & 0xFF;
			// negative
			if ((low8 >> 7) % 2)
				gateMDRVal = low8 | 0xFF00;
			// positive
			else 
				gateMDRVal = low8 & 0x00FF;

            //printf("STATE: %d\n", CURRENT_LATCHES.STATE_NUMBER);
            if (CURRENT_LATCHES.STATE_NUMBER == 31) {
                if ((CURRENT_LATCHES.MDR & 0x0080) != 0)
                    NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR | 0xFF00;
                else
                    NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR & 0x00FF;
            }
		}
	//}

	// State: 1, 5, 9, 23, 24
	if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) {
		int bit5 = (CURRENT_LATCHES.IR >> 5) % 2;
		int reg11_9 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
		int reg8_6 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
		int sr2;
		
		if (bit5) {
			sr2 = Low16bits(CURRENT_LATCHES.IR & 0x0001F);
			if ((CURRENT_LATCHES.IR >> 4) % 2)
				sr2 = Low16bits(sr2 | 0xFFE0);
		}
		else
			sr2 = CURRENT_LATCHES.REGS[CURRENT_LATCHES.IR & 0x007];

		int aluKVal = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
		// ADD state 1
		if (aluKVal == 0)
			gateALUVal = CURRENT_LATCHES.REGS[reg8_6] + sr2;
		// AND state 5 
		else if (aluKVal == 1)
			gateALUVal = CURRENT_LATCHES.REGS[reg8_6] & sr2;
		
		// XOR state 9
		else if (aluKVal == 2)
			gateALUVal = CURRENT_LATCHES.REGS[reg8_6] ^ sr2;

		// STW state 23, STB state 24 (PASSA)
		else if (aluKVal == 3)
			gateALUVal = reg11_9;

	}
	// State: 2, 3, 6, 7, 14, 15
	if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		int baseR = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
		int offset6 = (CURRENT_LATCHES.IR & 0x003F);
		int offset9 = (CURRENT_LATCHES.IR & 0x01FF);
		if (offset6 > 31)
			offset6 = offset6 | 0xFFFFFFC0;
		if (offset9 > 255)
			offset9 = offset9 | 0xFFFFFF00;
		//int offset11 = (CURRENT_LATCHES.IR & 0x07FF);
		int trapCode = Low16bits(CURRENT_LATCHES.IR & 0x00FF);

		int addr2Val = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
		if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
			if (addr2Val == 0)
				gateMARMUXVal = 0;
			else if (addr2Val == 1)
				gateMARMUXVal = CURRENT_LATCHES.REGS[baseR] + offset6;
			else if (addr2Val == 2)
				gateMARMUXVal = CURRENT_LATCHES.PC + CURRENT_LATCHES.REGS[baseR] + (offset9 << 1);
			// else if (addr2Val == 3)
			// 	gateMARMUXVal = CURRENT_LATCHES.REGS[baseR] + offset11;

			if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION))
				gateMARMUXVal = gateMARMUXVal;// << 1;
            else if (CURRENT_LATCHES.STATE_NUMBER == 15)
                gateMARMUXVal = trapCode << 1;
		}
	}

	int destR = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
	int sourceR = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
	int amount4 = CURRENT_LATCHES.IR & 0x000F;
	if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)) {
		if (!((CURRENT_LATCHES.IR >> 4) % 2))
			gateSHFVal = CURRENT_LATCHES.REGS[sourceR] << amount4;
		else if ((CURRENT_LATCHES.IR >> 5) % 2) {
			int negFlag = 0;
			int maskVal = 0x0000;
			int sumVal = 0x8000;
			gateSHFVal = CURRENT_LATCHES.REGS[sourceR];

			if (gateSHFVal > 32767)
				negFlag = 1;

			gateSHFVal = gateSHFVal >> amount4;
			if (negFlag) {
				for (int i = 0; i < amount4; i++) {
					maskVal += sumVal;
					sumVal = sumVal >> 1;
				}
			}
			printf("MASK: 0x%.4X\n", maskVal);
			gateSHFVal = gateSHFVal | maskVal;
		}
		else
			gateSHFVal = CURRENT_LATCHES.REGS[sourceR] >> amount4;
	}

}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */
    BUS = 0;
    if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION))
    	BUS = Low16bits(gatePCVal);
	else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
		BUS = Low16bits(gateMDRVal);
	else if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION))
		BUS = Low16bits(gateALUVal);
	else if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
		BUS = Low16bits(gateMARMUXVal);
	else if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION))
		BUS = Low16bits(gateSHFVal);
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */

	// States: 2, 3, 6, 7, 15, 18, 19
    if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    	if (CURRENT_LATCHES.STATE_NUMBER == 18 || CURRENT_LATCHES.STATE_NUMBER == 19)
    		NEXT_LATCHES.MAR = CURRENT_LATCHES.PC;
    	else
    		NEXT_LATCHES.MAR = BUS; // bus contains MARMUX
    }
    // States: 23, 24, 25, 28, 29, 33     
    if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    	
    	// States: 25, 28, 29, 33
    	if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    		if (CURRENT_LATCHES.STATE_NUMBER != 29)
                NEXT_LATCHES.MDR = Low16bits((MEMORY[CURRENT_LATCHES.MAR/2][1] << 8) + MEMORY[CURRENT_LATCHES.MAR/2][0]);
            // if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
            //     if ((NEXT_LATCHES.MDR & 0x0080) != 0)
            //         NEXT_LATCHES.MDR = NEXT_LATCHES.MDR & 0x00FF;
            //     else
            //         NEXT_LATCHES.MDR = NEXT_LATCHES.MDR | 0xFF00;
            // }
        }
    	// States: 23, 24
    	else {
    		int sr = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    		// Word
    		if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
    			NEXT_LATCHES.MDR = CURRENT_LATCHES.REGS[sr];
    		// Byte
    		else {
    			if (CURRENT_LATCHES.MAR % 2)
    				NEXT_LATCHES.MDR = (CURRENT_LATCHES.REGS[sr] & 0x00FF) << 8;
    			else {
    				if ((CURRENT_LATCHES.REGS[sr] & 0x0080) != 0)
                        NEXT_LATCHES.MDR = CURRENT_LATCHES.REGS[sr] & 0x00FF;
                    else
                        NEXT_LATCHES.MDR = CURRENT_LATCHES.REGS[sr] | 0xFF00;
                }
    		}
    	}
    }
    // States: 35
    if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION))
    	NEXT_LATCHES.IR = BUS; // bus contains MDR
    // States: 32
    if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    	int n = CURRENT_LATCHES.N & ((CURRENT_LATCHES.IR & 0x0800) >> 11);
    	int z = CURRENT_LATCHES.Z & ((CURRENT_LATCHES.IR & 0x0400) >> 10);
    	int p = CURRENT_LATCHES.P & ((CURRENT_LATCHES.IR & 0x0200) >> 9);

    	NEXT_LATCHES.BEN = n | z | p;
    }
    // States: 1, 5, 9, 13, 14, 20, 21, 27, 28, 31
    if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION)) {
    	int dr;
    	// State: 20, 21, 28
    	if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    		dr = 7;
    	// State: 1, 5, 9, 13, 14, 27, 31
    	else
    		dr = (CURRENT_LATCHES.IR & 0x0E00) >> 9;

    	NEXT_LATCHES.REGS[dr] = BUS; // bus contains ALU
    }
    // States: 1, 5, 9, 13, 27, 31
    if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    	NEXT_LATCHES.N = (BUS > 32767 ? 1 : 0);
    	NEXT_LATCHES.Z = (BUS == 0 ? 1 : 0);
    	NEXT_LATCHES.P = ((BUS > 0 && BUS <= 32767) ? 1 : 0);
    }
    // States: 12, 18, 19, 20, 21, 22, 30
    if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {

    	int pcMUXVal = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    	// State: 18, 19
    	if (pcMUXVal == 0)
    		NEXT_LATCHES.PC = BUS + 2;
    	// State: 30
    	else if (pcMUXVal == 1)
    		NEXT_LATCHES.PC = BUS;
    	// State: 12, 20, 21, 22
    	else if (pcMUXVal == 2) {
    		// State: 12, 20
    		if (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
    			int baseR = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
    			NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[baseR];
    		}
    		// State: 21
    		else if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3) {
    			NEXT_LATCHES.PC = BUS + ((CURRENT_LATCHES.IR & 0x07FF) << 1); 
    		}
    		// State: 22
    		else if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
    			int offset9 = (CURRENT_LATCHES.IR & 0x01FF);
				if (offset9 > 255)
					offset9 = offset9 | 0xFFFFFF00;
    			NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + (offset9 << 1)); 
    		}
    	}
    }

}
