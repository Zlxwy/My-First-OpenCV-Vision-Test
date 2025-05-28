// ����һ�����Ծ���ˢ����ʾ�ĳ��򣬾���仯�᲻��ˢ���ն˵���ʾ��������һ���Ե�˳����ʾ����
#include <iostream>
#include <windows.h> // ������ʱ����������Windowns����ϵͳ

/*������ƶ����ն���ʼλ��*/
void clearScreen(void) {
    COORD cursorPosition = { 0, 0 }; // �����ն˹���λ��
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y; // ��ȡ��Ļ�������ĳߴ�
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, cursorPosition, &dwConSize); // �����Ļ�ϵ��ַ�
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, cursorPosition, &dwConSize); // �����Ļ�ϵ�����
    SetConsoleCursorPosition(hConsole, cursorPosition); // ��������õ���ʼλ��
    // system("cls"); // ֱ�ӵ���Windows���ն����� cls ����ն�
}

int test_mat_refresh_disp(void) {
    const int rows = 10;
    const int cols = 10;
    int matrix[rows][cols] = { 0 };
    // ��ʼ������
    for (int i = 0; i < rows; ++i) for (int j = 0; j < cols; ++j) matrix[i][j] = i * cols + j;

    while (true) {
        clearScreen(); // �����Ļ���ƶ���굽����
        /*��ʾ����*/
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
        /*���¾���*/
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = (matrix[i][j] == 255) ? (0) : (matrix[i][j] + 1);
            }
        }
        Sleep(36);
    }
    return 0;
}
