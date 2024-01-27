#include "raylib.h"

typedef enum Screen { GAME=1, END } Screen;

typedef struct Ball {
  Vector2 position;
  Vector2 velocity;
  float radius;
  bool is_active;
} Ball;

typedef struct Paddle {
  Vector2 position;
  Vector2 velocity;
  Vector2 size;
  int lives;
} Paddle;

const int brick_width = 800 / 10;
const int brick_height = 450 / 24;

void draw_bricks(bool brick_map[6][10]) {
  Color brick_colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET};

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 10; j++) {
      if (brick_map[i][j]) {
        DrawRectangle((float)(j * brick_width), (float)(i * brick_height),
                      brick_width, brick_height, brick_colors[5 - i]);
        DrawRectangleLines(j * brick_width, i * brick_height, brick_width,
                           brick_height, BLACK);
      }
    }
  }
}

void draw_paddle(Paddle paddle) {
  DrawRectangle(paddle.position.x, paddle.position.y, paddle.size.x,
                paddle.size.y, GRAY);
}

void update_paddle_x(Paddle *paddle) {
  float dt = GetFrameTime();
  if (IsKeyDown(KEY_LEFT) && paddle->position.x >= 0)
    paddle->position.x -= paddle->velocity.x * dt;
  if (IsKeyDown(KEY_RIGHT) && paddle->position.x + paddle->size.x <= 800)
    paddle->position.x += paddle->velocity.x * dt;
}

void draw_lives(Paddle player) {
  int life_width = 20;
  int life_height = 10;

  for (int i = 0; i < player.lives; i++) {
    DrawRectangle(i * (life_width + 5) + 20, 430, life_width, life_height,
                  GREEN);
  }
}

void draw_ball(Ball ball) {
  DrawCircle(ball.position.x, ball.position.y, ball.radius, GRAY);
}

void update_ball_pos(Ball *ball) {
  float dt = GetFrameTime();

  ball->position.x += ball->velocity.x * dt;
  ball->position.y += ball->velocity.y * dt;
  float ball_x = ball->position.x;
  float ball_y = ball->position.y;

  if ((ball_x + ball->radius >= 800) || (ball_x - ball->radius <= 0)) {
    ball->velocity.x *= -1.0;
  }
  if ((ball_y - ball->radius <= 0)) {
    ball->velocity.y *= -1.0;
  }
}

void ball_paddle_collision(Ball *ball, Paddle paddle) {
  if (CheckCollisionCircleRec(ball->position, ball->radius,
                              (Rectangle){paddle.position.x, paddle.position.y,
                                          paddle.size.x, paddle.size.y})) {
    if (ball->velocity.y > 0) {
      ball->velocity.y *= -1;
      ball->velocity.x =
          (ball->position.x - (paddle.position.x + paddle.size.x / 2.0)) * 15.0;
    }
  }
}

int brick_collision(Ball *ball, bool brick_map[6][10], Sound bounce) {
  int brick_counter = 0;

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 10; j++) {
      if (brick_map[i][j]) {
        brick_counter++;
        if (CheckCollisionCircleRec(ball->position, ball->radius,
                                    (Rectangle){j * brick_width,
                                                i * brick_height, brick_width,
                                                brick_height})) {
          
          if(ball->velocity.y < 0) ball->velocity.y *= -1.0;
          brick_map[i][j] = false;
          PlaySound(bounce);
          goto end;
        }
      }
    }
  }

  end:
    return brick_counter;
}

void reset(Ball* ball, Paddle* player, bool brick_map[6][10]) {
  
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 10; j++) {
      brick_map[i][j] = true;
    }
  }

  ball->position = (Vector2){400.0, 380.0};
  ball->velocity = (Vector2){0.0, 0.0};
  ball->radius = 10.0;
  ball->is_active = false;

  player->position = (Vector2){360.0, 400}; 
  player->velocity = (Vector2){600.0, 0.0};
  player->size = (Vector2){80.0, 20.0}; 
  player->lives = 5;
}

int main(void) {
  const int canvas_width = 800;
  const int canvas_height = 450;

  // Settings
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  SetConfigFlags(FLAG_VSYNC_HINT);

  InitWindow(canvas_width, canvas_height, "breakout");
  InitAudioDevice();
  RenderTexture2D canvas = LoadRenderTexture(canvas_width, canvas_height);
  Sound end = LoadSound("assets/end.mp3");
  Sound bounce = LoadSound("assets/bounce.mp3");

  // Setup
  float time = 0;
  Screen screen = GAME;
  bool brick_map[6][10];
  Ball ball;
  Paddle player;
  int brick_counter = 0;

  reset(&ball, &player, brick_map);
  
  while (!WindowShouldClose()) {

    switch (screen) {
    case GAME:
      update_paddle_x(&player);

      if (ball.position.y - ball.radius >= canvas_height) {
        player.lives -= 1;

        ball.is_active = false;
        ball.velocity = (Vector2){0.0, 0.0};
        ball.position.y = 380;
      }

      if (ball.is_active) {
        update_ball_pos(&ball);
      } else {
        ball.position.x = player.position.x + player.size.x / 2.0;
      }

      if (IsKeyPressed(KEY_SPACE) && !ball.is_active) {
        ball.is_active = true;
        ball.velocity = (Vector2){0.0, 600.0};
        PlaySound(bounce);
      }

      ball_paddle_collision(&ball, player);
      brick_counter = brick_collision(&ball, brick_map, bounce);

      if (brick_counter == 0 || player.lives == 0) {
        screen = END;
        PlaySound(end);
      }

      time = GetTime();

      BeginTextureMode(canvas);
        ClearBackground(BLACK);
        draw_bricks(brick_map);
        draw_paddle(player);
        draw_ball(ball);
        draw_lives(player);
      EndTextureMode();
      break;
      case END:
      if (IsKeyPressed(KEY_ENTER)) {
        screen = GAME;
        StopSound(end);
        reset(&ball, &player, brick_map);
      }

      BeginTextureMode(canvas);
        ClearBackground(BLACK);
        DrawText("Press [Enter] to play again", 400-MeasureText("Press [Enter] to play again", 20), 225, 40, GREEN);
      EndTextureMode();
      break;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTexturePro(
        canvas.texture,
        (Rectangle){0.0, 0.0, (float)canvas_width, -(float)canvas_height},
        (Rectangle){0.0, 0.0, (float)GetScreenWidth(),
                    (float)GetScreenHeight()},
        (Vector2){0, 0}, 0.0, WHITE);
    EndDrawing();
  }

  UnloadRenderTexture(canvas);
  CloseWindow();
  return 0;
}
