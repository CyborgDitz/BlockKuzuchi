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
    int lives
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
    Player player;
    Ball ball;
    Block block;
    PowerUp powerUp[MAX_POWERUPS];
    } Game;

    //todo move init


    Player InitPlayer(Vector2 position) {
      Player player = {0};
      player.base.position = position;
      player.width = TILE_WIDTH * 5;
      player.height = TILE_HEIGHT;
      player.lives = 3;
      return player;
    };

    Ball InitBall(Vector2 position){
      Ball ball = {0};
      ball.base.position = position;
      ball.speed = BALL_SPEED;
      ball.radius = 16.0f;
      return ball;
    };

    Block InitBlock(Vector2 position,int bType){
      Block block = {0};
      block.base.position = position;
      block.base.isActive = true;
      block.bType = bType;
      return block;
    };
   Grid InitGrid(Grid* grid, int rows, int cols, int cellWidth, int cellHeight) {
    grid->rows       = rows;
    grid->cols       = cols;
    grid->cellWidth  = cellWidth;
    grid->cellHeight = cellHeight;
    return *grid;
  };
GameState game_state = GAME_START;

Vector2 GetGridCellPosition(const Grid *grid, int rowIndex, int colIndex) {
  return (Vector2){
    colIndex * grid->cellWidth,
    rowIndex * grid->cellHeight
};
}


//todo drawings

  void DrawPlayer(Player *player){
    DrawRectangle(player->base.position.x, player->base.position.y, player->width, player->height, PURPLE);
    DrawRectangleLines(player->base.position.x, player->base.position.y, player->width, player->height, DARKPURPLE);
  };

  void DrawBall(Ball *ball){
    DrawCircleV(ball->base.position, ball->radius, PINK);
    DrawCircleLines(ball->base.position.x, ball->base.position.x, ball->radius, DARKPURPLE);
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
}

//todo draw powerup, w grid ref

//todo updates
void UpdatePlayer(Player *player, float dt){
  if(IsKeyDown(KEY_A)){
   player->base.velocity.x = (-PLAYER_SPEED)*(1.003*dt);
  }
  if(IsKeyDown(KEY_D)){
    player->base.velocity.x = (PLAYER_SPEED)*(1.003*dt);
  }
    else {
    player->base.velocity.x = 0;
    }

};

  void UpdateGameState(void) {
    switch (game_state) {
      case GAME_START:
        ClearBackground(BLACK);
        //todo DrawStartScreen();
        if (IsKeyPressed(KEY_SPACE)) {
          game_state = GAME_PLAYING;
        }
      break;
      case GAME_PLAYING:
        //todo add update
      break;
      case GAME_OVER:
         ClearBackground(BLACK);
         //todo DrawGameOverScreen();
         if (IsKeyPressed(KEY_SPACE)) {
           game_state = GAME_START;
         }
      break;
      case GAME_WON:
          ClearBackground(BLACK);
          //todo DrawWinScreen();
          if (IsKeyPressed(KEY_SPACE)) {
            game_state = GAME_START;
          }
      break;
   }
}
