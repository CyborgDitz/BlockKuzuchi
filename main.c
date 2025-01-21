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

typedef enum {
    BLOCK_COLLISION,
    PLAYER_COLLISION,
    SCREEN_COLLISION,
    LIFE_METER_COLLISION,
    NUM_COLLISION_TYPES
} CollisionType;

typedef struct {
    Vector2 position;
    float width;
    float height;
    CollisionType type;
} Collider;

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
    ball.speed = speed * 2.0f;
    ball.velocity = (Vector2){ball.speed, -ball.speed};
    ball.isActive = false;
    return ball;
}

void DrawPlayer(Player *player) {
    DrawRectangle(player->position.x, player->position.y, TILE_WIDTH * 5, TILE_HEIGHT * 1, PURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->position, ball->radius, BLACK);
}

void DrawLifeMeter(Player *player) {
    int maxLives = 3;
    int meterWidth = TILE_WIDTH * 10;
    int meterHeight = TILE_HEIGHT / 2;
    int remainingLives = player->lives;
    int currentMeterWidth = (meterWidth * remainingLives) / maxLives;

    DrawRectangle((WINDOW_WIDTH - meterWidth) / 2, WINDOW_HEIGHT - meterHeight - 10, meterWidth, meterHeight, GRAY);
    DrawRectangle((WINDOW_WIDTH - meterWidth) / 2, WINDOW_HEIGHT - meterHeight - 10, currentMeterWidth, meterHeight, GREEN);
    DrawRectangleLines((WINDOW_WIDTH - meterWidth) / 2, WINDOW_HEIGHT - meterHeight - 10, meterWidth, meterHeight, BLACK);
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

bool CheckCollisionBallWithCollider(Ball *ball, Collider *collider) {
    if (ball->position.x + ball->radius > collider->position.x &&
        ball->position.x - ball->radius < collider->position.x + collider->width &&
        ball->position.y + ball->radius > collider->position.y &&
        ball->position.y - ball->radius < collider->position.y + collider->height) {
        return true;
    }
    return false;
}

void HandleCollision(Ball *ball, Collider *collider, CollisionType type) {
    switch (type) {
        case BLOCK_COLLISION:
            ball->velocity.y = -ball->velocity.y;
            break;

        case PLAYER_COLLISION:
            ball->velocity.y = -ball->velocity.y;
            break;

        case SCREEN_COLLISION:
            if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > WINDOW_WIDTH) {
                ball->velocity.x = -ball->velocity.x;
            }
            if (ball->position.y - ball->radius < 0) {
                ball->velocity.y = -ball->velocity.y;
            }
            if (ball->position.y + ball->radius > WINDOW_HEIGHT) {
                ball->velocity.y = -ball->velocity.y;
                ball->position.y = WINDOW_HEIGHT - ball->radius;
            }
            break;

        case LIFE_METER_COLLISION:
            ball->velocity.y = -ball->velocity.y;
            break;

        default:
            break;
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

    if (player->position.x < 0) {
        player->position.x = 0;
    }

    if (player->position.x + TILE_WIDTH * 5 > WINDOW_WIDTH) {
        player->position.x = WINDOW_WIDTH - TILE_WIDTH * 5;
    }
}

void UpdateBall(Ball *ball, Player *player, BlockData *blocks, float deltaTime, int blockSize) {
    if (!ball->isActive && IsKeyPressed(KEY_SPACE)) {
        ball->isActive = true;
        ball->velocity.x = 400.0f;
        ball->velocity.y = -400.0f;
    }

    if (ball->isActive) {
        ball->position = Vector2Add(ball->position, Vector2Scale(ball->velocity, deltaTime));

        int row = (int)(ball->position.y / TILE_HEIGHT);
        int col = (int)(ball->position.x / blockSize);

        if (row >= 0 && row < (WINDOW_HEIGHT / TILE_HEIGHT) && col >= 0 && col < (WINDOW_WIDTH / blockSize)) {
            BlockData *block = &blocks[row * (WINDOW_WIDTH / blockSize) + col];
            if (block->isActive) {
                Collider blockCollider = { (Vector2){col * blockSize, row * TILE_HEIGHT}, blockSize, TILE_HEIGHT, BLOCK_COLLISION };
                if (CheckCollisionBallWithCollider(ball, &blockCollider)) {
                    block->isActive = false;
                    HandleCollision(ball, &blockCollider, BLOCK_COLLISION);
                }
            }
        }

        Collider playerCollider = { player->position, TILE_WIDTH * 5, TILE_HEIGHT, PLAYER_COLLISION };
        if (CheckCollisionBallWithCollider(ball, &playerCollider)) {
            HandleCollision(ball, &playerCollider, PLAYER_COLLISION);
        }

        Collider lifeMeterCollider = { (Vector2){(WINDOW_WIDTH - TILE_WIDTH * 10) / 2, WINDOW_HEIGHT - TILE_HEIGHT / 2 - 10}, TILE_WIDTH * 10, TILE_HEIGHT / 2, LIFE_METER_COLLISION };
        if (CheckCollisionBallWithCollider(ball, &lifeMeterCollider)) {
            HandleCollision(ball, &lifeMeterCollider, LIFE_METER_COLLISION);
        }

        Collider screenCollider = { (Vector2){0, 0}, WINDOW_WIDTH, WINDOW_HEIGHT, SCREEN_COLLISION };
        HandleCollision(ball, &screenCollider, SCREEN_COLLISION);

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

    bool showTutorial = true;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            showTutorial = false;
        }

        UpdatePlayer(&player, deltaTime);
        UpdateBall(&ball, &player, (BlockData *)blocks, deltaTime, blockSize);

        BeginDrawing();
        ClearBackground(YELLOW);

        DrawWindow((BlockData *)blocks, blockSize);
        if (showTutorial) {
            DrawTutorial();
        }
        DrawPlayer(&player);
        DrawBall(&ball);
        DrawLifeMeter(&player);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
