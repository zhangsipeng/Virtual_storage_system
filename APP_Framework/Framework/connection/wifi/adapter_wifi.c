/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

/**
 * @file adapter_wifiiiii.c
 * @brief Implement the connection wifi adapter function
 * @version 1.1
 * @author AIIT XUOS Lab
 * @date 2021.07.25
 */

#include <adapter.h>
#include "adapter_wifi.h"
#ifdef ADD_XIZI_FETURES
#include <bus_pin.h>
#endif

#ifdef ADAPTER_HFA21_WIFI
extern AdapterProductInfoType Hfa21WifiAttach(struct Adapter *adapter);
#endif
#ifdef ADAPTER_ESP07S_WIFI
extern AdapterProductInfoType Esp07sWifiAttach(struct Adapter *adapter);
#endif

static int AdapterWifiRegister(struct Adapter *adapter)
{
    int ret = 0;

    strncpy(adapter->name, ADAPTER_WIFI_NAME, NAME_NUM_MAX);

    adapter->net_protocol = IP_PROTOCOL;
    adapter->adapter_status = UNREGISTERED;

    ret = AdapterDeviceRegister(adapter);
    if (ret < 0) {
        printf("AdapterWifi register error\n");
        return -1;
    }

    return ret;
}

int AdapterWifiInit(void)
{
    int ret = 0;

    struct Adapter *adapter = PrivMalloc(sizeof(struct Adapter));
    if (!adapter) {
        printf("AdapterWifiInit malloc error\n");
        PrivFree(adapter);
        return -1;
    }

    memset(adapter, 0, sizeof(struct Adapter));

    ret = AdapterWifiRegister(adapter);
    if (ret < 0) {
        printf("AdapterWifiInit register wifi adapter error\n");
        PrivFree(adapter);
        return -1;
    }

#ifdef ADAPTER_HFA21_WIFI
    AdapterProductInfoType product_info = Hfa21WifiAttach(adapter);
    if (!product_info) {
        printf("AdapterWifiInit hfa21 attach error\n");
        PrivFree(adapter);
        return -1;
    }

    adapter->product_info_flag = 1;
    adapter->info = product_info;
    adapter->done = product_info->model_done;

#endif
#ifdef ADAPTER_ESP07S_WIFI
    AdapterProductInfoType product_info = Esp07sWifiAttach(adapter);
    if (!product_info) {
        printf("AdapterWifiInit ESP07S attach error\n");
        PrivFree(adapter);
        return -1;
    }

    adapter->product_info_flag = 1;
    adapter->info = product_info;
    adapter->done = product_info->model_done;

#endif

    return ret;
}

/******************wifi TEST*********************/
#ifdef ADD_XIZI_FETURES
int AdapterWifiTest(void)
{
    char cmd[64];
    int baud_rate = BAUD_RATE_57600;

    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);

#ifdef ADAPTER_HFA21_DRIVER_EXT_PORT
    static BusType ch438_pin;
    ch438_pin = PinBusInitGet();
    struct PinParam pin_cfg;
    int ret = 0;

    struct BusConfigureInfo configure_info;
    configure_info.configure_cmd = OPE_CFG;
    configure_info.private_data = (void *)&pin_cfg;

    pin_cfg.cmd = GPIO_CONFIG_MODE;
    pin_cfg.pin = 22;
    pin_cfg.mode = GPIO_CFG_OUTPUT;

    ret = BusDrvConfigure(ch438_pin->owner_driver, &configure_info);

    struct PinStat pin_stat;
    struct BusBlockWriteParam write_param;
    struct BusBlockReadParam read_param;
    write_param.buffer = (void *)&pin_stat;

    pin_stat.val = GPIO_HIGH;

    pin_stat.pin = 22;
    BusDevWriteData(ch438_pin->owner_haldev, &write_param);

    int pin_fd;
    pin_fd = PrivOpen("/dev/pin_dev", O_RDWR);

    struct PinParam pin_param;
    pin_param.cmd = GPIO_CONFIG_MODE;
    pin_param.mode = GPIO_CFG_OUTPUT;
    pin_param.pin = 22;

    struct PrivIoctlCfg ioctl_cfg;
    ioctl_cfg.ioctl_driver_type = PIN_TYPE;
    ioctl_cfg.args = &pin_param;
    PrivIoctl(pin_fd, OPE_CFG, &ioctl_cfg);

    struct PinStat pin_stat;
    pin_stat.pin = 52;
    pin_stat.val = GPIO_HIGH;
    PrivWrite(pin_fd, &pin_stat, 1);

    PrivClose(pin_fd);
