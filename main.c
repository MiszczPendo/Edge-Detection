#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char *argv[])
{
    SDL_Surface *surface;
    SDL_Window *window;
    SDL_Renderer *renderer;
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

    printf("Pixel format: %s\n", SDL_GetPixelFormatName(surface->format->format));
    int width = surface->w;
    int height = surface->h;
    printf("Width: %d\n", width);
    printf("Height: %d\n", height);

    // Create a window
    window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 2, height, SDL_WINDOW_SHOWN);
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

    // Create a original texture from surface before edge detection
    SDL_Texture *origiranl_texture;
    origiranl_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(origiranl_texture == NULL)
    {
        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_RenderClear(renderer);

    // Copy original texture to rendering buffer
    SDL_Rect dstrect_original_texture = {0, 0, width, height};
    SDL_RenderCopy(renderer, origiranl_texture, NULL, &dstrect_original_texture);
    SDL_DestroyTexture(origiranl_texture);

    // Make copy of surface
    SDL_Surface *processed_surface;
    processed_surface = SDL_ConvertSurface(surface, surface->format, 0);
    if(processed_surface == NULL)
    {
        printf("Couldn't make a copy of surface. Error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Lock the surfaces for safe, direct access to the pixels
    if(SDL_LockSurface(surface) != 0 || SDL_LockSurface(processed_surface) != 0)
    {
        printf("Couldn't lock the surface. Error: %s.\n", SDL_GetError());
        SDL_FreeSurface(processed_surface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Implementation of Sobel algorithm for edge detection
    // Horizontal kernel
    //  -1  0  1
    //  -2  0  2
    //  -1  0  1
    const int HORIZONTAL[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

    // Vertical kernel
    //   1  2  1
    //   0  0  0
    //  -1 -2 -1
    const int VERTICAL[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
      
    Uint8 surrounding_pixel_value;            // Because of SDL_PIXELFORMAT_INDEX8
    int gradient_horizontal, gradient_vertical;
    int magnitude;
    int threshold = 200;

    // Loops going through every pixel (excluding borders)
    for(int y = 1; y < height - 1; y++)
    {
        for(int x = 1; x < width - 1; x++)
        {
            gradient_horizontal = 0;
            gradient_vertical = 0;

            // Loops going through pixels surrounding main pixel
            for(int j = -1; j <= 1; j++)
            {
                for(int i = -1; i <= 1; i++)
                {
                    surrounding_pixel_value = ((Uint8 *)surface->pixels)[(y + j) * surface->pitch + (x + i)];

                    gradient_horizontal += surrounding_pixel_value * HORIZONTAL[j + 1][i + 1];
                    gradient_vertical += surrounding_pixel_value * VERTICAL[j + 1][i + 1];
                }
            }

            // Gradient magnitude
            magnitude = (int) SDL_sqrt(SDL_pow(gradient_horizontal, 2) + SDL_pow(gradient_vertical, 2));

            // Set pixel color based on threshold (white if it's an edge or black if it's not)
            if(magnitude > threshold) {
                ((Uint8 *)processed_surface->pixels)[y * processed_surface->pitch + x] = 0xFF; // White
            } else {
                ((Uint8 *)processed_surface->pixels)[y * processed_surface->pitch + x] = 0x00; // Black
            }
        }
    }
    
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);

    SDL_UnlockSurface(processed_surface);

    // Create a processed texture from processed_surface after edge detection
    SDL_Texture *processed_texture;
    processed_texture = SDL_CreateTextureFromSurface(renderer, processed_surface);
    SDL_FreeSurface(processed_surface);
    if(processed_texture == NULL)
    {
        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Copy processed texture to rendering buffer
    SDL_Rect dstrect_processed_texture = {width, 0, width, height};
    SDL_RenderCopy(renderer, processed_texture, NULL, &dstrect_processed_texture);
    SDL_DestroyTexture(processed_texture);

    // Display content of rendering buffer
    SDL_RenderPresent(renderer);
    SDL_DestroyRenderer(renderer);

    // Loop to keep the program running
    SDL_Event event;
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