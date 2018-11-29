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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
/* MODIFY: you have to add all your new control signals */
    GATE_PSR,
    GATE_NPCR,
    GATE_SRR,
    GATE_SPTRR,
    LD_SRR,
    LD_PSR,
    LD_VTBR,
    LD_NPCR,
    LD_USPR,
    LD_SSPR,
    LD_SPTRR,
    REGMUX1, REGMUX0,
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
/* MODIFY: you can add more Get functions for your new control signals */
int GetGATE_PSR(int *x)         { return(x[GATE_PSR]); }
int GetGATE_NPCR(int *x)         { return(x[GATE_NPCR]); }
int GetGATE_SRR(int *x)         { return(x[GATE_SRR]); }
int GetGATE_SPTRR(int *x)         { return(x[GATE_SPTRR]); }
int GetLD_SRR(int *x)        { return(x[LD_SRR]); }
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }
int GetLD_VTBR(int *x)         { return(x[LD_VTBR]); }
int GetLD_NPCR(int *x)        { return(x[LD_NPCR]); }
int GetLD_USPR(int *x)        { return(x[LD_USPR]); }
int GetLD_SSPR(int *x)        { return(x[LD_SSPR]); }
int GetLD_SPTRR(int *x)         { return(x[LD_SPTRR]); }
int GetREGMUX(int *x)      { return((x[REGMUX1] << 1) + x[REGMUX0]); }

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

int RUN_BIT;    /* run bit */
int BUS;    /* value of the bus */

typedef struct System_Latches_Struct{

int PC,     /* program counter */
    MDR,    /* memory data register */
    MAR,    /* memory address register */
    IR,     /* instruction register */
    N,      /* n condition bit */
    Z,      /* z condition bit */
    P,      /* p condition bit */
    BEN;        /* ben register */

int READY;  /* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

/* additional lab 4 registers */
int PSR;
int SRR;
int VTBR;
int NPCR;
int USPR;
int SPTRR;

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

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    //PSR Initializer
    CURRENT_LATCHES.PSR = 0x8000;

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

   Begin your code here                        */
/***************************************************************/

/***************/
// DEBUG CODE  //
/***************/

void printSTATE(int x) {
	switch (x) {
		case 0 :
			printf("state: BR\n");
			break;
		case 1 :
			printf("state: ADD\n");
			break;
		case 5 :
			printf("state: AND\n");
			break;
		case 9 :
			printf("state: XOR\n");
			break;
		case 15 :
			printf("state: TRAP\n");
			break;
		case 13 :
			printf("state: SHF\n");
			break;
		case 14 :
			printf("state: LEA\n");
			break;
		case 2 :
			printf("state: LDB\n");
			break;
		case 6 :
			printf("state: LDW\n");
			break;
		case 7 :
			printf("state: STW\n");
			break;
		case 3 :
			printf("state: STB\n");
			break;
		case 12 :
			printf("state: JMP\n");
			break;
		case 20 :
			printf("state: JSRR\n");
			break;
		case 21 :
			printf("state: JSR\n");
			break;
		default :
			//printf("\n");
			break;
	}
}
/***************/

#define Low8bits(x) ((x) & 0xFF)

int mask(int x, int mask) {return ((x) & mask);}
int mask_shfR(int x, int mask, int shfVal) {return ((x) & mask) >> shfVal;}

int sext(int x, int numBits) {
	int neg = (x >> numBits-1) % 2;

	switch (numBits) {
		case 8 :
			return (neg ? (x | 0xFF00) : (x & 0x00FF));
		case 6 :
			return (neg ? (x | 0xFFC0) : (x & 0x003F));
		case 9 :
			return (neg ? (x | 0xFE00) : (x & 0x01FF));
		case 11 :
			return (neg ? (x | 0xF800) : (x & 0x07FF));
		default :
			printf("Not a valid SEXT amount\n");
			return -1;
	}
}

int sext_8(int x, int numBits) {
    if (x >> numBits-1 == 1)
        return x | 0xFF00;
    else
        return x & 0x00FF;
}

int sext_6(int x, int numBits) {
    if (x >> numBits-1 == 1)
        return x | 0xFFC0;
    else
        return x & 0x003F;
}

int sext_9(int x, int numBits) {
    if (x >> numBits-1 == 1)
        return x | 0xFE00;
    else
        return x & 0x01FF;
}

int sext_11(int x, int numBits) {
    if (x >> numBits-1 == 1)
        return x | 0xF800;
    else
        return x & 0x07FF;
}

int eval_Address() {
	// PC = 0, BaseReg = 1
	int addr1;
	if (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		int BaseReg = mask_shfR(CURRENT_LATCHES.IR, 0x01C0, 6);
		addr1 = CURRENT_LATCHES.REGS[BaseReg];
	}
	else {
		addr1 = CURRENT_LATCHES.PC;
	}

	// ZERO = 0, offset6 = 1, PCoffset9 = 2, PCoffset11 = 3
	int addr2;
	int addr2MuxCode = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	switch (addr2MuxCode) {
		case 0 :
			addr2 = 0;
			break;
		case 1 :
			addr2 = sext( mask(CURRENT_LATCHES.IR, 0x003F), 6);
			break;
		case 2 :
			addr2 = sext( mask(CURRENT_LATCHES.IR, 0x01FF), 9);
			break;
		case 3 :
			addr2 = sext( mask(CURRENT_LATCHES.IR, 0x07FF), 11);
			break;
		default :
			printf("Not a valid addr2mux code\n");
	}

	if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION))
		addr2 = addr2 << 1;

	return Low16bits(addr1 + addr2);
}

