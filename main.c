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
    STATE_START,
    STATE_PLAYING,
    STATE_OVER,
    STATE_WIN
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

GameState currentGameState = STATE_START;
PowerUp powerUps[MAX_POWERUPS] = {0};

Player InitPlayer(Vector2 pos) {
    Player pl = {0};
    pl.base.position = pos;
    pl.width = TILE_WIDTH * 5;
    pl.height = TILE_HEIGHT;
    pl.lives = 3;
    pl.base.isActive = true;
    return pl;
}

Ball InitBall(Vector2 pos) {
    Ball b = {0};
    b.base.position = pos;
    b.speed = BALL_SPEED;
    b.radius = 16.0f;
    return b;
}

Block InitBlock(Vector2 pos, int t) {
    Block blk = {0};
    blk.base.position = pos;
    blk.base.isActive = true;
    blk.type = t;
    return blk;
}

Vector2 ReflectBall(Ball *ball, Player *pl) {
    float vx = pl->base.velocity.x;
    float offset = (ball->base.position.x - pl->base.position.x) / pl->width - 0.5f;
    Vector2 dir = {offset + vx / PLAYER_SPEED, -1.0f};
    Vector2 rv = Vector2Scale(Vector2Normalize(dir), ball->speed);
    ball->speed *= 1.05f;
    return rv;
}

void DrawPlayer(Player *pl) {
    DrawRectangle(pl->base.position.x, pl->base.position.y, pl->width, pl->height, PURPLE);
    DrawRectangleLines(pl->base.position.x, pl->base.position.y, pl->width, pl->height, DARKPURPLE);
}

void DrawBall(Ball *b) {
    DrawCircleV(b->base.position, b->radius, PINK);
    if (!b->base.isActive) {
        Vector2 mousePos = GetMousePosition();
        Vector2 dir = Vector2Normalize(Vector2Subtract(mousePos, b->base.position));
        Vector2 endPos = Vector2Add(b->base.position, Vector2Scale(dir, 40.0f));
        DrawLineV(b->base.position, endPos, RED);
    }
}

void DrawBlocks(Block *blocks, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Block *blk = &blocks[i * cols + j];
            if (blk->base.isActive) {
                Color col = (blk->type == 0) ? WHITE : (blk->type == 1) ? BLACK : BLUE;
                DrawRectangle(j * BLOCK_SIZE, i * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, col);
                DrawRectangleLines(j * BLOCK_SIZE, i * TILE_HEIGHT, BLOCK_SIZE, TILE_HEIGHT, PURPLE);
            }
        }
    }
}

void DropPowerUp(Block *blk, PowerUp *pups) {
    if (GetRandomValue(0, 100) < DROP_CHANCE * 100) {
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (!pups[i].base.isActive) {
                pups[i].base.position = (Vector2){
                    blk->base.position.x + BLOCK_SIZE / 2,
                    blk->base.position.y + TILE_HEIGHT / 2
                };
                pups[i].base.velocity = (Vector2){0, 100.0f};
                pups[i].type = GetRandomValue(0, 1);
                pups[i].base.isActive = true;
                break;
            }
        }
    }
}

void UpdatePowerUps(PowerUp *pups, int max, float dt) {
    for (int i = 0; i < max; i++) {
        if (pups[i].base.isActive) {
            pups[i].base.position.y += pups[i].base.velocity.y * dt;
            if (pups[i].base.position.y > WINDOW_HEIGHT) {
                pups[i].base.isActive = false;
            }
        }
    }
}

typedef void (*PowerUpEffect)(Player *p);

void PowerUpExtraLife(Player *p) {
    p->lives++;
}

void PowerUpIncreasePaddleWidth(Player *p) {
    p->width += TILE_WIDTH / 2;
}

PowerUpEffect powerUpEffects[] = {
    PowerUpExtraLife,
    PowerUpIncreasePaddleWidth
};

void HandlePowerUpCollision(PowerUp *pw, Player *pl) {
    Rectangle pr = {pl->base.position.x, pl->base.position.y, pl->width, pl->height};
    Rectangle wr = {pw->base.position.x, pw->base.position.y, 20, 20};
    if (pw->base.isActive && CheckCollisionRecs(pr, wr)) {
        pw->base.isActive = false;
        if (pw->type == 0) powerUpEffects[0](pl);
        else if (pw->type == 1) powerUpEffects[1](pl);
    }
}

void DestroyBlock(Block *blk, Ball *b, PowerUp *pups, int r, int c) {
    blk->base.isActive = false;
    b->base.velocity.y = -b->base.velocity.y;
    DropPowerUp(blk, pups);
    bool allGone = true;
    for (int i = 0; i < r * c; i++) {
        if (blk[i].base.isActive) {
            allGone = false;
            break;
        }
    }
    if (allGone) currentGameState = STATE_WIN;
}

bool HandleBallCollisions(Ball *b, Player *pl, Block *blocks, int r, int c, PowerUp *pups) {
    if (b->base.position.x - b->radius < 0 || b->base.position.x + b->radius > WINDOW_WIDTH) {
        b->base.velocity.x = -b->base.velocity.x;
    }
    if (b->base.position.y - b->radius < 0) {
        b->base.velocity.y = -b->base.velocity.y;
    }
    Rectangle pr = {pl->base.position.x, pl->base.position.y, pl->width, pl->height};
    Rectangle br = {b->base.position.x - b->radius, b->base.position.y - b->radius, b->radius * 2, b->radius * 2};
    if (CheckCollisionRecs(pr, br)) {
        b->base.velocity = ReflectBall(b, pl);
    }
    int cc = b->base.position.x / BLOCK_SIZE;
    int rr = b->base.position.y / TILE_HEIGHT;
    if (rr >= 0 && rr < r && cc >= 0 && cc < c) {
        Block *blk = &blocks[rr * c + cc];
        if (blk->base.isActive) {
            DestroyBlock(blk, b, pups, r, c);
        }
    }
    if (b->base.position.y + b->radius > WINDOW_HEIGHT) {
        b->base.isActive = false;
        pl->lives--;
        if (pl->lives <= 0) currentGameState = STATE_OVER;
        return true;
    }
    return false;
}

