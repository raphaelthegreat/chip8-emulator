#include <stdio.h>
#include <Windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <thread>
#include "chip8.hpp"

// Display size
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

Chip8 chip8;
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

void update_texture(const Chip8& c8);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void setup_texture();

unsigned char pixels[SCREEN_HEIGHT][SCREEN_WIDTH][3];

int main()
{
	// Load game
	if (!chip8.load("assets/pong2.c8"))
		return 1;
	
	// Setup OpenGL
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(320, 320, "Window", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	setup_texture();

	while (!glfwWindowShouldClose(window)) {
		std::this_thread::sleep_for(std::chrono::microseconds(8));
		chip8.emulate_cycle();

		if (chip8.draw_flag) {
			glClear(GL_COLOR_BUFFER_BIT);

			update_texture(chip8);

			glfwSwapBuffers(window);
			chip8.draw_flag = false;
		}

		glfwGetWindowSize(window, &display_width, &display_height);

		display_width *= modifier;
		display_height *= modifier;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, display_width, display_height, 0);
		glMatrixMode(GL_MODELVIEW);

		glfwPollEvents();
	}


	return 0;
}

// Setup Texture
void setup_texture()
{
	// Clear screen
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
		for (int x = 0; x < SCREEN_WIDTH; ++x)
			pixels[y][x][0] = pixels[y][x][1] = pixels[y][x][2] = 0;

	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixels);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void update_texture(const Chip8& c8)
{
	// Update pixels
	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 64; ++x)
			if (c8.gfx[(y * 64) + x] == 0)
				pixels[y][x][0] = pixels[y][x][1] = pixels[y][x][2] = 0;	// Disabled
			else
				pixels[y][x][0] = pixels[y][x][1] = pixels[y][x][2] = 255;  // Enabled

	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixels);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);		glVertex2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
	glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
	glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0, display_height);
	glEnd();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod)
{    
	bool pressed = action;

	if (key == GLFW_KEY_1)	chip8.key[0x1] = pressed;
	if (key == GLFW_KEY_2)	chip8.key[0x2] = pressed;
	if (key == GLFW_KEY_3)	chip8.key[0x3] = pressed;
	if (key == GLFW_KEY_4)	chip8.key[0xC] = pressed;

	if (key == GLFW_KEY_Q)	chip8.key[0x4] = pressed;
	if (key == GLFW_KEY_W)	chip8.key[0x5] = pressed;
	if (key == GLFW_KEY_E)	chip8.key[0x6] = pressed;
	if (key == GLFW_KEY_R)	chip8.key[0xD] = pressed;

	if (key == GLFW_KEY_A)	chip8.key[0x7] = pressed;
	if (key == GLFW_KEY_S)	chip8.key[0x8] = pressed;
	if (key == GLFW_KEY_D)	chip8.key[0x9] = pressed;
	if (key == GLFW_KEY_F)	chip8.key[0xE] = pressed;

	if (key == GLFW_KEY_Z)	chip8.key[0xA] = pressed;
	if (key == GLFW_KEY_X)	chip8.key[0x0] = pressed;
	if (key == GLFW_KEY_C)	chip8.key[0xB] = pressed;
	if (key == GLFW_KEY_V)	chip8.key[0xF] = pressed;
}