int gateMARMUXVal;
void eval_MARMUX(void) {
	// TRAP = 0, ADDRESS = 1
	if (GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		gateMARMUXVal = eval_Address();
	}
	else {
		gateMARMUXVal = Low16bits(mask(CURRENT_LATCHES.IR, 0x00FF) << 1);
	}
}

int gatePCVal;
void eval_PC(void) {
	gatePCVal = CURRENT_LATCHES.PC;
}

int gateALUVal;
void eval_ALU(void) {
    int bit5 = (CURRENT_LATCHES.IR >> 5) % 2;
    int reg11_9 = mask_shfR(CURRENT_LATCHES.IR, 0x0E00, 9);
    int reg8_6 = mask_shfR(CURRENT_LATCHES.IR, 0x01C0, 6);
    int sr2;

    // check if 2nd source is register value or immediate value
    if (bit5) {
        sr2 = Low16bits(mask(CURRENT_LATCHES.IR, 0x0001F));
        
        // check if imm5 value is negative
        if ((CURRENT_LATCHES.IR >> 4) % 2)
            sr2 = Low16bits(sr2 | 0xFFE0);
    }
    else
        sr2 = CURRENT_LATCHES.REGS[mask(CURRENT_LATCHES.IR, 0x007)];

    int aluKVal = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    switch (aluKVal) {
        // ADD
        case 0 :
            gateALUVal = CURRENT_LATCHES.REGS[reg8_6] + sr2;
            break;
        // AND
        case 1 :
            gateALUVal = CURRENT_LATCHES.REGS[reg8_6] & sr2;
            break;
        // XOR
        case 2 :
            gateALUVal = CURRENT_LATCHES.REGS[reg8_6] ^ sr2;
            break;
        // PASSA
        case 3 :
            gateALUVal = CURRENT_LATCHES.REGS[reg11_9];
            break;
        default :
			printf("Not a valid aluK code\n");
    }
    printf("DR: %d\n", reg11_9); 
    printf("SR1: %d\n", reg8_6);
    printf("SR2: %d\n", sr2);
    printf("Res: %d\n", gateALUVal);
}

int gateSHFVal;
void eval_SHF(void) {
    int shfCode = mask_shfR(CURRENT_LATCHES.IR, 0x0030, 4);
    int dr = mask_shfR(CURRENT_LATCHES.IR, 0x0E00, 9);
    int sr = mask_shfR(CURRENT_LATCHES.IR, 0x01C0, 6);
    int shfAmount = mask(CURRENT_LATCHES.IR, 0x000F);

    int maskBits = 0;
    int sumBits = 0x8000;
    for (int i = 0; i < shfAmount; i++) {
		maskBits += sumBits;
		sumBits = sumBits >> 1;
	}

	// Check if the data is positive or negative
	int negFlag = 0;
    int data = Low16bits(CURRENT_LATCHES.REGS[sr]);
    if (data > 32767) {
    	negFlag = 1;
    }
    // invert the mask bits if the data is positive
    else {
    	maskBits = maskBits ^ 0xFFFF;
    }

    int bit15 = mask(data, 0x8000);
    switch (shfCode) {
    	case 0 :
    		gateSHFVal = Low16bits(data << shfAmount);
    		break;
    	case 1 :
    		gateSHFVal = Low16bits(data >> shfAmount);
    		break;
    	case 3 :
    		gateSHFVal = (negFlag ? (Low16bits((data >> shfAmount) | maskBits))
    							  : (Low16bits((data >> shfAmount) & maskBits)));
    		break;
    	default :
    		gateSHFVal = gateSHFVal;
    		break;
    }
}

