#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Surface *load_image(char *file);
SDL_Surface *sobel_algorithm(SDL_Surface *surface);
// void save_image(SDL_Surface *processed);

int main(int argc, char *argv[])
{
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

    // Load files with button images and store their dimensions (needed for window size)
    SDL_Surface *button_surface = load_image("src/icons/add_icon.png");
    int button_height = button_surface->h;
    int button_width = button_surface->w;

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 301 * 2, 263 + button_height, SDL_WINDOW_SHOWN); // I SET CONST WINDOW SIZE
    if(window == NULL)
    {
        printf("Couldn't create window. Error: %s.\n", SDL_GetError());
        // SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL)
    {
        printf("Couldn't create renderer. Error: %s.\n", SDL_GetError());
        SDL_DestroyWindow(window);
        // SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Set background color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    SDL_RenderClear(renderer);

    // Render buttons MODIFY IN FUTURE
    SDL_Texture *button_texture = SDL_CreateTextureFromSurface(renderer, button_surface);
    SDL_Rect dstrect_add_file = {5, 0, button_width, button_height};
    SDL_RenderCopy(renderer, button_texture, NULL, &dstrect_add_file);

    button_surface = load_image("src/icons/save_icon.png");
    button_texture = SDL_CreateTextureFromSurface(renderer, button_surface);
    SDL_Rect dstrect_save_file = {301 + 5, 0, button_width, button_height}; // CHANGE PLACE IN FUTURE
    SDL_RenderCopy(renderer, button_texture, NULL, &dstrect_save_file);
    SDL_FreeSurface(button_surface);
    SDL_DestroyTexture(button_texture);

    // Main while loop to keep the program running
    int only_once = 1;

    bool quit = false;
    SDL_Event event;
    while(!quit)
    {
        // Loop to handle events
        while(SDL_PollEvent(&event) == 1)
        {
            // Quit when user clicks exit button
            if(event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if(only_once == 1)
            {
                // Surface to hold loaded image
                SDL_Surface *surface = load_image(file);
                if(surface == NULL)
                {
                    printf("Couldn't load: %s. Error: %s.\n", file, IMG_GetError());
                    IMG_Quit();
                    SDL_Quit();
                    return 1;
                }

                // Surface dimensions
                int width = surface->w;
                int height = surface->h;

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

                // Copy original texture to rendering buffer
                SDL_Rect dstrect_original_texture = {0, button_height, width, height};
                SDL_RenderCopy(renderer, origiranl_texture, NULL, &dstrect_original_texture);
                SDL_DestroyTexture(origiranl_texture);

                SDL_Surface *processed_surface;
                processed_surface = sobel_algorithm(surface);
                if(processed_surface == NULL)
                {
                    printf("Couldn't perform Sobel edge detection algorithm. Error: %s.\n", SDL_GetError());
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_FreeSurface(surface);
                    IMG_Quit();
                    SDL_Quit();
                    return 1;
                }
                
                // Create a processed texture from processed surface after edge detection
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
                SDL_Rect dstrect_processed_texture = {width, button_height, width, height};
                SDL_RenderCopy(renderer, processed_texture, NULL, &dstrect_processed_texture);
                SDL_DestroyTexture(processed_texture);

                // Display content of rendering buffer
                SDL_RenderPresent(renderer);

                only_once++;
            }
        }
    }

    // Clean up resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


SDL_Surface *load_image(char *file)
{
    // Loading image from file into a surface to access pixel data
    SDL_Surface *surface = IMG_Load(file);
    if(surface == NULL)
    {
        return NULL;
    }

    return surface;
}


SDL_Surface *sobel_algorithm(SDL_Surface *surface)
{
    // Make copy of surface
    SDL_Surface *processed_surface;
    processed_surface = SDL_ConvertSurface(surface, surface->format, 0);
    if(processed_surface == NULL)
    {
        return NULL;
    }

    // Lock the surfaces for safe, direct access to the pixels
    if(SDL_LockSurface(surface) != 0 || SDL_LockSurface(processed_surface) != 0)
    {
        return NULL;
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
    for(int y = 1; y < surface->h - 1; y++)
    {
        for(int x = 1; x < surface->w - 1; x++)
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
            if(magnitude > threshold)
            {
                ((Uint8 *)processed_surface->pixels)[y * processed_surface->pitch + x] = 0xFF; // White
            }
            else
            {
                ((Uint8 *)processed_surface->pixels)[y * processed_surface->pitch + x] = 0x00; // Black
            }
        }
    }

    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);

    SDL_UnlockSurface(processed_surface);

    return processed_surface;
}