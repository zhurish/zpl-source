����˵����
abstract     		�������룬��Ҫ�Ƕ�kernel��IPCOM, SDK�ĳ��󣨷�PAL,HAL��
component		ģ�������DHCP,MODEM.SQLITE,SSH,WIFI,VOIP
make      			makefileϵͳ
platform  			�����⣬lib�� nsm�� os�� shell
product     		���̰弶��Ϣ����
service         		service��sntp�� syslog��ftp��tftp��telnet�� ping
startup         		�����������
tools			���ߣ�������̹���qmi����
application 		app���Խ�Aģ������
cli       			CLI
externsions  		���������ͷ�ļ�
include        		����ͷ�ļ�����
Makefile 			����app��Makefile
os_main.c   		app��main����

README.txt 

����ʹ�ã�

make app ARCH_TYPE=MIPS TSL=true		����TSL��openwrt�汾��CPU�ܹ�Ϊmipsel


����������debugĿ¼
bin		app�����̰�װĿ¼
etc		Ĭ�������ļ�Ŀ¼	
lib		����ľ�̬��
obj		��������ļ�Ŀ¼
sbin		tools���߿�ִ���ļ���װĿ¼




����ģ�飺

make/board.cfg	��Ҫ���幤����������汾��Ϣ
make/module.mk
	��_MODULELIST ����ģ��ȫ�ֱ��뿪��
	��PlatformModule����ģ��Ŀ¼����

make/module-config.mk
	����ģ��Ŀ¼·����Ԥ�����ȵ�



Դ��Ŀ¼ config.mk������Ҫ�����c�ļ��������ƣ�Makefile���밲װ