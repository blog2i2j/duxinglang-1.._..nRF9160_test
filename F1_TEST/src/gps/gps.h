/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <nrf_socket.h>
#include <net/socket.h>
#include <stdio.h>

extern bool app_gps_on;
extern bool app_gps_off;

extern void gps_init(void);
extern void GPSMsgProcess(void);
