#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

void render_buttons_with_text(SDL_Rect add_button_rect, SDL_Rect save_button_rect, SDL_Texture *add_button, SDL_Texture *save_button, SDL_Rect add_text_rect, SDL_Rect save_text_rect, SDL_Texture *add_text, SDL_Texture *save_text, SDL_Renderer *renderer);
bool is_clicked_in_rect(int mouse_x, int mouse_y, SDL_Rect *rect);
void load_save_file_dialog_box(char *file_path, size_t buffer_size, int option);
SDL_Surface *sobel_algorithm(SDL_Surface *surface);

int main(int argc, char *argv[])
{
    // Initialize SDL (defaults subsystems and Video)
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
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

    // Initialize SDL_ttf
    if(TTF_Init() != 0)
    {
        printf("Couldn't initialize SDL_ttf. Error: %s.\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Buttons icon resolution - 32 px (they are a square so width == height)
    SDL_Surface *add_button_surface = IMG_Load("src/icons/add_icon.png");
    if(add_button_surface == NULL)
    {
        printf("Couldn't load add icon. Error: %s.\n", IMG_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
    int button_res = add_button_surface->w;

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Edge Detection", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, button_res * 20, button_res, SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        printf("Couldn't create window. Error: %s.\n", SDL_GetError());
        SDL_FreeSurface(add_button_surface);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL)
    {
        printf("Couldn't create renderer. Error: %s.\n", SDL_GetError());
        SDL_FreeSurface(add_button_surface);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create textures for buttons
    SDL_Texture *add_button_texture = SDL_CreateTextureFromSurface(renderer, add_button_surface);
    SDL_FreeSurface(add_button_surface);

    SDL_Surface *save_button_surface = IMG_Load("src/icons/save_icon.png");
    if(save_button_surface == NULL)
    {
        printf("Couldn't load save icon. Error: %s.\n", IMG_GetError());
        SDL_DestroyTexture(add_button_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
    SDL_Texture *save_button_texture = SDL_CreateTextureFromSurface(renderer, save_button_surface);
    SDL_FreeSurface(save_button_surface);

    // Determines places where buttons are located
    SDL_Rect add_button_rect = {5, 0, button_res, button_res};
    SDL_Rect save_button_rect = {(button_res * 10) + 5, 0, button_res, button_res};

    // Open font from file - OpenSans 20 px
    TTF_Font *font = TTF_OpenFont("OpenSans.ttf", 20);
    if(font == NULL)
    {
        printf("Couldn't open font. Error: %s.\n", TTF_GetError());
        SDL_DestroyTexture(add_button_texture);
        SDL_DestroyTexture(save_button_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Color text_color = {0, 0, 0, 255}; // Black

    // Add button text
    SDL_Surface *add_text_surface = TTF_RenderText_Solid(font, "Load file", text_color);
    SDL_Texture *add_text_texture = SDL_CreateTextureFromSurface(renderer, add_text_surface);
    SDL_Rect add_text_rect = {button_res + 10, 0, add_text_surface->w, add_text_surface->h};
    SDL_FreeSurface(add_text_surface);

    // Save button text
    SDL_Surface *save_text_surface = TTF_RenderText_Solid(font, "Save file", text_color);
    SDL_Texture *save_text_texture = SDL_CreateTextureFromSurface(renderer, save_text_surface);
    SDL_Rect save_text_rect = {(button_res * 10 + button_res) + 10, 0, save_text_surface->w, save_text_surface->h};
    SDL_FreeSurface(save_text_surface);

    render_buttons_with_text(add_button_rect, save_button_rect, add_button_texture, save_button_texture, add_text_rect, save_text_rect, add_text_texture, save_text_texture, renderer);

    SDL_Surface *processed_surface;

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

                char file_path[512];

                // User clicked add file button
                if(is_clicked_in_rect(mouse_x, mouse_y, &add_button_rect))
                {
                    load_save_file_dialog_box(file_path, sizeof(file_path), 1);

                    // If user closed load file dialog box
                    if(file_path[0] == '\0')
                    {
                        printf("You didn't select image to load.\n");
                        continue;
                    }

                    // Load selected image into a surface to access pixel data
                    SDL_Surface *surface = IMG_Load(file_path);
                    if(surface == NULL)
                    {
                        printf("Couldn't load image. Error: %s.\n", IMG_GetError());
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
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
                    save_text_rect.x = width + button_res + 10;
                    render_buttons_with_text(add_button_rect, save_button_rect, add_button_texture, save_button_texture, add_text_rect, save_text_rect, add_text_texture, save_text_texture, renderer);

                    // Create a original texture from surface before edge detection
                    SDL_Texture *origiranl_texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if(origiranl_texture == NULL)
                    {
                        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
                        SDL_FreeSurface(surface);
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Copy original texture to rendering buffer
                    SDL_Rect dstrect_original_texture = {0, button_res, width, height};
                    SDL_RenderCopy(renderer, origiranl_texture, NULL, &dstrect_original_texture);
                    SDL_DestroyTexture(origiranl_texture);

                    // Create 8-bit surface
                    SDL_Surface *eight_bit_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
                    if(eight_bit_surface == NULL)
                    {
                        printf("Couldn't convert surface pixel format. Error: %s.\n", SDL_GetError());
                        SDL_FreeSurface(surface);
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Palette configuration for 8-bit surface
                    SDL_Color grayscale_palette[256];
                    for(int i = 0; i < 256; i++)
                    {
                        grayscale_palette[i].r = i;
                        grayscale_palette[i].g = i;
                        grayscale_palette[i].b = i;
                    }
                    if(SDL_SetPaletteColors(eight_bit_surface->format->palette, grayscale_palette, 0, 256) != 0)
                    {
                        printf("Couldn't set all of the colors. Error: %s.\n", SDL_GetError());
                        SDL_FreeSurface(eight_bit_surface);
                        SDL_FreeSurface(surface);
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Lock the surfaces for safe, direct access to the pixels
                    if(SDL_LockSurface(surface) != 0 || SDL_LockSurface(eight_bit_surface) != 0)
                    {
                        printf("Couldn't lock the surfaces. Error: %s.\n", SDL_GetError());
                        SDL_FreeSurface(eight_bit_surface);
                        SDL_FreeSurface(surface);
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }

                    // Convert image to grayscale
                    Uint8 r, g, b;
                    Uint32 rgb_pixel;
                    Uint8 grayscale_pixel;

                    for(int y = 0; y < height; y++)
                    {
                        for(int x = 0; x < width; x++)
                        {
                            rgb_pixel = ((Uint32 *)surface->pixels)[y * surface->pitch / 4 + x];

                            // Extract RGB values from pixel and calculate the value of gray pixel based on linear luminance
                            SDL_GetRGB(rgb_pixel, surface->format, &r, &g, &b);
                            grayscale_pixel = (Uint8) (0.299 * r + 0.587 * g + 0.114 * b);

                            ((Uint8 *)eight_bit_surface->pixels)[y * eight_bit_surface->pitch + x] = grayscale_pixel;
                        }
                    }

                    // Unlock the surfaces
                    SDL_UnlockSurface(surface);
                    SDL_FreeSurface(surface);
                    SDL_UnlockSurface(eight_bit_surface);

                    // Apply Sobel algorithm on converted surface
                    processed_surface = sobel_algorithm(eight_bit_surface);
                    if(processed_surface == NULL)
                    {
                        printf("Couldn't perform Sobel edge detection algorithm. Error: %s.\n", SDL_GetError());
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
                        IMG_Quit();
                        SDL_Quit();
                        return 1;
                    }
                    
                    // Create a processed texture from processed surface after edge detection
                    SDL_Texture *processed_texture = SDL_CreateTextureFromSurface(renderer, processed_surface);
                    if(processed_texture == NULL)
                    {
                        printf("Couldn't create texture. Error: %s.\n", SDL_GetError());
                        SDL_FreeSurface(processed_surface);
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(add_button_texture);
                        SDL_DestroyTexture(add_text_texture);
                        SDL_DestroyTexture(save_button_texture);
                        SDL_DestroyTexture(save_text_texture);
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_Quit();
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
                    load_save_file_dialog_box(file_path, sizeof(file_path), 2);

                    // If user closed save file dialog box
                    if(file_path[0] == '\0')
                    {
                        printf("You didn't select file to save image.\n");
                        continue;
                    }

                    // Save processed image to selected file
                    if(IMG_SavePNG(processed_surface, file_path) == 0)
                    {
                        SDL_FreeSurface(processed_surface);
                    }
                    else
                    {
                        printf("Couldn't save processed image. Error: %s.\n", IMG_GetError());
                    }
                }
            }
        }
    }

    // Clean up resources
    SDL_FreeSurface(processed_surface);
    TTF_CloseFont(font);
    SDL_DestroyTexture(add_button_texture);
    SDL_DestroyTexture(add_text_texture);
    SDL_DestroyTexture(save_button_texture);
    SDL_DestroyTexture(save_text_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}


void render_buttons_with_text(SDL_Rect add_button_rect, SDL_Rect save_button_rect, SDL_Texture *add_button, SDL_Texture *save_button, SDL_Rect add_text_rect, SDL_Rect save_text_rect, SDL_Texture *add_text, SDL_Texture *save_text, SDL_Renderer *renderer)
{
    // Set background color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    SDL_RenderClear(renderer);

    // Render buttons
    SDL_RenderCopy(renderer, add_button, NULL, &add_button_rect);
    SDL_RenderCopy(renderer, save_button, NULL, &save_button_rect);

    // Render text
    SDL_RenderCopy(renderer, add_text, NULL, &add_text_rect);
    SDL_RenderCopy(renderer, save_text, NULL, &save_text_rect);

    // Display rendered buttons with text
    SDL_RenderPresent(renderer);
}


bool is_clicked_in_rect(int mouse_x, int mouse_y, SDL_Rect *rect)
{
    return (mouse_x >= rect->x && mouse_x <= rect->x + rect->w && mouse_y >= rect->y && mouse_y <= rect->h);
}


void load_save_file_dialog_box(char *file_path, size_t buffer_size, int option)
{
    // Initialize and set up OPENFILENAME structure
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFile = file_path;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = buffer_size;

    // Option to load file
    if(option == 1)
    {
        ofn.lpstrFilter = "Images (*.png, *.jpg)\0*.png;*.jpg\0";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display open file dialog box
        if(GetOpenFileName(&ofn) != TRUE)
        {
            file_path[0] = '\0';
        }
    }
    // Option to save file
    else if(option == 2)
    {
        ofn.lpstrFilter = "PNG (*.png)\0*.png\0";
        ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = "png";

        // Display save file dialog box
        if(GetSaveFileName(&ofn) != TRUE)
        {
            file_path[0] = '\0';
        }
    }
}


SDL_Surface *sobel_algorithm(SDL_Surface *surface)
{
    // Make copy of surface
    SDL_Surface *processed_surface = SDL_DuplicateSurface(surface);
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
    const int HORIZONTAL_KERNEL[3][3] = {
        {-1, 0, 1}, 
        {-2, 0, 2}, 
        {-1, 0, 1}
    };

    const int VERTICAL_KERNEL[3][3] = {
        {1, 2, 1}, 
        {0, 0, 0}, 
        {-1, -2, -1}
    };
      
    Uint8 surrounding_pixel_value;
    int gradient_horizontal, gradient_vertical;
    int magnitude;
    int threshold = 100;

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

                    gradient_horizontal += surrounding_pixel_value * HORIZONTAL_KERNEL[j + 1][i + 1];
                    gradient_vertical += surrounding_pixel_value * VERTICAL_KERNEL[j + 1][i + 1];
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

    // Unlock the surfaces
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
    SDL_UnlockSurface(processed_surface);

    return processed_surface;
}