#ifndef CONSOLE_H
#define CONSOLE_H

#include <windows.h>
#include "game.h"

/* 控制台相关的东西：输入/输出句柄、以及被我们修改前的原始设置（退出时恢复用） */
typedef struct {
    HANDLE hInput;
    HANDLE hOutput;
    DWORD original_input_mode;
    CONSOLE_CURSOR_INFO original_cursor;
} GameConsole;

/* 初始化控制台：打开鼠标输入、隐藏光标、清屏 */
void console_init(GameConsole *con);

/* 退出前把控制台恢复成程序启动前的样子 */
void console_restore(GameConsole *con);

/* 把整块棋盘打印出来（开局时用一次） */
void console_draw_board(const Game *game);

/* 只刷新一个格子（落子后用，不用整屏重画，不会闪） */
void console_update_cell(GameConsole *con, const Game *game, int row, int col);

/* 刷新棋盘下面的状态行；extra_msg 是附加提示，没有就传 NULL */
void console_draw_status(GameConsole *con, const Game *game, const char *extra_msg);

/* 等待一次鼠标左键点击：落在棋盘内返回 1，并把格子坐标放进 *row *col；
   落在棋盘外返回 0 */
int console_wait_click(GameConsole *con, int *row, int *col);

/* 等待一次键盘按键（游戏结束后暂停画面用） */
void console_wait_key(GameConsole *con);

#endif
