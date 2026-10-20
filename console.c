#include <stdio.h>
#include "console.h"

/* 状态行在第几行（棋盘占 0~18 行，空一行，状态放第 20 行） */
#define STATUS_ROW (2 * BOARD_SIZE + 2)

/* 把光标移动到字符坐标 (x, y) */
static void goto_xy(GameConsole *con, int x, int y)
{
    COORD pos; // COORD 是 Windows 定义的结构体,只有两个 16 位 SHORT 字段：X（列）、Y（行）。
    pos.X = (SHORT)x;
    pos.Y = (SHORT)y; // 收窄转换:从宽类型往窄类型赋值
    SetConsoleCursorPosition(con->hOutput, pos); // 把光标移到屏幕 (X, Y) 处
}

/* ===== 诊断用：把事件信息写到 debug.log ===== */
static void log_event(const char *tag, DWORD event_type, const MOUSE_EVENT_RECORD *mouse)
{
    FILE *f = fopen("debug.log", "a");
    if (f == NULL) return;
    fprintf(f, "[%s] EventType=%lu", tag, (unsigned long)event_type);
    if (mouse != NULL)
    {
        fprintf(f, " Flags=0x%lX ButtonState=0x%lX Pos=(%ld,%ld)",
                (unsigned long)mouse->dwEventFlags,
                (unsigned long)mouse->dwButtonState,
                (long)mouse->dwMousePosition.X,
                (long)mouse->dwMousePosition.Y);
    }
    fprintf(f, "\n");
    fclose(f);
}

void console_init(GameConsole *con)
{
    /* 清空上一次的诊断日志 */
    {
        FILE *f = fopen("debug.log", "w");
        if (f != NULL) fclose(f);
    }

    // 1 拿句柄
    con->hInput = GetStdHandle(STD_INPUT_HANDLE);
    con->hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    // 2 存档底片
    /* 先把原来的设置(当前模式)读出来并存进结构体，留着退出时还原 */
    GetConsoleMode(con->hInput, &con->original_input_mode);    // 读标准输入的当前模式
    GetConsoleCursorInfo(con->hOutput, &con->original_cursor); // 读标准输出的当前光标信息

    /*
     * 先把控制台输出代码页切到 UTF-8（65001）。
     * 注意：必须在 SetConsoleMode 之前设置代码页，
     * 因为某些 Windows 版本上设置代码页会重置输入模式，
     * 导致后面开的鼠标输入失效。
     */
    SetConsoleOutputCP(65001);

    /*
     * 先清屏，再开鼠标模式。
     * system("cls") 会启动 cmd.exe 子进程，它会重置控制台输入模式、
     * 把刚开的 ENABLE_MOUSE_INPUT 关掉；所以必须放在 SetConsoleMode 之前。
     */
    system("cls");

    // 3 开鼠标模式 ★
    /*
     * 打开鼠标输入。
     * 故意不写 ENABLE_QUICK_EDIT_MODE（快速编辑模式）：
     * 开着它时，鼠标左键在终端里是"拖选文字"，程序根本收不到点击。
     * ENABLE_EXTENDED_FLAGS 是修改 QuickEdit 开关时必须搭配的标志(钥匙)。
     * 另外，"不写 = 关"，整组覆盖时，原来开着的位没写进 mode，就全被关掉。
     */
    // 把各开关拼成一个新模式。用"|"按位或，把三个标志的二进制位合进一个整数。
    DWORD mode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (!SetConsoleMode(con->hInput, mode))
    {
        DWORD err = GetLastError();
        log_event("SetConsoleMode FAILED", err, NULL);
    }
    else
    {
        DWORD verify;
        GetConsoleMode(con->hInput, &verify);
        log_event("SetConsoleMode OK, verify", verify, NULL);
    }

    // 4 藏光标
    /* 隐藏闪烁的光标，好看一点 */
    CONSOLE_CURSOR_INFO cursor = con->original_cursor; // 复制一份光标设置到局部变量cursor(在副本上改，底片保持原样)
    cursor.bVisible = FALSE; // 把副本的 "是否可见" 改成不可见（FALSE 就是 0）
    SetConsoleCursorInfo(con->hOutput, &cursor); // 用改好的副本更新控制台的输出
}

void console_restore(GameConsole *con)
{
    SetConsoleMode(con->hInput, con->original_input_mode);
    SetConsoleCursorInfo(con->hOutput, &con->original_cursor);
}

