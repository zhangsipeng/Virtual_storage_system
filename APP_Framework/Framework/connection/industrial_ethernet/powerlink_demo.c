#include <xizi.h>
#include <xsconfig.h>

#ifdef POWERLINK_MN
extern int OplkDemoMnConsole(int argc, char *argv[]);

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) |
    SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
    SHELL_CMD_PARAM_NUM(0) |
    SHELL_CMD_DISABLE_RETURN,
    OplkDemoMnConsole,
    OplkDemoMnConsole,
    openPOWERLINK demo MN (console version));
#endif

#ifdef POWERLINK_CN
extern int OplkDemoCnConsole(int argc, char *argv[]);

SHELL_EXPORT_CMD(
    SHELL_CMD_PERMISSION(0) |
    SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
    SHELL_CMD_PARAM_NUM(0) |
    SHELL_CMD_DISABLE_RETURN,
    OplkDemoCnConsole,
    OplkDemoCnConsole,
    openPOWERLINK demo CN (console version));
#endif
