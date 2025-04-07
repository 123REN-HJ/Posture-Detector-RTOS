/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-04     11231       the first version
 */
#ifndef APPLICATIONS_W25Q512_W25Q512_H_
#define APPLICATIONS_W25Q512_W25Q512_H_
#include <rtdevice.h>
#include <sys/errno.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include <board.h>
#include <drv_spi.h>
#include <rtthread.h>
#include "dfs_fs.h"
#include <dfs_posix.h>
#include <stdio.h>
#include <string.h>
void stop_record_sd(void);
void start_record_sd(void);
int sd_mnt_init(void);
#endif /* APPLICATIONS_W25Q512_W25Q512_H_ */
