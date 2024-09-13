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
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <random>
#include <strings.h>
#include <valarray>

/*------------枚举---------------*/
enum TILE_TYPE {
  none = 0,
  ground = 1,

  box1 = 50,
};

/*----------------------*/

/*------------Consfig---------------*/
const char *GameName = "跳一跳";
const int TILE_WIDTH = 36;
const int TILE_HEIGHT = 36;
const int WINDOW_WIDTH = 540;
const int WINDOW_HEIGHT = 432;
const int TILE_ROW = 12;
const int TILE_CLOUMN = 15;
/*----------------------*/

/*----------Game------------*/
SDL_Window *gWindow = nullptr;
SDL_Renderer *gRender = nullptr;
SDL_Rect srcrect;
SDL_Rect dstrect;
// SDL_FRect dstrectF;
SDL_Event event;
int view_scale = 1;
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

/*----------资源------------*/
SDL_Texture *tex_tile = nullptr;
SDL_Texture *tex_actor = nullptr;
SDL_Texture *tex_box = nullptr;
/*----------------------*/

/*----------随机------------*/
std::random_device rd;
std::mt19937 gen(rd());
/*----------------------*/

/*----------Player------------*/
int player_x = 50;
int player_y = 50;
int player_w = 36;
int player_h = 36;
int player_image_index = 0;
int player_image_nums = 2;
int player_vx = 0.0f;
int player_vy = 0.0f;

int player_jumpSpeed = -15;
int player_moveSpeed = 5;

bool onGround = false;

bool player_hflip = false;

//眨眼随机范围
std::uniform_int_distribution<int> countDownRange(120, 300);
int countDown = countDownRange(gen);
/*----------------------------*/

/*----------Collition------------*/
struct CollitionInfo {
  TILE_TYPE type;
  int tx, ty;
  bool right;
  bool left;
  bool top;
  bool bottom;
};

bool collition_tile(int, int, int, int);

CollitionInfo myCollitionInfo;
void resetCollitionInfo();
/*----------------------*/

/*----------utils------------*/
int sign(int val) { return (0 < val) - (val < 0); }
/*----------------------*/

/*----------相机------------*/

/*----------------------*/

/*----------Item------------*/
struct ITEM {
  TILE_TYPE type;
  int tx, ty;
  int px, py;
  int vy;
  bool isRun;
};

ITEM items[10];
int Item_Count = 0;

void updateItem(int index);
/*----------------------*/

int map[TILE_ROW][TILE_CLOUMN] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 50, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
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
  tex_box = IMG_LoadTexture(gRender, "img/box.png");
  if (tex_box == nullptr) {
    std::cout << "纹理创建失败: " << IMG_GetError() << std::endl;
    exit(1);
  }
  startTick = SDL_GetTicks();

  //初始化游戏
  for (int i = 0; i < TILE_ROW; i++) {
    for (int j = 0; j < TILE_CLOUMN; j++) {
      int type = map[i][j];
      if (type >= TILE_TYPE::box1) {
        items[Item_Count].type = static_cast<TILE_TYPE>(type);
        items[Item_Count].tx = j;
        items[Item_Count].ty = i;
        items[Item_Count].px = j * TILE_WIDTH;
        items[Item_Count].py = i * TILE_HEIGHT;
        items[Item_Count].vy = 0;
        items[Item_Count].isRun = false;

        Item_Count++;
      }
    }
  }

  resetCollitionInfo();
}

void handleInput() {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      run = false;
    }
  }
}

void update() {
  player_vy += gravity;

  int playerW = player_w * view_scale;
  int playerH = player_h * view_scale;

  if (!collition_tile(player_x, player_y + player_vy, playerW, playerH)) {
    player_y += player_vy;
    onGround = false;
  } else {
    // 精确碰撞
    int move_count = std::abs(player_vy);
    int move_num = sign(player_vy);

    for (int i = 0; i < move_count; i++) {
      if (!collition_tile(player_x, player_y + move_num, playerW, playerH)) {
        player_y += move_num;
      } else {
        if (move_num < 0) {
          player_vy = 0;
        }
        break;
      }
    }

    //落到地面上
    if (move_num > 0) {
      onGround = true;
      player_vy = 0;
    }
  }

  if (!--countDown) {
    player_image_index = int(!player_image_index);
    countDown = player_image_index ? 10 : countDownRange(gen);
  }

  int left = KeyStatus[SDL_SCANCODE_A];
  int right = KeyStatus[SDL_SCANCODE_D];
  bool startJump = KeyStatus[SDL_SCANCODE_SPACE];
  int h = right - left;

  if (h < 0) {
    player_hflip = true;
  } else if (h > 0) {
    player_hflip = false;
  }

  if (onGround && startJump) {
    player_vy = player_jumpSpeed;
  }

  if (myCollitionInfo.top) {
    int index = myCollitionInfo.type - TILE_TYPE::box1;
    bool isrun = items[index].isRun;
    if (!isrun) {
      items[index].isRun = !isrun;
      items[index].vy = -5;
    }
  }

  int movex = h * player_moveSpeed;
  if (!collition_tile(player_x + movex, player_y, playerW, playerH)) {
    player_x += movex;
  } else {
    int move_count = abs(movex);
    int move_num = sign(movex);
    for (int i = 0; i < move_count; i++) {
      if (!collition_tile(player_x + move_num, player_y, playerW, playerH)) {
        player_x += move_num;
      } else {
        break;
      }
    }
  }

  for (int i = 0; i < Item_Count; i++) {
    updateItem(i);
  }

  resetCollitionInfo();
}

