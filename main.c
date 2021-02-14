#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "images.h"

// Useful macros for drawing the interface and doing calculations
#define PADDING_LEFT 12
#define PADDING_RIGHT 8
#define PADDING_BOTTOM 8
#define PADDING_TOP 9

#define TOP_HEIGHT 37

#define TILE_SIZE 16

#define FIELD_X (PADDING_LEFT)
#define FIELD_Y (PADDING_TOP * 2 + TOP_HEIGHT)
#define FIELD_WIDTH (width * TILE_SIZE)
#define FIELD_HEIGHT (height * TILE_SIZE)

#define TILE_IS_NUMBER(i) (field[i] > 0 && field[i] < 9)

#define FIELD_MIN_WIDTH 9
#define FIELD_MIN_HEIGHT 9
#define FIELD_MAX_WIDTH 24
#define FIELD_MAX_HEIGHT 30
#define MIN_MINE_COUNT 10
#define MAX_MINE_COUNT ((width - 1) * (height - 1))

#define L_WIDTH (FIELD_WIDTH + PADDING_LEFT + PADDING_RIGHT)
#define L_HEIGHT (FIELD_HEIGHT + PADDING_BOTTOM + TOP_HEIGHT + PADDING_TOP * 2)

#define FACE_SIZE 26
#define FACE_X (ceil((L_WIDTH - 3) / 2) - (FACE_SIZE / 2) + 3)
#define FACE_Y (PADDING_TOP + 6)

// Render states for a tile
enum {
  TILE_EMPTY = 0,
  // Numbers 1-8 are implied
  TILE_UNCLICKED = 9,
  TILE_FLAG = 10,
  TILE_MAYBE = 11,
  TILE_MAYBEPRESS = 12,
  TILE_MINE = 13,
  TILE_REDMINE = 14,
  TILE_WRONG = 15
};

// Face button states
enum {
  FACE_SMILE,
  FACE_CLICK,
  FACE_OPEN,
  FACE_DEAD,
  FACE_WIN
};

SDL_Window* window;
SDL_Renderer* renderer;
unsigned window_scale = 1;

// Textures
SDL_Texture* face_click_tex;
SDL_Texture* face_dead_tex;
SDL_Texture* face_smile_tex;
SDL_Texture* face_open_tex;
SDL_Texture* face_win_tex;
SDL_Texture* game_corner_tex;
SDL_Texture* top_corner_tex;
SDL_Texture* tile_empty_tex;
SDL_Texture* tile_flag_tex;
SDL_Texture* tile_maybe_tex;
SDL_Texture* tile_maybepress_tex;
SDL_Texture* tile_mine_tex;
SDL_Texture* tile_redmine_tex;
SDL_Texture* tile_unclicked_tex;
SDL_Texture* tile_wrong_tex;
SDL_Texture* tile_num_tex[8];
SDL_Texture* timer_num_tex[10];
SDL_Texture* timer_minus_tex;

// Game state variables
unsigned width = 9;
unsigned height = 9;
unsigned timer = 0;
unsigned face = FACE_SMILE;

unsigned total_mines = 10;
int mines_left = 10;

int placed = 0;
int dead = 0;
int win = 0;

unsigned char* tiles = NULL;
unsigned char* field = NULL;

int face_pressed = 0;

int shift_down = 0;

struct {
  int left;
  int middle;
  int right;
} mouse_buttons = {
  0,
  0,
  0
};

struct {
  int x;
  int y;
} selected_tile = {
  -1,
  -1
};

