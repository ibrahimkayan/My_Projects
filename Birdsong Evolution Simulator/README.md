

## Overview

**Evochirp** is a low-level simulation of bird song evolution, implemented entirely in **x86-64 GNU Assembly**.  
It models the transformation of birdsongs across generations, applying species-specific rules to a sequence of musical notes and operations.

The project highlights how **Assembly language** can be used to simulate structured behaviors with full control over memory, registers, and system-level operations.

## Problem Description

Each birdsong starts with a sequence of notes (`C`, `T`, `D`) followed by one or more operators (`+`, `-`, `*`, `H`).  
Depending on the bird species — **Sparrow**, **Warbler**, or **Nightingale** — each operator modifies the song in a unique way.  
The program parses and applies these transformations step by step, printing the updated song after every operation.

## Key Features

- Parsing user input via syscall read  
- Tokenization of notes and operators with 4-byte alignment  
- Species-specific song transformation logic via `jmp` and labels  
- Modular code structure with dedicated handlers per species  
- Manual memory and stack management  
- Custom integer-to-string conversion for generation tracking  
- Syscall-based printing of evolving song states  

## Architecture

- **Memory Layout**: All buffers are statically allocated in the `.bss` section (no dynamic allocation).  
- **Registers Used**:
  - `%r14` – input token index  
  - `%r10` – output size tracker  
  - `%r12–%r13` – temp registers  
  - `%r15` – base pointer in nested routines  
- **Flow Control**: Species determined by first letter, then `jmp` to corresponding handler (`Sparrow`, `Warbler`, `Nightingale`).  
- **Processing**: Operators modify output buffer with custom logic.  
- **Rendering**: A `render_integer_as_string` function prints generation numbers with stack-safe syscall output.

## Compilation & Execution

To build and run the simulator (using Makefile):

```bash
make
./evochirp

## Guide
If the commands mentioned above does not work properly, try to run it manually by required commands (You may apply the help of AI) and test the inputs manually.
