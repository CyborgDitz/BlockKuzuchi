#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH 720
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define BLOCK_SIZE (TILE_WIDTH * 2)
#define BALL_SPEED 600.0f
#define PLAYER_SPEED 600.0f
#define MAX_POWERUPS 3
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
    float width;
    float height;
    int lives;
} Player;

typedef struct {
    Entity base;
    float radius;
    float speed;
} Ball;
  typedef struct {
    Entity base;
    int bType;
} Block;

typedef struct {
    Entity base;
    int pType;
} PowerUp;

typedef struct {
  int rows;
  int cols;
  int cellWidth;
  int cellHeight;
} Grid;

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
    Block block[3 * 10];
    PowerUp powerUp[MAX_POWERUPS];
    Grid grid
} Game;

LifeBar getLifeBar = {
  .width     = 200.0f,
  .height    = 20.0f,
  .offsetY   = 5.0f,
  .backColor = RED,
  .frontColor= GREEN
};

GameState game_state = GAME_START;

Vector2 GetGridCellPosition(const Grid *grid, int rowIndex, int colIndex) {
  return (Vector2){
    colIndex * grid->cellWidth,
    rowIndex * grid->cellHeight
  };
};
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

void DrawPlayer(Player *player){
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
};

void DrawBall(Ball *ball){
    DrawCircleV(ball->base.position, ball->radius, PINK);
    DrawCircleLines(ball->base.position.x, ball->base.position.y, ball->radius, DARKPURPLE);
};

void DrawBlocks(Block *blocks, const Grid *grid) {
  for (int rowIndex = 0; rowIndex < grid->rows; rowIndex++) {
    for (int columnIndex = 0; columnIndex < grid->cols; columnIndex++) {

      int index = rowIndex * grid->cols + columnIndex;
      Block *block = &blocks[index];
      if (block->base.isActive) {
        Vector2 position = GetGridCellPosition(grid, rowIndex, columnIndex);
        Color blockColor = (block->bType == 0) ? WHITE
                            : (block->bType == 1) ? BLACK
                            : BLUE;
        DrawRectangle(position.x, position.y, grid->cellWidth, grid->cellHeight, blockColor);
        DrawRectangleLines(position.x, position.y, grid->cellWidth, grid->cellHeight, PURPLE);
      }
    }
  }
};

void DrawLifeBar(Player *player){
  float healthPercentage = (float)player->lives / 3.0f;
  float barX = (WINDOW_WIDTH - getLifeBar.width) / 2.0f;
  float barY = (WINDOW_HEIGHT - getLifeBar.height) - getLifeBar.offsetY;
  float fillWidth = getLifeBar.width * healthPercentage;

  DrawRectangle(barX, barY, getLifeBar.width, getLifeBar.height, getLifeBar.backColor);
  DrawRectangle(barX, barY, fillWidth, getLifeBar.height, getLifeBar.frontColor);
};

void DrawStartScreen( ){
  DrawText("Press SPACE to start", WINDOW_WIDTH /2 -150, WINDOW_HEIGHT / 2 - 20, 20, PINK);
 };

void DrawGameOverScreen() {
    DrawText("GAME OVER", WINDOW_WIDTH /2 -150, WINDOW_HEIGHT / 2 - 20, 50, RED);
    DrawText("Enter to Restart", WINDOW_WIDTH / 2 -150, WINDOW_HEIGHT / 2 + 30, 50, RED);
};
void DrawWinScreen(){
    DrawText("YOU WIN!", WINDOW_WIDTH / 2 -150, WINDOW_HEIGHT / 2 - 20, 50, GREEN);
    DrawText("Press Enter to Restart", WINDOW_WIDTH / 2 -150, WINDOW_HEIGHT / 2 + 30, 50, PINK);
};

void DrawGame(Player *player, Ball *ball, Block *blocks, const Grid *grid) {
        ClearBackground(BLACK);
        DrawBlocks(blocks, grid);
        DrawPlayer(player);
        DrawBall(ball);
        DrawLifeBar(player);
};

Player InitPlayer(Vector2 position) {
  Player player;
  player.base.position = position;
  player.width = TILE_WIDTH * 5;
  player.height = TILE_HEIGHT;
  player.lives = 3;
  return player;
};

