// 这是一个测试矩阵刷新显示的程序，矩阵变化会不断刷新终端的显示，而不是一股脑地顺序显示下来
#include <iostream>
#include <windows.h> // 用于延时，仅适用于Windowns操作系统

/*将光标移动到终端起始位置*/
void clearScreen(void) {
    COORD cursorPosition = { 0, 0 }; // 定义终端光标的位置
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y; // 获取屏幕缓冲区的尺寸
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, cursorPosition, &dwConSize); // 清除屏幕上的字符
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, cursorPosition, &dwConSize); // 清除屏幕上的属性
    SetConsoleCursorPosition(hConsole, cursorPosition); // 将光标重置到起始位置
    // system("cls"); // 直接调用Windows的终端命令 cls 清空终端
}

int test_mat_refresh_disp(void) {
    const int rows = 10;
    const int cols = 10;
    int matrix[rows][cols] = { 0 };
    // 初始化矩阵
    for (int i = 0; i < rows; ++i) for (int j = 0; j < cols; ++j) matrix[i][j] = i * cols + j;

    while (true) {
        clearScreen(); // 清除屏幕并移动光标到行首
        /*显示矩阵*/
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
        /*更新矩阵*/
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = (matrix[i][j] == 255) ? (0) : (matrix[i][j] + 1);
            }
        }
        Sleep(36);
    }
    return 0;
}
