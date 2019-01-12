gcc提供了大量的警告选项，对代码中可能存在的问题提出警告，通常可以使用-Wall来开启以下警告:
           -Waddress -Warray-bounds (only with -O2) -Wc++0x-compat
           -Wchar-subscripts -Wimplicit-int -Wimplicit-function-declaration
           -Wcomment -Wformat -Wmain (only for C/ObjC and unless
           -ffreestanding) -Wmissing-braces -Wnonnull -Wparentheses
           -Wpointer-sign -Wreorder -Wreturn-type -Wsequence-point
           -Wsign-compare (only in C++) -Wstrict-aliasing -Wstrict-overflow=1
           -Wswitch -Wtrigraphs -Wuninitialized (only with -O1 and above)
           -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value
           -Wunused-variable
unused-function:警告声明但是没有定义的static函数;
unused- label:声明但是未使用的标签;
unused-parameter:警告未使用的函数参数;
unused-variable:声明但是未使用的本地变量;
unused-value:计算了但是未使用的值;
format:printf和scanf这样的函数中的格式字符串的使用不当;
implicit-int:未指定类型;
implicit-function:函数在声明前使用;
char- subscripts:使用char类作为数组下标(因为char可能是有符号数);
missingbraces:大括号不匹配;
parentheses: 圆括号不匹配;
return-type:函数有无返回值以及返回值类型不匹配;
sequence-point:违反顺序点的代码,比如 a[i] = c[i++];
switch:switch语句缺少default或者switch使用枚举变量为索引时缺少某个变量的case;
strict- aliasing=n:使用n设置对指针变量指向的对象类型产生警告的限制程度,默认n=3;只有在-fstrict-aliasing设置的情况下有效;
unknow-pragmas:使用未知的#pragma指令;
uninitialized:使用的变量为初始化,只在-O2时有效;

以下是在-Wall中不会激活的警告选项:
cast-align:当指针进行类型转换后有内存对齐要求更严格时发出警告;
sign- compare:当使用signed和unsigned类型比较时;
missing-prototypes:当函数在使用前没有函数原型时;
packed:packed 是gcc的一个扩展,是使结构体各成员之间不留内存对齐所需的空间,有时候会造成内存对齐的问题;
padded:也是gcc的扩展,使结构体成员之间进行内存对齐的填充,会造成结构体体积增大.
unreachable-code:有不会执行的代码时.
inline:当inline函数不再保持inline时 (比如对inline函数取地址);
disable-optimization:当不能执行指定的优化时.(需要太多时间或系统资源).
可以使用 -Werror时所有的警告都变成错误,使出现警告时也停止编译.需要和指定警告的参数一起使用.

优化:
gcc默认提供了5级优化选项的集合:
-O0:无优化(默认)
-O和-O1:使用能减少目标文件大小以及执行时间并且不会使编译时间明显增加的优化.在编译大型程序的时候会显著增加编译时内存的使用.
-O2: 包含-O1的优化并增加了不需要在目标文件大小和执行速度上进行折衷的优化.编译器不执行循环展开以及函数内联.此选项将增加编译时间和目标文件的执行性能.
-Os:专门优化目标文件大小,执行所有的不增加目标文件大小的-O2优化选项.并且执行专门减小目标文件大小的优化选项.
-O3: 打开所有-O2的优化选项并且增加 -finline-functions, -funswitch-loops,-fpredictive-commoning, -fgcse-after-reload and -ftree-vectorize优化选项.

-O1包含的选项-O1通常可以安全的和调试的选项一起使用:
           -fauto-inc-dec -fcprop-registers -fdce -fdefer-pop -fdelayed-branch
           -fdse -fguess-branch-probability -fif-conversion2 -fif-conversion
           -finline-small-functions -fipa-pure-const -fipa-reference
           -fmerge-constants -fsplit-wide-types -ftree-ccp -ftree-ch
           -ftree-copyrename -ftree-dce -ftree-dominator-opts -ftree-dse
           -ftree-fre -ftree-sra -ftree-ter -funit-at-a-time

以下所有的优化选项需要在名字前加上-f,如果不需要此选项可以使用-fno-前缀
defer-pop:延迟到只在必要时从函数参数栈中pop参数;
thread- jumps:使用跳转线程优化,避免跳转到另一个跳转;
branch-probabilities:分支优化;
cprop- registers:使用寄存器之间copy-propagation传值;
guess-branch-probability:分支预测;
omit- frame-pointer:可能的情况下不产生栈帧;

