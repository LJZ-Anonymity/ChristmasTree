#include <locale.h>
#include <stdlib.h>
#include <string.h> // 用于 memset/memcpy
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
const int NUM_COLORS = sizeof(light_colors) / sizeof(light_colors[0]);

// 最大灯光数量预估
#define MAX_LIGHTS 100 

// 存储灯光位置和颜色索引的结构体
typedef struct
{
    int x; // 列 (从 0 开始)
    int y; // 行 (从 0 开始)
    int color_index; // 颜色索引
} LightData;

// 全局变量用于存储灯光数据和总数
static LightData lights[MAX_LIGHTS];
static int light_count = 0;
static int current_y = 0; // 跟踪当前光标的行位置（用于定位灯）

// --- 辅助函数：打印居中空格 ---
static void print_padding(int width)
{
    int padding = (MAX_TREE_WIDTH - width) / 2;
    for (int j = 0; j < padding; j++)
    {
        printf(" ");
    }
}

// --- ANSI 代码：移动光标到指定位置 (y: 行, x: 列) ---
// y 是行号，x 是列号。注意 ANSI 是 1-based，而我们使用 0-based 内部计数。
static void goto_xy(int x, int y)
{
    // ANSI 序列: \033[<row>;<col>H
    // 我们需要将 0-based 的 y/x 转换为 1-based
    printf("\033[%d;%dH", y + 1, x + 1);
}


// --- 绘制单个树层的函数 ---
// base_width: 这一层的最宽行有多少个星号
// height: 这一层有多少行
// offset: 顶部不尖锐的行偏移量
// is_first_draw: 是否是第一次绘制（需要记录灯光位置）
static void draw_layer(int base_width, int height, int offset, int is_first_draw)
{
    int max_layer_stars = base_width;
    for (int i = 0; i < height; i++)
    {
        int stars;
        if (i < offset)
        {
            stars = max_layer_stars - (height - 1 - offset) * 2;
        }
        else
        {
            stars = max_layer_stars - (height - 1 - i) * 2;
        }

        // 居中空格的数量
        int padding = (MAX_TREE_WIDTH - stars) / 2;

        // 打印空格进行居中
        print_padding(stars);

        // 打印星号和灯
        printf("%s", GREEN_LEAF); // 提前设置树叶颜色
        for (int k = 0; k < stars; k++)
        {
            // 随机决定是否打印灯 (只在第一次绘制时执行随机逻辑)
            if (is_first_draw && rand() % LIGHT_PROB == 0)
            {
                int color_index = rand() % NUM_COLORS;

                // 记录灯光位置和颜色 (如果数组未满)
                if (light_count < MAX_LIGHTS) {
                    lights[light_count].x = padding + k; // 实际屏幕列
                    lights[light_count].y = current_y;   // 实际屏幕行
                    lights[light_count].color_index = color_index;
                    light_count++;
                }

                // 打印灯 (随机颜色 + 加粗)
                printf("%s%s*%s", light_colors[color_index], BOLD, RESET GREEN_LEAF);
            }
            else
            {
                // 如果是第一次绘制但不是灯，或者不是第一次绘制，直接打印树叶
                printf("*");
            }
        }
        printf(RESET "\n"); // 换行并重置颜色
        current_y++; // 更新全局行计数器
    }
}


// --- 绘制圣诞树整体框架 (第一帧) ---
static void draw_tree_frame_initial()
{
    light_count = 0; // 重置灯光计数
    current_y = 0;   // 重置行计数器

    // 清屏和移动光标到 (0, 0)
    printf("\033[2J\033[H");

    // --- 0. 顶部 Emoji 星星 ---
    printf("\n"); // 顶部的留白
    current_y++; // 顶部留白也占一行

    // 单字符宽度的 Unicode 符号 ★ (假设宽度为 1)
    print_padding(1);
    printf("%s%s★%s\n", GOLD, BOLD, RESET);
    current_y++;

    // --- 1. 树顶 (单层尖顶) ---
    draw_layer(1, 1, 0, 1);

    // --- 2. 树叶多层部分 ---
    draw_layer(7, 3, 0, 1);  // 第一层
    draw_layer(13, 4, 1, 1); // 第二层 (平顶)
    draw_layer(21, 5, 1, 1); // 第三层 (平顶)

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
        current_y++;
    }
    printf(RESET); // 重置颜色

    // --- 4. 圣诞祝福 ---
    printf("\n");
    current_y++;
    int wish_width = 18;
    print_padding(wish_width);
    printf("%s%s   Merry Christmas!%s\n", GOLD, BOLD, RESET);
    printf("\n");
    current_y += 2; // 祝福和末尾留白

    // 强制刷新输出流，确保立即显示
    fflush(stdout);
}

// --- 闪烁效果实现 (局部更新灯光) ---
static void flash_tree_lights()
{
    const int FLASH_DELAY_MS = 200; // 闪烁延时 200ms
    int frame = 0;

    // 预先绘制第一帧，并记录所有灯光位置
    draw_tree_frame_initial();
    while (1)
    {
        // 局部更新灯光
        for (int i = 0; i < light_count; i++)
        {
            // 移动光标到灯光位置
            goto_xy(lights[i].x, lights[i].y);

            // 随机选择新的颜色索引
            int color_index = rand() % NUM_COLORS;

            // 打印灯 (随机颜色 + 加粗) 或 打印树叶 (实现闪烁)
            if (frame % 2 == 0) {
                // 奇数帧：打印随机颜色灯光
                printf("%s%s*%s", light_colors[color_index], BOLD, RESET GREEN_LEAF);
            }
            else {
                // 偶数帧：打印树叶颜色，实现“熄灭”效果
                printf("%s*", GREEN_LEAF);
            }
        }

        // 移回光标到屏幕底部，避免输入干扰
        goto_xy(0, current_y);

        fflush(stdout); // 刷新输出
        SLEEP_MS(FLASH_DELAY_MS);

        frame++;
    }
}

int main()
{
    // 随机数种子只在 main 中初始化一次
    srand((unsigned int)time(NULL));

    // 保持闪烁效果的启动
    flash_tree_lights();
    return 0;
}