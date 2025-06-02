## Project Overview

Witcher Tracker is a command interpreter and inventory-event tracking system inspired by the world of *The Witcher*.  
This project simulates gameplay mechanics where the player manages Geralt’s inventory, potion formulas, and monster encounters through text-based commands.  
The system parses input commands, updates internal state (ingredients, potions, trophies), and produces responses according to a strict grammar specification.

## Features

- Command parsing with grammar validation  
- Inventory management for ingredients, potions, and monster trophies  
- Brewing potions using learned recipes  
- Tracking monster weaknesses and encounters  
- Strict handling of invalid input commands  
- Modular design using C++ object-oriented programming principles

## Guide
This project includes both C and C++ implementations, each with its own compilation and execution commands.
### For C Implementation:
Compile the C source code using gcc
Run the executable and provide input as required
```bash
gcc -o witchertracker src/main.c
./witchertracker < input.txt

### For C++ Implementation:
g++ -o witchertracker src/main.cpp
./witchertracker < input.txt



