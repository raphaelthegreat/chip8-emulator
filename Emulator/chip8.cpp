#include "chip8.hpp"
#include <iomanip>

void debug(unsigned short opcode) {
	std::cout << "Executing instruction: " << std::hex << "0x" << opcode << "\n";
}

unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::init()
{
	// Init pointers
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	draw_flag = true;

	// Clear Buffers
	for (int i = 0; i < WIDTH * HEIGHT; i++) gfx[i] = 0;
	for (int i = 0; i < STACK_SIZE; i++) stack[i] = 0;
	for (int i = 0; i < REGISTER_COUNT; i++) V[i] = 0;
	for (int i = 0; i < 16; i++) key[i] = 0;
	for (int i = 0; i < MEMORY_COUNT; i++) memory[i] = 0;
	for (int i = 0; i < 80; i++) memory[i] = chip8_fontset[i];

	srand(time(NULL));

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
}

bool Chip8::load(const std::string& program)
{
	// Open file
	FILE* pFile = fopen(program.c_str(), "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	char* buffer = new char[sizeof(char) * lSize];
	if (buffer == NULL)
	{
		std::cout << "Memory error\n";
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		std::cout << "Reading error\n";
		return false;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > lSize)
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		std::cout << "Error: ROM too big for memory\n";

	// Close file, free buffer
	fclose(pFile);
	delete buffer;
}

void Chip8::emulate_cycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];

	switch (opcode & 0xF000) {

	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen
			for (int i = 0; i < WIDTH * HEIGHT; i++) gfx[i] = 0x0;
			draw_flag = true;
			pc += 2; break;

		case 0x000E: // 0x00EE: Returns from subroutine
			--sp;
			pc = stack[sp];
			pc += 2;
			break;
		}
		break;

	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;
	
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;
	
	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[15] = 1; // Carry
			else
				V[15] = 0;

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005:
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[15] = 0; // Borrow
			else
				V[15] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2; break;

		case 0x0006:
			V[15] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2; break;

		case 0x0007:
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[15] = 0; // Borrow
			else
				V[15] = 1;

			
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2; break;

		case 0x000E:
			V[15] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2; break;

		default:
			std::cout << std::hex << "Unknown opcode [0x8000]: 0x" << opcode << "\n";
		}
		break;

	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2; break;
	
	case 0xB000:
		pc = V[0] + (opcode & 0x0FFF);
		break;

	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = (rand() & 0xFF) & (opcode & 0x00FF);
		pc += 2; break;

	case 0xD000:
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[15] = 0;
		for (int yline = 0; yline < height; yline++) {
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++) {
				if ((pixel & (0x80 >> xline)) != 0) {
					if (gfx[x + xline + ((y + yline) * 64)] == 1)
						V[15] = 1;
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		draw_flag = true;
		pc += 2;

		break;
	}

	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1:
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		
		default:
			std::cout << std::hex << "Unknown opcode [0xE000]: 0x" << opcode << "\n";
		}
		break;

	case 0xf000:
		switch (opcode & 0x00FF) {
		
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2; break;

		case 0x000A:
		{
			bool key_press = false;

			for (int i = 0; i < 16; i++) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					key_press = true;
				}
			}

			if (!key_press)
				return; // Skip cycle

			pc += 2;
			break;
		}

		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2; break;

		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2; break;

		case 0x001E:
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[15] = 1; // Overflow
			else
				V[15] = 0;

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2; break;

		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2; break;

		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[opcode & 0x0F00 >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055:
			for (int i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
				memory[I + i] = V[i];
			}

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2; break;

		case 0x0065:
			for (int i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
				 V[i] = memory[I + i];
			}

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2; break;

		default:
			std::cout << std::hex << "Unknown opcode [0xF000]: 0x" << opcode << "\n";
		}
		break;

	default:
		std::cout << std::hex << "Unknown opcode [0x0000]: 0x" << opcode << "\n";
	}

	//debug(opcode);

	if (delay_timer > 0) --delay_timer;
	if (sound_timer > 0) {
		if (sound_timer == 1) std::cout << "BEEP\n";
		--sound_timer;
	}
}

void Chip8::set_keys()
{
}
