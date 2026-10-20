#include "game.h"

/*
 * 检查刚在 (row, col) 落下的这颗子有没有形成五连。
 * 形成了返回 1，没有返回 0。
 * static 表示这个函数只在 game.c 内部使用，不对外提供。
 */
static int check_win(const Game *game, int row, int col)
{
    char player = game->cells[row][col];

    int dirs[4][2] = {
        {0, 1}, // 右
        {1, 0}, // 下
        {1, 1}, // 右下
        {1, -1} // 左下
    };

    for (int d = 0; d < 4; d++)
    {
        int dr = dirs[d][0];
        int dc = dirs[d][1];
        int count = 1;   /* 先把刚落下的这颗子算上 */
        int r, c;

        /* 沿着 (dr, dc) 方向数同色棋子 */
        r = row + dr;
        c = col + dc;
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE
               && game->cells[r][c] == player)
        {
            count++;
            r = r + dr;
            c = c + dc;
        }

        /* 沿着反方向数 */
        r = row - dr;
        c = col - dc;
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE
               && game->cells[r][c] == player)
        {
            count++;
            r = r - dr;
            c = c - dc;
        }

        if (count >= WIN_LENGTH)
        {
            return 1;
        }
    }

    return 0;
}

void game_init(Game *game)
{
    int r, c;

    for (r = 0; r < BOARD_SIZE; r++)
    {
        for (c = 0; c < BOARD_SIZE; c++)
        {
            game->cells[r][c] = EMPTY_CELL;
        }
    }

    game->current_player = 'X';   /* X 先手 */
    game->move_count = 0;
    game->status = GAME_RUNNING;
}

MoveResult game_place(Game *game, int row, int col)
{
    /* 逻辑层自己再检查一次边界，不完全信任外面传来的坐标 */
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    {
        return MOVE_OUT_OF_BOUNDS;
    }

    if (game->cells[row][col] != EMPTY_CELL)
    {
        return MOVE_OCCUPIED;
    }

    /* 落子 */
    game->cells[row][col] = game->current_player;
    game->move_count++;

    /*
     * 胜负判定在切换玩家之前做：
     * 现在要判断的是"刚刚落子的这个人"赢没赢。
     */
    if (check_win(game, row, col))
    {
        if (game->current_player == 'X')
        {
            game->status = GAME_X_WINS;
        }
        else
        {
            game->status = GAME_O_WINS;
        }
    }
    else if (game->move_count == BOARD_SIZE * BOARD_SIZE)
    {
        /* 81 个格子全满又没人赢，平局 */
        game->status = GAME_DRAW;
    }
    else
    {
        /* 既没赢也没平局，换人 */
        if (game->current_player == 'X')
        {
            game->current_player = 'O';
        }
        else
        {
            game->current_player = 'X';
        }
    }

    return MOVE_ACCEPTED;
}
