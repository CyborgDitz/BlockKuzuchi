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

typedef enum {
    GAME_START,
    GAME_PLAYING,
    GAME_OVER,
    GAME_WON
  } GameState;


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
typedef struct {
    float width;
    float height;
    float offsetY;
    Color backColor;
    Color frontColor;
}  LifeBar;
typedef struct {
    Player player;
    Ball ball;
    Block *blocks;
    PowerUp powerUps[MAX_POWERUPS];
    int rowCount;
    int columnCount;
} GameStateData;
LifeBar getLifeBar = {
    .width     = 200.0f,
    .height    = 20.0f,
    .offsetY   = 5.0f,
    .backColor = RED,
    .frontColor= GREEN
  };

GameState game_state = GAME_START;
PowerUp powerUps[MAX_POWERUPS] = {0};
typedef void (*PowerUpEffect)(Player *player);

void PowerUpExtraLife(Player *player) {
    player->lives++;
}

void PowerUpIncreasePaddleWidth(Player *player) {
    player->width += TILE_WIDTH / 2;
}

PowerUpEffect powerUpEffects[] = {
    PowerUpExtraLife,
    PowerUpIncreasePaddleWidth
};

Vector2 ReflectBall(Ball *ball, Player *player) {
    float playerVelocityX = player->base.velocity.x;
    float offset = (ball->base.position.x - player->base.position.x) / player->width - 0.5f;
    Vector2 direction = {offset + playerVelocityX / PLAYER_SPEED, -1.0f};
    Vector2 reflectedVelocity = Vector2Scale(Vector2Normalize(direction), ball->speed);
    ball->speed *= 1.05f;
    return reflectedVelocity;
}



void DropPowerUp(Block *block, PowerUp *powerUps) {
    if (GetRandomValue(0, 100) < DROP_CHANCE * 100) {
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

void DrawPlayer(Player *player) {
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
}

void DrawBall(Ball *ball) {
    DrawCircleV(ball->base.position, ball->radius, PINK);
    if (!ball->base.isActive) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 direction = Vector2Normalize(Vector2Subtract(mousePosition, ball->base.position));
        Vector2 endPosition = Vector2Add(ball->base.position, Vector2Scale(direction, 40.0f));
        DrawLineV(ball->base.position, endPosition, RED);
    }
}

void DrawBlocks(Block *blocks, int rowCount, int columnCount) {
    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
        for (int columnIndex = 0; columnIndex < columnCount; columnIndex++) {
            Block *block = &blocks[rowIndex * columnCount + columnIndex];
            if (block->base.isActive) {
                Color blockColor = (block->type == 0) ? WHITE : (block->type == 1) ? BLACK : BLUE;
                DrawRectangle(columnIndex * BLOCK_SIZE, rowIndex * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, blockColor);
                DrawRectangleLines(columnIndex * BLOCK_SIZE, rowIndex * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, PURPLE);
            }
        }
    }
}
void DrawPowerUp(PowerUp *powerUp) {
    if (powerUp->base.isActive) DrawCircleV(powerUp->base.position, 10, GREEN);
}

void DrawLifebar(Player *player) {
    float barWidth = 200.0f;
    float barHeight = 20.0f;
    float healthPercentage = (float)player->lives / 3.0f;
    float barX = (WINDOW_WIDTH - barWidth) / 2;
    float barY = WINDOW_HEIGHT - barHeight - 5;
    DrawRectangle(barX, barY, barWidth, barHeight, RED);
    DrawRectangle(barX, barY, barWidth * healthPercentage, barHeight, GREEN);
}

void DrawStartScreen() {
    DrawText("Press SPACE to start", 250, 300, 20, PINK);
}

void DrawGameOverScreen() {
    DrawText("GAME OVER", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 50, RED);
    DrawText("ENTER to Restart", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 30, 30, RED);
}

void DrawWinScreen() {
    DrawText("YOU WIN!", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 50, GREEN);
    DrawText("ENTER to Restart", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 30, 30, GREEN);
}


Player InitPlayer(Vector2 position) {
    Player player = {0};
    player.base.position = position;
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
    return ball;
}

