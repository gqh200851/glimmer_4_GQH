#ifndef GAME_H
#define GAME_H

/* 棋盘边长：9 行 9 列 */
#define BOARD_SIZE 9

/* 连续多少个子算赢 */
#define WIN_LENGTH 5

/* 空格子的标记 */
#define EMPTY_CELL '.'

/* 游戏当前处于什么状态 */
typedef enum {
    GAME_RUNNING,   /* 还在下 */
    GAME_X_WINS,    /* X 赢了 */
    GAME_O_WINS,    /* O 赢了 */
    GAME_DRAW       /* 平局 */
} GameStatus;

/* 一次落子的结果 */
typedef enum {
    MOVE_ACCEPTED,       /* 落子成功 */
    MOVE_OUT_OF_BOUNDS,  /* 点到棋盘外面了 */
    MOVE_OCCUPIED        /* 这个格子已经有棋子 */
} MoveResult;

/* 一盘游戏的全部数据 */
typedef struct {
    char cells[BOARD_SIZE][BOARD_SIZE]; /* 棋盘，cells[行][列] */
    char current_player;                /* 现在轮到谁落子 */
    int move_count;                     /* 已经落了多少个子 */
    GameStatus status;                  /* 游戏状态 */
} Game;

/* 把一盘新游戏初始化好 */
void game_init(Game *game);

/* 在 (row, col) 落子，并更新游戏数据和状态，返回这次落子的结果 */
MoveResult game_place(Game *game, int row, int col);

#endif
