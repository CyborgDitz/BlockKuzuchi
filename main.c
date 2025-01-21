#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 1280
#define TILE_WIDTH 32
#define TILE_HEIGHT 32

typedef enum {
    WHITE_BLOCK,
    BLACK_BLOCK,
    BLUE_BLOCK,
    NUM_BLOCK_TYPES
} BlockType;

typedef struct {
    bool isActive;
    BlockType type;
} BlockData;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float playerSpeed;
    bool isActive;
    int lives;
    float width;
    float height;
} Player;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    bool isActive;
} Ball;



Player InitializePlayer(Vector2 position, float speed) {
    Player player = {0};
    player.position = position;
    player.playerSpeed = speed;
    player.velocity = (Vector2){0, 0};
    player.lives = 3;
    player.position = position;
    player.width = TILE_WIDTH * 5;
    player.height = TILE_HEIGHT;
    return player;
}

Ball InitializeBall(float x, float y, float radius, float speed) {
    Ball ball;
    ball.position.x = x;
    ball.position.y = y;
    ball.radius = radius;
    ball.speed = speed * 6.0f;
    ball.velocity = (Vector2){ball.speed, -ball.speed};
    ball.isActive = false;
    return ball;
}

void DrawPlayer(Player *player) {
    DrawRectangle(player->position.x, player->position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->position.x, player->position.y, player->width, player->height, DARKPURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->position, ball->radius, BLACK);
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
    DrawText("- SPACE to yeet ball", centerX + 20, centerY + 60, 10, DARKGRAY);
}

void DrawWindow(BlockData *blocks, int blockSize) {
    for (int i = 0; i < (WINDOW_HEIGHT / TILE_HEIGHT); i++) {
        for (int j = 0; j < (WINDOW_WIDTH / blockSize); j++) {
            BlockData *block = &blocks[i * (WINDOW_WIDTH / blockSize) + j];
            if (block->isActive) {
                Color blockColor = BLACK;
                switch (block->type) {
                    case WHITE_BLOCK:
                        blockColor = WHITE;
                        break;
                    case BLACK_BLOCK:
                        blockColor = BLACK;
                        break;
                    case BLUE_BLOCK:
                        blockColor = BLUE;
                        break;
                    default:
                        blockColor = BLACK;
                }
                DrawRectangle(j * blockSize, i * TILE_HEIGHT, blockSize, TILE_HEIGHT, blockColor);
                DrawRectangleLines(j * blockSize, i * TILE_HEIGHT, blockSize, TILE_HEIGHT, PURPLE);
            }
        }
    }
}


void HandleCollision(Ball *ball) {
    if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > WINDOW_WIDTH) {
        ball->velocity.x = -ball->velocity.x;
    }
    if (ball->position.y - ball->radius < 0) {
        ball->velocity.y = -ball->velocity.y;
    }
    if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
        ball->isActive = false;
    }
}

void UpdatePlayer(Player *player, float deltaTime) {
    if (IsKeyDown(KEY_A)) {
        player->velocity.x = -player->playerSpeed * 2;
    } else if (IsKeyDown(KEY_D)) {
        player->velocity.x = player->playerSpeed * 2;
    } else {
        player->velocity.x = 0;
    }

    player->position = Vector2Add(player->position, Vector2Scale(player->velocity, deltaTime));

    player->position = player->position;

    if (player->position.x < 0) {
        player->position.x = 0;
    }

    if (player->position.x + TILE_WIDTH * 5 > WINDOW_WIDTH) {
        player->position.x = WINDOW_WIDTH - TILE_WIDTH * 5;
    }
}
void UpdateBall(Ball *ball, Player *player, BlockData *blocks, int blockSize, float deltaTime) {
    if (!ball->isActive) {
        ball->position.x = player->position.x + (TILE_WIDTH * 5) / 2;
        ball->position.y = player->position.y - ball->radius - 5;

        if (IsKeyPressed(KEY_SPACE)) {
            ball->isActive = true;

            float angle = GetRandomValue(-45, 45);
            float radians = DEG2RAD * angle;

            ball->velocity.x = ball->speed * cos(radians);
            ball->velocity.y = -fabs(ball->speed * sin(radians));
        }
    } else {
        ball->position = Vector2Add(ball->position, Vector2Scale(ball->velocity, deltaTime));

        if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
            ball->isActive = false;
            player->lives--;
        }

        int row = ball->position.y / TILE_HEIGHT;
        int col = ball->position.x / blockSize;

        if (row >= 0 && row < WINDOW_HEIGHT / TILE_HEIGHT && col >= 0 && col < WINDOW_WIDTH / blockSize) {
            BlockData *block = &blocks[row * (WINDOW_WIDTH / blockSize) + col];
            if (block->isActive) {
                block->isActive = false;
                ball->velocity.y = -ball->velocity.y;
            }
        }

        if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > WINDOW_WIDTH) {
            ball->velocity.x = -ball->velocity.x;
        }
        if (ball->position.y - ball->radius < 0) {
            ball->velocity.y = -ball->velocity.y;
        }
    }
}


bool AreBlocksCleared(BlockData *blocks, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (blocks[i * cols + j].isActive) {
                return false;
            }
        }
    }
    return true;
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BlockKuzuchi");

    int blockSize = TILE_WIDTH * 2;
    int rows = WINDOW_HEIGHT / TILE_HEIGHT;
    int cols = WINDOW_WIDTH / blockSize;
    BlockData blocks[rows * cols];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            blocks[i * cols + j].isActive = false;
        }
    }

    for (int j = 0; j < cols; j++) {
        blocks[0 * cols + j].isActive = true;
        blocks[0 * cols + j].type = WHITE_BLOCK;

        blocks[1 * cols + j].isActive = true;
        blocks[1 * cols + j].type = BLACK_BLOCK;

        blocks[2 * cols + j].isActive = true;
        blocks[2 * cols + j].type = BLUE_BLOCK;
    }

    Vector2 playerPosition = {100, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitializePlayer(playerPosition, 300.0f);
    Ball ball = InitializeBall(player.position.x + (TILE_WIDTH * 5) / 2, player.position.y - 16.0f - 5, 16.0f, 200.0f);

    bool showTutorial = true;
    bool gameEnded = false;
    bool playerWon = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE) && showTutorial) {
            showTutorial = false;
        }

        if (!gameEnded) {
            UpdatePlayer(&player, deltaTime);
            UpdateBall(&ball, &player, blocks, blockSize, deltaTime);

            if (player.lives <= 0) {
                gameEnded = true;
                playerWon = false;
            }

            if (AreBlocksCleared(blocks, rows, cols)) {
                gameEnded = true;
                playerWon = true;
            }
        } else {
            if (IsKeyPressed(KEY_R)) {
                player = InitializePlayer(playerPosition, 300.0f);
                ball = InitializeBall(player.position.x + (TILE_WIDTH * 5) / 2, player.position.y - 16.0f - 5, 16.0f, 200.0f);
                showTutorial = true;
                gameEnded = false;

                for (int j = 0; j < cols; j++) {
                    blocks[0 * cols + j].isActive = true;
                    blocks[0 * cols + j].type = WHITE_BLOCK;

                    blocks[1 * cols + j].isActive = true;
                    blocks[1 * cols + j].type = BLACK_BLOCK;

                    blocks[2 * cols + j].isActive = true;
                    blocks[2 * cols + j].type = BLUE_BLOCK;
                }
            }
        }

        BeginDrawing();
        ClearBackground(YELLOW);

        if (showTutorial) {
            DrawTutorial();
        } else {
            DrawWindow(blocks, blockSize);
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
