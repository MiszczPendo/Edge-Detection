#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Surface *open_file_dialog_to_load_file();
void render_buttons(SDL_Rect add_button_rect, SDL_Rect save_button_rect, SDL_Texture *add_button_texture, SDL_Texture *save_button_texture, SDL_Renderer *renderer);
SDL_Surface *sobel_algorithm(SDL_Surface *surface);
bool is_clicked_in_rect(int mouse_x, int mouse_y, SDL_Rect *rect);
// void save_image(SDL_Surface *processed);

int main(int argc, char *argv[])
{
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

    // Buttons icon resolution (they are a square so width == height)
    SDL_Surface *add_button_surface = IMG_Load("src/icons/add_icon.png");
    int button_res = add_button_surface->w;

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, button_res * 20, button_res, SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        printf("Couldn't create window. Error: %s.\n", SDL_GetError());
        SDL_FreeSurface(add_button_surface);
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
        SDL_FreeSurface(add_button_surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create textures
    SDL_Texture *add_button_texture = SDL_CreateTextureFromSurface(renderer, add_button_surface);   // Maybe I should add validation for creating texture
    SDL_FreeSurface(add_button_surface);
    SDL_Surface *save_button_surface = IMG_Load("src/icons/save_icon.png");
    SDL_Texture *save_button_texture = SDL_CreateTextureFromSurface(renderer, save_button_surface);
    SDL_FreeSurface(save_button_surface);

    // Determines places where buttons are located
    SDL_Rect add_button_rect = {5, 0, button_res, button_res};
    SDL_Rect save_button_rect = {(button_res * 10) + 5, 0, button_res, button_res};

    render_buttons(add_button_rect, save_button_rect, add_button_texture, save_button_texture, renderer);

    // Main loop to keep the program running
    bool quit = false;
    SDL_Event event;
    while(!quit)
    {
        // Loop to handle events
        while(SDL_PollEvent(&event) == 1)
        {
            // Quit when user clicked exit button
            if(event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                // Variables storing coordinates where user clicked
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;

                // User clicked add file button
                if(is_clicked_in_rect(mouse_x, mouse_y, &add_button_rect))
                {
                    printf("Add file!\n");

                    // Surface to hold loaded image
                    SDL_Surface *surface = open_file_dialog_to_load_file();
                    if(surface == NULL)
                    {
                        printf("Error: %s.\n", IMG_GetError());
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Surface resolution
                    int width = surface->w;
                    int height = surface->h;

                    // Resize window to fit 2 images on it
                    SDL_SetWindowSize(window, width * 2, height + button_res);
                    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

                    save_button_rect.x = width + 5;
                    render_buttons(add_button_rect, save_button_rect, add_button_texture, save_button_texture, renderer);

                    // Create a original texture from surface before edge detection
                    SDL_Texture *origiranl_texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if(origiranl_texture == NULL)
                    {
                        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Copy original texture to rendering buffer
                    SDL_Rect dstrect_original_texture = {0, button_res, width, height};
                    SDL_RenderCopy(renderer, origiranl_texture, NULL, &dstrect_original_texture);
                    SDL_DestroyTexture(origiranl_texture);

                    SDL_Surface *processed_surface = sobel_algorithm(surface);
                    if(processed_surface == NULL)
                    {
                        printf("Couldn't perform Sobel edge detection algorithm. Error: %s.\n", SDL_GetError());
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        SDL_FreeSurface(surface);
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }
                    
                    // Create a processed texture from processed surface after edge detection
                    SDL_Texture *processed_texture = SDL_CreateTextureFromSurface(renderer, processed_surface);
                    SDL_FreeSurface(processed_surface);
                    if(processed_texture == NULL)
                    {
                        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Copy processed texture to rendering buffer
                    SDL_Rect dstrect_processed_texture = {width, button_res, width, height};
                    SDL_RenderCopy(renderer, processed_texture, NULL, &dstrect_processed_texture);
                    SDL_DestroyTexture(processed_texture);

                    // Display content of rendering buffer
                    SDL_RenderPresent(renderer);
                }

                // User clicked save file button
                if(is_clicked_in_rect(mouse_x, mouse_y, &save_button_rect))
                {
                    printf("Save file!\n");
                }
            }
        }
    }

    // Clean up resources
    SDL_DestroyTexture(add_button_texture);
    SDL_DestroyTexture(save_button_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}


void render_buttons(SDL_Rect add_button_rect, SDL_Rect save_button_rect, SDL_Texture *add_button_texture, SDL_Texture *save_button_texture, SDL_Renderer *renderer)
{
    // Set background color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    SDL_RenderClear(renderer);

    // Render buttons
    SDL_RenderCopy(renderer, add_button_texture, NULL, &add_button_rect);
    SDL_RenderCopy(renderer, save_button_texture, NULL, &save_button_rect);

    // Display rendered buttons
    SDL_RenderPresent(renderer);
}


bool is_clicked_in_rect(int mouse_x, int mouse_y, SDL_Rect *rect)
{
    return (mouse_x >= rect->x && mouse_x <= rect->w && mouse_y >= rect->y && mouse_y <= rect->h);
}


SDL_Surface *open_file_dialog_to_load_file()
{
    SDL_Surface *surface;
    char file_name[512];

    // Initialize and set up OPENFILENAME structure
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFile = file_name;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(file_name);
    ofn.lpstrFilter = "Images (*.png, *.jpg)\0*.png;*.jpg\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display open file dialog box
    if (GetOpenFileName(&ofn) == TRUE)
    {
        // Loading selected image into a surface to access pixel data
        surface = IMG_Load(file_name);
        if(surface == NULL)
        {
            printf("Couldn't load image. ");
            return NULL;
        }
    }
    else
    {
        // User closed the open file dialog box
        printf("You didn't select an image. ");
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