#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    char *file = "test.png";            // In future add option to select a file
                                        // For now image is already in grayscale

    // Initialize SDL (defaults subsystems and Video)
    if(SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        printf("Couldn't initialize SDL. Error: %s.\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_image
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG; // Combining two flags together
    if((IMG_Init(img_flags) & img_flags) != img_flags)
    {
        printf("Couldn't initialize SDL_image. Error: %s.\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // Loading image from file into a surface to access pixel data
    surface = IMG_Load(file);
    if(surface == NULL)
    {
        printf("Couldn't load: %s. Error: %s.\n", file, IMG_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a window
    window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, surface->w, surface->h, 0);
    if(window == NULL)
    {
        printf("Couldn't create window. Error: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL)
    {
        printf("Couldn't create renderer. Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a texture from surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture == NULL)
    {
        printf("Couldn't create texture. Error: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Copy texture to rendering buffer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Display content of rendering buffer
    SDL_RenderPresent(renderer);            // I should use SDL_RenderClear() before drawing new frame

    while(1)
    {
        if(SDL_PollEvent(&event) == 1)
        {
            if(event.type == SDL_QUIT)
            {
                break;
            }
        }
    }

    // Shutdown all subsystems
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_FreeSurface(surface);
    IMG_Quit();
    SDL_Quit();

    return 0;
}