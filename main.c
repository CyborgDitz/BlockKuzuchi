#include <stdint.h>
#include <stdio.h>
#include "raylib.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 1280
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
int32_t map[WINDOW_HEIGHT][WINDOW_WIDTH];

typedef struct {
    bool isActive;
} BlockData;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float playerSpeed;
    bool isActive;
    int lives;
} Player;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    bool isActive;
} Ball;

void DrawTutorial(void);
void DrawWindow(int32_t *map);
void DrawPlayer(Player *player);
void DrawBall(Ball *ball);

typedef struct {
    void (*DrawTutorial)(void);
    void (*DrawWindow)(int32_t *map);
    void (*DrawPlayer)(Player *player);
    void (*DrawBall)(Ball *ball);
} Drawings;

Player InitializePlayer(Vector2 position, float speed) {
    Player player = {0};
    player.position = position;
    player.playerSpeed = speed;
    player.velocity = (Vector2){0, 0};
    return player;
}

Ball InitializeBall(float x, float y, float radius, float speed) {
    Ball ball;
    ball.position.x = x;
    ball.position.y = y;
    ball.radius = radius;
    ball.speed = speed;
    ball.velocity = (Vector2){speed, speed};
    ball.isActive = false;
    return ball;
}

void DrawPlayer(Player *player) {
    DrawRectangle(player->position.x, player->position.y, TILE_WIDTH * 5, TILE_HEIGHT * 1, PURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->position, ball->radius, BLACK);
}

void DrawTutorial() {
    DrawRectangle(10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 250, 113, BLUE);
    DrawText("Controls:", 20, 20, 10, BLACK);
    DrawText("- A/D to move", 40, 40, 10, DARKGRAY);
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

    player->position.x += player->velocity.x * deltaTime;
    player->position.y += player->velocity.y * deltaTime;
}

void UpdateBall(Ball *ball, Player *player, float deltaTime) {
    if (!ball->isActive && IsKeyPressed(KEY_SPACE)) {
        ball->isActive = true;
        ball->velocity.x = 200.0f;
        ball->velocity.y = -200.0f;
    }

    if (ball->isActive) {
        ball->position.x += ball->velocity.x * deltaTime;
        ball->position.y += ball->velocity.y * deltaTime;
    } else {
        ball->position.x = player->position.x + (TILE_WIDTH * 5) / 2;
        ball->position.y = player->position.y + TILE_HEIGHT;
    }
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BlockKuzuchi");

    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++) {
            if ((i * WINDOW_WIDTH + j) % 2 == 0) {
                map[i][j] = 1;
            }
        }
    }
    Vector2 playerPosition = {100, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitializePlayer(playerPosition, 300.0f);
    Ball ball = InitializeBall(player.position.x + (TILE_WIDTH * 5) / 2, player.position.y + TILE_HEIGHT, 16.0f, 200.0f);

    Drawings myDrawings;
    myDrawings.DrawTutorial = DrawTutorial;
    myDrawings.DrawWindow = DrawWindow;
    myDrawings.DrawPlayer = DrawPlayer;
    myDrawings.DrawBall = DrawBall;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        UpdatePlayer(&player, deltaTime);
        UpdateBall(&ball, &player, deltaTime);

        BeginDrawing();
        ClearBackground(YELLOW);

        myDrawings.DrawWindow((int32_t *)map);
        myDrawings.DrawTutorial();
        myDrawings.DrawPlayer(&player);
        myDrawings.DrawBall(&ball);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}