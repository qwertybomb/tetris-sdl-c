#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <SDL.h>
#include <SDL_ttf.h>

const int width = 250;
const int height = 500;

/*the grid size*/
#define rows (10)
#define cols (20)

/*size of each block in grid*/
const int scale = height / cols;


typedef enum {
    nothing,
    remove_me = 8,
    wall = 9,
    piece1 = '1',
    piece2 = '2',
    piece3 = '3',
    piece4 = '4',
    piece5 = '5',
    piece6 = '6',
    piece7 = '7',
} block;

const char *tetromino[7] = {
        "..1."
        "..1."
        "..1."
        "..1.",

        "..2."
        ".22."
        ".2.."
        "....",

        ".3.."
        ".33."
        "..3."
        "....",

        "...."
        ".44."
        ".44."
        "....",

        "..5."
        ".55."
        "..5."
        "....",

        "...."
        ".66."
        "..6."
        "..6.",

        "...."
        ".77."
        ".7.."
        ".7.."
};


int rotated_index(int px, int py, int r) {
    switch (r % 4) {
        case 0:
            return py * 4 + px;
        case 1:
            return 12 + py - (px * 4);
        case 2:
            return 15 - (py * 4) - px;
        case 3:
            return 3 - py + (px * 4);
    }
    return 0;
}

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h) {
    SDL_Rect rect = (SDL_Rect) {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}


void draw_rect_no_fill(SDL_Renderer *renderer, int x, int y, int w, int h) {
    SDL_Rect rect = (SDL_Rect) {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}


typedef struct tetromino_piece {
    int index;
    unsigned int rotation: 3;
    int pos_x, pos_y;
    bool jump;
} tetromino_piece;


bool does_piece_fit(tetromino_piece piece, unsigned char grid[]) {

    for (int px = 0; px < 4; ++px)
        for (int py = 0; py < 4; ++py) {
            int pi = rotated_index(px, py, piece.rotation);
            int fi = (piece.pos_y + py) * rows + (piece.pos_x + px);

            if (piece.pos_x + px >= 0 && piece.pos_x + px < rows) {
                if (piece.pos_y + py >= 0 && piece.pos_y + py < cols + 4) {
                    if (tetromino[piece.index - 1][pi] != '.' && grid[fi] != nothing)
                        return false;
                }
            }
        }
    return true;
}

void move_piece(tetromino_piece *piece, int mode, unsigned char grid[]) {
    switch (mode) {
        case 0: {
            piece->pos_x -= does_piece_fit(
                    (tetromino_piece) {piece->index, piece->rotation, piece->pos_x - 1,
                                       piece->pos_y,false}, grid);
            break;
        }
        case 1: {
            piece->pos_x += does_piece_fit(
                    (tetromino_piece) {piece->index, piece->rotation, piece->pos_x + 1,
                                       piece->pos_y,false}, grid);
            break;
        }
        case 2: {
            piece->pos_y += does_piece_fit(
                    (tetromino_piece) {piece->index, piece->rotation, piece->pos_x,
                                       piece->pos_y + 1,false}, grid);
            break;
        }
        case 3: {
            piece->rotation += does_piece_fit(
                    (tetromino_piece) {piece->index, piece->rotation + 1, piece->pos_x,
                                       piece->pos_y,false}, grid);
            break;
        }
        case 4: {

            while (does_piece_fit((tetromino_piece) {piece->index, piece->rotation, piece->pos_x,
                                                     piece->pos_y+1,false},grid)) {
                piece->pos_y++;
            }
            piece->jump = true;
        }
        default:
            break;
    }
}

void draw_piece_drop(SDL_Renderer* renderer, tetromino_piece piece, unsigned char grid[]) {

    while (does_piece_fit((tetromino_piece) {piece.index, piece.rotation, piece.pos_x,
                              piece.pos_y+1},grid)) {
        piece.pos_y++;
    }
    for(int px = 0; px < 4; ++px) {
        for (int py = 0; py < 4; ++py) {
            int x = (piece.pos_x + px) * scale;
            int y = (piece.pos_y + py-4) * scale;

            if(tetromino[piece.index-1][rotated_index(px,py,piece.rotation)] != '.') {
                 SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
                switch (piece.index+'0') {
                    case piece1:
                        SDL_SetRenderDrawColor(renderer, 210, 180, 140, 200);
                        break;
                    case piece2:
                        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);
                        break;
                    case piece3:
                        SDL_SetRenderDrawColor(renderer, 128, 0, 128, 200);
                        break;
                    case piece4:
                        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 200);
                        break;
                    case piece5:
                        SDL_SetRenderDrawColor(renderer, 128, 0, 0, 200);
                        break;
                    case piece6:
                        SDL_SetRenderDrawColor(renderer, 0, 0, 128, 200);
                        break;
                    case piece7:
                        SDL_SetRenderDrawColor(renderer, 255, 102, 0, 200);
                        break;
                }
                 draw_rect(renderer,x,y,scale,scale);
                 Uint8 r,g,b,a;
                 SDL_GetRenderDrawColor(renderer,&r,&g,&b,&a);
                 SDL_SetRenderDrawColor(renderer,r,g,b,255);
                 draw_rect_no_fill(renderer,x,y,scale,scale);
            }
        }
    }
}


