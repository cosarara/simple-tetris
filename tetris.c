#include <SDL.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

long now() {
	return SDL_GetTicks();
}


#define ROWS 30
#define COLS 10

/*
 * AAA   A     A    AA
 *  A    AAA   A   AA
 *             A
 * AA      A   A   AA
 * AA    AAA        AA
 */

const int shape_bar[4][4] = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{1, 1, 1, 1},
		{0, 0, 0, 0}
};

const int shape_square[4][4] = {
		{0, 0, 0, 0},
		{0, 1, 1, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0}
};

const int shape_j[4][4] = {
		{0, 0, 0, 0},
		{1, 0, 0, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0}
};

const int shape_l[4][4] = {
		{0, 0, 1, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
};

const int shape_s[4][4] = {
		{0, 1, 1, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
};

const int shape_z[4][4] = {
		{1, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
};

const int shape_t[4][4] = {
		{0, 1, 0, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
};

const int (*shapes[7])[4][4] = {
	&shape_bar,
	&shape_square,
	&shape_l,
	&shape_j,
	&shape_s,
	&shape_z,
	&shape_t,
};

typedef struct {
	int shape[4][4];
	int x;
	int y;
	int id;
} Piece;

typedef struct {
	int right;
	int left;
	int rot;
	int rotb;
	int drop;
	int fast;
} KeyTable;

void draw(SDL_Renderer* renderer, int map[ROWS][COLS], Piece piece) {
	// bg
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	// frame
	SDL_Rect rect_frame = {0, 0, COLS*14, ROWS*14};
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderDrawRect(renderer, &rect_frame);

	// map
	SDL_Rect rect_square = {0, 0, 12, 12};
	for (int row=0; row < ROWS; row++) {
		for (int col=0; col < COLS; col++) {
			int color = map[row][col];
			rect_square.y = row*14 + 1;
			rect_square.x = col*14 + 1;
			if (color != 0) {
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_RenderDrawRect(renderer, &rect_square);
			}
		}
	}
	// piece TODO: abstract
	for (int row=0; row < 4; row++) {
		for (int col=0; col < 4; col++) {
			int color = piece.shape[row][col];
			rect_square.y = (row + piece.y)*14 + 1;
			rect_square.x = (col + piece.x)*14 + 1;
			if (color != 0) {
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_RenderDrawRect(renderer, &rect_square);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

int rotate(Piece* piece) {
	Piece tmp;
	memcpy(&tmp, piece, sizeof(tmp));
	int n = 3;
	if (piece->id == 0) n = 4; // line
	for (int row=0; row < n; row++) {
		for (int col=0; col < n; col++) {
			piece->shape[row][col] = tmp.shape[n - col - 1][row];
		}
	}
	return 0;
}

int colliding (const int (*map)[ROWS][COLS], const Piece* piece) {
	for (int row=0; row < 4; row++) {
		for (int col=0; col < 4; col++) {
			int mapx = piece->x + col;
			int mapy = piece->y + row;
			if (mapy < 0 ||
				(piece->shape[row][col] &&
				 ((*map)[mapy][mapx] ||
				   mapx < 0 || mapy >= ROWS || mapx >= COLS))) {
				printf("collision\n");
				return 1;
			}
		}
	}
	return 0;
}

int place_on_map(int (*map)[ROWS][COLS], Piece* piece) {
	for (int row=0; row < 4; row++) {
		for (int col=0; col < 4; col++) {
			int mapx = piece->x + col;
			int mapy = piece->y + row;
			int tile = piece->shape[row][col];
			if (tile) {
				(*map)[mapy][mapx] = tile;
			}
		}
	}
	return 0;
}

void get_piece(Piece* piece) {
	piece->id = random() % 7;
	memcpy(piece->shape, shapes[piece->id], sizeof(piece->shape));
}

void drop_line(int (*map)[ROWS][COLS], int n) {
	for (int row=n; row > 0; row--) {
		if (row == ROWS) continue;
		memcpy(&(*map)[row], &(*map)[row-1], sizeof((*map)[row]));
	}
}

int clear_lines(int (*map)[ROWS][COLS]) {
	int n_lines = 0;
	for (int row=0; row < ROWS; row++) {
		int good = 1;
		for (int col=0; col < COLS; col++) {
			if (!(*map)[row][col]) {
				good = 0;
			}
		}
		if (good) {
			drop_line(map, row);
			n_lines++;
		}
	}
	return n_lines;
}

int step(int (*map)[ROWS][COLS], Piece *piece, int *cleared_lines) {
	piece->y += 1;
	if (!colliding(map, piece)) {
		return 0;
	}
	piece->y -= 1;
	place_on_map(map, piece);
	*cleared_lines = clear_lines(map);

	// new piece
	piece->x = (COLS/2-2);
	piece->y = 0;
	get_piece(piece);

	if (colliding(map, piece)) {
		return 2;
	}
	return 1;
}

int handle_input(KeyTable* key) {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			return 1;
		}
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
				case SDLK_RIGHT:
					key->right = 1;
					break;
				case SDLK_LEFT:
					key->left = 1;
					break;
				case SDLK_UP:
					key->rot = 1;
					break;
				case SDLK_SPACE:
					key->drop = 1;
					break;
				case SDLK_DOWN:
					key->fast = 1;
					break;
			}
		}
		if (e.type == SDL_KEYUP) {
			switch (e.key.keysym.sym) {
				case SDLK_RIGHT:
					key->right = 0;
					break;
				case SDLK_LEFT:
					key->left = 0;
					break;
				case SDLK_UP:
					key->rot = 0;
					break;
				case SDLK_DOWN:
					key->fast = 0;
					break;
			}
		}
	}
	return 0;
}

int gameover() {
	puts("game over");
	return 0;
}

int main() {
	srandom(time(NULL));
	int step_time = 500; // fall down a tile every step_time
	int input_step_time = 1000/15; // apply input at 15 fps
	long next_step = now() + step_time;
	long next_input_step = now() + input_step_time;
	int score = 0;
	int cleared_lines;

	int map[ROWS][COLS] = {0};
	Piece piece = {{0}, (COLS/2-2), 0, 0};
	get_piece(&piece);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow(
			"Tetris",                  // window title
			SDL_WINDOWPOS_UNDEFINED,           // initial x position
			SDL_WINDOWPOS_UNDEFINED,           // initial y position
			640,                               // width, in pixels
			480,                               // height, in pixels
			SDL_WINDOW_OPENGL                  // flags - see below
			);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	KeyTable key_table = {0};

	while (1) {
		if (handle_input(&key_table) == 1) {
			break;
		}
		if (now() > next_input_step) {
			next_input_step = now() + input_step_time;
			if (key_table.right) {
				piece.x += 1;
				if (colliding(&map, &piece)) {
					piece.x -= 1;
				}
			}
			if (key_table.left) {
				piece.x -= 1;
				if (colliding(&map, &piece)) {
					piece.x += 1;
				}
			}
			if (key_table.rot) {
				Piece bak;
				memcpy(&bak, &piece, sizeof(bak));
				rotate(&piece);
				if (colliding(&map, &piece)) {
					memcpy(&piece, &bak, sizeof(piece));
				}
				key_table.rot = 0; // can't hold rot
			}
		}
		if (key_table.drop || now() > next_step -
				(key_table.fast ? (7 * step_time / 8) : 0)) {
			int step_r = step(&map, &piece, &cleared_lines);
			if (step_r) {
				if (step_time > 120) {
					step_time -= 20;
				}
				key_table.drop = 0;
			}
			if (step_r == 2) {
				gameover();
				break;
			}
			next_step = now() + step_time;
		}
		// score
		switch (cleared_lines) {
			case 1:
				score += 40;
				break;
			case 2:
				score += 100;
				break;
			case 3:
				score += 300;
				break;
			case 4:
				score += 1200;
				break;
		}
		// drawing
		draw(renderer, map, piece);
	}
//end:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
