#include <cstddef> // ptrdiff_t
#include <cstdio>
int main(int argc, char* argv[])
{
int arr[5] = {1,2,3,4,5};
int *ptr = arr;
printf("%d\n",&ptr[4]-&ptr[0]);
// 两者是为了跨平台的 两者之间的差别是 ptrdiff_t 是有符号的 用于指针差
printf("%d\n", ptrdiff_t(&ptr[1]-&ptr[2]));
printf("%lud\n", (size_t)(&ptr[1]-&ptr[2])); // 回到真正的输出就有问题
// system("PAUSE");
return 0;
}