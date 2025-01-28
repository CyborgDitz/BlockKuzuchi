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
}


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
  
};


void UpdateGameState(void) {
    switch (game_state) {
      case GAME_START:
        ClearBackground(BLACK);
        DrawStartScreen();
        if (IsKeyPressed(KEY_SPACE)) {
          game_state = GAME_PLAYING;
        }
      break;
      case GAME_PLAYING:

      break;
      case GAME_OVER:
         ClearBackground(BLACK);
         DrawGameOverScreen();
         if (IsKeyPressed(KEY_SPACE)) {
           game_state = GAME_START;
         }
      break;
      case GAME_WON:
          ClearBackground(BLACK);
          DrawWinScreen();
          if (IsKeyPressed(KEY_SPACE)) {
            game_state = GAME_START;
          }
      break;
   }
}
int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pink Kuzhuchi");
  Game game;
  SetupGame(&game);

  while (!WindowShouldClose()) {
    float dt=GetFrameTime();
    UpdatePlayer(&game,dt);
    BeginDrawing();

    switch(game_state){
      case GAME_PLAYING

  }