// Draw the frame for the game
void draw_frame() {
  // Grey edges
  SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
  // Top panel frame top left
  { // Top
    SDL_Rect dst = {PADDING_LEFT - 3, PADDING_TOP, FIELD_WIDTH + 4, 2};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {PADDING_LEFT - 3, PADDING_TOP + 2, 2, TOP_HEIGHT - 4};
    SDL_RenderFillRect(renderer, &dst); 
  }
  // Mine field frame top left
  { // Top
    SDL_Rect dst = {PADDING_LEFT, PADDING_TOP * 2 + TOP_HEIGHT - 3, FIELD_WIDTH, 3};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {PADDING_LEFT - 3, PADDING_TOP * 2 + TOP_HEIGHT - 3, 3, height * TILE_SIZE + 3};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine count frame top left
  { // Top
    SDL_Rect dst = {PADDING_LEFT + 4, PADDING_TOP + 6, 40, 1};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {PADDING_LEFT + 4, PADDING_TOP + 7, 1, 23};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Timer frame top left
  { // Top
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 47, PADDING_TOP + 6, 40, 1};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 47, PADDING_TOP + 7, 1, 23};
    SDL_RenderFillRect(renderer, &dst);
  }

  // White edges
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  // Window frame top left
  { // Top
    SDL_Rect dst = {0, 0, L_WIDTH, 3};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {0, 0, 3, L_HEIGHT};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Top panel frame bottom right
  { // Bottom
    SDL_Rect dst = {PADDING_LEFT - 1, PADDING_TOP + TOP_HEIGHT - 2, L_WIDTH - PADDING_LEFT - PADDING_RIGHT + 2, 2};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT + 1, PADDING_TOP + 2, 2, TOP_HEIGHT - 2};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine field frame bottom right
  { // Bottom
    SDL_Rect dst = {PADDING_LEFT, L_HEIGHT - PADDING_BOTTOM, FIELD_WIDTH, 3};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT, PADDING_TOP * 2 + TOP_HEIGHT, 3, height * TILE_SIZE + 3};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine count frame bottom right
  { // Bottom
    SDL_Rect dst = {PADDING_LEFT + 5, PADDING_TOP + 6 + 24, 40, 1};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {PADDING_LEFT + 4 + 40, PADDING_TOP + 7, 1, 23};
    SDL_RenderFillRect(renderer, &dst);
  }
  // Timer frame bottom right
  { // Bottom
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 6 - 40, PADDING_TOP + 6 + 24, 40, 1};
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 7, PADDING_TOP + 7, 1, 23};
    SDL_RenderFillRect(renderer, &dst);
  }

  // Corner textures
  // Top frame corners
  { // Bottom left
    SDL_Rect src = {0, 0, top_corner.width, top_corner.height};
    SDL_Rect dst = {PADDING_LEFT - 3, PADDING_TOP + TOP_HEIGHT - 2, top_corner.width, top_corner.height};
    SDL_RenderCopy(renderer, top_corner_tex, &src, &dst);
  }
  { // Top right
    SDL_Rect src = {0, 0, top_corner.width, top_corner.height};
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT + 1, PADDING_TOP, top_corner.width, top_corner.height};
    SDL_RenderCopy(renderer, top_corner_tex, &src, &dst);
  }
  // Mine field corners
  { // Bottom left
    SDL_Rect src = {0, 0, game_corner.width, game_corner.height};
    SDL_Rect dst = {PADDING_LEFT - 3, L_HEIGHT - PADDING_BOTTOM, game_corner.width, game_corner.height};
    SDL_RenderCopy(renderer, game_corner_tex, &src, &dst);
  }
  { // Top right
    SDL_Rect src = {0, 0, game_corner.width, game_corner.height};
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT, PADDING_TOP * 2 + TOP_HEIGHT - 3, game_corner.width, game_corner.height};
    SDL_RenderCopy(renderer, game_corner_tex, &src, &dst);
  }
}

// Converts a number to 3 digits for displaying on the timer and mine counts
void num_to_digits(unsigned num, unsigned* digits) {
  char buf[4];
  sprintf(buf, "%03d", num);
  for (int i = 0; i < 3; i++) {
    digits[i] = buf[i] - '0';
  }
}

// Draw the number of mines remaining in the left LCD
void draw_mine_count() {
  unsigned digits[3];
  SDL_Rect src = {0, 0, timer_0.width, timer_0.height};
  num_to_digits(mines_left, digits);
  for (int i = 0; i < 3; i++) {
    SDL_Rect dst = {PADDING_LEFT + 5 + i * timer_0.width, PADDING_TOP + 7, timer_0.width, timer_0.height};
    SDL_RenderCopy(renderer, timer_num_tex[digits[i]], &src, &dst);
  }
}

