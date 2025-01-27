#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 720
#define TILE_WIDTH 30
#define TILE_HEIGHT 30
#define BLOCK_SIZE (TILE_WIDTH * 2)
#define BALL_SPEED 600.0f
#define PLAYER_SPEED 600.0f
#define MAX_POWERUPS 10

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool isActive;
} Entity;

typedef struct {
    Entity base;
    float speed;
    float radius;
} Ball;

typedef struct {
    Entity base;
    float width;
    float height;
    int lives;
} Player;

typedef struct {
    Entity base;
    int type;
} Block;

typedef struct {
    Vector2 position;
    bool isActive;
    int type;
} PowerUp;
Player InitPlayer(Vector2 position) {
    Player player = {0};
    player.base.position = position;
    player.base.velocity = (Vector2){0, 0};
    player.width = TILE_WIDTH * 5;
    player.height = TILE_HEIGHT;
    player.lives = 3;
    player.base.isActive = true;
    return player;
}


Ball InitBall(Vector2 position) {
    Ball ball = {0};
    ball.base.position = position;
    ball.speed = BALL_SPEED;
    ball.radius = 16.0f;
    ball.base.isActive = false;
    return ball;
}
Block InitBlock(Vector2 position, int type) {
    Block block = {0};
    block.base.position = position;
    block.base.isActive = true;
    block.type = type;
    return block;
}

PowerUp powerUps[MAX_POWERUPS] = {0};

Vector2 ReflectBall(Ball *ball, Player *player) {
    float hitOffset = (ball->base.position.x - player->base.position.x) / player->width - 0.5f;
    Vector2 direction = {hitOffset, -1.0f};
    return Vector2Scale(Vector2Normalize(direction), ball->speed);
}
void DrawPlayer(Player *player) {
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->base.position, ball->radius, BLACK);
    if (!ball->base.isActive) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 direction = Vector2Normalize(Vector2Subtract(mousePosition, ball->base.position));
        Vector2 aimLineEnd = Vector2Add(ball->base.position, Vector2Scale(direction, 40.0f));
        DrawLineV(ball->base.position, aimLineEnd, RED);
    }
}

void DrawBlocks(Block *blocks, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Block *block = &blocks[i * cols + j];
            if (block->base.isActive) {
                Color blockColor = (block->type == 0) ? WHITE : (block->type == 1) ? BLACK : BLUE;
                DrawRectangle(j * BLOCK_SIZE, i * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, blockColor);
                DrawRectangleLines(j * BLOCK_SIZE, i * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, PURPLE);
            }
        }
    }
}
void DropPowerUp(Block *block, PowerUp *powerUps, int rows, int cols) {
    if (!block->base.isActive) {
        int dropChance = GetRandomValue(0, 100);
        if (dropChance < 30) {
            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (!powerUps[i].isActive) {
                    powerUps[i].position = block->base.position;
                    powerUps[i].isActive = true;
                    powerUps[i].type = GetRandomValue(0, 1);
                    break;
                }
            }
        }
    }
}


void HandlePowerUpCollision(PowerUp *powerUp, Player *player) {
    if (powerUp->isActive && CheckCollisionRecs((Rectangle){player->base.position.x, player->base.position.y, player->width, player->height},
                                                (Rectangle){powerUp->position.x, powerUp->position.y, 20, 20})) {
        powerUp->isActive = false;
        if (powerUp->type == 0) {
            player->lives++;
        }
    }
}

bool HandleCollisions(Ball *ball, Player *player, Block *blocks, int rows, int cols) {
    if (ball->base.position.x - ball->radius < 0 || ball->base.position.x + ball->radius > WINDOW_WIDTH) {
        ball->base.velocity.x = -ball->base.velocity.x;
    }
    if (ball->base.position.y - ball->radius < 0) {
        ball->base.velocity.y = -ball->base.velocity.y;
    }

    Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
    Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};
    if (CheckCollisionRecs(playerRect, ballRect)) {
        ball->base.velocity = ReflectBall(ball, player);
    }

    int col = ball->base.position.x / BLOCK_SIZE;
    int row = ball->base.position.y / TILE_HEIGHT;
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        Block *block = &blocks[row * cols + col];
        if (block->base.isActive) {
            block->base.isActive = false;
            ball->base.velocity.y = -ball->base.velocity.y;
            DropPowerUp(block, powerUps, rows, cols);

        }

    }

    if (ball->base.position.y + ball->radius > WINDOW_HEIGHT) {
        ball->base.isActive = false;
        player->lives--;
        return true;
    }
    return false;
}

void UpdatePlayer(Player *player, float dt) {
    if (IsKeyDown(KEY_A)) {
        player->base.velocity.x = -PLAYER_SPEED;
    } else if (IsKeyDown(KEY_D)) {
        player->base.velocity.x = PLAYER_SPEED;
    } else {
        player->base.velocity.x = 0;
    }

    player->base.position = Vector2Add(player->base.position, Vector2Scale(player->base.velocity, dt));

    if (player->base.position.x < 0) {
        player->base.position.x = 0;
    }
    if (player->base.position.x + player->width > WINDOW_WIDTH) {
        player->base.position.x = WINDOW_WIDTH - player->width;
    }
}

