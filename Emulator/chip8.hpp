#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

#define WIDTH 64
#define HEIGHT 32
#define STACK_SIZE 16
#define REGISTER_COUNT 16
#define MEMORY_COUNT 4096

class Chip8 {
public:
	Chip8() = default;
	~Chip8() = default;

	void init();
	bool load(const std::string& program);

	void emulate_cycle();
	void set_keys();

public:
	bool draw_flag;

	// Display pixels
	unsigned char gfx[WIDTH * HEIGHT];

	// Keyboard
	unsigned char key[16];

protected:
	unsigned short opcode;

	unsigned char memory[MEMORY_COUNT]; // Memory of the system
	unsigned char V[REGISTER_COUNT]; // Registers (we need 16 8-bit registers)

	// We need the Index Register and Program Counter (pc)
	unsigned short I;
	unsigned short pc;

	// Timer registers
	unsigned char delay_timer;
	unsigned char sound_timer;

	// Stack of the interperter to remember the current program location
	unsigned short stack[STACK_SIZE];
	unsigned short sp;
};