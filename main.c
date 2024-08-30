#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>

// Max characters in the editor and max files
#define MAX_TEXT_LENGTH 1024
#define MAX_FILES 10

// Structure to manage each source file
typedef struct {
    char fileName[256];
    char textBuffer[MAX_TEXT_LENGTH];
    int isModified;
} SourceFile;

// Function declarations
void ClearEditor(SourceFile* file);
void RenderEditor(SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor, const char* textBuffer, SDL_Rect editorRect);
void RenderTabs(SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor, SourceFile* files, int currentFileIndex, int numFiles);
void OpenNewFile(SourceFile* files, int* numFiles, int* currentFileIndex, const char* fileName);
void SaveCurrentFile(SourceFile* file);
int IsValidCharacter(SDL_Keycode key);

int main(int argc, char* args[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }

    // Create the window
    SDL_Window* window = SDL_CreateWindow("ErfanIDE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create the renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load the font
    TTF_Font* font = TTF_OpenFont("C:\\Users\\erfan\\Downloads\\SDL2-2.30.6\\fonts\\calibri.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Set text color to white
    SDL_Color textColor = { 255, 255, 255, 255 };

    // Source file management
    SourceFile files[MAX_FILES];
    int numFiles = 0; // Start with no files open
    int currentFileIndex = -1;

    // Open a new file to start with
    OpenNewFile(files, &numFiles, &currentFileIndex, "main.c");

    // Main loop flag
    int quit = 0;

    // Event handler
    SDL_Event e;

    // Main loop
    while (!quit) {
        // Handle events on the queue
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);

                // Handle tab clicks
                for (int i = 0; i < numFiles; i++) {
                    if (x > 10 + i * 150 && x < 10 + i * 150 + 140 && y > 80 && y < 110) {
                        currentFileIndex = i;
                        printf("Switched to file: %s\n", files[i].fileName);
                    }
                }

            }
            else if (e.type == SDL_KEYDOWN) {
                // Handle Backspace separately
                if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(files[currentFileIndex].textBuffer) > 0) {
                    files[currentFileIndex].textBuffer[strlen(files[currentFileIndex].textBuffer) - 1] = '\0'; // Handle backspace
                    files[currentFileIndex].isModified = 1;
                }
                // Handle valid character input
                else if (IsValidCharacter(e.key.keysym.sym)) {
                    if (strlen(files[currentFileIndex].textBuffer) < MAX_TEXT_LENGTH - 1) {
                        // Handle Shift for capital letters
                        char key[2];
                        if (SDL_GetModState() & KMOD_SHIFT) {
                            key[0] = toupper(e.key.keysym.sym);
                        }
                        else {
                            key[0] = e.key.keysym.sym;
                        }
                        key[1] = '\0';
                        strcat_s(files[currentFileIndex].textBuffer, MAX_TEXT_LENGTH, key);
                        files[currentFileIndex].isModified = 1;
                    }
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the welcome text
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Welcome to ErfanIDE!", textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // Render tabs for open files
        RenderTabs(renderer, font, textColor, files, currentFileIndex, numFiles);

        // Define the text editor area
        SDL_Rect editorRect = { 10, 150, 780, 440 }; // Position and size
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color for the editor box
        SDL_RenderDrawRect(renderer, &editorRect);

        // Render the text inside the editor for the current file
        RenderEditor(renderer, font, textColor, files[currentFileIndex].textBuffer, editorRect);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Clean up and shut down SDL
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// Function to clear the editor's text
void ClearEditor(SourceFile* file) {
    file->textBuffer[0] = '\0';
    file->isModified = 0;
}

// Function to render the text inside the editor area
void RenderEditor(SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor, const char* textBuffer, SDL_Rect editorRect) {
    if (textBuffer == NULL || strlen(textBuffer) == 0) {
        // Skip rendering if text is empty
        printf("Text buffer is empty, skipping rendering.\n");
        return;
    }

    // Create the surface for the text
    SDL_Surface* editorSurface = TTF_RenderText_Solid(font, textBuffer, textColor);
    if (editorSurface == NULL) {
        printf("Failed to create editor surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    // Create texture from surface
    SDL_Texture* editorTexture = SDL_CreateTextureFromSurface(renderer, editorSurface);
    if (editorTexture == NULL) {
        printf("Failed to create editor texture! SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(editorSurface);
        return;
    }

    SDL_Rect renderQuad = { editorRect.x + 5, editorRect.y + 5, editorSurface->w, editorSurface->h };
    SDL_RenderCopy(renderer, editorTexture, NULL, &renderQuad);

    SDL_FreeSurface(editorSurface);
    SDL_DestroyTexture(editorTexture);
}

// Function to render tabs for open files
void RenderTabs(SDL_Renderer* renderer, TTF_Font* font, SDL_Color textColor, SourceFile* files, int currentFileIndex, int numFiles) {
    for (int i = 0; i < numFiles; i++) {
        SDL_Color tabColor = (i == currentFileIndex) ? (SDL_Color) { 0, 255, 0, 255 } : textColor;
        SDL_Surface* tabSurface = TTF_RenderText_Solid(font, files[i].fileName, tabColor);
        if (tabSurface == NULL) {
            printf("Failed to create tab surface! SDL_ttf Error: %s\n", TTF_GetError());
            continue;
        }

        SDL_Texture* tabTexture = SDL_CreateTextureFromSurface(renderer, tabSurface);
        if (tabTexture == NULL) {
            printf("Failed to create tab texture! SDL Error: %s\n", SDL_GetError());
            SDL_FreeSurface(tabSurface);
            continue;
        }

        SDL_Rect tabRect = { 10 + i * 150, 80, tabSurface->w, tabSurface->h };
        SDL_RenderCopy(renderer, tabTexture, NULL, &tabRect);

        SDL_FreeSurface(tabSurface);
        SDL_DestroyTexture(tabTexture);
    }
}

// Function to open a new file
void OpenNewFile(SourceFile* files, int* numFiles, int* currentFileIndex, const char* fileName) {
    if (*numFiles < MAX_FILES) {
        *currentFileIndex = *numFiles;
        strcpy_s(files[*numFiles].fileName, sizeof(files[*numFiles].fileName), fileName);
        ClearEditor(&files[*numFiles]);
        (*numFiles)++;
        printf("Opened new file: %s\n", fileName);
    }
    else {
        printf("Cannot open more files. Maximum limit reached.\n");
    }
}

// Function to save the current file
void SaveCurrentFile(SourceFile* file) {
    FILE* fp = fopen(file->fileName, "w");
    if (fp) {
        fprintf(fp, "%s", file->textBuffer);
        fclose(fp);
        file->isModified = 0;
        printf("Saved file: %s\n", file->fileName);
    }
    else {
        printf("Failed to save file: %s\n", file->fileName);
    }
}

// Function to check if a key is a valid character for text input
int IsValidCharacter(SDL_Keycode key) {
    // Allow only valid ASCII characters (letters, numbers, punctuation, space, etc.)
    return (key >= SDLK_SPACE && key <= SDLK_z);
}