void reset_piece(tetromino_piece *piece,unsigned char grid[]) {
    *piece = (tetromino_piece) {rand() % 7 + 1, 0, rows / 2 - 2, 0,false};
}






void reset_grid(unsigned char grid[]) {
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols + 4; ++j)
            grid[j * rows + i] = (i == 0 || i == rows - 1 || j == cols + 3) ? wall : nothing;
}


void draw_grid(SDL_Renderer *renderer, unsigned char grid[]) {

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
            int x = i * scale;
            int y = j * scale;
            switch (grid[j * rows + i + 40]) {
                default:
                case nothing:
                    continue;
                    break;
                case remove_me:
                    SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
                    break;
                case wall:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    break;
                case piece1:
                    SDL_SetRenderDrawColor(renderer, 210, 180, 140, 255);
                    break;
                case piece2:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    break;
                case piece3:
                    SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
                    break;
                case piece4:
                    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
                    break;
                case piece5:
                    SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
                    break;
                case piece6:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
                    break;
                case piece7:
                    SDL_SetRenderDrawColor(renderer, 255, 102, 0, 255);
                    break;
            }
            draw_rect(renderer, x, y, scale, scale);
        }
    }
}


int get_key_pressed(SDL_Event event) {
    switch (event.key.keysym.sym) {
        case SDLK_LEFT:
            return 0;
        case SDLK_RIGHT:
            return 1;
        case SDLK_DOWN:
            return 2;
        case 'r':
            return 3;
        case SDLK_RETURN:
            return 4;
        default:
            return -1;
    }
}


bool held = false;

/*handle key press*/
void handle_key_press(tetromino_piece *current_piece, int *key_pressed, unsigned char grid[]) {
    /*when key is pressed and not held. it can rotate*/
    if (*key_pressed == 3) {
        if(!held) move_piece(current_piece, 3, grid);
        held = true;
    } else {
        move_piece(current_piece, *key_pressed, grid);
    }
    *key_pressed = -1;
}

/*for setting up text*/
void get_text_and_rect(SDL_Renderer *renderer, int x, int y, char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect) {
    int text_width;
    int text_height;
    SDL_Surface *surface;
    SDL_Color textColor = {255, 255, 255, 255};

    surface = TTF_RenderText_Blended(font, text, textColor);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    text_width = surface->w;
    text_height = surface->h;
    SDL_FreeSurface(surface);
    rect->x = x;
    rect->y = y;
    rect->w = text_width;
    rect->h = text_height;
}


size_t get_points(size_t lines_cleared) {
    switch (lines_cleared) {
        case 0:
            return 0;
        case 1:
            return 50;
        case 2:
            return 150;
        case 3:
            return 350;
        default:
            return 1000;
    }
    return 0;
}

void update_score_text(size_t score, char *text) {
    sprintf(text, "score: %zu", score);
}




