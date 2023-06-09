#include <rtthread.h>
/*
RW007(SPI1 ) Kendryte Sipeed MAX bit io

 pin numbers    function    pin number on rw007 board
  io 15	        SPI1_SCK 	    	SCK    
  io 8 	        SPI1_MISO	    	MISO   
  io 10	        SPI1_MOSI	    	MOSI   
  io 9 	        CS/BOOT1 	    	CS     
  io 6 	        INT/BUSY 	    	D9     
  io 7 	        RESET    	    	D8     

 set in mencuconfig after suring hardware well
board driver config-> enble spi1
(15) spi1 clk pin number
(10) spi1 d0 pin number
(8)  spi1 d1 pin number
(*)  spi1 enble ss1 --->
     (9) spi1 ss1 pin number
verified

*/


#ifdef PKG_USING_RW007
#include <rtdevice.h>
#include <drv_spi.h>
#include <board.h>
#include <spi_wifi_rw007.h>

extern void spi_wifi_isr(int vector);

static int rw007_gpio_init(void)
{
    int cnt = 15;

    /* Configure IO */
    rt_pin_mode(RW007_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLDOWN);

    /* Reset rw007 and config mode */
    rt_pin_write(RW007_RST_PIN, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(RW007_RST_PIN, PIN_HIGH);

    /* Wait rw007 ready(exit busy stat) */
    while(!rt_pin_read(RW007_INT_BUSY_PIN))
    {
        rt_thread_mdelay(100);
        if (--cnt < 0)
            return -1;
    }

    rt_thread_mdelay(200);
    rt_pin_mode(RW007_INT_BUSY_PIN, PIN_MODE_INPUT_PULLUP);

    return 0;
}

int wifi_spi_device_init(void)
{
    char sn_version[32] = {0};

    if (rw007_gpio_init() != 0)
    {
        rt_kprintf("rw007 wait busy timeout\n");
        return -1;
    }

    rt_hw_wifi_init(RW007_SPIDEV_NAME);

    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    rw007_sn_get(sn_version);
    rt_kprintf("\nrw007  sn: [%s]\n", sn_version);
    rw007_version_get(sn_version);
    rt_kprintf("rw007 ver: [%s]\n\n", sn_version);

    return 0;
}
INIT_APP_EXPORT(wifi_spi_device_init);

static void int_wifi_irq(void * p)
{
    ((void)p);
    spi_wifi_isr(0);
}

void spi_wifi_hw_init(void)
{
    rt_pin_attach_irq(RW007_INT_BUSY_PIN, PIN_IRQ_MODE_FALLING, int_wifi_irq, 0);
    rt_pin_irq_enable(RW007_INT_BUSY_PIN, RT_TRUE);
}

#endif /* RW007_USING_STM32_DRIVERS */