// Draw the number of seconds elapsed in the right LCD
void draw_timer() {
  unsigned digits[3];
  SDL_Rect src = {0, 0, timer_0.width, timer_0.height};
  num_to_digits(timer, digits);
  for (int i = 0; i < 3; i++) {
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 46 + i * timer_0.width, PADDING_TOP + 7, timer_0.width, timer_0.height};
    SDL_RenderCopy(renderer, timer_num_tex[digits[i]], &src, &dst);
  }
}

// Draw the face button
void draw_face() {
  SDL_Rect src = {0, 0, face_smile.width, face_smile.height};
  SDL_Rect dst = {FACE_X, FACE_Y, face_smile.width, face_smile.height};

  switch (face) {
    case FACE_SMILE:
      SDL_RenderCopy(renderer, face_smile_tex, &src, &dst);
      break;
    case FACE_CLICK:
      SDL_RenderCopy(renderer, face_click_tex, &src, &dst);
      break;
    case FACE_OPEN:
      SDL_RenderCopy(renderer, face_open_tex, &src, &dst);
      break;
    case FACE_DEAD:
      SDL_RenderCopy(renderer, face_dead_tex, &src, &dst);
      break;
    case FACE_WIN:
      SDL_RenderCopy(renderer, face_win_tex, &src, &dst);
      break;
    default:
      break;
  }
}

// Draw a tile with a given render state at a given position (tile-indexed)
void draw_tile(unsigned state, unsigned x, unsigned y) {
  SDL_Rect src = {0, 0, TILE_SIZE, TILE_SIZE};
  SDL_Rect dst = {x * TILE_SIZE + FIELD_X, y * TILE_SIZE + FIELD_Y, TILE_SIZE, TILE_SIZE};

  switch (state) {
    case TILE_UNCLICKED:
      SDL_RenderCopy(renderer, tile_unclicked_tex, &src, &dst);
      break;
    case TILE_EMPTY:
      SDL_RenderCopy(renderer, tile_empty_tex, &src, &dst);
      break;
    case TILE_FLAG:
      SDL_RenderCopy(renderer, tile_flag_tex, &src, &dst);
      break;
    case TILE_MAYBE:
      SDL_RenderCopy(renderer, tile_maybe_tex, &src, &dst);
      break;
    case TILE_MAYBEPRESS:
      SDL_RenderCopy(renderer, tile_maybepress_tex, &src, &dst);
      break;
    case TILE_MINE:
      SDL_RenderCopy(renderer, tile_mine_tex, &src, &dst);
      break;
    case TILE_REDMINE:
      SDL_RenderCopy(renderer, tile_redmine_tex, &src, &dst);
      break;
    case TILE_WRONG:
      SDL_RenderCopy(renderer, tile_wrong_tex, &src, &dst);
      break;
    default:
      SDL_RenderCopy(renderer, tile_num_tex[state - 1], &src, &dst);
      break;
  }
}

// Repaint the interface
void repaint() {
  SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
	SDL_RenderClear(renderer);

  draw_frame();
  
  draw_timer();
  draw_mine_count();

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      draw_tile(tiles[y * width + x], x, y);
    }
  }

  if (selected_tile.x > -1 && selected_tile.y > -1) {
    if (tiles[selected_tile.y * width + selected_tile.x] == TILE_MAYBE) {
      draw_tile(TILE_MAYBEPRESS, selected_tile.x, selected_tile.y);
    } else {
      draw_tile(TILE_EMPTY, selected_tile.x, selected_tile.y);
    }
  }

  draw_face();

  SDL_RenderPresent(renderer);
}

