/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-27     11231       the first version
 */
#ifndef APPLICATIONS_ADC_USR_ADC_H_
#define APPLICATIONS_ADC_USR_ADC_H_
#include <serial.h>
#include <sys/errno.h>
#include <ringbuff.h>
#include <string.h>
#include <stdio.h>
#include <rtdbg.h>
#include <rtdevice.h>
#include <rtthread.h>
rt_err_t adc_init(void);
rt_err_t adc_start(void);
rt_err_t adc_close(void);

#endif /* APPLICATIONS_ADC_USR_ADC_H_ */
