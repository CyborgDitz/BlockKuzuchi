#include <stdint.h>
#include <stdio.h>
#include "raylib.h"

#define gWindowHeight 1024
#define gWindowWidth 1024

#define gTileWidth 32
#define gTileHeight 32
int32_t map[gWindowHeight][gWindowWidth];

typedef struct {
    bool destroyed;
} BlockData;
typedef struct {
    Vector2 position;
    float playerSpeed;
}Player;

void DrawTutorial(void);
void DrawWindow(int32_t *map);

typedef struct {
    void (*DrawTutorial)(void);
    void (*DrawWindow)(int32_t *map);
} Drawings;

void DrawTutorial() {
    DrawRectangle(10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 250, 113, BLUE);
    DrawText("Controls:", 20, 20, 10, BLACK);
    DrawText("- Right/Left to move", 40, 40, 10, DARKGRAY);
}

void DrawWindow(int32_t *map) {
    for (int i = 0; i < gWindowHeight; i++) {
        for (int j = 0; j < gWindowWidth; j++) {

            if (*(map + i * gWindowWidth + j) == 1) {
                DrawRectangle(j * gTileWidth, i * gTileHeight, gTileWidth, gTileHeight, WHITE);
            }
        }
    }
}
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta) {
    if (IsKeyDown(KEY_LEFT)) player->position.x -= PLAYER_HOR_SPD*delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += PLAYER_HOR_SPD*delta;
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

    Drawings myDrawings;
    myDrawings.DrawTutorial = DrawTutorial;
    myDrawings.DrawWindow = DrawWindow;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(YELLOW);

        myDrawings.DrawWindow((int32_t *)map);
        myDrawings.DrawTutorial();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}