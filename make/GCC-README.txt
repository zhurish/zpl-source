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




 --all-warnings              此开关缺少可用文档  
  --extra-warnings            此开关缺少可用文档  
  -W                          不建议使用此开关；请改用 -Wextra  
  -Wabi                       当结果与 ABI  
                              相容的编译器的编译结果不同时给出警告  
  -Waddress                   使用可疑的内存地址时给出警告  
  -Waggregate-return          当返回结构、联合或数组时给出警告  
  -Waliasing                  为可能的虚参重叠给出警告  
  -Walign-commons             对 COMMON 块对齐的警告  
  -Wall                       启用大部分警告信息  
  -Wampersand                 若延续字符常量中缺少 & 则给出警告  
  -Warray-bounds              当数组访问越界时给出警告  
  -Warray-temporaries         创建临时数组时给出警告  
  -Wassign-intercept          当 Objective-C  
                              赋值可能为垃圾回收所介入时给出警告  
  -Wattributes                当对属性的使用不合适时给出警告  
  -Wbad-function-cast         当把函数转换为不兼容类型时给出警告  
  -Wbuiltin-macro-redefined   当内建预处理宏未定义或重定义时给出警告  
  -Wc++-compat                当在 C 语言中使用了 C 与 C++  
                              交集以外的构造时给出警告  
  -Wc++0x-compat              当 C++ 构造的意义在 ISO C++ 1998 和 ISO  
                              C++ 200x 中不同时给出警告  
  -Wcast-align                当转换指针类型导致对齐边界增长时给出警告  
  -Wcast-qual                 当类型转换丢失限定信息时给出警告  
  -Wchar-subscripts           当下标类型为“char”时给出警告  
  -Wcharacter-truncation      对被截断的字符表达式给出警告  
  -Wclobbered                 对能为"longjmp"或"vfork"所篡改的变量给出警告  
  -Wcomment                   对可能嵌套的注释和长度超过一个物理行长的  
                              C++ 注释给出警告  
  -Wcomments                  -Wcomment 的同义词  
  -Wconversion                当隐式类型转换可能改变值时给出警告  
  -Wconversion-extra          对大多数隐式类型转换给出警告  
  -Wconversion-null           将 NULL 转换为非指针类型时给出警告  
  -Wcoverage-mismatch         Warn in case profiles in -fprofile-use do not  
                              match  
  -Wcpp                       Warn when a #warning directive is encountered  
  -Wctor-dtor-privacy         当所有构造函数和析构函数都是私有时给出警告  
  -Wdeclaration-after-statement 当声明出现在语句后时给出警告  
  -Wdeprecated                使用不建议的编译器特性、类、方法或字段时给出警告  
  -Wdeprecated-declarations   对 __attribute__((deprecated)) 声明给出警告  
  -Wdisabled-optimization     当某趟优化被禁用时给出警告  
  -Wdiv-by-zero               对编译时发现的零除给出警告  
  -Wdouble-promotion          对从“float”到“double”的隐式转换给出警告  
  -Weffc++                    对不遵循《Effetive  
                              C++》的风格给出警告  
  -Wempty-body                当 if 或 else 语句体为空时给出警告  
  -Wendif-labels              当 #elif 和 #endif  
                              后面跟有其他标识符时给出警告  
  -Wenum-compare              对不同枚举类型之间的比较给出警告  
  -Werror-implicit-function-declaration 不建议使用此开关；请改用  
                              -Werror=implicit-function-declaration  
  -Wextra                     打印额外(可能您并不想要)的警告信息  
  -Wfloat-equal               当比较浮点数是否相等时给出警告  
  -Wformat                    对 printf/scanf/strftime/strfmon  
                              中的格式字符串异常给出警告  
  -Wformat-contains-nul       当格式字符串包含 NUL 字节时给出警告  
  -Wformat-extra-args         当传递给格式字符串的参数太多时给出警告  
  -Wformat-nonliteral         当格式字符串不是字面值时给出警告  
  -Wformat-security           当使用格式字符串的函数可能导致安全问题时给出警告  
  -Wformat-y2k                当 strftime 格式给出 2  
                              位记年时给出警告  
  -Wformat-zero-length        对长度为 0 的格式字符串给出警告  
  -Wformat=                   此开关缺少可用文档  
  -Wignored-qualifiers        当类型限定符被忽略时给出警告。  
  -Wimplicit                  对隐式函数声明给出警告  
  -Wimplicit-function-declaration 对隐式函数声明给出警告  
  -Wimplicit-int              当声明未指定类型时给出警告  
  -Wimplicit-interface        对带有隐式接口的调用给出警告  
  -Wimplicit-procedure        对没有隐式声明的过程调用给出警告  
  -Winit-self                 对初始化为自身的变量给出警告。  
  -Winline                    当内联函数无法被内联时给出警告  
  -Wint-to-pointer-cast       当将一个大小不同的整数转换为指针时给出警告  
  -Wintrinsic-shadow          如果用户过程有与内建过程相同的名字则警告  
  -Wintrinsics-std            当内建函数不是所选标准的一部分时给出警告  
  -Winvalid-offsetof          对“offsetof”宏无效的使用给出警告  
  -Winvalid-pch               在找到了 PCH  
                              文件但未使用的情况给出警告  
  -Wjump-misses-init          当跳转略过变量初始化时给出警告  
  -Wlarger-than-              此开关缺少可用文档  
  -Wlarger-than=<N>           当目标文件大于 N 字节时给出警告  
  -Wline-truncation           对被截断的源文件行给出警告  
  -Wlogical-op                当逻辑操作结果似乎总为真或假时给出警告  
  -Wlong-long                 当使用 -pedantic 时不对“long  
                              long”给出警告  
  -Wmain                      对可疑的“main”声明给出警告  
  -Wmissing-braces            若初始值设定项中可能缺少花括号则给出警告  
  -Wmissing-declarations      当全局函数没有前向声明时给出警告  
  -Wmissing-field-initializers 若结构初始值设定项中缺少字段则给出警告  
  -Wmissing-format-attribute  当函数可能是 format  
                              属性的备选时给出警告  
  -Wmissing-include-dirs      当用户给定的包含目录不存在时给出警告  
  -Wmissing-noreturn          当函数可能是 __attribute__((noreturn))  
                              的备选时给出警告  
  -Wmissing-parameter-type    K&R  
                              风格函数参数声明中未指定类型限定符时给出警告  
  -Wmissing-prototypes        全局函数没有原型时给出警告  
  -Wmudflap                   当构造未被 -fmudflap 处理时给出警告  
  -Wmultichar                 使用多字节字符集的字符常量时给出警告  
  -Wnested-externs            当“extern”声明不在文件作用域时给出警告  
  -Wnoexcept                  Warn when a noexcept expression evaluates to  
                              false even though the expression can't actually  
                              throw  
  -Wnon-template-friend       在模板内声明未模板化的友元函数时给出警告  
  -Wnon-virtual-dtor          当析构函数不是虚函数时给出警告  
  -Wnonnull                   当将 NULL 传递给需要非 NULL  
                              的参数的函数时给出警告  
  -Wnormalized=<id|nfc|nfkc>  对未归一化的 Unicode 字符串给出警告  
  -Wold-style-cast            程序使用 C  
                              风格的类型转换时给出警告  
  -Wold-style-declaration     对声明中的过时用法给出警告  
  -Wold-style-definition      使用旧式形参定义时给出警告  
  -Woverflow                  算术表示式溢出时给出警告  
  -Woverlength-strings        当字符串长度超过标准规定的可移植的最大长度时给出警告  
  -Woverloaded-virtual        重载虚函数名时给出警告  
  -Woverride-init             覆盖无副作用的初始值设定时给出警告  
  -Wpacked                    当 packed  
                              属性对结构布局不起作用时给出警告  
  -Wpacked-bitfield-compat    当紧实位段的偏移量因 GCC 4.4  
                              而改变时给出警告  
  -Wpadded                    当需要填补才能对齐结构成员时给出警告  
  -Wparentheses               可能缺少括号的情况下给出警告  
  -Wpmf-conversions           当改变成员函数指针的类型时给出警告  
  -Wpointer-arith             当在算术表达式中使用函数指针时给出警告  
  -Wpointer-sign              赋值时如指针符号不一致则给出警告  
  -Wpointer-to-int-cast       将一个指针转换为大小不同的整数时给出警告  
  -Wpragmas                   对错误使用的 pragma 加以警告  
  -Wproperty-assign-default   Warn if a property for an Objective-C object has  
                              no assign semantics specified  
  -Wprotocol                  当继承来的方法未被实现时给出警告  
  -Wreal-q-constant           Warn about real-literal-constants with 'q'  
                              exponent-letter  
  -Wredundant-decls           对同一个对象多次声明时给出警告  
  -Wreorder                   编译器将代码重新排序时给出警告  
  -Wreturn-type               当 C  
                              函数的返回值默认为“int”，或者 C++  
                              函数的返回类型不一致时给出警告  
  -Wselector                  当选择子有多个方法时给出警告  
  -Wsequence-point            当可能违反定序点规则时给出警告  
  -Wshadow                    当一个局部变量掩盖了另一个局部变量时给出警告  
  -Wsign-compare              在有符号和无符号数间进行比较时给出警告  
  -Wsign-promo                当重载将无符号数提升为有符号数时给出警告  
  -Wstack-protector           当因为某种原因堆栈保护失效时给出警告  
  -Wstrict-aliasing           当代码可能破坏强重叠规则时给出警告  
  -Wstrict-aliasing=          当代码可能破坏强重叠规则时给出警告  
  -Wstrict-null-sentinel      将未作转换的 NULL  
                              用作哨兵时给出警告  
  -Wstrict-overflow           禁用假定有符号数溢出行为未被定义的优化  
  -Wstrict-overflow=          禁用假定有符号数溢出行为未被定义的优化  
  -Wstrict-prototypes         使用了非原型的函数声明时给出警告  
  -Wstrict-selector-match     当备选方法的类型签字不完全匹配时给出警告  
  -Wsuggest-attribute=const   Warn about functions which might be candidates  
                              for __attribute__((const))  
  -Wsuggest-attribute=noreturn 当函数可能是 __attribute__((noreturn))  
                              的备选时给出警告  
  -Wsuggest-attribute=pure    Warn about functions which might be candidates  
                              for __attribute__((pure))  
  -Wsurprising                对“可疑”的构造给出警告  
  -Wswitch                    当使用枚举类型作为开关变量，没有提供  
                              default 分支，但又缺少某个 case  
                              时给出警告  
  -Wswitch-default            当使用枚举类型作为开关变量，但没有提供“default”分支时给出警告  
  -Wswitch-enum               当使用枚举类型作为开关变量但又缺少某个  
                              case 时给出警告  
  -Wsync-nand                 当 __sync_fetch_and_nand 和  
                              __sync_nand_and_fetch  
                              内建函数被使用时给出警告  
  -Wsynth                     不建议使用。此开关不起作用。  
  -Wsystem-headers            不抑制系统头文件中的警告  
  -Wtabs                      允许使用不符合规范的制表符  
  -Wtraditional               使用了传统 C  
                              不支持的特性时给出警告  
  -Wtraditional-conversion    原型导致的类型转换与无原型时的类型转换不同时给出警告  
  -Wtrampolines               Warn whenever a trampoline is generated  
  -Wtrigraphs                 当三字母序列可能影响程序意义时给出警告  
  -Wtype-limits               当由于数据类型范围限制比较结果永远为真或假时给出警告  
  -Wundeclared-selector       当使用 @selector()  
                              却不作事先声明时给出警告  
  -Wundef                     当 #if  
                              指令中用到未定义的宏时给出警告  
  -Wunderflow                 数字常量表达式下溢时警告  
  -Wuninitialized             自动变量未初始化时警告  
  -Wunknown-pragmas           对无法识别的 pragma 加以警告  
  -Wunsafe-loop-optimizations 当循环因为不平凡的假定而不能被优化时给出警告  
  -Wunsuffixed-float-constants 对不带后缀的浮点常量给出警告  
  -Wunused                    启用所有关于“XX未使用”的警告  
  -Wunused-but-set-parameter  Warn when a function parameter is only set,  
                              otherwise unused  
  -Wunused-but-set-variable   Warn when a variable is only set, otherwise unused  
  -Wunused-dummy-argument     对未使用的哑元给出警告。  
  -Wunused-function           有未使用的函数时警告  
  -Wunused-label              有未使用的标号时警告  
  -Wunused-macros             当定义在主文件中的宏未被使用时给出警告  
  -Wunused-parameter          发现未使用的函数指针时给出警告  
  -Wunused-result             当一个带有 warn_unused_result  
                              属性的函数的调用者未使用前者的返回值时给出警告  
  -Wunused-value              当一个表达式的值未被使用时给出警告  
  -Wunused-variable           有未使用的变量时警告  
  -Wvariadic-macros           指定 -pedantic  
                              时不为可变参数宏给出警告  
  -Wvla                       使用变长数组时警告  
  -Wvolatile-register-var     当一个寄存器变量被声明为 volatile  
                              时给出警告  
  -Wwrite-strings             在 C++  
                              中，非零值表示将字面字符串转换为‘char  
                              *’时给出警告。在 C  
                              中，给出相似的警告，但这种类型转换是符合  
                              ISO C 标准的。  
  -frequire-return-statement  Functions which return values must end with  
                              return statements  