#include <stdint.h>
#include <stdio.h>

#include "raylib.h"

#define G_WINDOW_HEIGHT 1024
#define G_WINDOW_WIDTH 1024

#define G_TILE_WIDTH 32
#define G_TILE_HEIGHT 32

typedef struct {
    int32_t x, y;
} position;

int32_t map[G_WINDOW_HEIGHT][G_WINDOW_WIDTH];

typedef struct {
    int32_t width;
    int32_t height;
    int32_t color;
    bool destroyed;
} blockData;
//todo blocks.

int main(void)
{
    InitWindow(800, 600, "BlockKuzuchi");
    SetTargetFPS(60);

    for (int i = 0; i < G_WINDOW_HEIGHT; i++) {
        for (int j = 0; j < G_WINDOW_WIDTH; j++) {
            if ((i * G_WINDOW_WIDTH + j) % 2 == 0) {
                map[i][j] = 1;
            }
        }
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(YELLOW);

        for (int i = 0; i < G_WINDOW_HEIGHT; i++) {
            for (int j = 0; j < G_WINDOW_WIDTH; j++) {
                if (map[i][j] == 1) {
                    DrawRectangle(j * G_TILE_WIDTH, i * G_TILE_HEIGHT, G_TILE_WIDTH, G_TILE_HEIGHT, WHITE);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