Block InitBlock(Vector2 position, int type) {
    Block block = {0};
    block.base.position = position;
    block.base.isActive = true;
    block.type = type;
    return block;
}
void DrawGame(Player *player, Ball *ball, Block *blocks, int rowCount, int columnCount, PowerUp *powerUps) {
    ClearBackground(BLACK);
    DrawBlocks(blocks, rowCount, columnCount);
    DrawPlayer(player);
    DrawBall(ball);
    for (int i = 0; i < MAX_POWERUPS; i++) DrawPowerUp(&powerUps[i]);
    DrawLifebar(player);
}
void DestroyBlock(Block *block, Ball *ball, PowerUp *powerUps, int rowCount, int columnCount) {
    block->base.isActive = false;
    ball->base.velocity.y = -ball->base.velocity.y;
    DropPowerUp(block, powerUps);
    bool allBlocksGone = true;
    for (int i = 0; i < rowCount * columnCount; i++) {
        if (block[i].base.isActive) {
            allBlocksGone = false;
            break;
        }
    }
    if (allBlocksGone) game_state = GAME_WON;
}
void HandleBallWallCollision(Ball *ball) {
    if (ball->base.position.x - ball->radius < 0 || ball->base.position.x + ball->radius > WINDOW_WIDTH) {
        ball->base.velocity.x = -ball->base.velocity.x;
    }
    if (ball->base.position.y - ball->radius < 0) {
        ball->base.velocity.y = -ball->base.velocity.y;
    }
}

void HandleBallPlayerCollision(Ball *ball, Player *player) {
    Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
    Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};

    if (CheckCollisionRecs(playerRect, ballRect)) {
        Vector2 collisionPoint = Vector2Subtract(ball->base.position, player->base.position);
        collisionPoint = Vector2Normalize(collisionPoint);

        if (ball->base.position.x < player->base.position.x) {
            ball->base.velocity.x = -ball->base.velocity.x;
        }

        if (ball->base.position.y < player->base.position.y) {
            ball->base.velocity.y = -ball->base.velocity.y;
        } else if (ball->base.position.y > player->base.position.y + player->height) {
            ball->base.velocity.y = -ball->base.velocity.y;
        }

        ball->base.velocity = Vector2Scale(Vector2Normalize(ball->base.velocity), ball->speed);
    }
}

void HandleBallBlockCollision(Ball *ball, Block *blocks, int rowCount, int columnCount, PowerUp *powerUps) {
    int columnIndex = (ball->base.position.x) / BLOCK_SIZE;
    int rowIndex = (ball->base.position.y) / TILE_HEIGHT;
    if (rowIndex >= 0 && rowIndex < rowCount && columnIndex >= 0 && columnIndex < columnCount) {
        Block *block = &blocks[rowIndex * columnCount + columnIndex];
        if (block->base.isActive) {
            Rectangle blockRect = {block->base.position.x, block->base.position.y, BLOCK_SIZE, TILE_HEIGHT};
            Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};

            if (CheckCollisionRecs(ballRect, blockRect)) {
                DestroyBlock(block, ball, powerUps, rowCount, columnCount);

                Vector2 normal = {0, 0};

                if (ball->base.position.x < block->base.position.x) {
                    normal = (Vector2){1, 0};
                } else if (ball->base.position.x > block->base.position.x + BLOCK_SIZE) {
                    normal = (Vector2){-1, 0};
                }

                if (ball->base.position.y < block->base.position.y) {
                    normal = (Vector2){0, 1};
                } else if (ball->base.position.y > block->base.position.y + TILE_HEIGHT) {
                    normal = (Vector2){0, -1};
                }

                ball->base.velocity = Vector2Reflect(ball->base.velocity, normal);
                ball->base.velocity = Vector2Scale(Vector2Normalize(ball->base.velocity), ball->speed);
            }
        }
    }
}
void HandlePowerUpCollision(PowerUp *powerUp, Player *player) {
    Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
    Rectangle powerUpRect = {powerUp->base.position.x, powerUp->base.position.y, 20, 20};
    if (powerUp->base.isActive && CheckCollisionRecs(playerRect, powerUpRect)) {
        powerUp->base.isActive = false;
        if (powerUp->type == 0) powerUpEffects[0](player);
        else if (powerUp->type == 1) powerUpEffects[1](player);
    }
}
bool HandleBallLossCondition(Ball *ball, Player *player) {
    if (ball->base.position.y + ball->radius > WINDOW_HEIGHT) {
        ball->base.isActive = false;
        player->lives--;
        if (player->lives <= 0) game_state = GAME_OVER;
        return true;
    }
    return false;
}

bool HandleBallCollisions(Ball *ball, Player *player, Block *blocks, int rowCount, int columnCount, PowerUp *powerUps) {
    HandleBallWallCollision(ball);
    HandleBallPlayerCollision(ball, player);
    HandleBallBlockCollision(ball, blocks, rowCount, columnCount, powerUps);
    return HandleBallLossCondition(ball, player);
}