// Place mines and numbers, ignoring the first click position
void place_mines(unsigned first_x, unsigned first_y) {
  // Place mines
  for (int i = 0; i < mines_left; i++) {
    int placed = 0;
    while (!placed) {
      int x = rand() % width;
      int y = rand() % height;
      if (x != first_x && y != first_y && field[y * width + x] != TILE_MINE) {
        field[y * width + x] = TILE_MINE;
        placed = 1;
      }
    }
  }

  // Place numbers
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (field[y * width + x] == TILE_MINE) {
        continue;
      }

      int nearby_mines = 0;

      // Check left
      if (x != 0 && field[y * width + (x - 1)] == TILE_MINE) {
        nearby_mines++;
      }
      // Check up left
      if (x != 0 && y != 0 && field[(y - 1) * width + (x - 1)] == TILE_MINE) {
        nearby_mines++;
      }
      // Check up
      if (y != 0 && field[(y - 1) * width + x] == TILE_MINE) {
        nearby_mines++;
      }
      // Check up right
      if (y != 0 && x != width - 1 && field[(y - 1) * width + (x + 1)] == TILE_MINE) {
        nearby_mines++;
      }
      // Check right
      if (x != width - 1 && field[y * width + (x + 1)] == TILE_MINE) {
        nearby_mines++;
      }
      // Check down right
      if (x != width - 1 && y != height - 1 && field[(y + 1) * width + (x + 1)] == TILE_MINE) {
        nearby_mines++;
      }
      // Check down
      if (y != height - 1 && field[(y + 1) * width + x] == TILE_MINE) {
        nearby_mines++;
      }
      // Check down left
      if (x != 0 && y != height - 1 && field[(y + 1) * width + (x - 1)] == TILE_MINE) {
        nearby_mines++;
      }

      if (nearby_mines != 0) {
        field[y * width + x] = nearby_mines;
      } else {
        field[y * width + x] = TILE_EMPTY;
      }
    }
  }

  placed = 1;
}

// Start a new game
void reset_game() {
  width = 9;
  height = 9;
  timer = 0;
  face = FACE_SMILE;

  selected_tile.x = -1;
  selected_tile.y = -1;
  face_pressed = 0;

  total_mines = 2;
  mines_left = total_mines;

  placed = 0;
  dead = 0;
  win = 0;

  if (tiles != NULL) {
    free(tiles);
  }

  if (field != NULL) {
    free(field);
  }

  tiles = (unsigned char*)calloc(width * height, sizeof(unsigned char));
  field = (unsigned char*)calloc(width * height, sizeof(unsigned char));

  // Initialize tiles
  for (int i = 0; i < width * height; i++) {
    tiles[i] = TILE_UNCLICKED;
  }

  SDL_SetWindowSize(window, L_WIDTH * window_scale, L_HEIGHT * window_scale);
  SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);

  repaint();
}

// Mine was clicked, reveal all the mines and wrong flags
void game_over() {
  dead = 1;

  for (int i = 0; i < width * height; i++) {
    if (field[i] == TILE_MINE) {
      if (tiles[i] == TILE_UNCLICKED) {
        tiles[i] = TILE_MINE;
      }
    }
  }
}

// Check whether the game has been won
void check_win() {
  if (mines_left != 0) {
    return;
  }

  unsigned uncovered = 0;
  for (int i = 0; i < width * height; i++) {
    if (tiles[i] != TILE_UNCLICKED && tiles[i] != TILE_FLAG && tiles[i] != TILE_MAYBE) {
      uncovered++;
    }
  }

  if (uncovered == width * height - total_mines) {
    win = 1;
    face = FACE_WIN;
  }
}

// Recursive function to clear any empty tiles, up until a number tile is reached
void flood_fill(unsigned x, unsigned y) {
  int i = y * width + x;

  tiles[i] = field[i];

  unsigned min_x = x != 0 ? x - 1 : x;
  unsigned min_y = y != 0 ? y - 1 : y;
  unsigned max_x = x != width - 1 ? x + 1 : x;
  unsigned max_y = y != height - 1 ? y + 1 : y;

  for (unsigned ny = min_y; ny <= max_y; ny++) {
    for (unsigned nx = min_x; nx <= max_x; nx++) {
      if (nx == x && ny == x) {
        continue;
      }

      unsigned ni = ny * width + nx;

      // If the current tile is a number tile and the next tile is a number tile, skip it
      if (TILE_IS_NUMBER(i) && TILE_IS_NUMBER(ni)) {
        continue;
      }

      // Otherwise, recurse with the next tile, if it's eligible
      if (
        field[ni] < TILE_UNCLICKED &&
        tiles[ni] == TILE_UNCLICKED
      ) {
        flood_fill(nx, ny);
      }
    }
  }
}

