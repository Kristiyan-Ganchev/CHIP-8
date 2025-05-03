//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include "chip8.h"
void init_SDL(SDL_Window** window, SDL_Renderer** renderer, int width = 640, int height = 320) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    *window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!*window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }
}
int main(int argc, char* argv[])
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    chip8 chip8;

    chip8.init();
    chip8.load("../Pong.ch8");//test_opcode.ch8 IBM Logo.ch8
    init_SDL(&window, &renderer);

    bool running = true;
    SDL_Event e;

    while (running) {
        running = !chip8.handle_input();
        chip8.fetch();
        chip8.decode();
        chip8.print_display(renderer);
        if (chip8.delay_timer > 0)
        {
            --chip8.delay_timer;
        }
        if (chip8.sound_timer > 0)
        {
            --chip8.sound_timer;
        }
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}