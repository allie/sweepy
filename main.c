#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
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
  FACE_SMILE = 0,
  FACE_CLICK,
  FACE_OPEN,
  FACE_DEAD,
  FACE_WIN
};

// Timer states
enum {
  // 0-9 are implied
  TIMER_MINUS = 10
};

SDL_Window* window;
SDL_Renderer* renderer;
unsigned window_scale = 1;

// Textures
SDL_Texture* face_tex[5];
SDL_Texture* tile_tex[16];
SDL_Texture* timer_tex[11];
SDL_Texture* game_corner_tex;
SDL_Texture* top_corner_tex;
SDL_Texture* font_tex;

// Difficulty presets
typedef struct {
  unsigned width;
  unsigned height;
  unsigned total_mines;
} Difficulty;

Difficulty beginner = { 9, 9, 10 };
Difficulty intermediate = { 16, 16, 40 };
Difficulty expert = { 30, 16, 99 };

// Game state variables
unsigned width = 9;
unsigned height = 9;
unsigned face = FACE_SMILE;

unsigned total_mines = 10;
int mines_left = 10;

int started = 0;
int dead = 0;
int win = 0;

int timer_stopped = 0;
unsigned timer = 0;
struct timespec start_time;

int maybe_enabled = 1;

unsigned char* tiles = NULL;
unsigned char* field = NULL;

int face_pressed = 0;

int shift_down = 0;

struct {
  int left;
  int middle;
  int right;
} mouse_buttons = { 0, 0, 0 };

int multiselect = 0;
struct {
  int x;
  int y;
} selected_tile = { -1, -1 };

char* popup_str = NULL;
int popup_showing = 0;
SDL_TimerID popup_timer;

// Render a string onto the screen
void draw_string(const char* str, unsigned x, unsigned y) {
  unsigned current_x = x;
  for (int i = 0; i < strlen(str); i++) {
    unsigned char c = str[i] - ' ';
    unsigned short position = font_positions[c];
    unsigned char width = font_widths[c];

    SDL_Rect src = { position, 0, width, font.height };
    SDL_Rect dst = { current_x, y, width, font.height };

    SDL_RenderCopy(renderer, font_tex, &src, &dst);

    current_x += width;
  }
}

// Calculate the total width of a string
unsigned string_width(const char* str) {
  unsigned width = 0;
  for (int i = 0; i < strlen(str); i++) {
    unsigned char c = str[i] - ' ';
    width += font_widths[c];
  }
  return width;
}

