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
    Vector2 velocity;
    float playerSpeed;
} Player;


void DrawTutorial(void);
void DrawWindow(int32_t *map);
void DrawPlayer(Player *player);

typedef struct {
    void (*DrawTutorial)(void);
    void (*DrawWindow)(int32_t *map);
    void (*DrawPlayer)(Player *player);
} Drawings;

Player InitializePlayer(Vector2 position, float speed) {
    Player player = {0};
    player.position = position;
    player.playerSpeed = speed;
    return player;
}

void DrawPlayer(Player *player) {
    DrawRectangle(player->position.x, player->position.y, TILE_WIDTH * 5, TILE_HEIGHT * 1, PURPLE);
}

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
void UpdatePlayer(Player *player, float deltaTime) {
    if (IsKeyDown(KEY_A)) {
        player->velocity.x = -player->playerSpeed;
    } else if (IsKeyDown(KEY_D)) {
        player->velocity.x = player->playerSpeed;
    } else {
        player->velocity.x = 0;
    }

    if (IsKeyDown(KEY_W)) {
        player->velocity.y = -player->playerSpeed;
    } else if (IsKeyDown(KEY_S)) {
        player->velocity.y = player->playerSpeed;
    } else {
        player->velocity.y = 0;  // No vertical movement
    }

    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;
}

int main(void)
{
    InitWindow(800, 600, "BlockKuzuchi");
    Player player = InitializePlayer((Vector2){100, 100}, 300.0f);
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
    myDrawings.DrawPlayer = DrawPlayer;
    Player player = InitializePlayer((Vector2){100, 100}, 300.0f);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdatePlayer(&player, deltaTime);

        BeginDrawing();
        ClearBackground(YELLOW);

        myDrawings.DrawWindow((int32_t *)map);
        myDrawings.DrawTutorial();
        myDrawings.DrawPlayer(&player);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