// Reveal a tile after mouseup while hovering over it
void handle_tile_click(unsigned x, unsigned y) {
  if (!placed) {
    place_mines(x, y);
  }

  int i = y * width + x;

  if (tiles[i] != TILE_UNCLICKED && tiles[i] != TILE_MAYBE) {
    return;
  }

  if (field[i] == TILE_MINE) {
    tiles[i] = TILE_REDMINE;
    game_over();
  } else if (field[i] != TILE_EMPTY) {
    tiles[i] = field[i];
    check_win();
  } else {
    flood_fill(x, y);
  }
}

// Handle mousedown event
void handle_mousedown(unsigned button) {
  if (button == SDL_BUTTON_LEFT) {
    mouse_buttons.left = 1;
  } else if (button == SDL_BUTTON_MIDDLE) {
    mouse_buttons.middle = 1;
  } else if (button == SDL_BUTTON_RIGHT) {
    mouse_buttons.right = 1;
  }

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= window_scale;
  y /= window_scale;

  // Change the face, but ignore right clicks
  if (!(mouse_buttons.right && !mouse_buttons.left && !mouse_buttons.middle)) {
    if (x >= FACE_X && x < FACE_X + face_smile.width && y >= FACE_Y && y < FACE_Y + face_smile.height) {
      face_pressed = 1;
      face = FACE_CLICK;
    } else if (!dead && !win) {
      face = FACE_OPEN;
    }
  }

  // Clicks within the playfield
  if (
    !dead &&
    !win &&
    x >= FIELD_X &&
    x < FIELD_X + FIELD_WIDTH &&
    y >= FIELD_Y &&
    y < FIELD_Y + FIELD_HEIGHT
  ) {
    unsigned tx = (x - FIELD_X) / TILE_SIZE;
    unsigned ty = (y - FIELD_Y) / TILE_SIZE;
    unsigned i = ty * width + tx;

    // Left click only
    if (mouse_buttons.left && !mouse_buttons.right && !mouse_buttons.middle) {
      if (tiles[i] == TILE_UNCLICKED || tiles[i] == TILE_MAYBE) {
        selected_tile.x = tx;
        selected_tile.y = ty;
      }
    }
    // Right click only
    if (mouse_buttons.right && !mouse_buttons.left && !mouse_buttons.middle) {
      if (tiles[i] == TILE_UNCLICKED) {
        tiles[i] = TILE_FLAG;
        mines_left--;
        check_win();
      } else if (tiles[i] == TILE_FLAG) {
        tiles[i] = TILE_MAYBE;
        mines_left++;
      } else if (tiles[i] == TILE_MAYBE) {
        tiles[i] = TILE_UNCLICKED;
      }
      mouse_buttons.right = 0;
    }
  }

  repaint();
}

// Handle mouseup event
void handle_mouseup(unsigned button) {
  selected_tile.x = -1;
  selected_tile.y = -1;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= window_scale;
  y /= window_scale;

  // Change the face, if it's pressed down
  if (face_pressed && x >= FACE_X && x < FACE_X + face_smile.width && y >= FACE_Y && y < FACE_Y + face_smile.height) {
    reset_game();
  }

  // Clicks within the playfield
  else if (
    mouse_buttons.left &&
    !mouse_buttons.right &&
    !mouse_buttons.middle &&
    !dead &&
    !win &&
    !face_pressed &&
    x >= FIELD_X &&
    x < FIELD_X + FIELD_WIDTH &&
    y >= FIELD_Y &&
    y < FIELD_Y + FIELD_HEIGHT
  ) {
    int tx = (x - FIELD_X) / TILE_SIZE;
    int ty = (y - FIELD_Y) / TILE_SIZE;
    handle_tile_click(tx, ty);
  }

  if (!win) {
    face = dead ? FACE_DEAD : FACE_SMILE;
    face_pressed = 0;
  }

  if (button == SDL_BUTTON_LEFT) {
    mouse_buttons.left = 0;
  } else if (button == SDL_BUTTON_MIDDLE) {
    mouse_buttons.middle = 0;
  } else if (button == SDL_BUTTON_RIGHT) {
    mouse_buttons.right = 0;
  }

  repaint();
}

