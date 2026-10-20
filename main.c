#include "game.h"
#include "console.h"

int main(void)
{
    Game game;
    GameConsole con;

    game_init(&game);   // 棋盘清空 · status=进行中
    console_init(&con); // 开鼠标 · 存底片 · 清屏

    /* 先把空棋盘和状态行画出来 */
    console_draw_board(&game);
    console_draw_status(&con, &game, NULL);

    while (game.status == GAME_RUNNING)
    {
        int row, col;
        MoveResult result;

        /* 点在棋盘外面就继续等，并给出提示 */
        while (console_wait_click(&con, &row, &col) == 0)
        {
            console_draw_status(&con, &game, "点在棋盘外面了，请点棋盘的格子");
        }

        result = game_place(&game, row, col);

        if (result == MOVE_OCCUPIED)
        {
            /* 格子有棋子：什么都没变，提示一下，回合还是当前玩家 */
            console_draw_status(&con, &game, "这个格子已经有棋子了");
            continue;
        }

        if (result == MOVE_ACCEPTED)
        {
            /* 只更新刚落下的这颗子，再刷新状态行 */
            console_update_cell(&con, &game, row, col);
            console_draw_status(&con, &game, NULL);
        }
    }

    /* 游戏结束：状态行已经是最终结果，等按键后恢复控制台再退出 */
    console_draw_status(&con, &game, "按任意键退出");
    console_wait_key(&con);
    console_restore(&con);

    return 0;
}