void UpdatePlayer(Player *player, float deltaTime) {
    if (IsKeyDown(KEY_A)) player->base.velocity.x = -PLAYER_SPEED;
    else if (IsKeyDown(KEY_D)) player->base.velocity.x = PLAYER_SPEED;
    else player->base.velocity.x = 0;
    player->base.position = Vector2Add(player->base.position, Vector2Scale(player->base.velocity, deltaTime));
    if (player->base.position.x < 0) player->base.position.x = 0;
    if (player->base.position.x + player->width > WINDOW_WIDTH) player->base.position.x = WINDOW_WIDTH - player->width;
}

void UpdateBall(Ball *ball, Player *player, Block *blocks, int rowCount, int columnCount, PowerUp *powerUps, float deltaTime) {
    if (!ball->base.isActive) {
        ball->speed = BALL_SPEED;
        ball->base.position.x = player->base.position.x + player->width / 2;
        ball->base.position.y = player->base.position.y - ball->radius - 5;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 direction = Vector2Subtract(GetMousePosition(), ball->base.position);
            ball->base.velocity = Vector2Scale(Vector2Normalize(direction), ball->speed);
            ball->base.isActive = true;
        }
    } else {
        ball->base.position = Vector2Add(ball->base.position, Vector2Scale(ball->base.velocity, deltaTime));
        HandleBallCollisions(ball, player, blocks, rowCount, columnCount, powerUps);
    }
}

void RestartGame(GameStateData *gameData) {
    gameData->rowCount = 3;
    gameData->columnCount = WINDOW_WIDTH / BLOCK_SIZE;

    gameData->player = InitPlayer((Vector2){WINDOW_WIDTH / 2 - TILE_WIDTH * 2.5f, WINDOW_HEIGHT - TILE_HEIGHT * 2});
    gameData->ball = InitBall((Vector2){gameData->player.base.position.x + gameData->player.width / 2, gameData->player.base.position.y - 20});

    for (int rowIndex = 0; rowIndex < gameData->rowCount; rowIndex++) {
        for (int columnIndex = 0; columnIndex < gameData->columnCount; columnIndex++) {
            gameData->blocks[rowIndex * gameData->columnCount + columnIndex] = InitBlock((Vector2){columnIndex * BLOCK_SIZE, rowIndex * TILE_HEIGHT}, rowIndex % 3);
            gameData->blocks[rowIndex * gameData->columnCount + columnIndex].base.isActive = true;
        }
    }

    gameData->player.lives = 3;
    for (int i = 0; i < MAX_POWERUPS; i++) {
        gameData->powerUps[i].base.isActive = false;
    }
}

void UpdateGameState(GameStateData *gameData, float deltaTime) {
    switch (game_state) {
        case GAME_START:
            ClearBackground(BLACK);
            DrawStartScreen();
            if (IsKeyPressed(KEY_SPACE)) {
                game_state = GAME_PLAYING;
            }
            break;

        case GAME_PLAYING:
            UpdatePlayer(&gameData->player, deltaTime);
            UpdateBall(&gameData->ball, &gameData->player, gameData->blocks, gameData->rowCount, gameData->columnCount, gameData->powerUps, deltaTime);
            UpdatePowerUps(gameData->powerUps, MAX_POWERUPS, deltaTime);
            for (int i = 0; i < MAX_POWERUPS; i++) {
                HandlePowerUpCollision(&gameData->powerUps[i], &gameData->player);
            }

            if (gameData->player.lives <= 0) {
                game_state = GAME_OVER;
            } else {
                bool allBlocksDestroyed = true;
                for (int i = 0; i < gameData->rowCount * gameData->columnCount; i++) {
                    if (gameData->blocks[i].base.isActive) {
                        allBlocksDestroyed = false;
                        break;
                    }
                }
                if (allBlocksDestroyed) {
                    game_state = GAME_WON;
                }
            }
            break;

        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER)) {
                game_state = GAME_START;
                RestartGame(gameData);
            }
            break;

        case GAME_WON:
            if (IsKeyPressed(KEY_ENTER)) {
                game_state = GAME_START;
                RestartGame(gameData);
            }
            break;

        default:
            break;
    }
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Block Kuzuchi");

    GameStateData gameData;
    RestartGame(&gameData);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        UpdateGameState(&gameData, deltaTime);

        BeginDrawing();
        switch (game_state) {
            case GAME_START:
                ClearBackground(BLACK);
                DrawStartScreen();
                break;

            case GAME_PLAYING:
                DrawGame(&gameData.player, &gameData.ball, gameData.blocks, gameData.rowCount, gameData.columnCount, gameData.powerUps);
                break;

            case GAME_OVER:
                ClearBackground(BLACK);
                DrawGameOverScreen();
                break;

            case GAME_WON:
                ClearBackground(BLACK);
                DrawWinScreen();
                break;

            default:
                break;
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}


