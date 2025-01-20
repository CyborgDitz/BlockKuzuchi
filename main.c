#include <stdint.h>
#include <stdio.h>
#include "raylib.h"

#define WINDOW_HEIGHT 1024
#define WINDOW_WIDTH 1024

#define TILE_WIDTH 32
#define TILE_HEIGHT 32
int32_t map[WINDOW_HEIGHT][WINDOW_WIDTH];

typedef struct {
    bool destroyed;
} BlockData;
typedef struct {
    Vector2 position;
    float playerSpeed;
} Player;

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
    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++) {

            if (*(map + i * WINDOW_WIDTH + j) == 1) {
                DrawRectangle(j * TILE_WIDTH, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, WHITE);
            }
        }
    }
}
void UpdatePlayer(Player *player, float delta) {
    if (IsKeyDown(KEY_LEFT)) player->position.x -= delta;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += delta;
};
int main(void)
{
    InitWindow(800, 600, "BlockKuzuchi");

    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++) {
            if ((i * WINDOW_WIDTH + j) % 2 == 0) {
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