void UpdateBall(Ball *ball, Player *player, Block *blocks, int rows, int cols, float dt) {
    if (!ball->base.isActive) {
        ball->base.position.x = player->base.position.x + player->width / 2;
        ball->base.position.y = player->base.position.y - ball->radius - 5;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ball->base.isActive = true;
            Vector2 direction = Vector2Subtract(GetMousePosition(), ball->base.position);
            ball->base.velocity = Vector2Scale(Vector2Normalize(direction), ball->speed);
        }
    } else {
        ball->base.position = Vector2Add(ball->base.position, Vector2Scale(ball->base.velocity, dt));
        HandleCollisions(ball, player, blocks, rows, cols);
    }
}

void DrawPowerUp(PowerUp *powerUp) {
    if (powerUp->isActive) {
        DrawCircleV(powerUp->position, 10, GREEN);
    }
}
void DrawLifebar(Player *player) {
    const float lifebarWidth = 200.0f;
    const float lifebarHeight = 20.0f;
    float lifebarProgress = (float)player->lives / 3.0f;

    float lifebarX = (WINDOW_WIDTH - lifebarWidth) / 2;
    float lifebarY = WINDOW_HEIGHT - lifebarHeight - 5;

    DrawRectangle(lifebarX, lifebarY, lifebarWidth, lifebarHeight, RED);

    DrawRectangle(lifebarX, lifebarY, lifebarWidth * lifebarProgress, lifebarHeight, GREEN);
}

void DrawStartScreen() {
    ClearBackground(YELLOW);

    DrawText("Block Kuzuchi!", (WINDOW_WIDTH - MeasureText("Block Kuzuchi!", 40)) / 2, WINDOW_HEIGHT / 2 - 40, 40, DARKBLUE);
    DrawText("Press ENTER to Start", (WINDOW_WIDTH - MeasureText("Press ENTER to Start", 30)) / 2, WINDOW_HEIGHT / 2 + 10, 30, DARKBLUE);
}

void DrawPauseScreen() {
    ClearBackground(YELLOW);

    DrawText("PAUSED", (WINDOW_WIDTH - MeasureText("PAUSED", 40)) / 2, WINDOW_HEIGHT / 2 - 40, 40, DARKBLUE);
    DrawText("Press ENTER to Start", (WINDOW_WIDTH - MeasureText("Press ENTER to Start", 30)) / 2, WINDOW_HEIGHT / 2 + 10, 30, DARKBLUE);
}
void DrawGameOver() {
    DrawText("GAME OVER", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 50, RED);
    DrawText("ENTER to Restart", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 30, 30, RED);
}
void DrawWinScreen() {
    DrawText("YOU WIN!", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 50, PINK);
    DrawText("ENTER to Restart", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 30, 30, PINK);
}

bool CheckWinCondition(Block *blocks, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        if (blocks[i].base.isActive) {
            return false;
        }
    }
    return true;
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BlockKuzuchi");

    int rows = WINDOW_HEIGHT / TILE_HEIGHT;
    int cols = WINDOW_WIDTH / BLOCK_SIZE;
    Block blocks[rows * cols];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            blocks[i * cols + j].base.isActive = i < 3;
            blocks[i * cols + j].type = i % 3;
        }
    }

    Vector2 playerPosition = {WINDOW_WIDTH / 2 - TILE_WIDTH * 2.5f, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitPlayer(playerPosition);

    Ball ball = InitBall((Vector2){player.base.position.x + player.width / 2, player.base.position.y - 20});

    bool gameStarted = false;
    bool gameOver = false;
    bool gameWon = false;

    while (!WindowShouldClose() && !gameStarted) {
    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    DrawStartScreen();

    if (IsKeyPressed(KEY_ENTER)) {
        gameStarted = true;
    }

    EndDrawing();
}

while (!WindowShouldClose()) {
    if (gameOver) {
        BeginDrawing();
        ClearBackground(YELLOW);
        DrawGameOver();

        if (IsKeyPressed(KEY_ENTER)) {
            player = InitPlayer(playerPosition);
            ball = InitBall((Vector2){player.base.position.x + player.width / 2, player.base.position.y - 20});
            gameOver = false;

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    blocks[i * cols + j].base.isActive = i < 3;
                    blocks[i * cols + j].type = i % 3;
                }
            }

            for (int i = 0; i < MAX_POWERUPS; i++) {
                powerUps[i].isActive = false;
            }
        }

        EndDrawing();
        continue;
    }

    if (gameWon) {
        BeginDrawing();
        ClearBackground(YELLOW);
        DrawWinScreen();

        if (IsKeyPressed(KEY_ENTER)) {
            player = InitPlayer(playerPosition);
            ball = InitBall((Vector2){player.base.position.x + player.width / 2, player.base.position.y - 20});
            gameWon = false;

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    blocks[i * cols + j].base.isActive = i < 3;
                    blocks[i * cols + j].type = i % 3;
                }
            }

            for (int i = 0; i < MAX_POWERUPS; i++) {
                powerUps[i].isActive = false;
            }
        }

        EndDrawing();
        continue;
    }

    float dt = GetFrameTime();
    UpdatePlayer(&player, dt);
    UpdateBall(&ball, &player, blocks, rows, cols, dt);

    for (int i = 0; i < MAX_POWERUPS; i++) {
        HandlePowerUpCollision(&powerUps[i], &player);
    }

    if (CheckWinCondition(blocks, rows, cols)) {
        gameWon = true;
    }

    BeginDrawing();
    ClearBackground(YELLOW);

    DrawBlocks(blocks, rows, cols);
    DrawPlayer(&player);
    DrawBall(&ball);
    DrawLifebar(&player);

    for (int i = 0; i < MAX_POWERUPS; i++) {
        DrawPowerUp(&powerUps[i]);
    }

    EndDrawing();
}


    CloseWindow();
    return 0;
}
