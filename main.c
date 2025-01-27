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
#define DROP_CHANCE 0.3f
bool gameOver = false;
bool gameWon = false;

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
    Entity base;
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
    float paddleVelocity = player->base.velocity.x;
    float hitOffset = (ball->base.position.x - player->base.position.x) / player->width - 0.5f;
    Vector2 direction = {hitOffset + paddleVelocity / PLAYER_SPEED, -1.0f};
    Vector2 reflectedVelocity = Vector2Scale(Vector2Normalize(direction), ball->speed);
    ball->speed *= 1.05f;

    return reflectedVelocity;
}



void DrawPlayer(Player *player) {
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->base.position, ball->radius, PINK);
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
void DropPowerUp(Block *block, PowerUp *powerUps) {
    float dropChance = DROP_CHANCE;

    if (GetRandomValue(0, 100) < dropChance * 100) {

        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (!powerUps[i].base.isActive) {
                powerUps[i].base.position = (Vector2){
                    block->base.position.x + BLOCK_SIZE / 2,
                    block->base.position.y + TILE_HEIGHT / 2
                };

                powerUps[i].base.velocity = (Vector2){0, 100.0f};
                powerUps[i].type = GetRandomValue(0, 1);
                powerUps[i].base.isActive = true;
                break;
            }
        }
    }
}
void UpdatePowerUps(PowerUp *powerUps, int maxPowerUps, float deltaTime) {
    for (int i = 0; i < maxPowerUps; i++) {
        if (powerUps[i].base.isActive) {
            powerUps[i].base.position.y += powerUps[i].base.velocity.y * deltaTime;

            if (powerUps[i].base.position.y > WINDOW_HEIGHT) {
                powerUps[i].base.isActive = false;
            }
        }
    }
}

typedef void (*PowerUpEffect)(Player *player);

void PowerUpExtraLife(Player *player) {
    player->lives++;
}

void PowerUpIncreasePaddleWidth(Player *player) {
    player->width += TILE_WIDTH/2;
}

PowerUpEffect powerUpEffects[] = {
    PowerUpExtraLife,
    PowerUpIncreasePaddleWidth
};

void HandlePowerUpCollision(PowerUp *powerUp, Player *player) {
    if (powerUp->base.isActive && CheckCollisionRecs((Rectangle){player->base.position.x, player->base.position.y, player->width, player->height},
                                                (Rectangle){powerUp->base.position.x, powerUp->base.position.y, 20, 20})) {
        powerUp->base.isActive = false;
        if (powerUp->type == 0) {
            powerUpEffects[0](player);
        } else if (powerUp->type == 1) {
            powerUpEffects[1](player);
        }
    }
}

void DestroyBlock(Block *block, Ball *ball, PowerUp *powerUps, int rows, int cols) {
    block->base.isActive = false;
    ball->base.velocity.y = -ball->base.velocity.y;
    DropPowerUp(block, powerUps);
}

bool HandleBallCollisions(Ball *ball, Player *player, Block *blocks, int rows, int cols, PowerUp *powerUps) {
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
            DestroyBlock(block, ball, powerUps, rows, cols);
        }
    }
    if (ball->base.position.y + ball->radius > WINDOW_HEIGHT) {
        ball->base.isActive = false;
        player->lives--;
        if (player->lives <= 0) {
            gameOver = true;
        }
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

void UpdateBall(Ball *ball, Player *player, Block *blocks, int rows, int cols, PowerUp *powerUps, float dt) {
    if (!ball->base.isActive) {
        ball->speed = BALL_SPEED;

        ball->base.position.x = player->base.position.x + player->width / 2;
        ball->base.position.y = player->base.position.y - ball->radius - 5;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ball->base.isActive = true;
            Vector2 direction = Vector2Subtract(GetMousePosition(), ball->base.position);
            ball->base.velocity = Vector2Scale(Vector2Normalize(direction), ball->speed);
        }
    } else {
        ball->base.position = Vector2Add(ball->base.position, Vector2Scale(ball->base.velocity, dt));
        HandleBallCollisions(ball, player, blocks, rows, cols, powerUps);
    }
}



void DrawPowerUp(PowerUp *powerUp) {
    if (powerUp->base.isActive) {
        DrawCircleV(powerUp->base.position, 10, GREEN);
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

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Block Kuzuchi");

    int rows = WINDOW_HEIGHT / TILE_HEIGHT;
    int cols = WINDOW_WIDTH / BLOCK_SIZE;
    Block blocks[rows * cols];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            blocks[i * cols + j] = InitBlock((Vector2){j * BLOCK_SIZE, i * TILE_HEIGHT}, i % 3);
            blocks[i * cols + j].base.isActive = (i < 3);
        }
    }

    Vector2 playerPosition = {WINDOW_WIDTH / 2 - TILE_WIDTH * 2.5f, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitPlayer(playerPosition);

    Ball ball = InitBall((Vector2){player.base.position.x + player.width / 2, player.base.position.y - 20});

    bool gameStarted = false;

    while (!WindowShouldClose() && !gameStarted) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Press SPACE to start", 250, 300, 20, PINK);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {
            gameStarted = true;
        }
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (gameOver) {
            break;
        }

        UpdatePlayer(&player, deltaTime);
        UpdateBall(&ball, &player, blocks, rows, cols, powerUps, deltaTime);
        UpdatePowerUps(powerUps, MAX_POWERUPS, deltaTime);

        for (int i = 0; i < MAX_POWERUPS; i++) {
            HandlePowerUpCollision(&powerUps[i], &player);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawBlocks(blocks, rows, cols);
        DrawPlayer(&player);
        DrawBall(&ball);

        for (int i = 0; i < MAX_POWERUPS; i++) {
            DrawPowerUp(&powerUps[i]);
        }

        DrawLifebar(&player);

        if (player.lives <= 0) {
            DrawText("Game Over!", 320, 300, 40, RED);
        }

        if (gameWon) {
            DrawText("You Win!", 320, 300, 40, GREEN);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
