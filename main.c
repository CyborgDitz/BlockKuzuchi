#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 720
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define BALL_SPEED 320.0f
#define PLAYER_SPEED 300.0f

typedef struct {
    Vector2 position;
    Vector2 velocity;
} BaseEntity;

typedef enum {
    WHITE_BLOCK,
    BLACK_BLOCK,
    BLUE_BLOCK,
    NUM_BLOCK_TYPES
} BlockType;

typedef struct {
    BaseEntity base;
    BlockType type;
    bool isActive;
} BlockData;

typedef struct {
    BaseEntity base;
    float speed;
    int lives;
    float width;
    float height;
} Player;

typedef struct {
    BaseEntity base;
    float speed;
    float radius;
    bool isActive;
} Ball;

Player InitializePlayer(Vector2 position) {
    return (Player){{position, {0, 0}}, PLAYER_SPEED, 3, TILE_WIDTH * 5, TILE_HEIGHT};
}

Ball InitializeBall(float x, float y, float radius) {
    return (Ball){{{x, y}, {0, 0}}, BALL_SPEED, radius, false};
}

void DrawPlayer(Player *player) {
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->base.position, ball->radius, BLACK);
}

void DrawLifeMeter(Player *player) {
    int maxLives = 3;
    int lifeBarWidth = TILE_WIDTH * 10;
    int lifeBarHeight = TILE_HEIGHT / 2;
    int currentLifeWidth = (lifeBarWidth * player->lives) / maxLives;
    DrawRectangle(0, WINDOW_HEIGHT - lifeBarHeight - 10, lifeBarWidth, lifeBarHeight, GRAY);
    DrawRectangle(0, WINDOW_HEIGHT - lifeBarHeight - 10, currentLifeWidth, lifeBarHeight, GREEN);
    DrawRectangleLines(0, WINDOW_HEIGHT - lifeBarHeight - 10, lifeBarWidth, lifeBarHeight, BLACK);
}

void DrawTutorial() {
    int boxWidth = 250;
    int boxHeight = 113;
    int centerX = WINDOW_WIDTH / 2 - boxWidth / 2;
    int centerY = WINDOW_HEIGHT / 2 - boxHeight / 2;
    DrawRectangle(centerX, centerY, boxWidth, boxHeight, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(centerX, centerY, boxWidth, boxHeight, BLUE);
    DrawText("Controls:", centerX + 20, centerY + 20, 14, BLACK);
    DrawText("- A/D to move", centerX + 20, centerY + 40, 10, DARKGRAY);
    DrawText("- SPACE to start ball", centerX + 20, centerY + 60, 10, DARKGRAY);
}

void DrawBlocks(BlockData *blocks, int blockSize, int rows, int cols) {
    int blockAreaWidth = cols * blockSize;
    int xOffset = (WINDOW_WIDTH - blockAreaWidth) / 2;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            BlockData *block = &blocks[i * cols + j];
            if (block->isActive) {
                Color blockColor = (block->type == WHITE_BLOCK) ? WHITE : (block->type == BLACK_BLOCK) ? BLACK : BLUE;
                DrawRectangle(xOffset + j * blockSize, i * TILE_HEIGHT, blockSize, TILE_HEIGHT, blockColor);
                DrawRectangleLines(xOffset + j * blockSize, i * TILE_HEIGHT, blockSize, TILE_HEIGHT, PURPLE);
            }
        }
    }
}

bool HandleCollision(Ball *ball, Player *player, BlockData *blocks, int blockSize, int rows, int cols) {
    if (ball->base.position.x - ball->radius < 0 || ball->base.position.x + ball->radius > WINDOW_WIDTH) {
        ball->base.velocity.x = -ball->base.velocity.x;
    }
    if (ball->base.position.y - ball->radius < 0) {
        ball->base.velocity.y = -ball->base.velocity.y;
    }
    Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
    Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};
    if (CheckCollisionRecs(playerRect, ballRect)) {
        ball->base.velocity.y = -fabsf(ball->base.velocity.y);
        ball->base.velocity.x = ((ball->base.position.x - player->base.position.x) / player->width - 0.5f) * ball->speed;
    }
    int col = (ball->base.position.x - (WINDOW_WIDTH - cols * blockSize) / 2) / blockSize;
    int row = ball->base.position.y / TILE_HEIGHT;
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        BlockData *block = &blocks[row * cols + col];
        if (block->isActive) {
            block->isActive = false;
            ball->base.velocity.y = -ball->base.velocity.y;
        }
    }
    if (ball->base.position.y + ball->radius > WINDOW_HEIGHT) {
        ball->isActive = false;
        player->lives--;
        return true;
    }
    return false;
}

