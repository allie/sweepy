#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "images.h"

// Useful macros for drawing the interface
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

#define SCALE 2

#define L_WIDTH (FIELD_WIDTH + PADDING_LEFT + PADDING_RIGHT)
#define L_HEIGHT (FIELD_HEIGHT + PADDING_BOTTOM + TOP_HEIGHT + PADDING_TOP * 2)

#define FACE_SIZE 26
#define FACE_X (ceil((L_WIDTH - 3) / 2) - (FACE_SIZE / 2) + 3)
#define FACE_Y (PADDING_TOP + 6)

// Render states for a tile
enum {
  TILE_UNCLICKED = 0,
  // Numbers 1-8 are implied
  TILE_EMPTY = 9,
  TILE_FLAG = 10,
  TILE_MAYBE = 11,
  TILE_MINE = 12,
  TILE_REDMINE = 13,
  TILE_WRONG = 14
};

// Face button states
enum {
  FACE_SMILE,
  FACE_CLICK,
  FACE_OPEN,
  FACE_DEAD
};

SDL_Window* window;
SDL_Renderer* renderer;

// Textures
SDL_Texture* face_click_tex;
SDL_Texture* face_dead_tex;
SDL_Texture* face_smile_tex;
SDL_Texture* face_open_tex;
SDL_Texture* game_corner_tex;
SDL_Texture* top_corner_tex;
SDL_Texture* tile_empty_tex;
SDL_Texture* tile_flag_tex;
SDL_Texture* tile_maybe_tex;
SDL_Texture* tile_mine_tex;
SDL_Texture* tile_redmine_tex;
SDL_Texture* tile_unclicked_tex;
SDL_Texture* tile_wrong_tex;
SDL_Texture* tile_num_tex[8];
SDL_Texture* timer_num_tex[10];

// Game state variables
unsigned width = 9;
unsigned height = 9;
unsigned mines_left = 10;
unsigned timer = 0;
unsigned face = FACE_SMILE;

unsigned char* tiles = NULL;
unsigned char* field = NULL;

int mouse_down = 0;
int face_pressed = 0;
struct {
  int x;
  int y;
} selected_tile;

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
      SDL_RenderCopy(renderer, tile_num_tex[state], &src, &dst);
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
    draw_tile(TILE_EMPTY, selected_tile.x, selected_tile.y);
  }

  draw_face();

  SDL_RenderPresent(renderer);
}

// Start a new game
void reset_game() {
  width = 9;
  height = 9;
  mines_left = 10;
  timer = 0;
  face = FACE_SMILE;
  selected_tile.x = -1;
  selected_tile.y = -1;
  face_pressed = 0;

  if (tiles != NULL) {
    free(tiles);
  }

  tiles = (unsigned char*)calloc(width * height, sizeof(unsigned char));

  SDL_SetWindowSize(window, L_WIDTH * SCALE, L_HEIGHT * SCALE);
  SDL_RenderSetLogicalSize(renderer, L_WIDTH, L_HEIGHT);

  repaint();
}

// Reveal a tile after mouseup while hovering over it
void handle_tile_click(unsigned x, unsigned y) {
  tiles[y * width + x] = TILE_MINE;
}

// Handle mousedown event
void handle_mousedown() {
  mouse_down = 1;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= SCALE;
  y /= SCALE;

  if (x >= FACE_X && x < FACE_X + face_smile.width && y >= FACE_Y && y < FACE_Y + face_smile.height) {
    face_pressed = 1;
    face = FACE_CLICK;
  } else {
    face = FACE_OPEN;
  }

  if (x >= FIELD_X && x < FIELD_X + FIELD_WIDTH && y >= FIELD_Y && y < FIELD_Y + FIELD_HEIGHT) {
    int tx = (x - FIELD_X) / TILE_SIZE;
    int ty = (y - FIELD_Y) / TILE_SIZE;

    if (tiles[ty * width + tx] == TILE_UNCLICKED) {
      selected_tile.x = tx;
      selected_tile.y = ty;
    }
  }

  repaint();
}

// Handle mouseup event
void handle_mouseup() {
  mouse_down = 0;
  selected_tile.x = -1;
  selected_tile.y = -1;

  // TODO: check if dead
  face = FACE_SMILE;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x /= SCALE;
  y /= SCALE;

  if (face_pressed && x >= FACE_X && x < FACE_X + face_smile.width && y >= FACE_Y && y < FACE_Y + face_smile.height) {
    reset_game();
  }

  else if (x >= FIELD_X && x < FIELD_X + FIELD_WIDTH && y >= FIELD_Y && y < FIELD_Y + FIELD_HEIGHT) {
    int tx = (x - FIELD_X) / TILE_SIZE;
    int ty = (y - FIELD_Y) / TILE_SIZE;
    handle_tile_click(tx, ty);
  }

  face_pressed = 0;
  
  repaint();
}

// Handle mousemove event
void handle_mousemove() {

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

  tile_mine_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_mine.width, tile_mine.height);
  SDL_UpdateTexture(tile_mine_tex, NULL, tile_mine.pixels, tile_mine.width * 3);

  tile_redmine_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_redmine.width, tile_redmine.height);
  SDL_UpdateTexture(tile_redmine_tex, NULL, tile_redmine.pixels, tile_redmine.width * 3);

  tile_unclicked_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_unclicked.width, tile_unclicked.height);
  SDL_UpdateTexture(tile_unclicked_tex, NULL, tile_unclicked.pixels, tile_unclicked.width * 3);

  tile_wrong_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_wrong.width, tile_wrong.height);
  SDL_UpdateTexture(tile_wrong_tex, NULL, tile_wrong.pixels, tile_wrong.width * 3);

  for (int i = 0; i < 8; i++) {
    tile_num_tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, tile_num[i].width, tile_num[i].height);
    SDL_UpdateTexture(tile_num_tex[i], NULL, tile_num[i].pixels, tile_num[i].width * 3);
  }

  for (int i = 0; i < 10; i++) {
    timer_num_tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, timer_num[i].width, timer_num[i].height);
    SDL_UpdateTexture(timer_num_tex[i], NULL, timer_num[i].pixels, timer_num[i].width * 3);
  }
}

int main() {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return -1;
	}

  window = SDL_CreateWindow(
		"mines",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		L_WIDTH * SCALE,
		L_HEIGHT * SCALE,
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
          handle_mousedown();
          break;
        case SDL_MOUSEBUTTONUP:
          handle_mouseup();
          break;
			}
		}
    // Throttle at least a tiny bit so it doesn't use 100% CPU
    SDL_Delay(1);
	}

  return 0;
}
