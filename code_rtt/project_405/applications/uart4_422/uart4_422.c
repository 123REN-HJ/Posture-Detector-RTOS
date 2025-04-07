#include <uart4_422.h>

rt_device_t uart4_dev;
struct serial_configure uart4_configs = RT_SERIAL_CONFIG_DEFAULT;
void uart4_dma_init(void)
{
    rt_err_t ret = 0;
    uart4_dev = rt_device_find("uart4");
     if(uart4_dev == RT_NULL){
         LOG_E("rt_device_find[uart1] failed...\n");
         return -EINVAL;
     }
     /*打开串口*/
     ret = rt_device_open(uart4_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_DMA_TX);
     if(ret < 0){
         LOG_E("rt_device_open[uart4] failed...\n");
         rt_kprintf("ret : %d\n",ret);
     }
     /*配置串口属性*/
     uart4_configs.baud_rate=BAUD_RATE_921600;
     rt_device_control(uart4_dev, RT_DEVICE_CTRL_CONFIG, (void *)&uart4_configs);
}
INIT_BOARD_EXPORT(uart4_dma_init);

void uart4_tx_dma(char* buffer)
{
    rt_device_write(uart4_dev,0,buffer ,strlen(buffer));
}
