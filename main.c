//Version 1.1
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MEM_SIZE 100    // total number of memory cells
#define DECK_SIZE 100   // total number of input cards
#define MAX_VARS 9

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

typedef struct { // Which ever line the name is on, set mem addr to that instruction
    char name[16];
    short addr;
} Var;

Var vars[MAX_VARS];
short var_count = 0;
short next_var_addr = 1;

typedef struct {
    char name[16];
    short addr;
} Label;

Label labels[MEM_SIZE];
short label_count = 0;

short get_var_addr(const char *name) {
    for (int i = 0; i < var_count; i++)
        if (!strcmp(vars[i].name, name))
            return vars[i].addr;

    strcpy(vars[var_count].name, name);
    vars[var_count].addr = next_var_addr;
    var_count++;
    return next_var_addr++;
}

short get_label_addr(const char *name) {
    for (int i = 0; i < label_count; i++)
        if (!strcmp(labels[i].name, name))
            return labels[i].addr;
    return -1;
}

// Get opcode from instruction
short opcode_from_command(const char *m) {
    if (!strcmp(m, "IN")) return 0;
    if (!strcmp(m, "LOAD")) return 1;
    if (!strcmp(m, "ADD")) return 2;
    if (!strcmp(m, "TEST")) return 3;
    if (!strcmp(m, "SHIFT")) return 4;
    if (!strcmp(m, "OUT")) return 5;
    if (!strcmp(m, "STORE")) return 6;
    if (!strcmp(m, "SUB")) return 7;
    if (!strcmp(m, "JUMP")) return 8;
    if (!strcmp(m, "HALT")) return 9;
    return -1;
}

// Assemble numeric-only program
void assemble(const char *filename, CPU *cpu) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("asm open");
        exit(1);
    }

    // First pass: collect labels
    char line[256];
    int pc = 10;
    
    while (fgets(line, sizeof(line), f)) {
        char word[32];
        char *p = line;
        
        // Skip leading whitespace
        while (*p && isspace(*p)) p++;
        
        // Skip empty lines and comments
        if (*p == '\0' || *p == '\n' || *p == '#') continue;
        
        // Parse first word
        if (sscanf(p, "%31s", word) != 1) continue;
        
        // Check if first word is a label
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == ':') {
            // Found a label
            word[len - 1] = '\0';
            strcpy(labels[label_count].name, word);
            labels[label_count].addr = pc;
            label_count++;
            
            // Move past label and check for instruction on same line
            p += len;
            while (*p && isspace(*p)) p++;
            
            // If nothing else on line (except comments), continue
            if (!*p || *p == '\n' || *p == '#') continue;
            
            // Re-parse next word after label
            if (sscanf(p, "%31s", word) != 1) continue;
        }
        
        // Check if this line has VAR
        if (!strcmp(word, "VAR")) {
            char name[16];
            if (sscanf(p, "%*s %15s", name) == 1) {
                get_var_addr(name); // Just register in first pass
            }
            continue;
        }
        
        // Check if this line has an instruction
        short opcode = opcode_from_command(word);
        if (opcode >= 0) {
            if (opcode == 9) { // HALT
                pc++;
            } else {
                // Has operand, increment pc
                pc++;
            }
        }
    }

    // Reset file pointer for second pass
    rewind(f);

    // Second pass: assemble instructions
    pc = 10;
    char word[32];

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        
        // Skip leading whitespace
        while (*p && isspace(*p)) p++;
        
        // Skip empty lines and comments
        if (*p == '\0' || *p == '\n' || *p == '#') continue;
        
        // Parse first word
        if (sscanf(p, "%31s", word) != 1) continue;
        
        // Check if first word is a label
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == ':') {
            // Label found, move past it
            word[len - 1] = '\0';
            p += len;
            while (*p && isspace(*p)) p++;
            
            // If nothing else on line, continue
            if (!*p || *p == '\n' || *p == '#') continue;
            
            // Read next word (should be instruction)
            if (sscanf(p, "%31s", word) != 1) continue;
        }

        if (!strcmp(word, "VAR")) { 
            char name[16]; 
            if (sscanf(p, "%*s %15s", name) != 1) continue;
            short addr = get_var_addr(name); 

            // Look for '=' and value in the line
            char *eq_pos = strchr(p, '=');
            if (eq_pos) {
                int value; 
                if (sscanf(eq_pos, "= %d", &value) == 1) {
                    cpu->memory[addr] = value; 
                }
            }

            continue; 
        }

        short opcode = opcode_from_command(word);
        if (opcode < 0) {
            printf("Unknown instruction: %s\n", word);
            exit(1);
        }

        if (opcode == 9) { // HALT
            cpu->memory[pc++] = 900;
            continue;
        }

        char operand[16];
        if (sscanf(p, "%*s %15s", operand) != 1) {
            printf("Missing operand for %s\n", word);
            exit(1);
        }

        short addr;
        if (isdigit(operand[0]) || operand[0] == '-') {
            addr = atoi(operand);
        } else {
            // Check if it's a label first
            short label_addr = get_label_addr(operand);
            if (label_addr >= 0) {
                addr = label_addr;
            } else {
                // Otherwise, it's a variable
                addr = get_var_addr(operand);
            }
        }

        cpu->memory[pc++] = opcode * 100 + addr;
    }

    fclose(f);
}

