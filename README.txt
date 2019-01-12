工程说明：
abstract     		抽象层代码，主要是对kernel，IPCOM, SDK的抽象（分PAL,HAL）
component		模块组件，DHCP,MODEM.SQLITE,SSH,WIFI,VOIP
make      			makefile系统
platform  			公共库，lib， nsm， os， shell
product     		厂商板级信息定义
service         		service，sntp， syslog，ftp，tftp，telnet， ping
startup         		启动服务程序
tools			工具，额外进程管理，qmi拨号
application 		app，对接A模块和物管
cli       			CLI
externsions  		第三方库和头文件
include        		顶层头文件定义
Makefile 			顶层app的Makefile
os_main.c   		app的main启动

README.txt 

编译使用：

make app ARCH_TYPE=MIPS TSL=true		编译TSL的openwrt版本，CPU架构为mipsel


编译后会生成debug目录
bin		app主进程安装目录
etc		默认配置文件目录	
lib		编译的静态库
obj		编译过程文件目录
sbin		tools工具可执行文件安装目录




增加模块：

make/board.cfg	主要定义工具链，编译版本信息
make/module.mk
	在_MODULELIST 增加模块全局编译开关
	在PlatformModule增加模块目录定义

make/module-config.mk
	增加模块目录路径，预定义宏等等



源码目录 config.mk定义需要编译的c文件，库名称；Makefile编译安装