Ball InitBall(Vector2 position){
  Ball ball;
  ball.base.position = position;
  ball.speed = BALL_SPEED;
  ball.radius = 16.0f;
  return ball;
};

Block InitBlock(Vector2 position,int bType){
  Block block;
  block.base.position = position;
  block.base.isActive = true;
  block.bType = bType;
  return block;
};

Grid InitGrid(int rows, int cols, int cellWidth, int cellHeight) {
  Grid grid;
  grid.rows       = rows;
  grid.cols       = cols;
  grid.cellWidth  = cellWidth;
  grid.cellHeight = cellHeight;
  return grid;
};
void DropPowerUp(Block *block, PowerUp *powerUps, const Grid *grid) {
  if (GetRandomValue(0, 100) < DROP_CHANCE * 100) {
    int rowIndex = (block->base.position.y / grid->cellHeight);
    int columnIndex = (block->base.position.x / grid->cellWidth);

    if (rowIndex >= 0 && rowIndex < grid->rows && columnIndex >= 0 && columnIndex < grid->cols) {
      for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].base.isActive) {
          powerUps[i].base.position = (Vector2){
            columnIndex * grid->cellWidth + grid->cellWidth / 2,
            rowIndex * grid->cellHeight + grid->cellHeight / 2
          };

          powerUps[i].base.velocity = (Vector2){0, 100.0f};
          powerUps[i].pType = GetRandomValue(0, 1);
          powerUps[i].base.isActive = true;
          break;
        }
      }
    }
  }
}



void SetupGame(Game *game) {
  int startPositionX = (WINDOW_WIDTH /2 - TILE_WIDTH * 2);
  int startPositionY = (WINDOW_HEIGHT - TILE_HEIGHT * 2);
game->player = InitPlayer((Vector2){startPositionX,startPositionY});
game->ball = InitBall (game->player.base.position);

Grid grid =InitGrid(3, 10, TILE_WIDTH, TILE_HEIGHT);

  for (int rowIndex = 0; rowIndex < grid.rows; rowIndex++) {
    for (int columnIndex = 0; columnIndex < grid.cols; columnIndex++) {
      Vector2 position = GetGridCellPosition(&grid, rowIndex, columnIndex);
      int blockType = (rowIndex + columnIndex) % 3;
      game->block[rowIndex * grid.cols + columnIndex] = InitBlock(position, blockType);
    }

  }
  game->grid = grid;
};

void BallWallCollision(Ball *ball) {
    if (ball->base.position.x - ball->radius < 0 || ball->base.position.x + ball->radius > WINDOW_WIDTH) {
        ball->base.velocity.x = -ball->base.velocity.x;
    }
    if (ball->base.position.y - ball->radius < 0) {
        ball->base.velocity.y = -ball->base.velocity.y;
    }
}

void BallPlayerCollision(Ball *ball, Player *player) {
    Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
    Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};

    if (CheckCollisionRecs(playerRect, ballRect)) {
        Vector2 collisionPoint = Vector2Subtract(ball->base.position, player->base.position);
        collisionPoint = Vector2Normalize(collisionPoint);
        ball->base.velocity = Vector2Reflect(ball->base.velocity, collisionPoint);
        ball->base.velocity = Vector2Scale(Vector2Normalize(ball->base.velocity), ball->speed);
    }
}

void BallBlockCollision(Ball *ball, Block *blocks, const Grid *grid,PowerUp *powerUps) {
    int columnIndex = ball->base.position.x / grid->cellWidth;
    int rowIndex = ball->base.position.y / grid->cellHeight;

    if (rowIndex >= 0 && rowIndex < grid->rows && columnIndex >= 0 && columnIndex < grid->cols) {
        Block *block = &blocks[rowIndex * grid->cols + columnIndex];
        if (block->base.isActive) {
            Rectangle blockRect = {block->base.position.x, block->base.position.y, grid->cellWidth, grid->cellHeight};
            Rectangle ballRect = {ball->base.position.x - ball->radius, ball->base.position.y - ball->radius, ball->radius * 2, ball->radius * 2};

            if (CheckCollisionRecs(ballRect, blockRect)) {

                block->base.isActive = false;
                Vector2 normal = {0, 0};

                if (ball->base.position.x < block->base.position.x) {
                    normal.x = 1;
                } else if (ball->base.position.x > block->base.position.x + grid->cellWidth) {
                    normal.x = -1;
                }

                if (ball->base.position.y < block->base.position.y) {
                    normal.y = 1;
                } else if (ball->base.position.y > block->base.position.y + grid->cellHeight) {
                    normal.y = -1;
                }
                ball->base.velocity = Vector2Reflect(ball->base.velocity, normal);


                DropPowerUp(block, powerUps);
            }
        }
    }
}

