#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"
#include "vtysh.h"

DEFSH (VTYSH_ALL, show_time_queues_cmd_vtysh, 
  "show time-queues", 
  "Show running system information\n"
  "Show Time Queues\n")

DEFSH (VTYSH_ALL, password_text_cmd_vtysh, 
  "password LINE", 
  "Assign the terminal connection password\n"
  "The UNENCRYPTED (cleartext) line password\n")

DEFSH (VTYSH_ALL, show_system_cmd_vtysh, 
  "show system", 
  "Show running system information\n"
  "Displays system information\n")

DEFSH (VTYSH_ALL, config_disable_cmd_vtysh, 
  "disable", 
  "Turn off privileged mode command\n")

DEFSH (VTYSH_ALL, no_hostname_cmd_vtysh, 
  "no hostname [HOSTNAME]", 
  "Negate a command or set its defaults\n"
  "Reset system's network name\n"
  "Host name of this router\n")

DEFSH (VTYSH_ALL, config_terminal_cmd_vtysh, 
  "configure terminal", 
  "Configuration from vty interface\n"
  "Configuration terminal\n")

DEFSH (VTYSH_ALL, config_terminal_no_length_cmd_vtysh, 
  "terminal no length", 
  "Set terminal line parameters\n"
  "Negate a command or set its defaults\n"
  "Set number of lines on a screen\n")

DEFSH (VTYSH_ALL, enable_password_cmd_vtysh, 
  "enable password (8|) WORD", 
  "Modify enable password parameters\n"
  "Assign the privileged level password\n"
  "Specifies a HIDDEN password will follow\n"
  "dummy string \n"
  "The HIDDEN 'enable' password string\n")

DEFSH (VTYSH_ALL, copy_runningconfig_startupconfig_cmd_vtysh, 
  "copy running-config startup-config", 
  "Copy configuration\n"
  "Copy running config to... \n"
  "Copy running config to startup config (same as write file)\n")

DEFSH (VTYSH_ALL, no_banner_motd_cmd_vtysh, 
  "no banner motd", 
  "Negate a command or set its defaults\n"
  "Set banner string\n"
  "Strings for motd\n")

DEFSH (VTYSH_ALL, config_terminal_length_cmd_vtysh, 
  "terminal length <0-512>", 
  "Set terminal line parameters\n"
  "Set number of lines on a screen\n"
  "Number of lines on screen (0 for no pausing)\n")

DEFSH (VTYSH_ALL, no_enable_password_cmd_vtysh, 
  "no enable password", 
  "Negate a command or set its defaults\n"
  "Modify enable password parameters\n"
  "Assign the privileged level password\n")

DEFSH (VTYSH_ALL, system_description_cmd_vtysh, 
  "description .LINE", 
  "Set system's description\n"
  "This system's description\n")

DEFSH (VTYSH_ALL, show_version_cmd_vtysh, 
  "show version", 
  "Show running system information\n"
  "Displays version\n")

DEFSH (VTYSH_ALL, service_terminal_length_cmd_vtysh, 
  "service terminal-length <0-512>", 
  "Set up miscellaneous service\n"
  "System wide terminal length configuration\n"
  "Number of lines of VTY (0 means no line control)\n")

DEFSH (VTYSH_ALL, enable_password_text_cmd_vtysh, 
  "enable password LINE", 
  "Modify enable password parameters\n"
  "Assign the privileged level password\n"
  "The UNENCRYPTED (cleartext) 'enable' password\n")

DEFSH (VTYSH_ALL, service_password_encrypt_cmd_vtysh, 
  "service password-encryption", 
  "Set up miscellaneous service\n"
  "Enable encrypted passwords\n")

DEFSH (VTYSH_ALL, banner_motd_default_cmd_vtysh, 
  "banner motd default", 
  "Set banner string\n"
  "Strings for motd\n"
  "Default string\n")

DEFSH (VTYSH_ALL, banner_motd_file_cmd_vtysh, 
  "banner motd file [FILE]", 
  "Set banner\n"
  "Banner for motd\n"
  "Banner from a file\n"
  "Filename\n")

DEFSH (VTYSH_ALL, password_cmd_vtysh, 
  "password (8|) WORD", 
  "Assign the terminal connection password\n"
  "Specifies a HIDDEN password will follow\n"
  "dummy string \n"
  "The HIDDEN line password string\n")

DEFSH (VTYSH_ALL, no_system_description_cmd_vtysh, 
  "no description", 
  "Negate a command or set its defaults\n"
  "system description\n")

DEFSH (VTYSH_ALL, hostname_cmd_vtysh, 
  "hostname WORD", 
  "Set system's network name\n"
  "This system's network name\n")

DEFSH (VTYSH_ALL, no_service_password_encrypt_cmd_vtysh, 
  "no service password-encryption", 
  "Negate a command or set its defaults\n"
  "Set up miscellaneous service\n"
  "Enable encrypted passwords\n")

DEFSH (VTYSH_ALL, show_startup_config_cmd_vtysh, 
  "show startup-config", 
  "Show running system information\n"
  "Contentes of startup configuration\n")

DEFSH (VTYSH_ALL, config_enable_cmd_vtysh, 
  "enable", 
  "Turn on privileged mode command\n")

DEFSH (VTYSH_ALL, no_service_terminal_length_cmd_vtysh, 
  "no service terminal-length [<0-512>]", 
  "Negate a command or set its defaults\n"
  "Set up miscellaneous service\n"
  "System wide terminal length configuration\n"
  "Number of lines of VTY (0 means no line control)\n")

DEFSH (VTYSH_ALL, show_commandtree_cmd_vtysh, 
  "show commandtree", 
  "Show running system information\n"
  "Show command tree\n")

void
vtysh_cmd_init (void)
{
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_service_password_encrypt_cmd_vtysh);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_time_queues_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_ENABLE_LEVEL, &banner_motd_default_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &password_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &system_description_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &enable_password_text_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &service_password_encrypt_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_VIEW_LEVEL, &show_commandtree_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &hostname_cmd_vtysh);
  install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &copy_runningconfig_startupconfig_cmd_vtysh);
  install_element (VIEW_NODE, CMD_CONFIG_LEVEL, &config_terminal_length_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &enable_password_cmd_vtysh);
  install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_startup_config_cmd_vtysh);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_system_cmd_vtysh);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_commandtree_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_service_terminal_length_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &service_terminal_length_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_ENABLE_LEVEL, &no_banner_motd_cmd_vtysh);
  install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &config_terminal_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &password_text_cmd_vtysh);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_version_cmd_vtysh);
  install_element (VIEW_NODE, CMD_CONFIG_LEVEL, &config_terminal_no_length_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_ENABLE_LEVEL, &banner_motd_file_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_system_description_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_enable_password_cmd_vtysh);
  install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &config_disable_cmd_vtysh);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &config_enable_cmd_vtysh);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_hostname_cmd_vtysh);
}