int gateMDRVal;
void eval_MDR(void) {
    // WORD = 1, BYTE = 0
    if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
        gateMDRVal = CURRENT_LATCHES.MDR;
    else {
        int low8 = Low8bits(CURRENT_LATCHES.MDR);
        
        // sign extend the byte value
        if (mask_shfR(low8,0x0080,7) == 1)
            gateMDRVal = low8 | 0xFF00;
        else
            gateMDRVal = low8 & 0x00FF;
    }
}

void latch_MAR() {
	NEXT_LATCHES.MAR = BUS;
}

void latch_MDR() {
	int reg11_9 = mask_shfR(CURRENT_LATCHES.IR, 0x0E00, 9);
    int reg8_6 = mask_shfR(CURRENT_LATCHES.IR, 0x01C0, 6);

	if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) && CURRENT_LATCHES.READY) {
		if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
			NEXT_LATCHES.MDR = Low16bits((MEMORY[CURRENT_LATCHES.MAR/2][1] << 8) + (MEMORY[CURRENT_LATCHES.MAR/2][0]));
		}
		// This is for state 29, which differs in data size
		// however, the appC states MDR still gets the entire 16bits of mem
		// The desired byte is selected in the following state, 31
		else {
            int byteUsed = MEMORY[CURRENT_LATCHES.MAR/2][CURRENT_LATCHES.MAR%2];
            if (mask_shfR(byteUsed, 0x0080, 7))
                NEXT_LATCHES.MDR = Low16bits(byteUsed | 0xff00);
            else
                NEXT_LATCHES.MDR = Low16bits(mask(byteUsed, 0x00ff));
		}
	}
	else {
		int sr = mask_shfR(CURRENT_LATCHES.IR, 0x0E00, 9);
		int isAWord = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
		NEXT_LATCHES.MDR = (isAWord ? CURRENT_LATCHES.REGS[sr] : mask(CURRENT_LATCHES.REGS[sr], 0x00FF));
	}
}

void latch_IR() {
	printf("Current IR: 0x%.4x\n", BUS );
	NEXT_LATCHES.IR = BUS;
}

void latch_BEN() {
	int n = CURRENT_LATCHES.N & mask_shfR(CURRENT_LATCHES.IR, 0x0800, 11);
	int z = CURRENT_LATCHES.Z & mask_shfR(CURRENT_LATCHES.IR, 0x0400, 10);
	int p = CURRENT_LATCHES.P & mask_shfR(CURRENT_LATCHES.IR, 0x0200, 9);

	NEXT_LATCHES.BEN = n | z | p;
}

void latch_REG() {
	// if DRMUX = 1, dr = 7 else dr = reg11_9
	int dr = (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) ? 7 : mask_shfR(CURRENT_LATCHES.IR, 0x0E00, 9));
	NEXT_LATCHES.REGS[dr] = BUS;
}

void latch_CC() {
	NEXT_LATCHES.N = (BUS > 32767 ? 1 : 0);
	NEXT_LATCHES.Z = (BUS == 0 ? 1 : 0);
	NEXT_LATCHES.P = ((BUS > 0 && BUS <= 32767) ? 1 : 0);
}

void latch_PC() {
	int pcMuxCode = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);

	switch (pcMuxCode) {
		case 0 :
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
			break;
		case 1 :
			NEXT_LATCHES.PC = BUS;
			break;
		case 2 :
			NEXT_LATCHES.PC = eval_Address();
			break;
		default :
			printf("Not a valid PCmux code\n");
	}
}

