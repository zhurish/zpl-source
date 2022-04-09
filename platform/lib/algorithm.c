#include "auto_include.h"
#include "algorithm.h"


void bubbling_sort(int arr[], int n) 
{
    int i, j, temp;
    int flag = 0;   // flag用来校验每轮循环有没有移动元素,0为没有移动，1为移动
    // 每次将一个元素送到末尾，n个元素，执行n次
    for (i = 0; i < n; ++i) {
        flag = 0;
        // 之前的循环已经将i个元素送到末尾，不需要再次比较，故减去，因为跟后一个元素比较，为了避免溢出，故减一
        for (j = 0; j < n - i - 1; ++j) {
            // 如果当前的元素比后一个元素小，就交换
            if (arr[j] > arr[j + 1]) {
                flag = 1;   // 有数据交换
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
        // 没有数据交换，提前结束
        if (flag == 0) {
            break;
        }
    }
}