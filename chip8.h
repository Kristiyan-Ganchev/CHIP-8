#include <vcruntime_string.h>
#include <iostream>
#include <fstream>
#include <string>
class chip8 {
public:
	unsigned char memory[4096];
	unsigned char gfx[64 * 32];
	unsigned short PC;
	unsigned short I;
	unsigned short stack[16];
	unsigned short sp;
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned char v[16];
	unsigned short instruction;
	bool set_vx_to_vy_8XY6_and_8XYE=true;
	SDL_Keycode keymap[16] = {
	SDLK_1, SDLK_2, SDLK_3, SDLK_4,  // 0-3
	SDLK_q, SDLK_w, SDLK_e, SDLK_r,  // 4-7
	SDLK_a, SDLK_s, SDLK_d, SDLK_f,  // 8-B
	SDLK_z, SDLK_x, SDLK_c, SDLK_v   // C-F
	};
	unsigned char inputs[16];
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

	inline void init() {
		PC = 0x200;
		instruction = 0;
		I = 0;
		sp = 0x0;
		memset(gfx, 0, sizeof(gfx));
		memset(memory, 0, sizeof(memory));
		memset(stack, 0, sizeof(stack));
		memset(v, 0, sizeof(v));
		memset(inputs, 0, sizeof(inputs));
		for (int i = 0; i < 80; ++i)
		{
			memory[0x50 + i] = chip8_fontset[i];
		}
		delay_timer = CHAR_MAX;
		sound_timer = CHAR_MAX;
	}
	inline void load(std::string filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (file.is_open()) {
			std::streampos size = file.tellg(); // How big is the file?
			char* buffer = new char[size];      // Allocate buffer to hold it

			file.seekg(0, std::ios::beg);        // Go back to start
			file.read(buffer, size);             // Read the file into buffer
			file.close();                        // Done reading

			// Copy buffer into memory starting at 0x200
			for (int i = 0; i < size; ++i) {
				memory[0x200 + i] = buffer[i];
			}

			delete[] buffer; // Clean up
		}
		else {
			std::cerr << "Failed to open ROM: " << filename << std::endl;
		}

	}
	inline void fetch() {
		 if (PC < 0x200 || PC >= 0x1000) { // Check if PC is outside the valid range of memory
        std::cerr << "Error: Invalid PC value (" << std::hex << PC << ") during fetch!" << std::endl;
        system("pause");  // Pause to investigate the issue
    }

    instruction = memory[PC] << 8 | memory[PC + 1];  // Fetch the instruction (2 bytes)
    
    std::cout << "Fetched instruction: " << std::hex << instruction << " at PC: " << std::hex << PC << std::endl;
    
    PC += 2; 
	}
	inline void decode() {
		switch (instruction&0xF000) {
			case 0x0000:{
				switch (instruction & 0x00FF) {
					case 0x00E0: {
					memset(gfx, 0, sizeof(gfx) / sizeof(gfx[0]) * sizeof(gfx[0]));
					break;
					}
					case 0x00EE: {
					std::cout << "RET: Before pop, sp: " << sp << std::endl;
					--sp;  // Decrement sp to point to the correct return address
					PC = stack[sp];  // Pop the return address from the stack
					std::cout << "RET: After pop, new sp: " << sp << ", new PC: " << PC << std::endl;
					break;
					}
				break;
				}
				break;
			}
			case 0x1000: {
				PC = instruction & 0x0FFF;
				break;
			}
			case 0x2000: {
				std::cout << "CALL: Before push, PC: " << std::hex << PC << ", sp: " << sp << std::endl;
				stack[sp] = PC;  // Push the current PC onto the stack
				++sp;  // Increment sp after the push

				PC = instruction & 0x0FFF;  // Set the PC to the subroutine address

				std::cout << "CALL: After push, new PC: " << std::hex << PC << ", new sp: " << sp << std::endl;
				break;
			}
			case 0x3000: {
				if (v[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)) PC += 2;
				break;
			}
			case 0x4000: {
				if (v[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)) PC += 2;
				break;
			}
			case 0x5000: {
				if (v[(instruction & 0x0F00) >> 8] == v[(instruction & 0x00F0) >> 4]) PC += 2;
				break;
			}
			case 0x6000: {
				v[(instruction & 0x0F00)>>8] = instruction & 0x00FF;
				break;
			}
			case 0x7000: {
				v[(instruction & 0x0F00)>>8] += instruction & 0x00FF;
				break;
			}
			case 0x8000: {
				switch (instruction & 0x000F) {
					case 0x0000: {
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0001: {
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] | v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0002: {
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] & v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0003: {
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] ^ v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0004: {
						if (v[(instruction & 0x0F00) >> 8] + v[(instruction & 0x00F0) >> 4] > 255) v[0xF] = 1;
						else v[0xF] = 0;
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] + v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0005: {
						if (v[(instruction & 0x0F00) >> 8] > v[(instruction & 0x00F0) >> 4]) v[0xF] = 1;
						else v[0xF] = 0;
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] - v[(instruction & 0x00F0) >> 4];
						break;
					}
					case 0x0006: {
						if (set_vx_to_vy_8XY6_and_8XYE) {
							v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x00F0) >> 4];
						}
						v[0xF] = v[(instruction & 0x0F00) >> 8] & 0x01;
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8]>>1;
						break;
					}
					case 0x0007: {
						if (v[(instruction & 0x0F00) >> 8] > v[(instruction & 0x00F0) >> 4]) v[0xF] = 1;
						else v[0xF] = 0;
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x00F0) >> 4]- v[(instruction & 0x0F00) >> 8];
						break;
					}
					case 0x000E: {
						if (set_vx_to_vy_8XY6_and_8XYE) {
							v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x00F0) >> 4];
						}
						v[0xF] = v[(instruction & 0x0F00) >> 8] & 0x01;
						v[(instruction & 0x0F00) >> 8] = v[(instruction & 0x0F00) >> 8] << 1;
						break;
					}
				}
				break;
			}
			case 0x9000: {
				if (v[instruction & 0x0F00 >> 8] != v[instruction & 0x00F0 >> 4]) PC += 2;
				break;
			}
			case 0xA000: {
				I = instruction & 0x0FFF;
				break;
			}
			case 0xB000: {
				PC = instruction & 0x0FFF + v[0x0];
				break;
			}
			case 0xC000: {
				srand(time(0));
				v[(instruction & 0x0F00) >> 8] =  (rand() % 256) &(instruction&0x00FF);
				break;
			}	
			case 0xD000: {
				char x= v[(instruction & 0x0F00)>>8];
				char y= v[(instruction & 0x00F0)>>4];
				unsigned char height = instruction & 0x000F;
				unsigned short pixel;
				v[0xF] = 0;
				for (int yLine = 0; yLine < height; yLine++)
				{
					pixel = memory[I+yLine];
					for (int xLine = 0; xLine < 8; xLine++)
					{
						if ((pixel & 0x80 >> xLine) != 0) {
							if (gfx[(x+xLine+((y+yLine)*64))] == 1) {
								v[0xF] = 1;
							}
							gfx[(x+xLine+((y+yLine)*64))]^=1;
						}
					}
				}
				break;
			}
			//EX9E and EXA1-------------------------------------------------------------------------------------------------------------------------------------
			case 0xE000: {
				switch (instruction & 0x00FF) {
					case 0x009E: {
						if (inputs[v[(instruction & 0x0F00) >> 8]] == 1) PC += 2;
						break;
					}
					case 0x00A1: {
						if (inputs[v[(instruction & 0x0F00) >> 8]] == 0) PC += 2;
						break;
					}
				}
				break;
			}
			case 0xF000: {
				switch (instruction & 0x00FF) {
					case 0x0007:{
						v[(instruction & 0x0F00) >> 8] = delay_timer;
						break;
					}
					case 0x0015: {
						delay_timer=v[(instruction & 0x0F00) >> 8];
						break;
					}
					case 0x0018: {
						sound_timer=v[(instruction & 0x0F00) >> 8];
						break;
					}
					case 0x001E: {
						I += v[(instruction & 0x0F00) >> 8];
						break;
					}
					case 0x000A: {
						bool key_found = false;
						for (int i = 0; i < 16; ++i) {
							if (inputs[i]) {
								v[(instruction & 0x0F00) >> 8] = i;
								key_found = true;
								break;
							}
						}
						if (!key_found)
							PC -= 2;
						break;
					}
					case 0x0029: {
						I = 0x50 + (5* v[(instruction & 0x0F00) >> 8]);
						break;
					}
					case 0x0033: {
						memory[I] = v[(instruction & 0x0F00) >> 8] / 100;
						memory[I+1] = (v[(instruction & 0x0F00) >> 8] / 10) % 10;
						memory[I+2] = (v[(instruction & 0x0F00) >> 8] % 100) %10;
						break;
					}
					case 0x0055: {
						for (int i = 0; i <= ((instruction&0x0F00)>>8); i++)
						{
							memory[I+i]=v[i];
						}
						break;
					}
					case 0x0065: {
						for (int i = 0; i <= ((instruction & 0x0F00) >> 8); i++)
						{
							v[i]=memory[I + i];
						}
						break;
					}
				}
				break;
			}
		}
	}
	inline bool handle_input() {
		bool quit=false;
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
					break;
				}
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
						{
							quit = true;
							break;
						}
						case SDLK_x:
						{
							inputs[0] = 1;
							break;
						}
						case SDLK_1:
						{
							inputs[1] = 1;
							break;
						}case SDLK_2:
						{
							inputs[2] = 1;
							break;
						}case SDLK_3:
						{
							inputs[3] = 1;
							break;
						}case SDLK_q:
						{
							inputs[4] = 1;
							break;
						}case SDLK_w:
						{
							inputs[5] = 1;
							break;
						}case SDLK_e:
						{
							inputs[6] = 1;
							break;
						}case SDLK_a:
						{
							inputs[7] = 1;
							break;
						}case SDLK_s:
						{
							inputs[8] = 1;
							break;
						}case SDLK_d:
						{
							inputs[9] = 1;
							break;
						}case SDLK_z:
						{
							inputs[10] = 1;
							break;
						}case SDLK_c:
						{
							inputs[11] = 1;
							break;
						}case SDLK_4:
						{
							inputs[12] = 1;
							break;
						}case SDLK_r:
						{
							inputs[13] = 1;
							break;
						}case SDLK_f:
						{
							inputs[14] = 1;
							break;
						}case SDLK_v:
						{
							inputs[15] = 1;
							break;
						}
					}
					break;
				}
				case SDL_KEYUP: {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
						{
							quit = true;
							break;
						}
						case SDLK_x:
						{
							inputs[0] = 0;
							break;
						}
						case SDLK_1:
						{
							inputs[1] = 0;
							break;
						}case SDLK_2:
						{
							inputs[2] = 0;
							break;
						}case SDLK_3:
						{
							inputs[3] = 0;
							break;
						}case SDLK_q:
						{
							inputs[4] = 0;
							break;
						}case SDLK_w:
						{
							inputs[5] = 0;
							break;
						}case SDLK_e:
						{
							inputs[6] = 0;
							break;
						}case SDLK_a:
						{
							inputs[7] = 0;
							break;
						}case SDLK_s:
						{
							inputs[8] = 0;
							break;
						}case SDLK_d:
						{
							inputs[9] = 0;
							break;
						}case SDLK_z:
						{
							inputs[10] = 0;
							break;
						}case SDLK_c:
						{
							inputs[11] = 0;
							break;
						}case SDLK_4:
						{
							inputs[12] = 0;
							break;
						}case SDLK_r:
						{
							inputs[13] = 0;
							break;
						}case SDLK_f:
						{
							inputs[14] = 0;
							break;
						}case SDLK_v:
						{
							inputs[15] = 0;
							break;
						}
					}
					break;
				}
			}
		}
		return quit;
	}
	inline void print_display(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Set color to black (for clearing the screen)
		SDL_RenderClear(renderer);  // Clear the screen

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Set color to white for pixels

		for (int i = 0; i < 64 * 32; i++) {
			if (gfx[i] == 1) {
				// Calculate x and y coordinates based on the index
				int x = (i % 64) * 10;  // 10 pixels per width
				int y = (i / 64) * 10;  // 10 pixels per height

				// Draw rectangle for each pixel
				SDL_Rect pixelRect = { x, y, 10, 10 };  // 10x10 pixel squares
				SDL_RenderFillRect(renderer, &pixelRect);  // Fill the rectangle with the current color
			}
		}

		SDL_RenderPresent(renderer);  // Update the screen
	}
};