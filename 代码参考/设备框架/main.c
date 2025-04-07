/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-24     RT-Thread    first version
 */
#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>



int main(void)
{
    rt_device_t dev;
    dev = rt_device_find("demo");
    rt_device_init(dev);
    rt_device_open(dev,RT_DEVICE_OFLAG_RDWR);
    rt_device_close(dev);
    return 0;
}











