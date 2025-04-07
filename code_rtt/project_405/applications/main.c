/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-26     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <board.h>
#include <rtdevice.h>
#include <sys/errno.h>
#include <usr_adc.h>
#include <w25q512.h>
#include <uart4_422.h>
#define USING_JY901 0
#define USING_ADC   0
#define USING_LED   1

/*
HSI和HSE选择在board.h里面设置
 注意每次修改RT-Thread-Setting都会重置rtconfig.h
 修改完之后加上下面自动挂载宏
#define RT_USING_DFS_MNTTABLE
选择控制台
#define RT_CONSOLE_DEVICE_NAME
增大缓存 否则长时间运行会崩
#define RT_SERIAL_RB_BUFSZ 10240
*/

int LED0_PIN;
int LED1_PIN;
int LED2_PIN;
extern int sd_ready;
int main(void)
{
#if USING_LED
    int cnt=0;
    LED0_PIN = rt_pin_get("PC.4");
    LED1_PIN = rt_pin_get("PC.5");
    LED2_PIN = rt_pin_get("PB.0");
    int Power_Pin = rt_pin_get("PB.5");
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(Power_Pin, PIN_MODE_OUTPUT);
    rt_pin_write(LED0_PIN, PIN_HIGH);
    rt_pin_write(LED1_PIN, PIN_LOW);
    rt_pin_write(LED2_PIN, PIN_LOW);
    rt_pin_write(Power_Pin, PIN_HIGH);/*供电打开Pmos*/
#endif

#if USING_JY901
    rt_device_t dev_901;
    dev_901 = rt_device_find("jy901");
    rt_device_init(dev_901);
    rt_device_open(dev_901,RT_DEVICE_OFLAG_OPEN);
#endif

#if USING_ADC
    adc_init();
    adc_start();
    HAL_Delay(5000);
#endif
    /*自动挂载有问题，delay等sd初始化完毕进行手动挂载*/
    rt_thread_mdelay(1000);
    sd_mnt_init();
    if(sd_ready)     rt_pin_write(LED2_PIN, PIN_HIGH);
    while(1)
    {
        /*1S闪烁 监控程序运行心跳*/
        if(cnt%2){
            rt_pin_write(LED0_PIN, PIN_HIGH);
            rt_thread_mdelay(1000);
        }
        else {
            rt_pin_write(LED0_PIN, PIN_LOW);
            rt_thread_mdelay(1000);
        }
        cnt++;
    }

    return RT_EOK;
}


