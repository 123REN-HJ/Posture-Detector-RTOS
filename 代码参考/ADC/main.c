#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <board.h>
#include <rtdevice.h>

rt_adc_device_t dev;
rt_thread_t th1;

void read_adc1_entry(void *parameter)
{
    rt_uint32_t val = 0;
    while(1){
        val = rt_adc_read(dev, 5);
        rt_kprintf("val:%d\n",val);
        rt_thread_mdelay(5000);
    }
}

int main(void)
{
    rt_err_t ret = 0;
    dev = (rt_adc_device_t)rt_device_find("adc1");
    if(dev == RT_NULL){
        LOG_E("rt_device_find (adc1) failed...\n");
        return -EINVAL;
    }

    ret = rt_adc_enable(dev, 5);
    if(ret < 0){
        LOG_E("rt_adc_enable (adc1) failed...\n");
        return ret;
    }

    th1 = rt_thread_create("th1", read_adc1_entry, NULL, 512, 10, 5);
    if(th1 == RT_NULL){
        LOG_E("rt_thread_create failed...\n");
        return -ENOMEM;
    }

    rt_thread_startup(th1);

    return RT_EOK;
}