// Draw the frame for the game
void draw_frame() {
  // Grey edges
  SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
  // Top panel frame top left
  { // Top
    SDL_Rect dst = {
      PADDING_LEFT - 3,
      PADDING_TOP,
      FIELD_WIDTH + 4,
      2
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {
      PADDING_LEFT - 3,
      PADDING_TOP + 2,
      2,
      TOP_HEIGHT - 4
    };
    SDL_RenderFillRect(renderer, &dst); 
  }
  // Mine field frame top left
  { // Top
    SDL_Rect dst = {
      PADDING_LEFT,
      PADDING_TOP * 2 + TOP_HEIGHT - 3,
      FIELD_WIDTH,
      3
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {
      PADDING_LEFT - 3,
      PADDING_TOP * 2 + TOP_HEIGHT - 3,
      3,
      height * TILE_SIZE + 3
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine count frame top left
  { // Top
    SDL_Rect dst = {
      PADDING_LEFT + 4,
      PADDING_TOP + 6,
      40,
      1
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {
      PADDING_LEFT + 4,
      PADDING_TOP + 7,
      1,
      23
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Timer frame top left
  { // Top
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT - 47,
      PADDING_TOP + 6,
      40,
      1
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT - 47,
      PADDING_TOP + 7,
      1,
      23
    };
    SDL_RenderFillRect(renderer, &dst);
  }

  // White edges
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  // Window frame top left
  { // Top
    SDL_Rect dst = {
      0,
      0,
      L_WIDTH,
      3
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Left
    SDL_Rect dst = {
      0,
      0,
      3,
      L_HEIGHT
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Top panel frame bottom right
  { // Bottom
    SDL_Rect dst = {
      PADDING_LEFT - 1,
      PADDING_TOP + TOP_HEIGHT - 2,
      L_WIDTH - PADDING_LEFT - PADDING_RIGHT + 2,
      2
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT + 1,
      PADDING_TOP + 2,
      2,
      TOP_HEIGHT - 2
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine field frame bottom right
  { // Bottom
    SDL_Rect dst = {
      PADDING_LEFT,
      L_HEIGHT - PADDING_BOTTOM,
      FIELD_WIDTH,
      3
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT,
      PADDING_TOP * 2 + TOP_HEIGHT,
      3,
      height * TILE_SIZE + 3
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Mine count frame bottom right
  { // Bottom
    SDL_Rect dst = {
      PADDING_LEFT + 5,
      PADDING_TOP + 6 + 24,
      40,
      1
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {
      PADDING_LEFT + 4 + 40,
      PADDING_TOP + 7,
      1,
      23
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  // Timer frame bottom right
  { // Bottom
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT - 6 - 40,
      PADDING_TOP + 6 + 24,
      40,
      1
    };
    SDL_RenderFillRect(renderer, &dst);
  }
  { // Right
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT - 7,
      PADDING_TOP + 7,
      1,
      23
    };
    SDL_RenderFillRect(renderer, &dst);
  }

  // Corner textures
  // Top frame corners
  { // Bottom left
    SDL_Rect src = {
      0,
      0,
      top_corner.width,
      top_corner.height
    };
    SDL_Rect dst = {
      PADDING_LEFT - 3,
      PADDING_TOP + TOP_HEIGHT - 2,
      top_corner.width,
      top_corner.height
    };
    SDL_RenderCopy(renderer, top_corner_tex, &src, &dst);
  }
  { // Top right
    SDL_Rect src = {
      0,
      0,
      top_corner.width,
      top_corner.height
    };
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT + 1,
      PADDING_TOP,
      top_corner.width,
      top_corner.height
    };
    SDL_RenderCopy(renderer, top_corner_tex, &src, &dst);
  }
  // Mine field corners
  { // Bottom left
    SDL_Rect src = {
      0,
      0,
      game_corner.width,
      game_corner.height
    };
    SDL_Rect dst = {
      PADDING_LEFT - 3,
      L_HEIGHT - PADDING_BOTTOM,
      game_corner.width,
      game_corner.height
    };
    SDL_RenderCopy(renderer, game_corner_tex, &src, &dst);
  }
  { // Top right
    SDL_Rect src = {
      0,
      0,
      game_corner.width,
      game_corner.height
    };
    SDL_Rect dst = {
      L_WIDTH - PADDING_RIGHT,
      PADDING_TOP * 2 + TOP_HEIGHT - 3,
      game_corner.width,
      game_corner.height
    };
    SDL_RenderCopy(renderer, game_corner_tex, &src, &dst);
  }
}

// Converts a number to 3 digits for displaying on the timer and mine counts
void num_to_digits(int num, int* digits) {
  // Clamp to [-999, 999]
  if (num < -999) {
    num = -999;
  } else if (num > 999) {
    num = 999;
  }

  char buf[4];
  sprintf(buf, "%03d", abs(num));

  for (int i = 0; i < 3; i++) {
    digits[i] = buf[i] - '0';
  }

  // Negative numbers should start with a negative sign
  // Replace the first digit; Windows minesweeper does it this way as well
  // If you place enough flags to get past -99, the counter will "wrap" to -01
  if (num < 0) {
    digits[0] = TIMER_MINUS;
  }
}

// Draw the number of mines remaining in the left LCD
void draw_mine_count() {
  int digits[3];
  SDL_Rect src = {0, 0, timer_0.width, timer_0.height};
  num_to_digits(mines_left, digits);
  for (int i = 0; i < 3; i++) {
    SDL_Rect dst = {PADDING_LEFT + 5 + i * timer_0.width, PADDING_TOP + 7, timer_0.width, timer_0.height};
    SDL_RenderCopy(renderer, timer_tex[digits[i]], &src, &dst);
  }
}

// Draw the number of seconds elapsed in the right LCD
void draw_timer() {
  int digits[3];
  SDL_Rect src = {0, 0, timer_0.width, timer_0.height};
  num_to_digits(timer, digits);
  for (int i = 0; i < 3; i++) {
    SDL_Rect dst = {L_WIDTH - PADDING_RIGHT - 46 + i * timer_0.width, PADDING_TOP + 7, timer_0.width, timer_0.height};
    SDL_RenderCopy(renderer, timer_tex[digits[i]], &src, &dst);
  }
}

// Draw the face button
void draw_face() {
  SDL_Rect src = {0, 0, face_smile.width, face_smile.height};
  SDL_Rect dst = {FACE_X, FACE_Y, face_smile.width, face_smile.height};
  SDL_RenderCopy(renderer, face_tex[face], &src, &dst);
}

// Draw a tile with a given render state at a given position (tile-indexed)
void draw_tile(unsigned state, unsigned x, unsigned y) {
  SDL_Rect src = {0, 0, TILE_SIZE, TILE_SIZE};
  SDL_Rect dst = {x * TILE_SIZE + FIELD_X, y * TILE_SIZE + FIELD_Y, TILE_SIZE, TILE_SIZE};
#ifdef DEBUG_MODE
  if (field[y * width + x] == TILE_MINE)
    SDL_RenderCopy(renderer, tile_tex[TILE_MINE], &src, &dst);
  else
#endif
  SDL_RenderCopy(renderer, tile_tex[state], &src, &dst);
}

// Pop up a message in the corner
void draw_popup_message() {
  unsigned width = string_width(popup_str);
  SDL_Rect dst = { 0, 0, width + 2, font.height + 2 };
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192);
  SDL_RenderFillRect(renderer, &dst);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  draw_string(popup_str, 1, 1);
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
    if (multiselect) {
      // Determine the indices of the surrounding tiles
      unsigned min_x = selected_tile.x != 0 ? selected_tile.x - 1 : selected_tile.x;
      unsigned min_y = selected_tile.y != 0 ? selected_tile.y - 1 : selected_tile.y;
      unsigned max_x = selected_tile.x != width - 1 ? selected_tile.x + 1 : selected_tile.x;
      unsigned max_y = selected_tile.y != height - 1 ? selected_tile.y + 1 : selected_tile.y;

      for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
          int i = y * width + x;
          if (tiles[i] == TILE_MAYBE) {
            draw_tile(TILE_MAYBEPRESS, x, y);
          } else if (tiles[i] != TILE_FLAG) {
            if (tiles[i] > 0 && tiles[i] < 9) {
              draw_tile(tiles[i], x, y);
            } else {
              draw_tile(TILE_EMPTY, x, y);
            }
          }
        }
      }
    } else {
      if (tiles[selected_tile.y * width + selected_tile.x] == TILE_MAYBE) {
        draw_tile(TILE_MAYBEPRESS, selected_tile.x, selected_tile.y);
      } else {
        draw_tile(TILE_EMPTY, selected_tile.x, selected_tile.y);
      }
    }
  }

  draw_face();

  if (popup_showing) {
    draw_popup_message();
  }

  SDL_RenderPresent(renderer);
}

// Callback to emit an event indicating that the popup is ready to close
Uint32 popup_callback(Uint32 interval, void* param) {
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);

  return interval;
}

// Destroy the current popup message
void close_popup() {
  if (popup_str != NULL) {
    free(popup_str);
    popup_str = NULL;
  }
  popup_showing = 0;
  SDL_RemoveTimer(popup_timer);

  repaint();
}

// Show a popup message for a period of time
void show_popup(const char* str) {
  if (popup_showing) {
    close_popup();
  }
  SDL_RemoveTimer(popup_timer);

  popup_str = strdup(str);
  popup_showing = 1;
  popup_timer = SDL_AddTimer(2000, popup_callback, NULL);

  repaint();
}

// Change the window scale by a given amount, with 1x being the minimum
void rescale_window(int delta) {
  window_scale += delta;
  window_scale = window_scale < 1 ? 1 : window_scale;
  SDL_SetWindowSize(window, L_WIDTH * window_scale, L_HEIGHT * window_scale);
  SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);

  char str[100];
  sprintf(str, "Window scale: %dx", window_scale);
  show_popup(str);

  repaint();
}

// Place mines and numbers, ignoring the first click position
void place_mines(unsigned first_x, unsigned first_y) {
  // Place mines
  for (int m = 0; m < total_mines; m++) {
    int placed = 0;
    while (!placed) {
      int x = rand() % width;
      int y = rand() % height;
      int i = y * width + x;

      if ((x != first_x || y != first_y) && field[i] != TILE_MINE) {
        field[i] = TILE_MINE;
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

      // Determine the indices of the surrounding tiles
      unsigned min_x = x != 0 ? x - 1 : x;
      unsigned min_y = y != 0 ? y - 1 : y;
      unsigned max_x = x != width - 1 ? x + 1 : x;
      unsigned max_y = y != height - 1 ? y + 1 : y;

      // Check surrounding tiles for mines
      for (unsigned ny = min_y; ny <= max_y; ny++) {
        for (unsigned nx = min_x; nx <= max_x; nx++) {
          if (nx == x && ny == y) {
            continue;
          }

          if (field[ny * width + nx] == TILE_MINE) {
            nearby_mines++;
          }
        }
      }

      if (nearby_mines != 0) {
        field[y * width + x] = nearby_mines;
      } else {
        field[y * width + x] = TILE_EMPTY;
      }
    }
  }

  started = 1;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  // Windows minesweeper starts the timer at 1 on the first click
  timer = 1;
  start_time.tv_sec--;
}

// Start a new game
void reset_game(unsigned new_width, unsigned new_height, unsigned new_total_mines) {
  // Don't allow resetting the game when the left or middle mouse are down
  if (mouse_buttons.left || mouse_buttons.middle) {
    return;
  }

  width = new_width > FIELD_MAX_WIDTH ? FIELD_MAX_WIDTH : new_width;
  height = new_height > FIELD_MAX_HEIGHT ? FIELD_MAX_HEIGHT : new_height;

  total_mines = new_total_mines > MAX_MINE_COUNT ? MAX_MINE_COUNT : new_total_mines;
  mines_left = total_mines;

  timer_stopped = 0;
  timer = 0;
  face = FACE_SMILE;

  selected_tile.x = -1;
  selected_tile.y = -1;
  face_pressed = 0;

  started = 0;
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
  for (int i = 0; i < width * height; i++) {
    if (tiles[i] == TILE_FLAG && field[i] != TILE_MINE) {
      tiles[i] = TILE_WRONG;
    } else if (field[i] == TILE_MINE) {
      if (tiles[i] == TILE_UNCLICKED) {
        tiles[i] = TILE_MINE;
      }
    }
  }
}

// Check whether the game has been won
void check_win() {
  if (mines_left < 0) {
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
    mines_left = 0;

    // Reveal any tiles that are still covered
    for (int i = 0; i < width * height; i++) {
      if (field[i] == TILE_MINE) {
        tiles[i] = TILE_FLAG;
      }
    }
  }
}

// Recursive function to clear any empty tiles, up until a number tile is reached
void flood_fill(unsigned x, unsigned y) {
  int i = y * width + x;

  // Uncover the current tile, if it's not flagged
  if (tiles[i] != TILE_FLAG) {
    tiles[i] = field[i];
  }

  // Determine the indices of the surrounding tiles
  unsigned min_x = x != 0 ? x - 1 : x;
  unsigned min_y = y != 0 ? y - 1 : y;
  unsigned max_x = x != width - 1 ? x + 1 : x;
  unsigned max_y = y != height - 1 ? y + 1 : y;

  for (unsigned ny = min_y; ny <= max_y; ny++) {
    for (unsigned nx = min_x; nx <= max_x; nx++) {
      // Skip the current tile, only check surrounding ones
      if (nx == x && ny == y) {
        continue;
      }

      // Calculate the index of the next tile to check
      unsigned ni = ny * width + nx;

      // If the current tile is a number tile and the next tile is a number tile, skip it
      if (TILE_IS_NUMBER(i) && TILE_IS_NUMBER(ni)) {
        continue;
      }

      // Otherwise, recurse with the next tile, if it's eligible
      // The next tile must be empty or a number tile, and unclicked
      if (
        field[ni] < TILE_UNCLICKED &&
        (tiles[ni] == TILE_UNCLICKED || tiles[ni] == TILE_FLAG || tiles[ni] == TILE_MAYBE)
      ) {
        flood_fill(nx, ny);
      }
    }
  }
}

// Reveal a tile after mouseup while hovering over it
void handle_tile_click(unsigned x, unsigned y) {
  // Place mines during the first click
  if (!started) {
    place_mines(x, y);
  }

  // Determine the indices of the tiles to check
  unsigned min_x;
  unsigned min_y;
  unsigned max_x;
  unsigned max_y;

  // If multiselecting, the current tile and all surrounding tiles will be checked
  // The current tile must be an uncovered number tile, and the number of flags must
  // equal the current tile's number, or else nothing will happen
  if (multiselect) {
    min_x = x != 0 ? x - 1 : x;
    min_y = y != 0 ? y - 1 : y;
    max_x = x != width - 1 ? x + 1 : x;
    max_y = y != height - 1 ? y + 1 : y;

    int i = y * width + x;

    // Return if multiselecting on a non-number tile
    if (tiles[i] == 0 || tiles[i] > 8) {
      return;
    }

    // Check if there are enough flags
    int flag_count = 0;
    for (int ny = min_y; ny <= max_y; ny++) {
      for (int nx = min_x; nx <= max_x; nx++) {
        if (nx == x && ny == y) {
          continue;
        }

        if (tiles[ny * width + nx] == TILE_FLAG) {
          flag_count++;
        }
      }
    }
    if (flag_count != tiles[i]) {
      return;
    }
  }
  // If there is only one selected tile, set the iteration bounds to
  // just the current tile index
  else {
    min_x = x;
    min_y = y;
    max_x = x;
    max_y = y;
  }

  // Reveal the selected tile(s)
  for (int ny = min_y; ny <= max_y; ny++) {
    for (int nx = min_x; nx <= max_x; nx++) {
      int i = ny * width + nx;

      // If single clicking and this tile is already uncovered, do nothing
      if (!multiselect && tiles[i] != TILE_UNCLICKED && tiles[i] != TILE_MAYBE) {
        return;
      }

      // If this tile is flagged, skip it
      if (tiles[i] == TILE_FLAG) {
        continue;
      }

      // If this tile is a mine, reveal it and set the dead flag
      if (field[i] == TILE_MINE) {
        tiles[i] = TILE_REDMINE;
        dead = 1;
      }
      // If the tile is empty, flood fill it
      else if (field[i] == TILE_EMPTY) {
        flood_fill(x, y);
      }
      // If the tile is not empty, uncover it
      else {
        tiles[i] = field[i];
      }
    }
  }

  // If a mine was hit, game over
  if (dead) {
    game_over();
  }
  // Otherwise, check for a win after this click
  else {
    check_win();
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

  // Figure out if multiselecting
  if (
    (shift_down && mouse_buttons.left) ||
    (mouse_buttons.left && mouse_buttons.right) ||
    mouse_buttons.middle
  ) {
    multiselect = 1;
  }

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= window_scale;
  y /= window_scale;

  // Change the face, but ignore right clicks
  if (!(mouse_buttons.right && !mouse_buttons.left)) {
    if (
      x >= FACE_X &&
      x < FACE_X + face_smile.width &&
      y >= FACE_Y &&
      y < FACE_Y + face_smile.height &&
      !mouse_buttons.middle
    ) {
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

    // Multiselect
    if (multiselect) {
      selected_tile.x = tx;
      selected_tile.y = ty;
    }
    // Left click
    else if (mouse_buttons.left && !mouse_buttons.right && !mouse_buttons.middle) {
      if (tiles[i] == TILE_UNCLICKED || tiles[i] == TILE_MAYBE) {
        selected_tile.x = tx;
        selected_tile.y = ty;
      }
    }
    // Right click only
    else if (mouse_buttons.right && !mouse_buttons.left && !mouse_buttons.middle) {
      if (tiles[i] == TILE_UNCLICKED) {
        tiles[i] = TILE_FLAG;
        mines_left--;
      } else if (tiles[i] == TILE_FLAG) {
        tiles[i] = maybe_enabled ? TILE_MAYBE : TILE_UNCLICKED;
        mines_left++;
      } else if (tiles[i] == TILE_MAYBE) {
        tiles[i] = TILE_UNCLICKED;
      }
    }
  }

  repaint();
}

// Handle mouseup event
void handle_mouseup(unsigned button) {
  // Determine if this was a single right click
  // If so, we won't treat it as a tile click, because flags are placed on mousedown
  int right_click = (
    mouse_buttons.right &&
    !mouse_buttons.left &&
    !mouse_buttons.middle &&
    button == SDL_BUTTON_RIGHT
  );

  // Reset the selected tile
  selected_tile.x = -1;
  selected_tile.y = -1;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= window_scale;
  y /= window_scale;

  // Change the face, if it's pressed down
  if (
    face_pressed &&
    x >= FACE_X &&
    x < FACE_X + face_smile.width &&
    y >= FACE_Y &&
    y < FACE_Y + face_smile.height
  ) {
    // Since the face is being explicitly pressed, reset the mouse state before resetting
    // In all other cases, resetting shouldn't work while the left/middle mouse are being held
    mouse_buttons.left = 0;
    mouse_buttons.middle = 0;
    mouse_buttons.right = 0;
    reset_game(width, height, total_mines);
  }

  // Clicks within the playfield
  else if (
    !right_click &&
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

  multiselect = 0;

  repaint();
}

// Handle mousemove event
void handle_mousemove() {
  if (!mouse_buttons.left && !multiselect) {
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

    if (multiselect || (tiles[i] == TILE_UNCLICKED || tiles[i] == TILE_MAYBE)) {
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
    case SDLK_EQUALS:
      rescale_window(1);
      break;
    case SDLK_ESCAPE:
      show_popup("Timer stopped");
      timer_stopped = 1;
      break;
    case SDLK_MINUS:
      rescale_window(-1);
      break;
    case SDLK_F2:
    case SDLK_r:
      reset_game(width, height, total_mines);
      break;
    case SDLK_b: {
      if (!started) {
        show_popup("Difficulty: beginner");
        reset_game(beginner.width, beginner.height, beginner.total_mines);
      }
      break;
    }
    case SDLK_i: {
      if (!started) {
        show_popup("Difficulty: intermediate");
        reset_game(intermediate.width, intermediate.height, intermediate.total_mines);
      }
      break;
    }
    case SDLK_e: {
      if (!started) {
        show_popup("Difficulty: expert");
        reset_game(expert.width, expert.height, expert.total_mines);
      }
      break;
    }
    case SDLK_SLASH: {
      maybe_enabled = !maybe_enabled;
      if (maybe_enabled) {
        show_popup("(?) marks enabled");
      } else {
        show_popup("(?) marks disabled");
      }
      break;
    }
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
  Image face_images[] = {
    face_smile,
    face_click,
    face_open,
    face_dead,
    face_win
  };

  Image tile_images[] = {
    tile_empty,
    tile_1,
    tile_2,
    tile_3,
    tile_4,
    tile_5,
    tile_6,
    tile_7,
    tile_8,
    tile_unclicked,
    tile_flag,
    tile_maybe,
    tile_maybepress,
    tile_mine,
    tile_redmine,
    tile_wrong
  };

  Image timer_num_images[] = {
    timer_0,
    timer_1,
    timer_2,
    timer_3,
    timer_4,
    timer_5,
    timer_6,
    timer_7,
    timer_8,
    timer_9,
    timer_minus
  };

  // Load face textures
  for (int i = 0; i < 5; i++) {
    face_tex[i] = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_RGB24,
      SDL_TEXTUREACCESS_STATIC,
      face_images[i].width,
      face_images[i].height
    );
    SDL_UpdateTexture(face_tex[i], NULL, face_images[i].pixels, face_images[i].width * 3);
  }

  // Load tile textures
  for (int i = 0; i < 16; i++) {
    tile_tex[i] = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_RGB24,
      SDL_TEXTUREACCESS_STATIC,
      tile_images[i].width,
      tile_images[i].height
    );
    SDL_UpdateTexture(tile_tex[i], NULL, tile_images[i].pixels, tile_images[i].width * 3);
  }

  for (int i = 0; i < 11; i++) {
    timer_tex[i] = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_RGB24,
      SDL_TEXTUREACCESS_STATIC,
      timer_num_images[i].width,
      timer_num_images[i].height
    );
    SDL_UpdateTexture(timer_tex[i], NULL, timer_num_images[i].pixels, timer_num_images[i].width * 3);
  }

  // Load game frame textures
  game_corner_tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB24,
    SDL_TEXTUREACCESS_STATIC,
    game_corner.width,
    game_corner.height
  );
  SDL_UpdateTexture(game_corner_tex, NULL, game_corner.pixels, game_corner.width * 3);
  
  top_corner_tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB24,
    SDL_TEXTUREACCESS_STATIC,
    top_corner.width,
    top_corner.height
  );
  SDL_UpdateTexture(top_corner_tex, NULL, top_corner.pixels, top_corner.width * 3);

  // Load font texture
  font_tex = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA5551,
    SDL_TEXTUREACCESS_STATIC,
    font.width,
    font.height
  );
  SDL_UpdateTexture(font_tex, NULL, font.pixels, font.width * 2);
  SDL_SetTextureBlendMode(font_tex, SDL_BLENDMODE_BLEND);
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

  srand(time(NULL));

  window = SDL_CreateWindow(
    "Sweepy",
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

  reset_game(beginner.width, beginner.height, beginner.total_mines);

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
        case SDL_USEREVENT:
          close_popup();
          break;
        default:
          break;
      }
    }

    // Update the timer if the game is running and the timer is not stopped
    if (started && !timer_stopped && !dead && !win) {
      struct timespec current_time;
      clock_gettime(CLOCK_MONOTONIC, &current_time);

      time_t diff = current_time.tv_sec - start_time.tv_sec;
      long nsec_diff = current_time.tv_nsec - start_time.tv_nsec;
      if (nsec_diff < 0) {
        diff--;
      }

      if (diff > timer) {
        timer = (unsigned)diff;
        repaint();
      }
    }

    // Throttle at least a tiny bit so it doesn't use 100% CPU
    // This seems to run as well as, or better than, using SDL_WaitEvent,
    // and it allows for accurate timer updates
    SDL_Delay(1);
  }

  cleanup();

  return 0;
}