// Executes a single instruction
void execute(CPU *cpu, short instruction, short memory_address, Deck *deck) {
    switch (instruction) {
        case 0: { // Input from deck
            short to_load = deck->deck[deck->deck_counter++];
            cpu->memory[memory_address] = to_load;
            break;
        }

        case 1: // Load memory to ACC
            cpu->accumulator = cpu->memory[memory_address];
            break;

        case 2: // Add memory to ACC
            cpu->accumulator += cpu->memory[memory_address];
            break;

        case 3: // Branch if negative
            if (cpu->accumulator < 0)
                cpu->pc = memory_address - 1;
            break;

        case 4: { // Shift ACC
            short l = (cpu->memory[memory_address] / 10) % 10;
            short r = cpu->memory[memory_address] % 10;
            for (int i = 0; i < l; i++) cpu->accumulator *= 10;
            for (int i = 0; i < r; i++) cpu->accumulator /= 10;
            break;
        }

        case 5: // Output memory
            printf("%d\n", cpu->memory[memory_address]);
            break;

        case 6: // Store ACC to memory
            cpu->memory[memory_address] = cpu->accumulator;
            break;

        case 7: // Subtract memory from ACC
            cpu->accumulator -= cpu->memory[memory_address];
            break;

        case 8: // Jump (save return at 99)
            cpu->memory[99] = 800 + cpu->pc;
            cpu->pc = memory_address - 1;
            break;

        case 9: // HALT
            cpu->running = 0;
            break;

        default:
            cpu->running = 0;
            break;
    }
}

// Run CPU
void run(CPU *cpu, Deck *deck) {
    while (cpu->running) {
        short instr_code = cpu->memory[cpu->pc];
        cpu->instr_reg = instr_code / 100;
        short addr = instr_code % 100;

        execute(cpu, cpu->instr_reg, addr, deck);
        cpu->pc++;

        if (cpu->pc >= MEM_SIZE) cpu->running = 0;
    }
}

// Load deck
void load_deck(Deck *deck, const char *filename) {
    FILE *fptr = fopen(filename, "r");
    if (!fptr) { deck->deck_counter = 0; return; }
    short counter = 0, number;
    while (counter < DECK_SIZE && fscanf(fptr, "%hd", &number) == 1)
        deck->deck[counter++] = number;
    deck->deck_counter = 0;
    fclose(fptr);
}

int has_asmc_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    return dot && !strcmp(dot, ".asmc");
}

// Main
int main(int argc, char *argv[]) {

    if (argc == 2 && !strcmp(argv[1], "--help")) {
        printf(
            "cardiac — CARDIAC-like assembler & VM\n\n"
            "Usage:\n"
            "  cardiac <program.asmc> [-in deck.txt]\n\n"
            "Options:\n"
            "  -in <file>      Input deck file (default: deck.txt)\n"
            "  --help          Show this help\n"
            "  --version       Show version\n"
        );
        return 0;
    }

    if (argc == 2 && !strcmp(argv[1], "--version")) {
        printf("cardiac version 0.1.1\n");
        return 0;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: cardiac <program.asmc> [-in deck.txt]\n");
        return 1;
    }

    if (!has_asmc_extension(argv[1])) {
        fprintf(stderr, "cardiac: error: '%s' is not a .asmc file\n", argv[1]);
        return 1;
    }

    const char *asm_file = argv[1];
    const char *deck_file = "deck.txt";

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-in") && i + 1 < argc) {
            deck_file = argv[++i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    CPU cpu = {0};
    Deck deck = {0};

    cpu.running = 1;
    cpu.pc = 10;

    assemble(asm_file, &cpu);
    load_deck(&deck, deck_file);
    cpu.memory[0] = 1;

    run(&cpu, &deck);
    return 0;
}