void console_draw_board(const Game *game)
{
    int row, col;

    for (row = 0; row < BOARD_SIZE; row++)
    {
        /* 格子上边线：+---+---+...+ */
        for (col = 0; col < BOARD_SIZE; col++)
        {
            printf("+---");
        }
        printf("+\n");

        /* 棋子行：| X | . |...| */
        for (col = 0; col < BOARD_SIZE; col++)
        {
            printf("| %c ", game->cells[row][col]);
        }
        printf("|\n");
    }

    /* 棋盘最底下的边线 */
    for (col = 0; col < BOARD_SIZE; col++)
    {
        printf("+---");
    }
    printf("+\n");
}

void console_update_cell(GameConsole *con, const Game *game, int row, int col)
{
    /*
     * 棋子在屏幕上的字符坐标：
     *   X = 4 * 列 + 2，Y = 2 * 行 + 1
     */
    goto_xy(con, 4 * col + 2, 2 * row + 1);
    printf("%c", game->cells[row][col]);
}

void console_draw_status(GameConsole *con, const Game *game, const char *extra_msg)
{
    int i;

    goto_xy(con, 0, STATUS_ROW);

    /* 先用空格把这行旧的内容盖掉，不然新内容比旧内容短时会有残留 */
    for (i = 0; i < 66; i++)
    {
        printf(" ");
    }
    goto_xy(con, 0, STATUS_ROW);

    if (game->status == GAME_RUNNING)
    {
        printf("当前回合：%c    已落子：%d 步", game->current_player, game->move_count);
    }
    else if (game->status == GAME_X_WINS)
    {
        printf("X 获胜！共 %d 步", game->move_count);
    }
    else if (game->status == GAME_O_WINS)
    {
        printf("O 获胜！共 %d 步", game->move_count);
    }
    else
    {
        printf("平局，棋盘已经下满，共 %d 步", game->move_count);
    }

    if (extra_msg != NULL)
    {
        printf("    %s", extra_msg);
    }
}

int console_wait_click(GameConsole *con, int *row, int *col)
{
    INPUT_RECORD ir; // 声明: 向系统要了一块名字叫 ir 的、能装下 INPUT_RECORD 的内存
    DWORD num_read;  // 声明：这次调用实际读到的事件条数

    while (1) // 阻塞等待：队列里没事件时，程序停在这一行，等事件来了才返回。
    {
        ReadConsoleInput(con->hInput, &ir, 1, &num_read);

        if (ir.EventType == MOUSE_EVENT)
        {
            MOUSE_EVENT_RECORD *mouse = &ir.Event.MouseEvent;
            log_event("MOUSE", ir.EventType, mouse);

            if (mouse->dwEventFlags == 0 &&
                (mouse->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
                // 普通点击(按下或松开) 且 按着的是左键 可以确定是 左键被按下
            {
                int x = mouse->dwMousePosition.X;
                int y = mouse->dwMousePosition.Y;

                /* 字符坐标转棋盘坐标 */
                int r = y / 2;
                int c = x / 4;

                if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE)
                {
                    log_event("MOUSE OUT OF BOUNDS", ir.EventType, mouse);
                    return 0;   /* 此时点在棋盘外面 */
                }

                *row = r;
                *col = c;
                log_event("MOUSE ACCEPTED", ir.EventType, mouse);
                return 1;
            }
        }
        else if (ir.EventType == MENU_EVENT)
        {
            FILE *f = fopen("debug.log", "a");
            if (f != NULL)
            {
                fprintf(f, "[MENU] dwCommandId=0x%lX (%lu)\n",
                        (unsigned long)ir.Event.MenuEvent.dwCommandId,
                        (unsigned long)ir.Event.MenuEvent.dwCommandId);
                fclose(f);
            }
        }
        else if (ir.EventType == KEY_EVENT)
        {
            FILE *f = fopen("debug.log", "a");
            if (f != NULL)
            {
                fprintf(f, "[KEY] bKeyDown=%d vk=0x%X ascii=%d('%c')\n",
                        ir.Event.KeyEvent.bKeyDown,
                        ir.Event.KeyEvent.wVirtualKeyCode,
                        (int)ir.Event.KeyEvent.uChar.AsciiChar,
                        ir.Event.KeyEvent.uChar.AsciiChar);
                fclose(f);
            }
        }
        else
        {
            log_event("OTHER", ir.EventType, NULL);
        }
    }
}

void console_wait_key(GameConsole *con)
{
    INPUT_RECORD ir;
    DWORD num_read;

    while (1)
    {
        ReadConsoleInput(con->hInput, &ir, 1, &num_read);

        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
        {
            return;
        }
    }
}
