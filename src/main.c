#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define bmp_signature_offset 0
#define bmp_filesize_offset 2
#define bmp_width_offset 18
#define bmp_height_offset 22

int window_width = 200;
int window_height = 150;
bool done = 0;

typedef struct {
        int path;
        int height;
        int width;
        int bit_depth;
} imagemetadata;

int main(int argc, char **argv) {
        if (argc != 2) {
                printf("%s takes one argument (file path).\n", argv[0]);
                return 1;
        }
        FILE *imageptr = fopen(argv[1], "rb");
        if (!imageptr) {
                printf("Failed to open file %s.\n", argv[1]);
        }
        char *fullpath;
        realpath(argv[1], fullpath);

        // check whether the given file is a bitmap
        uint8_t signature[2];
        if (fread(signature, 1, 2, imageptr) != 2 || signature[0] != 'B' || signature[1] != 'M') {
                printf("File %s is not a bitmap.\n", argv[1]);
                fclose(imageptr);
                return -1;
        }

        uint16_t bit_depth;
        fseek(imageptr, 28, SEEK_SET);
        fread(&bit_depth, sizeof(uint16_t), 1, imageptr);
        printf("Bit depth: %d\n", bit_depth);
        // set image dimensions as window height
        if (fseek(imageptr, bmp_width_offset, SEEK_SET) != 0 ||
            fread(&window_width, sizeof(int32_t), 1, imageptr) != 1 ||
            fread(&window_height, sizeof(int32_t), 1, imageptr) != 1) {
                printf("Failed to read dimensions.\n");
                return -1;
        }

        char title[1024];
        sprintf(title, "CImageView: %s", fullpath);

        SDL_Window *window =
            SDL_CreateWindow(title, window_width, window_height, SDL_WINDOW_RESIZABLE);

        if (!window) {
                printf("Window creation failed: %s\n", SDL_GetError());
                return 1;
        }

        uint32_t bmp_data_offset;
        fseek(imageptr, 10, SEEK_SET);
        fread(&bmp_data_offset, sizeof(uint32_t), 1, imageptr);
        printf("Data offset (value): %d\n", bmp_data_offset);

        int bmp_pitch = ((window_width * 3 + 3) & ~3);
        uint8_t *pixels = malloc(bmp_pitch * window_height);
        fseek(imageptr, bmp_data_offset, SEEK_SET);
        fread(pixels, 1, bmp_pitch * window_height, imageptr);

        SDL_Surface *windowsurface = SDL_GetWindowSurface(window);
        printf("Surface format: %s\n", SDL_GetPixelFormatName(windowsurface->format));

        Uint8 *dst_bytes = (Uint8 *)windowsurface->pixels;
        for (int y = 0; y < window_height; y++) {
                for (int x = 0; x < window_width; x++) {
                        Uint8 *src = pixels + (y * bmp_pitch + x * 3);
                        Uint8 *dst =
                            dst_bytes + ((window_height - 1 - y) * windowsurface->pitch + x * 4);
                        dst[0] = src[0]; // B
                        dst[1] = src[1]; // G
                        dst[2] = src[2]; // R
                        dst[3] = 0;      // X (unused)
                }
        }

        printf("Width: %d, Height: %d, bmp_pitch: %d\n", window_width, window_height, bmp_pitch);
        SDL_UpdateWindowSurface(window);
        printf("Surface pitch: %d, width * 4: %d\n", windowsurface->pitch, window_width * 4);
        printf("Data offset: %d\n", bmp_data_offset);
        while (!done) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_EVENT_QUIT) {
                                done = 1;
                        }
                }
        }
        SDL_DestroyWindow(window);
        SDL_Quit();
        fclose(imageptr);
        return 0;
}
