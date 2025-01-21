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
} Player;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float speed;
    float radius;
    bool isActive;
} Ball;

typedef struct {
    bool collisionOccurred;
} CollisionData;

void DrawTutorial(void);
void DrawWindow(BlockData *blocks, int blockSize);
void DrawPlayer(Player *player);
void DrawBall(Ball *ball);
void DrawBlocks(BlockData *blocks, int blockSize);

typedef struct {
    void (*DrawTutorial)(void);
    void (*DrawWindow)(BlockData *blocks, int blockSize);
    void (*DrawPlayer)(Player *player);
    void (*DrawBall)(Ball *ball);
    void (*DrawBlocks)(BlockData *blocks, int blockSize);
} Drawings;

Player InitializePlayer(Vector2 position, float speed) {
    Player player = {0};
    player.position = position;
    player.playerSpeed = speed;
    player.velocity = (Vector2){0, 0};
    player.lives = 3;
    return player;
}

Ball InitializeBall(float x, float y, float radius, float speed) {
    Ball ball;
    ball.position.x = x;
    ball.position.y = y;
    ball.radius = radius;
    ball.speed = speed;
    ball.velocity = (Vector2){speed, -speed};
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
    DrawText("- SPACE to yeet ball", 40, 60, 10, DARKGRAY);
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
            }
        }
    }
}

void DrawBlocks(BlockData *blocks, int blockSize) {
    for (int i = 0; i < 3; i++) {
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
            }
        }
    }
}

void UpdatePlayer(Player *player, float deltaTime, CollisionData *collisionData) {
    if (IsKeyDown(KEY_A)) {
        player->velocity.x = -player->playerSpeed;
    } else if (IsKeyDown(KEY_D)) {
        player->velocity.x = player->playerSpeed;
    } else {
        player->velocity.x = 0;
    }

    player->position = Vector2Add(player->position, Vector2Scale(player->velocity, deltaTime));

    if (player->position.x < 0) {
        player->position.x = 0;
        collisionData->collisionOccurred = true;
    }

    if (player->position.x + TILE_WIDTH * 5 > WINDOW_WIDTH) {
        player->position.x = WINDOW_WIDTH - TILE_WIDTH * 5;
        collisionData->collisionOccurred = true;
    }
}

void UpdateBall(Ball *ball, Player *player, BlockData *blocks, float deltaTime, CollisionData *collisionData, int blockSize) {
    if (!ball->isActive && IsKeyPressed(KEY_SPACE)) {
        ball->isActive = true;
        ball->velocity.x = 200.0f;
        ball->velocity.y = -200.0f;
        collisionData->collisionOccurred = false;
    }

    if (ball->isActive) {
        ball->position = Vector2Add(ball->position, Vector2Scale(ball->velocity, deltaTime));

        int row = (int)(ball->position.y / TILE_HEIGHT);
        int col = (int)(ball->position.x / blockSize);

        if (row >= 0 && row < (WINDOW_HEIGHT / TILE_HEIGHT) && col >= 0 && col < (WINDOW_WIDTH / blockSize)) {
            BlockData *block = &blocks[row * (WINDOW_WIDTH / blockSize) + col];
            if (block->isActive) {
                block->isActive = false;
                ball->velocity.y = -ball->velocity.y;
                collisionData->collisionOccurred = true;
            }
        }

        if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > WINDOW_WIDTH) {
            ball->velocity.x = -ball->velocity.x;
            collisionData->collisionOccurred = true;
        }

        if (ball->position.y - ball->radius < 0) {
            ball->velocity.y = -ball->velocity.y;
            collisionData->collisionOccurred = true;
        }

        if (ball->position.y + ball->radius >= player->position.y &&
            ball->position.y - ball->radius <= player->position.y + TILE_HEIGHT &&
            ball->position.x >= player->position.x && ball->position.x <= player->position.x + TILE_WIDTH * 5) {
            ball->velocity.y = -ball->velocity.y;
            collisionData->collisionOccurred = true;
        }

        if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
            ball->velocity.y = -ball->velocity.y;
            ball->position.y = WINDOW_HEIGHT - ball->radius;
            collisionData->collisionOccurred = true;
        }
    } else {
        ball->position.x = player->position.x + (TILE_WIDTH * 5) / 2;
        ball->position.y = player->position.y - ball->radius - 5;
    }
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BlockKuzuchi");

    int blockSize = TILE_WIDTH * 2;
    BlockData blocks[WINDOW_HEIGHT / TILE_HEIGHT][WINDOW_WIDTH / blockSize];

    for (int i = 0; i < (WINDOW_HEIGHT / TILE_HEIGHT); i++) {
        for (int j = 0; j < (WINDOW_WIDTH / blockSize); j++) {
            blocks[i][j].isActive = false;
        }
    }

    for (int j = 0; j < (WINDOW_WIDTH / blockSize); j++) {
        blocks[0][j].isActive = true;
        blocks[0][j].type = WHITE_BLOCK;

        blocks[1][j].isActive = true;
        blocks[1][j].type = BLACK_BLOCK;

        blocks[2][j].isActive = true;
        blocks[2][j].type = BLUE_BLOCK;
    }

    Vector2 playerPosition = {100, WINDOW_HEIGHT - TILE_HEIGHT * 2};
    Player player = InitializePlayer(playerPosition, 300.0f);
    Ball ball = InitializeBall(player.position.x + (TILE_WIDTH * 5) / 2, player.position.y - 16.0f - 5, 16.0f, 200.0f);

    Drawings myDrawings;
    myDrawings.DrawTutorial = DrawTutorial;
    myDrawings.DrawWindow = DrawWindow;
    myDrawings.DrawPlayer = DrawPlayer;
    myDrawings.DrawBall = DrawBall;
    myDrawings.DrawBlocks = DrawBlocks;

    CollisionData collisionData = {false};

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        UpdatePlayer(&player, deltaTime, &collisionData);
        UpdateBall(&ball, &player, (BlockData *)blocks, deltaTime, &collisionData, blockSize);

        BeginDrawing();
        ClearBackground(YELLOW);

        myDrawings.DrawWindow((BlockData *)blocks, blockSize);
        myDrawings.DrawBlocks((BlockData *)blocks, blockSize);
        myDrawings.DrawTutorial();
        myDrawings.DrawPlayer(&player);
        myDrawings.DrawBall(&ball);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}