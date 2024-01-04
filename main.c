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

    printf("Pixel format: %s\n", SDL_GetPixelFormatName(surface->format->format));          // Different images might be in a different pixel format
    int width = surface->w;
    int height = surface->h;
    printf("Width: %d\n", width);
    printf("Height: %d\n", height);

    // Create a window
    window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        printf("Couldn't create window. Error: %s.\n", SDL_GetError());
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL)
    {
        printf("Couldn't create renderer. Error: %s.\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Locking the surface for safe, direct access to the pixels
    if(SDL_LockSurface(surface) != 0)
    {
        printf("Couldn't lock the surface. Error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // WORK IN PROGRESS
    // Implementation of Sobel algorithm for edge detection
    // Add validation for pixel format
    Uint8 pixel;            // Because of SDL_PIXELFORMAT_INDEX8
    int pixel_index;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            pixel_index = y * surface->pitch + x;

            pixel = ((Uint8 *)surface->pixels)[pixel_index];
            pixel += 0x80;

            ((Uint8 *)surface->pixels)[pixel_index] = pixel;
        }
    }

    SDL_UnlockSurface(surface);

    // Create a texture from surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if(texture == NULL)
    {
        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Copy texture to rendering buffer
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);

    // Display content of rendering buffer
    SDL_RenderPresent(renderer);            // I should use SDL_RenderClear() before drawing new frame
    SDL_DestroyRenderer(renderer);

    // Loop to keep the program running
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

    // Clean up resources
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}