// Handle mousemove event
void handle_mousemove() {
  if (!mouse_buttons.left) {
    return;
  }

  int needs_repaint = 0;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= window_scale;
  y /= window_scale;

  // If the face button was pressed in a prior mousedown event
  if (face_pressed) {
    // If the mouse is within the face button, press it down
    if (
      x >= FACE_X &&
      x < FACE_X + face_smile.width &&
      y >= FACE_Y &&
      y < FACE_Y + face_smile.height
    ) {
      if (face != FACE_CLICK) {
        needs_repaint = 1;
      }
      face = FACE_CLICK;
    }
    // Otherwise, reset it
    else if (face != FACE_SMILE && face != FACE_DEAD) {
      face = win ? FACE_WIN : (dead ? FACE_DEAD : FACE_SMILE);
      needs_repaint = 1;
    }
  } else if (
    !dead &&
    !win &&
    x >= FIELD_X &&
    x < FIELD_X + FIELD_WIDTH &&
    y >= FIELD_Y &&
    y < FIELD_Y + FIELD_HEIGHT
  ) {
    unsigned tx = (x - FIELD_X) / TILE_SIZE;
    unsigned ty = (y - FIELD_Y) / TILE_SIZE;
    unsigned i = ty * width + tx;

    if (tiles[i] == TILE_UNCLICKED || tiles[i] == TILE_MAYBE) {
      selected_tile.x = tx;
      selected_tile.y = ty;
      needs_repaint = 1;
    } else if (selected_tile.x != -1 && selected_tile.y != -1) {
      selected_tile.x = -1;
      selected_tile.y = -1;
      needs_repaint = 1;
    }
  } else if (selected_tile.x != -1 && selected_tile.y != -1) {
    selected_tile.x = -1;
    selected_tile.y = -1;
    needs_repaint = 1;
  }

  if (needs_repaint) {
    repaint();
  }
}

// Handle keypress
void handle_keyup(SDL_Keysym sym) {
  switch (sym.sym) {
    case SDLK_EQUALS:
      window_scale++;
      SDL_SetWindowSize(window, L_WIDTH * window_scale, L_HEIGHT * window_scale);
      SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);
      repaint();
      break;
    case SDLK_MINUS:
      window_scale = window_scale > 1 ? window_scale - 1 : 1;
      SDL_SetWindowSize(window, L_WIDTH * window_scale, L_HEIGHT * window_scale);
      SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);
      break;
    case SDLK_F2:
    case SDLK_r:
      reset_game();
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      shift_down = 0;
      break;
    default:
      break;
  }
}

// Handle key down
void handle_keydown(SDL_Keysym sym) {
  switch (sym.sym) {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      shift_down = 1;
      break;
    default:
      break;
  }
}

