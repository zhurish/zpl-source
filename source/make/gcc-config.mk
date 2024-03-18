##ZPLOS_LDLIBS += -std=c99 
ZPLOS_CFLAGS += -std=gnu99
#ZPLOS_CFLAGS += -std=gnu99 -fgnu99-inline
ZPLOS_CPPFLAGS += -std=c++14 -Wno-write-strings
## -D_GLIBCXX_USE_CXX11_ABI=0
# 
#
# WANRING
#	
ZPLOS_CFLAGS += -MMD -MP 
ZPLOS_CFLAGS += -fmessage-length=0 
ZPLOS_CFLAGS += -fsigned-char #-fstack-protector 
ZPLOS_CFLAGS += -Wfatal-errors -Wall -Wextra -Wunused #-Wstack-protector 
#
ZPLOS_CPPFLAGS += -MMD -MP -Wall -Wextra -Wfatal-errors -fsigned-char #-fstack-protector
#ZPLOS_CFLAGS += thread-jumps #:使用跳转线程优化,避免跳转到另一个跳转;
#
#-Wall
#ZPLOS_GCC_WALL += -Waddress -Wbool-compare -Wbool-operation -Wc++11-compat  -Wc++14-compat  -Wchar-subscripts -Wcomment \
		-Wformat -Wint-in-bool-context  -Wlogical-not-parentheses -Wmaybe-uninitialized -Wmemset-elt-size -Wmemset-transposed-args \
        -Wmissing-attributes -Wmultistatement-macros \
        -Wnonnull -Wnonnull-compare -Wopenmp-simd -Wparentheses -Wpointer-sign -Wreorder -Wrestrict \
        -Wreturn-type -Wsequence-point -Wsizeof-pointer-div -Wsizeof-pointer-memaccess -Wstrict-aliasing \
        -Wstrict-overflow=1 -Wstringop-truncation -Wswitch -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas \
        -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var
#ZPLOS_GCC_WALL += -Warray-bounds=1 # (only with -O2)
#ZPLOS_GCC_WALL += -Wcatch-value # (C++ and Objective-C++ only)
#ZPLOS_GCC_WALL += -Wduplicate-decl-specifier # (C and Objective-C only)
#ZPLOS_GCC_WALL += -Wsign-compare # (only in C++)
#ZPLOS_GCC_WALL += -Wenum-compare  # (in C/ObjC; this is on by default in C++)
#ZPLOS_GCC_WALL += -Wimplicit # (C and Objective-C only) 
#ZPLOS_GCC_WALL += -Wimplicit-int # (C and Objective-C only) 
#ZPLOS_GCC_WALL += -Wimplicit-function-declaration # (C and Objective-C only) 
ZPLOS_GCC_WALL += -Winit-self # (only for C++)
#ZPLOS_GCC_WALL += -Wmissing-braces # (only for C/ObjC)
#ZPLOS_GCC_WALL += -Wmisleading-indentation # (only for C/C++) 
#ZPLOS_GCC_WALL += -Wnarrowing # (only for C++)
#ZPLOS_GCC_WALL += -Wmain # (only for C/ObjC and unless -ffreestanding) 

#-Wextra
#ZPLOS_GCC_WEXTRA += -Wclobbered -Wcast-function-type -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wmissing-field-initializers \
            -Woverride-init -Wtype-limits -Wuninitialized -Wshift-negative-value -Wunused-parameter -Wunused-but-set-parameter 
#ZPLOS_GCC_WEXTRA += -Wmissing-parameter-type # (C only) 
#ZPLOS_GCC_WEXTRA += -Wold-style-declaration # (C only) 
#ZPLOS_GCC_WEXTRA += -Wsign-compare # (C only) 	

ZPLOS_GCC_WARNING += -Wcast-align
#ZPLOS_GCC_WARNING += -Walloc-zero
#ZPLOS_GCC_WARNING += -Wshadow=compatible-local
ZPLOS_GCC_WARNING += -Wduplicated-cond
ZPLOS_GCC_WARNING += -Wundef
ZPLOS_GCC_WARNING += -Wsizeof-pointer-memaccess
ZPLOS_GCC_WARNING += -Wmemset-transposed-args
#-Wmaybe-uninitialized

# -Werror
#ZPLOS_GNU_WERROR += -Werror=format
ZPLOS_GNU_WERROR += -Werror=format-contains-nul #当格式字符串包含 NUL 字节时给出警告  
ZPLOS_GNU_WERROR += -Werror=format-extra-args #当传递给格式字符串的参数太多时给出警告  
#ZPLOS_GNU_WERROR += -Werror=format-nonliteral #当格式字符串不是字面值时给出警告  
ZPLOS_GNU_WERROR += -Werror=format-security #当使用格式字符串的函数可能导致安全问题时给出警告   
ZPLOS_GNU_WERROR += -Werror=format-zero-length #对长度为 0 的格式字符串给出警告 	
#ZPLOS_GNU_WERROR += -Werror=double-promotion    #对从“float”到“double”的隐式转换给出警告