void PowerUpCollision(PowerUp *powerUp, Player *player) {
  Rectangle playerRect = {player->base.position.x, player->base.position.y, player->width, player->height};
  Rectangle powerUpRect = {powerUp->base.position.x, powerUp->base.position.y, 20, 20};

  if (powerUp->base.isActive && CheckCollisionRecs(playerRect, powerUpRect)) {
    powerUp->base.isActive = false;
    powerUpEffects[powerUp->pType](player);
  }
};


void UpdatePlayer(Player *player, float dt) {
  int movementDir = 0;

  if (IsKeyDown(KEY_A)) {
    movementDir = -1;
  }
  else if (IsKeyDown(KEY_D)) {
    movementDir = 1;
  }
  switch (movementDir) {
    case -1:
      player->base.velocity.x = -PLAYER_SPEED * (1.003f * dt);
    break;
    case 1:
      player->base.velocity.x = PLAYER_SPEED * (1.003f * dt);
    break;
    default:
      player->base.velocity.x = 0.0f;
    break;
  }
}

void UpdateBall(Ball *ball, Player *player, Block *blocks, const Grid *grid, float dt) {
  ball->speed = BALL_SPEED;
  if(!ball->base.isActive)
  {
    Vector2 ballStartPos = {
      player->base.position.x + player->width * 0.5f,
      player->base.position.y - (ball->radius + 5.0f)
    };
    ball->base.position = ballStartPos;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 direction = Vector2Subtract(GetMousePosition(), ballStartPos);
      ball->base.velocity = direction;
      ball->base.isActive = true;
    }
  } else
    {ball->base.position = Vector2Add(ball->base.position, Vector2Scale(ball->base.velocity,dt));}
  BallWallCollision(ball);
  BallPlayerCollision(ball, player);
  BallBlockCollision(ball, blocks, grid /**, powerUps**/);
};
void UpdatePowerUps(PowerUp *powerUps, int maxPowerUps, float dt) {
  for (int i = 0; i < maxPowerUps; i++) {
    if (powerUps[i].base.isActive) {
      powerUps[i].base.position.y += powerUps[i].base.velocity.y * dt;
      if (powerUps[i].base.position.y > WINDOW_HEIGHT) {
        powerUps[i].base.isActive = false;
      }
    }
  }
}

void UpdateGameState(Game *game) {
  switch (game_state) {
    case GAME_START:
      ClearBackground(BLACK);
    DrawStartScreen();
    if (IsKeyPressed(KEY_SPACE)) {
      game_state = GAME_PLAYING;
    }
    break;

    case GAME_PLAYING:
        if (game->player.lives <= 0) {
          game_state = GAME_OVER;
        }
    bool allBlocksDestroyed = true;
    for (int i = 0; i < 3 * 10; i++) {
      if (game->block[i].base.isActive) {
        allBlocksDestroyed = false;
        break;
      }
    }

    if (allBlocksDestroyed) {
      game_state = GAME_WON;
    }
    ClearBackground(BLACK);
    DrawGame(&game->player, &game->ball, game->block, &game->grid);
    break;

    case GAME_OVER:
      ClearBackground(BLACK);
    DrawGameOverScreen();
    if (IsKeyPressed(KEY_SPACE)) {
      game_state = GAME_START;
      SetupGame(game);
    }
    break;

    case GAME_WON:
      ClearBackground(BLACK);
    DrawWinScreen();
    if (IsKeyPressed(KEY_SPACE)) {
      game_state = GAME_START;
      SetupGame(game);
    }
    break;

    default:
      break;
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pink Kuzhuchi");
  Game game;
  SetupGame(&game);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    UpdatePlayer(&game.player, dt);
    UpdateBall(&game.ball, &game.player, game.block, &game.grid, dt);
    UpdateGameState(&game);

    BeginDrawing();

    EndDrawing();
  }
  CloseWindow();
  return 0;
}