// Load textures from image data
void load_textures() {
  Image tile_num[] = {
    tile_1,
    tile_2,
    tile_3,
    tile_4,
    tile_5,
    tile_6,
    tile_7,
    tile_8
  };

  Image timer_num[] = {
    timer_0,
    timer_1,
    timer_2,
    timer_3,
    timer_4,
    timer_5,
    timer_6,
    timer_7,
    timer_8,
    timer_9
  };

  face_click_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, face_click.width, face_click.height);
  SDL_UpdateTexture(face_click_tex, NULL, face_click.pixels, face_click.width * 3);

  face_dead_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, face_dead.width, face_dead.height);
  SDL_UpdateTexture(face_dead_tex, NULL, face_dead.pixels, face_dead.width * 3);

  face_smile_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, face_smile.width, face_smile.height);
  SDL_UpdateTexture(face_smile_tex, NULL, face_smile.pixels, face_smile.width * 3);

  face_open_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, face_open.width, face_open.height);
  SDL_UpdateTexture(face_open_tex, NULL, face_open.pixels, face_open.width * 3);

  face_win_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, face_win.width, face_win.height);
  SDL_UpdateTexture(face_win_tex, NULL, face_win.pixels, face_win.width * 3);

  game_corner_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, game_corner.width, game_corner.height);
  SDL_UpdateTexture(game_corner_tex, NULL, game_corner.pixels, game_corner.width * 3);
  
  top_corner_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, top_corner.width, top_corner.height);
  SDL_UpdateTexture(top_corner_tex, NULL, top_corner.pixels, top_corner.width * 3);

  tile_empty_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_empty.width, tile_empty.height);
  SDL_UpdateTexture(tile_empty_tex, NULL, tile_empty.pixels, tile_empty.width * 3);

  tile_flag_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_flag.width, tile_flag.height);
  SDL_UpdateTexture(tile_flag_tex, NULL, tile_flag.pixels, tile_flag.width * 3);

  tile_maybe_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_maybe.width, tile_maybe.height);
  SDL_UpdateTexture(tile_maybe_tex, NULL, tile_maybe.pixels, tile_maybe.width * 3);

  tile_maybepress_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_maybepress.width, tile_maybepress.height);
  SDL_UpdateTexture(tile_maybepress_tex, NULL, tile_maybepress.pixels, tile_maybepress.width * 3);

  tile_mine_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_mine.width, tile_mine.height);
  SDL_UpdateTexture(tile_mine_tex, NULL, tile_mine.pixels, tile_mine.width * 3);

  tile_redmine_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_redmine.width, tile_redmine.height);
  SDL_UpdateTexture(tile_redmine_tex, NULL, tile_redmine.pixels, tile_redmine.width * 3);

  tile_unclicked_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_unclicked.width, tile_unclicked.height);
  SDL_UpdateTexture(tile_unclicked_tex, NULL, tile_unclicked.pixels, tile_unclicked.width * 3);

  tile_wrong_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_wrong.width, tile_wrong.height);
  SDL_UpdateTexture(tile_wrong_tex, NULL, tile_wrong.pixels, tile_wrong.width * 3);

  timer_minus_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, timer_minus.width, timer_minus.height);
  SDL_UpdateTexture(timer_minus_tex, NULL, timer_minus.pixels, timer_minus.width * 3);

  for (int i = 0; i < 8; i++) {
    tile_num_tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_num[i].width, tile_num[i].height);
    SDL_UpdateTexture(tile_num_tex[i], NULL, tile_num[i].pixels, tile_num[i].width * 3);
  }

  for (int i = 0; i < 10; i++) {
    timer_num_tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, timer_num[i].width, timer_num[i].height);
    SDL_UpdateTexture(timer_num_tex[i], NULL, timer_num[i].pixels, timer_num[i].width * 3);
  }
}

void cleanup() {
  if (tiles != NULL) {
    free(tiles);
  }

  if (field != NULL) {
    free(field);
  }
}

int main() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return -1;
	}

  srand(time(0));

  window = SDL_CreateWindow(
		"mines",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		L_WIDTH * window_scale,
		L_HEIGHT * window_scale,
		SDL_WINDOW_OPENGL
	);

  renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  load_textures();

  reset_game();

  int running = 1;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
				case SDL_QUIT:
          running = 0;
          break;
        case SDL_MOUSEBUTTONDOWN:
          handle_mousedown(e.button.button);
          break;
        case SDL_MOUSEBUTTONUP:
          handle_mouseup(e.button.button);
          break;
        case SDL_MOUSEMOTION:
          handle_mousemove();
          break;
        case SDL_KEYDOWN:
          handle_keydown(e.key.keysym);
          break;
        case SDL_KEYUP:
          handle_keyup(e.key.keysym);
          break;
			}
		}
    // Throttle at least a tiny bit so it doesn't use 100% CPU
    SDL_Delay(1);
	}

  cleanup();

  return 0;
}
