/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-22     11231       the first version
 */
#ifndef APPLICATIONS_UART4_422_UART4_422_H_
#define APPLICATIONS_UART4_422_UART4_422_H_

#include <serial.h>
#include <sys/errno.h>
#include <ringbuff.h>
#include <string.h>
#include <stdio.h>
#include <rtdbg.h>
#include <rtdevice.h>
#include <rtthread.h>

void uart4_dma_init(void);
void uart4_tx_dma(char* buffer);
#endif /* APPLICATIONS_UART4_422_UART4_422_H_ */