void UpdatePlayer(Player *player, float deltaTime) {
    player->base.velocity.x = (IsKeyDown(KEY_A) ? -player->speed : IsKeyDown(KEY_D) ? player->speed : 0);
    player->base.position = Vector2Add(player->base.position, Vector2Scale(player->base.velocity, deltaTime));
    if (player->base.position.x < 0) player->base.position.x = 0;
    if (player->base.position.x + player->width > WINDOW_WIDTH) player->base.position.x = WINDOW_WIDTH - player->width;
}

void UpdateBall(Ball *ball, Player *player, BlockData *blocks, int blockSize, int rows, int cols, float deltaTime) {
    if (!ball->isActive) {
        ball->base.position.x = player->base.position.x + player->width / 2;
        ball->base.position.y = player->base.position.y - ball->radius - 5;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 direction = Vector2Normalize(Vector2Subtract(GetMousePosition(), ball->base.position));
            ball->base.velocity = Vector2Scale(direction, ball->speed);
            ball->isActive = true;
        }
    } else {
        ball->base.position = Vector2Add(ball->base.position, Vector2Scale(ball->base.velocity, deltaTime));
        HandleCollision(ball, player, blocks, blockSize, rows, cols);
    }
}

bool AreBlocksCleared(BlockData *blocks, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        if (blocks[i].isActive) return false;
    }
    return true;
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Block Kuzuchi");
    int blockSize = TILE_WIDTH * 2;
    int rows = 6, cols = 10;
    BlockData blocks[rows * cols];
    for (int i = 0; i < rows * cols; i++) {
        blocks[i].isActive = (i / cols < 3);
        blocks[i].type = (i / cols == 0) ? WHITE_BLOCK : (i / cols == 1) ? BLACK_BLOCK : BLUE_BLOCK;
        blocks[i].base.position = (Vector2){0, 0};
        blocks[i].base.velocity = (Vector2){0, 0};
    }
    Vector2 playerPos = {100, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitializePlayer(playerPos);
    Ball ball = InitializeBall(player.base.position.x + player.width / 2, player.base.position.y - 16.0f - 5, 16.0f);
    bool showTutorial = true, gameEnded = false, playerWon = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        if (IsKeyPressed(KEY_SPACE) && showTutorial) showTutorial = false;
        if (!gameEnded) {
            UpdatePlayer(&player, deltaTime);
            UpdateBall(&ball, &player, blocks, blockSize, rows, cols, deltaTime);
            if (player.lives <= 0 || AreBlocksCleared(blocks, rows, cols)) {
                gameEnded = true;
                playerWon = AreBlocksCleared(blocks, rows, cols);
            }
        } else if (IsKeyPressed(KEY_R)) {
            player = InitializePlayer(playerPos);
            ball = InitializeBall(player.base.position.x + player.width / 2, player.base.position.y - 16.0f - 5, 16.0f);
            for (int i = 0; i < rows * cols; i++) {
                blocks[i].isActive = (i / cols < 3);
                blocks[i].type = (i / cols == 0) ? WHITE_BLOCK : (i / cols == 1) ? BLACK_BLOCK : BLUE_BLOCK;
            }
            showTutorial = true;
            gameEnded = false;
        }
        BeginDrawing();
        ClearBackground(YELLOW);
        if (showTutorial) {
            DrawTutorial();
        } else {
            DrawBlocks(blocks, blockSize, rows, cols);
            DrawPlayer(&player);
            DrawBall(&ball);
            DrawLifeMeter(&player);
            if (gameEnded) {
                const char *message = playerWon ? "You Win! Press R to Restart" : "Game Over! Press R to Restart";
                DrawText(message, WINDOW_WIDTH / 2 - MeasureText(message, 20) / 2, WINDOW_HEIGHT / 2, 20, RED);
            }
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