void write_to_mem() {
	int bytePos = mask(CURRENT_LATCHES.MAR, 0x0001);
	// word
	if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
		MEMORY[CURRENT_LATCHES.MAR/2][1] = Low16bits(mask(CURRENT_LATCHES.MDR, 0xff00));
		MEMORY[CURRENT_LATCHES.MAR/2][0] = Low8bits(mask(CURRENT_LATCHES.MDR, 0x00ff));
	}
	// byte	
	else {
		if (bytePos)
			MEMORY[CURRENT_LATCHES.MAR/2][1] = Low16bits(CURRENT_LATCHES.MDR);
		else
			MEMORY[CURRENT_LATCHES.MAR/2][0] = Low16bits(CURRENT_LATCHES.MDR);
	}
}

void eval_micro_sequencer() {

    /* 
    * Evaluate the address of the next state according to the 
    * micro sequencer logic. Latch the next microinstruction.
    */

    printf("STATE: %d\t CYCLE: %d\n", CURRENT_LATCHES.STATE_NUMBER, CYCLE_COUNT+1);
    int j0 = CURRENT_LATCHES.MICROINSTRUCTION[J0];
    int j1 = CURRENT_LATCHES.MICROINSTRUCTION[J1];
    int j2 = CURRENT_LATCHES.MICROINSTRUCTION[J2];
    int j3 = CURRENT_LATCHES.MICROINSTRUCTION[J3];
    int j4 = CURRENT_LATCHES.MICROINSTRUCTION[J4];
    int j5 = CURRENT_LATCHES.MICROINSTRUCTION[J5];

    int ir11 = mask_shfR(CURRENT_LATCHES.IR, 0x0800, 11);
    int cond = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);

    switch (cond) {
        case 1 :
            j1 = j1 || CURRENT_LATCHES.READY;
            break;
        case 2 :
            j2 = j2 || CURRENT_LATCHES.BEN;
            break;
        case 3 :
            j0 = j0 || ir11;
            break;
        default :
            break;
    }

    

    if (CURRENT_LATCHES.MICROINSTRUCTION[IRD]) {
        NEXT_LATCHES.STATE_NUMBER = mask( mask_shfR(CURRENT_LATCHES.IR, 0xF000, 12), 0x003F );
        // debug code
	    printSTATE(mask( mask_shfR(CURRENT_LATCHES.IR, 0xF000, 12), 0x003F ));
	    //
    }
    else
        NEXT_LATCHES.STATE_NUMBER = (j5 << 5) + (j4 << 4) + (j3 << 3) + (j2 << 2) + (j1 << 1) + j0;

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

	if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
		memCycles++;
		if (memCycles == 4)
			NEXT_LATCHES.READY = 1;

		if (NEXT_LATCHES.READY && GetR_W(CURRENT_LATCHES.MICROINSTRUCTION))
			write_to_mem();
	}
	else {
		memCycles = 0;
		NEXT_LATCHES.READY = 0;
	}
}

void eval_bus_drivers() {

    /* 
    * Datapath routine emulating operations before driving the bus.
    * Evaluate the input of tristate drivers 
    *         Gate_MARMUX,
    *         Gate_PC,
    *         Gate_ALU,
    *         Gate_SHF,
    *         Gate_MDR.
    */

	eval_MARMUX();
	eval_PC();
    eval_ALU();
    eval_SHF();
    eval_MDR();

}

void drive_bus() {

    /* 
    * Datapath routine for driving the bus from one of the 5 possible 
    * tristate drivers. 
    */

    BUS = 0;
    if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)) 			BUS = Low16bits(gatePCVal);
    else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) 	BUS = Low16bits(gateMDRVal);
    else if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) 	BUS = Low16bits(gateALUVal);
    else if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) 	BUS = Low16bits(gateMARMUXVal);
    else if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)) 	BUS = Low16bits(gateSHFVal);

}

void latch_datapath_values() {

    /* 
    * Datapath routine for computing all functions that need to latch
    * values in the data path at the end of this cycle.  Some values
    * require sourcing the bus; therefore, this routine has to come 
    * after drive_bus.
    */

    if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION)) latch_MAR();
    if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) latch_MDR();
    if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION)) latch_IR();
    if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION)) latch_BEN();
    if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION)) latch_REG();
    if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)) latch_CC();
    if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)) latch_PC();

}