#ZPLOS_GNU_WERROR += -Werror=vla                     #使用变长数组时警告  
ZPLOS_GNU_WERROR += -Werror=volatile-register-var   #当一个寄存器变量被声明为 volatile

ZPLOS_GNU_WERROR += -Werror=div-by-zero
#ZPLOS_GNU_WERROR += -Werror=alloc-zero
ZPLOS_GNU_WERROR += -Werror=duplicated-cond
#ZPLOS_GNU_WERROR += -Werror=shadow=compatible-local
#ZPLOS_GNU_WERROR += -Werror=undef
#ZPLOS_GNU_WERROR += -Werror=sizeof-pointer-memaccess
#ZPLOS_GNU_WERROR += -Werror=memset-transposed-args
#ZPLOS_GNU_WERROR += -Werror=missing-braces
#ZPLOS_GCC_WERROR += -Werror=underflow
#ZPLOS_GCC_WERROR += -Werror=intrinsic-shadow
ZPLOS_GNU_WERROR += -Werror=return-type 
#ZPLOS_GNU_WERROR += -Werror=uninitialized
ZPLOS_GNU_WERROR += -Werror=format-extra-args 
ZPLOS_GNU_WERROR += -Werror=address
ZPLOS_GNU_WERROR += -Werror=unreachable-code 
ZPLOS_GNU_WERROR += -Werror=chkp  
ZPLOS_GNU_WERROR += -Werror=unused-value 
ZPLOS_GCC_WERROR += -Werror=implicit-int 
ZPLOS_GCC_WERROR += -Werror=missing-parameter-type 
ZPLOS_GNU_WERROR += -Werror=parentheses 
ZPLOS_GNU_WERROR += -Werror=char-subscripts 
ZPLOS_GNU_WERROR += -Werror=pointer-arith
ZPLOS_GNU_WERROR += -Werror=invalid-memory-model 
ZPLOS_GNU_WERROR += -Werror=sizeof-pointer-memaccess 
ZPLOS_GNU_WERROR += -Werror=overflow 
ZPLOS_GNU_WERROR += -Werror=format-security 
#ZPLOS_GNU_WERROR += -Werror=shadow 
ZPLOS_GNU_WERROR += -Werror=array-bounds 
ZPLOS_GNU_WERROR += -Werror=unsafe-loop-optimizations 
ZPLOS_GNU_WERROR += -Werror=init-self  
ZPLOS_GNU_WERROR += -Werror=unused-function 
ZPLOS_GNU_WERROR += -Werror=redundant-decls 
ZPLOS_GNU_WERROR += -Werror=unused-variable 
ZPLOS_GCC_WERROR += -Werror=missing-prototypes 
ZPLOS_GNU_WERROR += -Werror=sequence-point 
#ZPLOS_GNU_WERROR += -Werror=float-equal 
#ZPLOS_GCC_WERROR += -Werror=strict-prototypes 
ZPLOS_GNU_WERROR += -Werror=overlength-strings 
ZPLOS_GNU_WERROR += -Werror=unused-label 
ZPLOS_GNU_WERROR += -Werror=shift-count-overflow  
ZPLOS_GCC_WERROR += -Werror=int-conversion 
ZPLOS_GNU_WERROR += -Werror=unsafe-loop-optimizations 
ZPLOS_GNU_WERROR += -Werror=memset-transposed-args  
#ZPLOS_GCC_WERROR += -Werror=implicit-function-declaration  
ZPLOS_GNU_WERROR += -Werror=enum-compare 
ZPLOS_GNU_WERROR += -Werror=ignored-qualifiers 
ZPLOS_GNU_WERROR += -Werror=type-limits  
#ZPLOS_GNU_WERROR += -Werror=incompatible-pointer-types 

ZPLOS_CFLAGS += -Wnested-externs $(ZPLOS_GCC_WARNING) $(ZPLOS_GNU_WERROR) $(ZPLOS_GCC_WERROR)
#			  
#
ZPLOS_CPPFLAGS += -fcheck-new -Wnon-virtual-dtor $(ZPLOS_GCC_WARNING) $(ZPLOS_GNU_WERROR) $(ZPLOS_GPP_WERROR) -fpermissive
#-fsyntax-only
#			 

