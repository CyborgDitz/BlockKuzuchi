#include <stdint.h>
#include <stdio.h>

#include "raylib.h"

#define gWindowHeight 1024
#define gWindowWidth 1024

#define gTileWidth 32
#define gTileHeight 32

typedef struct {
    int32_t x, y;
} position;


int32_t map[gWindowHeight][gWindowWidth];

typedef struct {
    int32_t width;
    int32_t height;
    int32_t color;
    bool destroyed;
} blockData;
//todo blocks.

void DrawTutorial()
{
    DrawRectangle( 10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines( 10, 10, 250, 113, BLUE);
    DrawText("Controls:", 20, 20, 10, BLACK);
    DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
};
int main(void)
{
    InitWindow(800, 600, "BlockKuzuchi");

    for (int i = 0; i < gWindowHeight; i++) {
        for (int j = 0; j < gWindowWidth; j++) {
            if ((i * gWindowWidth + j) % 2 == 0) {
                map[i][j] = 1;
            }
        }
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(YELLOW);

        for (int i = 0; i < gWindowHeight; i++) {
            for (int j = 0; j < gWindowWidth; j++) {
                if (map[i][j] == 1) {
                    DrawRectangle(j * gTileWidth, i * gTileHeight, gTileWidth, gTileHeight, WHITE);
                }
            }
        }
        DrawTutorial();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
