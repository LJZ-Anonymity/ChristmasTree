#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// --- 平台依赖头文件和延时宏定义 ---
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
    // POSIX systems (Linux, macOS)
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000) // usleep takes microseconds
#endif

// --- ANSI 颜色代码定义 ---
#define RESET           "\033[0m"
#define BOLD            "\033[1m"

// 树元素颜色
#define GOLD            "\033[93m"          // 亮黄色/金色
#define GREEN_LEAF      "\033[32m"          // 树叶颜色 (绿色)
#define BROWN_TRUNK     "\033[38;5;94m"     // 树干颜色 (深棕色)

// --- 结构常量定义 ---
#define MAX_TREE_WIDTH  25  // 整体最大宽度，用于居中计算
#define LIGHT_PROB      5   // 1/LIGHT_PROB 的概率打印灯

// 闪烁灯颜色数组 (多种亮色)
const char* const light_colors[] =
{
    "\033[31m",  // 红色
    "\033[34m",  // 蓝色
    "\033[35m",  // 洋红色
    "\033[36m",  // 青色
    "\033[37m",  // 白色
    GOLD         // 亮黄色/金色
};
// 使用 sizeof 表达式计算数组元素数量是 C/C++ 的标准做法
const int NUM_COLORS = sizeof(light_colors) / sizeof(light_colors[0]);

// --- 辅助函数：打印居中空格 ---
static void print_padding(int width)
{
    int padding = (MAX_TREE_WIDTH - width) / 2;
    for (int j = 0; j < padding; j++)
    {
        printf(" ");
    }
}

// --- 绘制单个树层的函数 ---
// base_width: 这一层的最宽行有多少个星号
// height: 这一层有多少行
// offset: 顶部不尖锐的行偏移量 (0: 尖顶; >0: 平顶)
static void draw_layer(int base_width, int height, int offset)
{
    int max_layer_stars = base_width;
    for (int i = 0; i < height; i++)
    {
        int stars;
        // 核心优化：简化星号数量的计算逻辑
        if (i < offset)
        {
            // 固定顶部宽度
            stars = max_layer_stars - (height - 1 - offset) * 2;
        }
        else
        {
            // 正常三角形递减
            stars = max_layer_stars - (height - 1 - i) * 2;
        }

        // 打印空格进行居中
        print_padding(stars);

        // 打印星号和灯
        printf("%s", GREEN_LEAF); // 提前设置树叶颜色
        for (int k = 0; k < stars; k++)
        {
            // 随机决定是否打印灯
            if (rand() % LIGHT_PROB == 0)
            {
                int color_index = rand() % NUM_COLORS;
                // 打印灯 (随机颜色 + 加粗)
                printf("%s%s*%s", light_colors[color_index], BOLD, RESET GREEN_LEAF);
            }
            else
            {
                // 打印树叶
                printf("*");
            }
        }
        printf(RESET "\n"); // 换行并重置颜色
    }
}

// --- 绘制圣诞树整体框架 ---
static void draw_tree_frame()
{
    // --- 0. 顶部 Emoji 星星 ---
    printf("\n"); // 顶部的留白
    // 单字符宽度的 Unicode 符号 ★ (假设宽度为 1)
    print_padding(1);
    printf("%s%s★%s\n", GOLD, BOLD, RESET); // 亮黄色/金色 + 加粗

    // --- 1. 树顶 (单层尖顶) ---
    draw_layer(1, 1, 0);

    // --- 2. 树叶多层部分 ---
    draw_layer(7, 3, 0);  // 第一层
    draw_layer(13, 4, 1); // 第二层 (平顶)
    draw_layer(21, 5, 1); // 第三层 (平顶)

    // --- 3. 树干部分 (棕色) ---
    int trunk_width = 3;
    int trunk_height = 3;
    printf("%s%s", BROWN_TRUNK, BOLD); // 提前设置树干颜色和加粗
    for (int i = 0; i < trunk_height; i++)
    {
        print_padding(trunk_width);
        for (int k = 0; k < trunk_width; k++)
        {
            printf("*");
        }
        printf("\n");
    }
    printf(RESET); // 重置颜色

    // --- 4. 圣诞祝福 ---
    printf("\n");
    // 假设祝福语长度为 18
    int wish_width = 18;
    print_padding(wish_width);
    printf("%s%s   Merry Christmas!%s\n", GOLD, BOLD, RESET);
    printf("\n");
}


// --- 闪烁效果实现 ---
static void flash_tree()
{
    // ANSI 代码：\033[H 移动光标到左上角
    const char* const CLEAR_SCREEN_AND_MOVE_CURSOR = "\033[H";
    const int FLASH_DELAY_MS = 200; // 闪烁延时 200ms

    while (1)
    {
        // 1. 绘制第一帧 (灯光颜色 A)
        draw_tree_frame();

        // 延时
        SLEEP_MS(FLASH_DELAY_MS);

        // 2. 清除屏幕
        printf("%s", CLEAR_SCREEN_AND_MOVE_CURSOR);

        // 3. 绘制第二帧 (由于颜色是随机的，每次调用都会产生略微不同的闪烁效果)
        draw_tree_frame();

        // 延时
        SLEEP_MS(FLASH_DELAY_MS);

        // 4. 清除屏幕
        printf("%s", CLEAR_SCREEN_AND_MOVE_CURSOR);
    }
}

int main()
{
    // 优化：随机数种子只在 main 中初始化一次
    srand((unsigned int)time(NULL));

    // 保持闪烁效果的启动
    flash_tree();
    return 0;
}