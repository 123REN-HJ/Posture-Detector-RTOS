#include <usr_adc.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <sys/errno.h>
rt_adc_device_t adc_vin_dev;
rt_adc_device_t adc_temp_dev;
rt_thread_t adc_th;

#define ADC_VIN_CHANNEL  10 //PC0
#define ADC_TEMP_CHANNEL 11 //PC1

void read_adc1_entry(void *parameter)
{
    rt_uint32_t vin_val = 0;
    rt_uint32_t temp_val = 0;
    while(1){
        vin_val = rt_adc_read(adc_vin_dev, ADC_VIN_CHANNEL);
        temp_val = rt_adc_read(adc_temp_dev, ADC_TEMP_CHANNEL);
        rt_kprintf("vin_val:%d\n",vin_val);
        rt_kprintf("temp_val:%d\n",temp_val);
        rt_thread_mdelay(100);
    }
}

rt_err_t adc_init(void)
{
    rt_err_t ret = 0;
    adc_vin_dev = (rt_adc_device_t)rt_device_find("adc1");
    if(adc_vin_dev == RT_NULL){
        LOG_E("rt_device_find (adc1) failed...\n");
        return -EINVAL;
    }
    adc_temp_dev = (rt_adc_device_t)rt_device_find("adc1");
        if(adc_temp_dev == RT_NULL){
            LOG_E("rt_device_find (adc1) failed...\n");
            return -EINVAL;
     }
    ret = rt_adc_enable(adc_vin_dev, ADC_VIN_CHANNEL);
    if(ret < 0){
        LOG_E("rt_adc_enable (adc1) failed...\n");
        return ret;
    }
    ret = rt_adc_enable(adc_temp_dev, ADC_TEMP_CHANNEL);
        if(ret < 0){
            LOG_E("rt_adc_enable (adc1) failed...\n");
            return ret;
    }
    LOG_E("ADC_thread_Init OK...\n");


    return 0;
}
rt_err_t adc_start(void)
{
    rt_err_t ret=0;
    adc_th = rt_thread_create("t_adc", read_adc1_entry, NULL, 512, 10, 5);
    if(adc_th == RT_NULL){
        LOG_E("adc_thread_create failed...\n");
        return -ENOMEM;
    }
    ret = rt_thread_startup(adc_th);
    if(ret < 0){
      LOG_E("thread_startup[ADC] failed...\n");
      rt_kprintf("ret : %d\n",ret);
      return ret;
    }
    return RT_EOK;
}
rt_err_t adc_close(void)
{
    rt_err_t ret;
    ret = rt_thread_delete (adc_th);
    if(ret < 0){
      LOG_E("rt_thread_delete[ADC] failed...\n");
      rt_kprintf("ret : %d\n",ret);
      return ret;
    }
    return RT_EOK;
}