int main() {
    srand(time(0));

    bool quit = false;
    /*--------------sdl-------------*/
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Event event;
    SDL_Window *window = SDL_CreateWindow("Tetris!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          width, height, SDL_RENDERER_ACCELERATED);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);


    unsigned char grid[rows * cols + 40];
    reset_grid(grid);
    tetromino_piece current_piece = (tetromino_piece) {(rand()/(double)RAND_MAX) * 7 + 1, 0, rows / 2 - 2, 1};
    unsigned char display_grid[rows * cols + 40];
    int key_pressed = -1;
    int frame_count_mod = 0;


    int lines[cols];
    int lines_length = 0;
    //clear lines




    TTF_Font *score_font = TTF_OpenFont("../Helvetica.ttf", scale);
    SDL_Texture *score_text_texture;
    SDL_Rect score_text_rect;
    char score_text[130];
    size_t score = 0;



    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    key_pressed = get_key_pressed(event);
                    break;
                case SDL_KEYUP:
                    held = false;
                    break;
                default:
                    break;
            }
        }


        if(frame_count_mod % 35 == 0){
            static int force_down_counter = 0;
           static bool score_changed = true;
            handle_key_press(&current_piece, &key_pressed, grid);
            /*force current piece down*/
            if (force_down_counter % 240 == 0 || current_piece.jump ) {
                if (does_piece_fit((tetromino_piece) {current_piece.index, current_piece.rotation, current_piece.pos_x,
                                                      current_piece.pos_y + 1}, grid))
                    current_piece.pos_y++;
                else {
                    /*copy piece to grid*/
                    for (int px = 0; px < 4; ++px)
                        for (int py = 0; py < 4; ++py)
                            if (tetromino[current_piece.index - 1][rotated_index(px, py, current_piece.rotation)] !=
                                '.')
                                grid[(current_piece.pos_y + py) * rows + current_piece.pos_x + px] =
                                        current_piece.index + '0';
                    /*check to clear line*/
                    for (int py = 0; py < 4; ++py) {
                        if (py < cols + 3) {
                            bool line = true;
                            for (int x = 1; x < rows - 1; ++x)
                                line &= grid[(current_piece.pos_y + py) * rows + x] > wall;

                            if (line)
                                score_changed = true, lines[lines_length++] = current_piece.pos_y + py;
                        }
                    }
                    tetromino_piece temp = current_piece;
                    reset_piece(&current_piece,grid);

                    /*clear lines*/
                    for (int i = 0; i < lines_length; ++i)
                        for (int px = 1; px < rows - 1; px++) {
                            for (int py = lines[i]; py > 0; py--)
                                grid[py * rows + px] = grid[(py - 1) * rows + px];
                            grid[px] = 0;
                        }
                    /*game over*/
                    if (!does_piece_fit(
                            (tetromino_piece) {current_piece.index, current_piece.rotation, current_piece.pos_x,
                                               current_piece.pos_y + 1}, grid)||temp.pos_y<4) {
                        reset_piece(&current_piece,grid);
                        reset_grid(grid);
                        lines_length = 0;
                        score = 0;
                        score_changed = true;
                    }

                }
            }

            if (score_changed) {
                /*get score text texture*/
                score += get_points(lines_length);
                update_score_text(score, score_text);
                get_text_and_rect(renderer, scale*1.25, scale * 1.5, score_text, score_font, &score_text_texture,
                                  &score_text_rect);
            }




            //reset length
            lines_length = 0;
            force_down_counter++;
            score_changed = false;
        }


        /*drawing*/

        /*draw grid*/
        memcpy(display_grid, grid, sizeof(grid));

        /*draw current piece*/
        for (int px = 0; px < 4; ++px)
            for (int py = 0; py < 4; ++py)
                if (tetromino[current_piece.index - 1][rotated_index(px, py, current_piece.rotation)] != '.' &&
                    (current_piece.pos_y + py) * rows + current_piece.pos_x + px >= 0)
                    display_grid[(current_piece.pos_y + py) * rows + current_piece.pos_x + px] =
                            current_piece.index + '0';


        /*clear screen black*/
        SDL_SetRenderDrawColor(renderer, 0, 0,0, 255);
        SDL_RenderClear(renderer);

        draw_piece_drop(renderer,current_piece,grid);
        draw_grid(renderer, display_grid);


        /*draw text*/
        SDL_RenderCopy(renderer, score_text_texture, 0, &score_text_rect);


        /*update screen with renderer*/
        SDL_RenderPresent(renderer);

        frame_count_mod++;
    }
    TTF_Quit();

    /*free memory*/
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
