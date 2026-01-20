# Cardiac ASM Interpreter

A custom assembly language interpreter for the CARDIAC (CARDboard Illustrative Aid to Computation) computer, featuring label support for jump commands and loops.

## Overview

This project implements a CARDIAC assembly language interpreter with support for variables, labels, and structured control flow. Programs are written in `.asmc` files and executed by the interpreter.

## Running and Downloading

### Download

You can download the latest release binary directly from GitHub:

MacOS / Linux:

```bash
curl -fsSL https://raw.githubusercontent.com/CoryPearl/cardiac/main/install.sh | sh
```

Windows:

```bash
powershell -Command "Invoke-WebRequest -Uri https://github.com/CoryPearl/cardiac/releases/latest/download/cardiac.exe -OutFile '%BIN_PATH%'"
```

### Running

Then run it as so:

```bash
cardiac file_name.asmc
```

You can optinaly add the -in tag to add a deck as input like so:

```bash
cardiac file_name.asmc -in deck_name.txt
```

The interpreter will execute the program and display output to stdout.

## Instruction Set

### Commands

| Instruction      | Description                                          | Example                         |
| ---------------- | ---------------------------------------------------- | ------------------------------- |
| **IN or INP**    | Input a card into memory                             | `IN x or IN mem_location`       |
| **LOAD or CLA**  | Load from memory to accumulator                      | `LOAD x or LOAD mem_location`   |
| **ADD**          | Add memory value to accumulator                      | `ADD x or ADD mem_location`     |
| **SUB**          | Subtract memory from accumulator                     | `SUB x or SUB mem_location`     |
| **STORE or STO** | Store accumulator to memory address                  | `STORE x or STORE mem_location` |
| **TEST or TAC**  | If accumulator is negative, jump to address/label    | `TEST loop`                     |
| **SHIFT or SFT** | Shift accumulator by decimal places (left and right) | `SHIFT 12` # Left 1, Right 1    |
| **OUT**          | Print a memory address value                         | `OUT x or OUT mem_location`     |
| **JUMP or JMP**  | Jump to memory address/label                         | `JUMP start`                    |
| **HALT or HRS**  | Stop program execution                               | `HALT`                          |

### Memory Layout

- **Memory slot 0**: Reserved for bootstrapping (value 001)
- **Memory slots 1-9**: Available for variables (auto-assigned)
- **Memory slots 10-98**: Available for program instructions (auto-assigned)
- **Memory slot 99**: Reserved for jump instruction return address

## Language Features

### Variables

Variables are declared using the `VAR` keyword and can be assigned an initial value:

```asmc
VAR x = 5
VAR y        # Declared without initial value
```

- Variables are automatically assigned to memory locations 1-9 in order
- Maximum of 9 variables per program
- Variables can be redefined

### Labels

Labels provide named targets for jump instructions, enabling loops and structured control flow. Labels can be used with `JUMP` and `TEST` instructions instead of numeric memory addresses.

#### Label Syntax

Labels are defined by appending a colon (`:`) to an identifier:

```asmc
labelname: INSTRUCTION operand
```

Labels can appear in two ways:

1. **On the same line as an instruction** (recommended):

```asmc
start: OUT x # Label and instruction on same line
loop: LOAD y # Another label with instruction
```

2. **On their own line** (if followed by instructions later):

```asmc
loop:        # Label on its own line
LOAD x       # Instruction follows on next line
```

**Important**: Labels **cannot** be on a separate line from code. The following is **not allowed**:

```asmc
# BAD ❌
start:
    OUT x

# GOOD ✅
start: OUT x
```

#### Using Labels with JUMP

Labels can be used with the `JUMP` instruction to create loops or function-like jumps:

```asmc
start: LOAD x
OUT x
JUMP start   # Jump back to start (creates infinite loop)

end: HALT
JUMP end     # Jump forward to end label
```

#### Using Labels with TEST

Labels can be used with the `TEST` instruction for conditional jumps:

```asmc
LOAD x
SUB y
TEST loop    # If accumulator < 0, jump to loop label
OUT x        # Otherwise, continue here

loop: HALT   # Jump target
```

#### Label Rules and Features

- **Label names**: Can contain letters, numbers, and underscores (same rules as variable names)
- **Case sensitivity**: Label names are case-sensitive (`Start:` and `start:` are different)
- **Forward references**: Labels can be used before they are defined:

```asmc
JUMP later   # Jump forward to a label defined later
# ... more code ...
later: OUT x # Label defined here
```

- **Backward references**: Labels can jump backwards to create loops:

```asmc
loop: OUT x  # Label definition
# ... more code ...
JUMP loop    # Jump back to loop
```

- **Multiple labels**: You can have multiple labels in your program:

```asmc
start: LOAD x
middle: ADD y
end: STORE z
```

#### Common Label Patterns

**Simple Loop:**

```asmc
start: OUT counter
ADD one
STORE counter
JUMP start   # Loop back to start
```

**Conditional Loop:**

```asmc
start: LOAD counter
OUT counter
ADD one
STORE counter

LOAD max
SUB counter
TEST done    # If counter >= max, jump to done
JUMP start   # Otherwise, loop back

done: HALT   # Exit when done
```

**Function-like Jump:**

```asmc
# Main code
LOAD x
JUMP print   # Jump to "print" subroutine

# Print subroutine
print: OUT x
JUMP return  # Jump back (you'd need return address tracking)

return: HALT
```

### Comments

Comments are indicated with `#`. Make sure there's a space between the `#` and the end of the line:

```asmc
OUT x # This is a comment
```

## Example Program

Here's a complete example that demonstrates variables, labels, and looping:

```asmc
# Example program demonstrating labels and jump commands
# This program prints numbers in a loop

VAR num = 1
VAR three = 3
VAR one = 1

start: LOAD num # Load number into accumulator
OUT num # Print the number
ADD one # Add 1 to accumulator
STORE num # Store incremented value back

LOAD three # Load the limit (3)
SUB num # Subtract num (3 - num)
TEST loop # If result is negative (num > 3), jump to loop

JUMP start # Otherwise, jump back to start and continue

loop: HALT # Stop when done
```

This program will output:

```
1
2
3
```

## Program Structure

1. **Variable Declarations**: Declare variables at the top of your program
2. **Program Entry**: Use `start:` label to mark the entry point (must be on same line as first instruction)
3. **Control Flow**: Use `JUMP` and `TEST` with labels to create loops and conditionals
4. **Termination**: Always end with `HALT` to stop execution

## File Format

Programs are written in `.asmc` files. (Asembly Cardiac)

## License

This project is provided as-is for educational purposes.