void UpdatePlayer(Player *pl, float dt) {
    if (IsKeyDown(KEY_A)) pl->base.velocity.x = -PLAYER_SPEED;
    else if (IsKeyDown(KEY_D)) pl->base.velocity.x = PLAYER_SPEED;
    else pl->base.velocity.x = 0;
    pl->base.position = Vector2Add(pl->base.position, Vector2Scale(pl->base.velocity, dt));
    if (pl->base.position.x < 0) pl->base.position.x = 0;
    if (pl->base.position.x + pl->width > WINDOW_WIDTH) pl->base.position.x = WINDOW_WIDTH - pl->width;
}

void UpdateBall(Ball *b, Player *pl, Block *blocks, int r, int c, PowerUp *pups, float dt) {
    if (!b->base.isActive) {
        b->speed = BALL_SPEED;
        b->base.position.x = pl->base.position.x + pl->width / 2;
        b->base.position.y = pl->base.position.y - b->radius - 5;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 d = Vector2Subtract(GetMousePosition(), b->base.position);
            b->base.velocity = Vector2Scale(Vector2Normalize(d), b->speed);
            b->base.isActive = true;
        }
    } else {
        b->base.position = Vector2Add(b->base.position, Vector2Scale(b->base.velocity, dt));
        HandleBallCollisions(b, pl, blocks, r, c, pups);
    }
}

void DrawPowerUp(PowerUp *pw) {
    if (pw->base.isActive) DrawCircleV(pw->base.position, 10, GREEN);
}

void DrawLifebar(Player *pl) {
    float w = 200.0f;
    float h = 20.0f;
    float pct = (float)pl->lives / 3.0f;
    float x = (WINDOW_WIDTH - w) / 2;
    float y = WINDOW_HEIGHT - h - 5;
    DrawRectangle(x, y, w, h, RED);
    DrawRectangle(x, y, w * pct, h, GREEN);
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

void DrawGame(Player *pl, Ball *b, Block *blocks, int r, int c, PowerUp *pups) {
    ClearBackground(BLACK);
    DrawBlocks(blocks, r, c);
    DrawPlayer(pl);
    DrawBall(b);
    for (int i = 0; i < MAX_POWERUPS; i++) DrawPowerUp(&pups[i]);
    DrawLifebar(pl);
}

void RestartGame(Player *pl, Ball *b, Block *blocks, PowerUp *pups, int r, int c) {
    currentGameState = STATE_START;
    *pl = InitPlayer((Vector2){WINDOW_WIDTH / 2 - TILE_WIDTH * 2.5f, WINDOW_HEIGHT - TILE_HEIGHT * 2});
    *b = InitBall((Vector2){pl->base.position.x + pl->width / 2, pl->base.position.y - 20});
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            blocks[i * c + j] = InitBlock((Vector2){j * BLOCK_SIZE, i * TILE_HEIGHT}, i % 3);
            blocks[i * c + j].base.isActive = (i < 3);
        }
    }
    pl->lives = 3;
    for (int i = 0; i < MAX_POWERUPS; i++) pups[i].base.isActive = false;
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Block Kuzuchi");
    int rows = WINDOW_HEIGHT / TILE_HEIGHT;
    int cols = WINDOW_WIDTH / TILE_HEIGHT;
    Block blocks[rows * cols];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            blocks[i * cols + j] = InitBlock((Vector2){j * BLOCK_SIZE, i * TILE_HEIGHT}, i % 3);
            blocks[i * cols + j].base.isActive = (i < 3);
        }
    }

    Player player = InitPlayer((Vector2){WINDOW_WIDTH / 2 - TILE_WIDTH * 2.5f, WINDOW_HEIGHT - TILE_HEIGHT * 2});
    Ball ball = InitBall((Vector2){player.base.position.x + player.width / 2, player.base.position.y - 20});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyPressed(KEY_F1)) currentGameState = STATE_OVER;
        if (IsKeyPressed(KEY_F2)) currentGameState = STATE_WIN;

        BeginDrawing();
        switch (currentGameState) {
            case STATE_START:
                ClearBackground(BLACK);
                DrawStartScreen();
                if (IsKeyPressed(KEY_SPACE)) currentGameState = STATE_PLAYING;
                break;

            case STATE_PLAYING:
                UpdatePlayer(&player, dt);
                UpdateBall(&ball, &player, blocks, rows, cols, powerUps, dt);
                UpdatePowerUps(powerUps, MAX_POWERUPS, dt);
                for (int i = 0; i < MAX_POWERUPS; i++) {
                    HandlePowerUpCollision(&powerUps[i], &player);
                }
                DrawGame(&player, &ball, blocks, rows, cols, powerUps);
                break;

            case STATE_OVER:
                ClearBackground(BLACK);
                DrawGameOverScreen();
                if (IsKeyPressed(KEY_ENTER)) {
                    RestartGame(&player, &ball, blocks, powerUps, rows, cols);
                }
                break;

            case STATE_WIN:
                ClearBackground(BLACK);
                DrawWinScreen();
                if (IsKeyPressed(KEY_ENTER)) {
                    RestartGame(&player, &ball, blocks, powerUps, rows, cols);
                }
                break;
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
