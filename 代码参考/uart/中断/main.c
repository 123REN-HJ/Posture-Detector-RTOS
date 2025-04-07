/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-27     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <serial.h>

rt_device_t u1_dev;
struct rt_semaphore sem;
rt_thread_t u1_th;
struct serial_configure u1_configs = RT_SERIAL_CONFIG_DEFAULT;

rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&sem);
    return RT_EOK;
}

void serial_thread_entry(void *parameter)
{
    char buffer;
    while(1){
        while(rt_device_read(u1_dev, 0, &buffer, 1) != 1){
            rt_sem_take(&sem, RT_WAITING_FOREVER);
        }
        rt_kprintf("%c",buffer);
    }
}

int main(void)
{
    rt_err_t ret = 0;
    u1_dev = rt_device_find("uart1");
    if(u1_dev == RT_NULL){
        LOG_E("rt_device_find[uart1] failed...\n");
        return -EINVAL;
    }

    ret = rt_device_open(u1_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
    if(ret < 0){
        LOG_E("rt_device_open[uart1] failed...\n");
        return ret;
    }

    rt_device_control(u1_dev, RT_DEVICE_CTRL_CONFIG, (void *)&u1_configs);

    rt_device_set_rx_indicate(u1_dev, rx_callback);

    rt_sem_init(&sem,"rx_sem", 0, RT_IPC_FLAG_FIFO);

    u1_th = rt_thread_create("u1_recv",serial_thread_entry , NULL, 1024, 10, 5);
    rt_thread_startup(u1_th);
    rt_device_write(u1_dev, 0, "Uart1 config...\n", rt_strlen("Uart1 config...\n"));

    return RT_EOK;
}