-O2:以下是-O2在-O1基础上增加的优化选项:
           -falign-functions  -falign-jumps -falign-loops  -falign-labels
           -fcaller-saves -fcrossjumping -fcse-follow-jumps  -fcse-skip-blocks
           -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse
           -fgcse-lm -foptimize-sibling-calls -fpeephole2 -fregmove
           -freorder-blocks  -freorder-functions -frerun-cse-after-loop
           -fsched-interblock  -fsched-spec -fschedule-insns
           -fschedule-insns2 -fstrict-aliasing -fstrict-overflow -ftree-pre
           -ftree-vrp
cpu架构的优化选项,通常是-mcpu(将被取消);-march,-mtune

Debug选项:
在 gcc编译源代码时指定-g选项可以产生带有调试信息的目标代码,gcc可以为多个不同平台上帝不同调试器提供调试信息,默认gcc产生的调试信息是为 gdb使用的,可以使用-gformat 指定要生成的调试信息的格式以提供给其他平台的其他调试器使用.常用的格式有
-ggdb:生成gdb专用的调试信息,使用最适合的格式(DWARF 2,stabs等)会有一些gdb专用的扩展,可能造成其他调试器无法运行.
-gstabs:使用 stabs格式,不包含gdb扩展,stabs常用于BSD系统的DBX调试器.
-gcoff:产生COFF格式的调试信息,常用于System V下的SDB调试器;
-gxcoff:产生XCOFF格式的调试信息,用于IBM的RS/6000下的DBX调试器;
-gdwarf- 2:产生DWARF version2 的格式的调试信息,常用于IRIXX6上的DBX调试器.GCC会使用DWARF version3的一些特性.
可以指定调试信息的等级:在指定的调试格式后面加上等级:
如: -ggdb2 等,0代表不产生调试信息.在使用-gdwarf-2时因为最早的格式为-gdwarf2会造成混乱,所以要额外使用一个-glevel来指定调试信息的等级,其他格式选项也可以另外指定等级.

gcc可以使用-p选项指定生成信息以供porf使用.


-Wall 	会打开一些很有用的警告选项，建议编译时加此选项。
-W 
-Wextra 	打印一些额外的警告信息。
-w 	禁止显示所有警告信息。
-Wshadow 	当一个局部变量遮盖住了另一个局部变量，或者全局变量时，给出警告。很有用的选项，建议打开。 -Wall 并不会打开此项。
-Wpointer-arith 	对函数指针或者void *类型的指针进行算术操作时给出警告。也很有用。 -Wall 并不会打开此项。
-Wcast-qual 	当强制转化丢掉了类型修饰符时给出警告。 -Wall 并不会打开此项。
-Waggregate-return 	如果定义或调用了返回结构体或联合体的函数，编译器就发出警告。
-Winline 	无论是声明为 inline 或者是指定了-finline-functions 选项，如果某函数不能内联，编译器都将发出警告。如果你的代码含有很多 inline 函数的话，这是很有用的选项。
-Werror 	把警告当作错误。出现任何警告就放弃编译。
-Wunreachable-code 	如果编译器探测到永远不会执行到的代码，就给出警告。也是比较有用的选项。
-Wcast-align 	一旦某个指针类型强制转换导致目标所需的地址对齐增加时，编译器就发出警告。
-Wundef 	当一个没有定义的符号出现在 #if 中时，给出警告。
-Wredundant-decls 	如果在同一个可见域内某定义多次声明，编译器就发出警告，即使这些重复声明有效并且毫无差别。

-fsigned-bitfields 
-funsigned-bitfields 	如果没有明确声明`signed'或`unsigned'修饰符，这些选项用来定义有符号位域或无符号位域。缺省情况下，位域是有符号的，因为它们继承的基本整数类型，如int，是有符号数。
-Wstrict-prototypes 	如果函数的声明或定义没有指出参数类型，编译器就发出警告。很有用的警告。
-Wmissing-prototypes 	如果没有预先声明就定义了全局函数，编译器就发出警告。即使函数定义自身提供了函数原形也会产生这个警告。这个选项 的目的是检查没有在头文件中声明的全局函数。
-Wnested-externs 	如果某extern声明出现在函数内部，编译器就发出警告。