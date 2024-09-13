#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <strings.h>

/*------------Consfig---------------*/
const char *GameName = "跳一跳";
const int TILE_WIDTH = 18;
const int TILE_HEIGHT = 18;
const int WINDOW_WIDTH = 540;
const int WINDOW_HEIGHT = 432;
const int TILE_ROW = 12;
const int TILE_CLOUMN = 15;
/*----------------------*/

/*----------Player------------*/
int player_x = 50;
int player_y = 50;
int player_w = 18;
int player_h = 18;
int player_image_index = 0;
int player_image_nums = 2;
int player_vx = 0.0f;
int player_vy = 0.0f;

int player_jumpSpeed = -15;
int player_moveSpeed = 5;

bool onGround = false;
/*----------------------------*/

/*----------Textures------------*/
SDL_Texture *tex_tile = nullptr;
SDL_Texture *tex_actor = nullptr;

/*----------------------*/

/*----------Enums------------*/
enum TILE_TYPE {
  ground = 0,
};
/*----------------------*/

/*----------Collition------------*/
bool collition_tile(float, float, float, float);

/*----------------------*/

/*----------Game------------*/
SDL_Window *gWindow = nullptr;
SDL_Renderer *gRender = nullptr;
SDL_Rect srcrect;
SDL_Rect dstrect;
// SDL_FRect dstrectF;
SDL_Event event;
int view_scale = 2;
bool run = true;

float gravity = 1;

Uint32 startTick = 0;
float gameTime = 0.0f;
auto KeyStatus = SDL_GetKeyboardState(NULL);
void init();
void handleInput();
void render();
void update();
void clear();
/*----------------------*/

int map[TILE_ROW][TILE_CLOUMN] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

int main(int, char **) {
  init();

  while (run) {
    handleInput();

    auto ticks = SDL_GetTicks();
    float dt = (ticks - startTick) / 1000.0f;
    if (dt > (1 / 60.f)) {
      startTick = ticks;
    } else {
      continue;
    }
    gameTime += dt;
    update();
    render();
  }
  clear();
}

void init() {

  SDL_Init(SDL_INIT_EVERYTHING);
  gWindow =
      SDL_CreateWindow(GameName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == nullptr) {
    std::cout << "窗口创建失败: " << SDL_GetError() << std::endl;
    exit(1);
  }

  gRender = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  if (gRender == nullptr) {
    std::cout << "渲染器创建失败: " << SDL_GetError() << std::endl;
    exit(1);
  }

  IMG_Init(IMG_INIT_PNG);

  tex_tile = IMG_LoadTexture(gRender, "img/tile.png");
  if (tex_tile == nullptr) {
    std::cout << "纹理创建失败: " << IMG_GetError() << std::endl;
    exit(1);
  }
  tex_actor = IMG_LoadTexture(gRender, "img/actor.png");
  if (tex_actor == nullptr) {
    std::cout << "纹理创建失败: " << IMG_GetError() << std::endl;
    exit(1);
  }

  startTick = SDL_GetTicks();
}

void handleInput() {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      run = false;
    }
  }
}

void render() {

  SDL_SetRenderDrawColor(gRender, 255, 255, 255, 255);
  SDL_RenderClear(gRender);

  // 绘制地图
  for (int i = 0; i < TILE_ROW; i++) {
    for (int j = 0; j < TILE_CLOUMN; j++) {
      if (map[i][j] == 1) {
        int x = j * TILE_WIDTH * view_scale;
        int y = i * TILE_HEIGHT * view_scale;
        srcrect = {0, 0, TILE_WIDTH, TILE_HEIGHT};
        dstrect = {x, y, TILE_WIDTH * view_scale, TILE_HEIGHT * view_scale};
        SDL_RenderCopy(gRender, tex_tile, &srcrect, &dstrect);
      }
    }
  }

  srcrect = {player_w * player_image_index, 0, TILE_WIDTH, TILE_HEIGHT};
  dstrect = {player_x, player_y, player_w * view_scale, player_h * view_scale};
  SDL_RenderCopy(gRender, tex_actor, &srcrect, &dstrect);

  SDL_RenderPresent(gRender);
}

void update() {
  player_vy += gravity;
  if (!collition_tile(player_x, std::floor(player_y + player_vy),
                      player_w * view_scale, player_h * view_scale)) {
    player_y += player_vy;
    onGround = false;
  } else {
    if (player_vy > 0) {
      onGround = true;
      player_vy = 0;
    }
  }

  player_image_index = static_cast<int>(gameTime) % 2;

  int left = KeyStatus[SDL_SCANCODE_A];
  int right = KeyStatus[SDL_SCANCODE_D];
  bool startJump = KeyStatus[SDL_SCANCODE_SPACE];
  int h = right - left;

  if (onGround && startJump) {
    player_vy = player_jumpSpeed;
  }

  int movex = h * player_moveSpeed;
  if (!collition_tile(std::floor(player_x + movex), player_y,
                      player_w * view_scale, player_h * view_scale)) {
    player_x += movex;
  }
  // else {
  //   int index = 0;
  //   if (movex > 0) {
  //     index = int(player_x + movex) / (TILE_WIDTH * view_scale);
  //   } else {
  //     index = int(player_x - movex) / (TILE_WIDTH * view_scale);
  //   }
  //   player_x = index * (TILE_WIDTH * view_scale);
  // }

  std::cout << "x: " << player_x << "---"
            << "y: " << player_y << std::endl;
}

void clear() {
  SDL_DestroyTexture(tex_tile);
  SDL_DestroyRenderer(gRender);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
  IMG_Quit();
}

bool collition_tile(float x, float y, float w, float h) {
  int lt_x = x / (TILE_WIDTH * view_scale);
  int lt_y = y / (TILE_HEIGHT * view_scale);

  int lb_x = x / (TILE_WIDTH * view_scale);
  int lb_y = (y + h) / (TILE_HEIGHT * view_scale);

  int rt_x = (x + w) / (TILE_WIDTH * view_scale);
  int rt_y = y / (TILE_HEIGHT * view_scale);

  int rb_x = (x + w) / (TILE_WIDTH * view_scale);
  int rb_y = (y + h) / (TILE_HEIGHT * view_scale);

  return map[lt_y][lt_x] || map[lb_y][lb_x] || map[rt_y][rt_x] ||
         map[rb_y][rb_x];
}