#endif

    AdapterDeviceOpen(adapter);
    // AdapterDeviceControl(adapter, OPE_INT, &baud_rate);

    AdapterDeviceSetUp(adapter);
    AdapterDeviceSetAddr(adapter, "192.168.64.253", "192.168.66.1", "255.255.252.0");
    AdapterDevicePing(adapter, "36.152.44.95");
    AdapterDeviceNetstat(adapter);

    const char *ip = "192.168.64.60";
    const char *port = "12345";
    enum NetRoleType net_role = CLIENT;
    enum IpType ip_type = IPV4;
    AdapterDeviceConnect(adapter, net_role, ip, port, ip_type);

    const char *wifi_msg = "Wifi Test";
    int len = strlen(wifi_msg);
    for(int i = 0;i < 10; ++i) {
        AdapterDeviceSend(adapter, wifi_msg, len);
        PrivTaskDelay(4000);
    }

    char wifi_recv_msg[128];
    for(int j=0;j<10;++j){
        AdapterDeviceRecv(adapter, wifi_recv_msg, 128);
        PrivTaskDelay(1000);
    }
}
#endif

#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(AdapterWifiTest,a wifi adpter sample);
#endif
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, AdapterWifiTest, AdapterWifiTest, show adapter wifi information);
#endif

int wifiopen(void)
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    return AdapterDeviceOpen(adapter);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, wifiopen, wifiopen, open adapter wifi );
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifiopen,a wifi adpter sample);
#endif
int wificlose(void)
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    return AdapterDeviceClose(adapter);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC)|SHELL_CMD_PARAM_NUM(0)|SHELL_CMD_DISABLE_RETURN, wificlose, wificlose, close adapter wifi );
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wificlose,a wifi adpter sample);
#endif
int wifisetup(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    struct WifiParam param;
    memset(&param,0,sizeof(struct WifiParam));
    strncpy((char *)param.wifi_ssid, argv[1], strlen(argv[1]));
    strncpy((char *)param.wifi_pwd, argv[2], strlen(argv[2]));

    adapter->adapter_param = &param;

    return AdapterDeviceSetUp(adapter);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(3)|SHELL_CMD_DISABLE_RETURN, wifisetup, wifisetup, setup adapter wifi );
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifisetup,a wifi adpter sample:wifisetup <server|client>);
#endif
int wifiaddrset(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    char *ip = argv[1];
    char *gateway = argv[2];
    char *netmask = argv[3];

    AdapterDeviceSetAddr(adapter, ip, gateway, netmask);
    AdapterDevicePing(adapter, "36.152.44.95");///< ping www.baidu.com
    return AdapterDeviceNetstat(adapter);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(4)|SHELL_CMD_DISABLE_RETURN, wifiaddrset, wifiaddrset, addrset adapter wifi);
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifiaddrset,a wifi adpter sample:wifiaddrset <server|client>);
#endif
int wifiping(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    printf("ping %s\n",argv[1]);
    return AdapterDevicePing(adapter, argv[1]);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(3), wifiping, wifiping, wifiping adapter );
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifiping,a wifi adpter sample:wifiping <server|client>);
#endif
int wificonnect(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    char *ip = argv[1];
    char *port = argv[2];
    enum NetRoleType net_role = CLIENT;
    enum IpType ip_type = IPV4;

    if(0 == strncmp("tcp",argv[3],strlen("tcp"))) {
        adapter->socket.protocal = SOCKET_PROTOCOL_TCP;
    }

    if(0 == strncmp("udp",argv[3],strlen("udp"))) {
        adapter->socket.protocal = SOCKET_PROTOCOL_UDP;
    }

    return AdapterDeviceConnect(adapter, net_role, ip, port, ip_type);
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(4)|SHELL_CMD_DISABLE_RETURN, wificonnect, wificonnect, wificonnect adapter);
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wificonnect,a wifi adpter sample:wificonnect <server|client>);
#endif
int wifisend(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);

    const char *wifi_msg = argv[1];
    int len = strlen(wifi_msg);
    for(int i = 0;i < 10; ++i) {
        AdapterDeviceSend(adapter, wifi_msg, len);
        PrivTaskDelay(1000);
    }
    return 0;
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(3)|SHELL_CMD_DISABLE_RETURN, wifisend, wifisend, wifisend adapter wifi information);
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifisend,a wifi adpter sample:wifisend <server|client>);
#endif
int wifirecv(int argc, char *argv[])
{
    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);

    char wifi_recv_msg[128];
    while (1) {
        AdapterDeviceRecv(adapter, wifi_recv_msg, 128);
        PrivTaskDelay(1000);
        printf("wifi recv [%s]\n",wifi_recv_msg);
    }
}
#ifdef ADD_XIZI_FETURES
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_PARAM_NUM(3)|SHELL_CMD_DISABLE_RETURN, wifirecv, wifirecv, wifirecv adapter wifi information);
#endif
#ifdef ADD_RTTHREAD_FETURES
MSH_CMD_EXPORT(wifirecv,a wifi adpter sample:wifirecv <server|client>);
#endif