void render() {

  SDL_SetRenderDrawColor(gRender, 115, 76, 35, 255);
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

      if (map[i][j] >= TILE_TYPE::box1) {
        int index = map[i][j] - TILE_TYPE::box1;

        int x = items[index].px;
        int y = items[index].py;
        // int x = j * TILE_WIDTH * view_scale;
        // int y = i * TILE_HEIGHT * view_scale;
        srcrect = {0, 0, TILE_WIDTH, TILE_HEIGHT};
        dstrect = {x, y, TILE_WIDTH * view_scale, TILE_HEIGHT * view_scale};
        SDL_RenderCopy(gRender, tex_box, &srcrect, &dstrect);
      }
    }
  }

  srcrect = {player_w * player_image_index, 0, TILE_WIDTH, TILE_HEIGHT};
  dstrect = {player_x, player_y, player_w * view_scale, player_h * view_scale};

  int angle = 0;
  SDL_RenderCopyEx(gRender, tex_actor, &srcrect, &dstrect, angle, nullptr,
                   player_hflip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

  SDL_RenderPresent(gRender);
}

void clear() {
  SDL_DestroyTexture(tex_tile);
  SDL_DestroyRenderer(gRender);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
  IMG_Quit();
}

bool collition_tile(int x, int y, int w, int h) {
  int mapWidth = TILE_CLOUMN;
  int mapHeight = TILE_ROW;

  int tile_width = TILE_WIDTH * view_scale;
  int tile_height = TILE_HEIGHT * view_scale;

  SDL_Rect self_box = {x, y, w, h};

  bool isCollition = false;

  // 检测左上角的tile
  int lt_x = x / tile_width;
  int lt_y = y / tile_height;
  if (map[lt_y][lt_x]) {
    SDL_Rect lt_box = {lt_x * tile_width, lt_y * tile_height, tile_width,
                       tile_height};
    if (SDL_HasIntersection(&self_box, &lt_box)) {
      isCollition = true;

      myCollitionInfo.left = (lt_box.x + lt_box.w) > self_box.x;
      myCollitionInfo.top = (lt_box.y + lt_box.h) > self_box.y;
      myCollitionInfo.type = static_cast<TILE_TYPE>(map[lt_y][lt_x]);
    }
  }

  // 检测右上角的tile
  int rt_x = (x + w) / tile_width;
  int rt_y = y / tile_height;
  if (map[rt_y][rt_x]) {
    SDL_Rect rt_box = {rt_x * tile_width, rt_y * tile_height, tile_width,
                       tile_height};
    if (SDL_HasIntersection(&self_box, &rt_box)) {
      isCollition = true;

      myCollitionInfo.right = self_box.x + self_box.w > rt_box.x;
      myCollitionInfo.top = (rt_box.y + rt_box.h) > self_box.y;
      myCollitionInfo.type = static_cast<TILE_TYPE>(map[rt_y][rt_x]);
    }
  }

  // 检测左下角的tile
  int lb_x = x / tile_width;
  int lb_y = (y + h) / tile_height;
  if (map[lb_y][lb_x]) {
    SDL_Rect lb_box = {lb_x * tile_width, lb_y * tile_height, tile_width,
                       tile_height};
    if (SDL_HasIntersection(&self_box, &lb_box)) {
      isCollition = true;

      myCollitionInfo.left = (lb_box.x + lb_box.w) > self_box.x;
      myCollitionInfo.bottom = self_box.y + self_box.h > lb_box.y;
      myCollitionInfo.type = static_cast<TILE_TYPE>(map[lb_y][lb_x]);
    }
  }

  // 检测右下角的tile
  int rb_x = (x + w) / tile_width;
  int rb_y = (y + h) / tile_height;
  if (map[rb_y][rb_x]) {
    SDL_Rect rb_box = {rb_x * tile_width, rb_y * tile_height, tile_width,
                       tile_height};
    if (SDL_HasIntersection(&self_box, &rb_box)) {
      isCollition = true;

      myCollitionInfo.right = self_box.x + self_box.w > rb_box.x;
      myCollitionInfo.bottom = self_box.y + self_box.h > rb_box.y;
      myCollitionInfo.type = static_cast<TILE_TYPE>(map[lb_y][lb_x]);
    }
  }

  return isCollition;
}

void resetCollitionInfo() {
  myCollitionInfo.type = TILE_TYPE::none;
  myCollitionInfo.tx = 0;
  myCollitionInfo.ty = 0;
  myCollitionInfo.left = false;
  myCollitionInfo.bottom = false;
  myCollitionInfo.top = false;
  myCollitionInfo.right = false;
}

void updateItem(int index) {
  if (!items[index].isRun)
    return;
  int maxY = items[index].ty * TILE_HEIGHT;
  items[index].vy += gravity;
  items[index].py += items[index].vy;
  if (items[index].py > maxY) {
    items[index].py = maxY;
    items[index].vy = 0;
    items[index].isRun = false;
  }
}
