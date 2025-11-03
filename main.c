#include <stdio.h>
#include <unistd.h>

#define MEM_SIZE 100    // total number of memory cells
#define DECK_SIZE 100   // total number of input cards

// Holds the deck data and current reading position
typedef struct {
    short deck[DECK_SIZE];
    short deck_counter;
} Deck;

// Represents the CPU state and memory
typedef struct {
    short memory[MEM_SIZE];
    short instr_reg;     // current instruction code
    short pc;            // program counter
    short running;       // 1 = running, 0 = halted
    short accumulator;   // main register for arithmetic
} CPU;

// Executes a single instruction
void execute(CPU *cpu, short instruction, short memory_address, Deck *deck) {
    switch (instruction) {
        case 0: // Input data from deck into memory
            short to_load = deck->deck[deck->deck_counter++];
            cpu->memory[memory_address] = to_load;
            break;

        case 1: // Load value from memory into accumulator
            cpu->accumulator = cpu->memory[memory_address];
            break;

        case 2: // Add memory value to accumulator
            cpu->accumulator += cpu->memory[memory_address];
            break;

        case 3: // Branch if accumulator is negative
            if (cpu->accumulator < 0)
                cpu->pc = memory_address - 1; // adjust since pc increments later
            break;

        case 4: // No operation
            break;

        case 5: // Output memory value to screen
            printf("%d\n", cpu->memory[memory_address]);
            break;

        case 6: // Store accumulator value into memory
            cpu->memory[memory_address] = cpu->accumulator;
            break;

        case 7: // Subtract memory value from accumulator
            cpu->accumulator -= cpu->memory[memory_address];
            break;

        case 8: // Jump to address (save return location at cell 99)
            cpu->memory[99] = 800 + cpu->pc;
            cpu->pc = memory_address - 1;
            break;

        case 9: // Halt program
            cpu->running = 0;
            break;

        default: // Stop if unknown instruction
            cpu->running = 0;
            break;
    }
}

// Prints the contents of memory as a 10x10 grid
void print_memory(const CPU *cpu) {
    printf("\033[2J\033[1;1H");
    printf("=== CPU Memory ===\n");
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            int idx = row * 10 + col;
            printf("%03d ", cpu->memory[idx]);
        }
        printf("\n");
    }
    printf("=================\n");
}

// Runs the CPU until halted
void run(CPU *cpu, Deck *deck) {
    while (cpu->running == 1) {
        short instr_code = cpu->memory[cpu->pc]; // fetch instruction
        cpu->instr_reg = instr_code / 100;       // get opcode
        short addr = instr_code % 100;           // get memory address

        execute(cpu, cpu->instr_reg, addr, deck); // execute step
        cpu->pc++;                                // move to next instruction

        if (cpu->pc >= MEM_SIZE) {                  // stop if out of bounds
            cpu->running = 0;
        }

        // print_memory(cpu);
        // usleep(100*1000);
    }
}

// Loads numbers from deck.txt into deck memory
void load_deck(Deck *deck) {
    FILE *fptr = fopen("deck.txt", "r");
    if (!fptr) {
        perror("Error opening deck.txt");
        deck->deck_counter = 0;
        return;
    }

    short counter = 0, number;
    while (counter < DECK_SIZE && fscanf(fptr, "%hd", &number) == 1)
        deck->deck[counter++] = number;

    deck->deck_counter = 0;
    fclose(fptr);
}

int main() {
    CPU cpu = {0};   // initialize CPU with zeroed memory/registers
    Deck deck = {0}; // initialize deck

    cpu.running = 1; // enable CPU
    cpu.pc = 0;      // start from memory cell 0
    cpu.memory[0] = 1; // ensure memory[0] = 1 as required

    load_deck(&deck); // load deck.txt contents
    run(&cpu, &deck); // start execution

    return 0;
}