#ifdef ADD_NUTTX_FETURES

enum
{
    APT_WIFI_PARAM_IP,
    APT_WIFI_PARAM_PORT,
    APT_WIFI_PARAM_SSID,
    APT_WIFI_PARAM_PWD,
    APT_WIFI_PARAM_GW,
    APT_WIFI_PARAM_SERVER,
    APT_WIFI_PARAM_MASK,
    APT_WIFI_PARAM_PING,
    APT_WIFI_PARAM_NUM
};

#define APT_WIFI_PARAM_LEN 20

char wifi_param[APT_WIFI_PARAM_NUM][APT_WIFI_PARAM_LEN] = {0};

#define CHECK_RET(__func) \
ret = __func; \
if(ret != 0){ \
    printf("%s %d failed\n", __func__, __LINE__); \
    AdapterDeviceClose(adapter); \
    return ret; \
};

void AdapterWifiGetParam(int argc, char *argv[])
{
    int i, j;
    char *param_str[] = {"ip", "port", "ssid", "pwd", "gw", "server", "mask", "ping"};
    char *default_str[] =
    {"192.168.137.34", "12345", "test", "tttttttt", "192.168.137.71", "192.168.137.1", "255.255.255.0", "220.181.38.251"};

    for(i = 0; i < APT_WIFI_PARAM_NUM; i ++)
    {
        memset(wifi_param[i], 0, APT_WIFI_PARAM_LEN);
        strcpy(wifi_param[i], default_str[i]);
    }

    for(i = 0; i < argc; i ++)
    {
        for(j = 0; j < APT_WIFI_PARAM_NUM; j ++)
        {
            if(strncmp(argv[i], param_str[j], strlen(param_str[j])) == 0)
            {
                printf("wifi %d: %s\n", j, argv[i] + strlen(param_str[j]) + 1);
                strcpy(wifi_param[j], argv[i] + strlen(param_str[j]) + 1);
            }
        }
    }

    printf("--- wifi parameter ---\n");
    for(i = 0; i < APT_WIFI_PARAM_NUM; i ++)
    {
        printf("%7.7s = %s\n", param_str[i], wifi_param[i]);
    }
    printf("----------------------\n");
}


int AdapterWifiTest(int argc, char *argv[])
{
    int i, ret;

    struct Adapter* adapter =  AdapterDeviceFindByName(ADAPTER_WIFI_NAME);
    AdapterWifiGetParam(argc, argv);

    enum NetRoleType net_role = CLIENT;
    enum IpType ip_type = IPV4;
    struct WifiParam param;
    memset(&param, 0, sizeof(struct WifiParam));
    strncpy((char *)param.wifi_ssid, wifi_param[APT_WIFI_PARAM_SSID], strlen(wifi_param[APT_WIFI_PARAM_SSID]));
    strncpy((char *)param.wifi_pwd, wifi_param[APT_WIFI_PARAM_PWD], strlen(wifi_param[APT_WIFI_PARAM_PWD]));

    adapter->adapter_param = &param;

    CHECK_RET(AdapterDeviceOpen(adapter));
    CHECK_RET(AdapterDeviceSetUp(adapter));

    CHECK_RET(AdapterDeviceSetAddr(adapter, wifi_param[APT_WIFI_PARAM_IP], wifi_param[APT_WIFI_PARAM_GW],
        wifi_param[APT_WIFI_PARAM_MASK]));

    CHECK_RET(AdapterDeviceNetstat(adapter));

    adapter->socket.protocal = SOCKET_PROTOCOL_TCP;
    CHECK_RET(AdapterDeviceConnect(adapter, net_role, wifi_param[APT_WIFI_PARAM_SERVER],
        wifi_param[APT_WIFI_PARAM_PORT], ip_type));

    const char *wifi_msg = "Wifi Test";
    for(i = 0; i < 10; i++)
    {
        AdapterDeviceSend(adapter, wifi_msg, strlen(wifi_msg));
        PrivTaskDelay(4000);
    }

    char wifi_recv_msg[128];
    for(i = 0; i < 10; i ++)
    {
        AdapterDeviceRecv(adapter, wifi_recv_msg, 128);
        PrivTaskDelay(1000);
    }

//    printf("ping %s\n", wifi_param[APT_WIFI_PARAM_PING]);
//
//    CHECK_RET(AdapterDevicePing(adapter, wifi_param[APT_WIFI_PARAM_PING]));
//    AdapterDeviceDisconnect(adapter, NULL);
    ret = AdapterDeviceClose(adapter);
    return ret;
}

#endif

