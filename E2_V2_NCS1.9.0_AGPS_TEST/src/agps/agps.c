/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdio.h>
#include <string.h>
#include <zephyr.h>
#include <nrf_modem_at.h>
#include <nrf_modem_gnss.h>
#include <modem/lte_lc.h>
#include <date_time.h>
#include <logger.h>
#include "agps.h"

#define AGPS_DEBUG

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE) || defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
static struct k_work_q gnss_work_q;

//#define CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND
#define GNSS_WORKQ_THREAD_STACK_SIZE 2304
#define GNSS_WORKQ_THREAD_PRIORITY   5

K_THREAD_STACK_DEFINE(gnss_workq_stack_area, GNSS_WORKQ_THREAD_STACK_SIZE);
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE || CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
#include "assistance.h"

static struct nrf_modem_gnss_agps_data_frame last_agps;
static struct k_work agps_data_get_work;
static volatile bool requesting_assistance;
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE */

#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
static struct k_work_delayable ttff_test_got_fix_work;
static struct k_work_delayable ttff_test_prepare_work;
static struct k_work ttff_test_start_work;
static uint32_t time_to_fix;
static uint32_t time_blocked;
#endif

static const char update_indicator[] = {'\\', '|', '/', '-'};

static struct nrf_modem_gnss_pvt_data_frame last_pvt;
static uint64_t fix_timestamp;

K_MSGQ_DEFINE(nmea_queue, sizeof(struct nrf_modem_gnss_nmea_data_frame *), 10, 4);
static K_SEM_DEFINE(pvt_data_sem, 0, 1);
static K_SEM_DEFINE(time_sem, 0, 1);

static struct k_poll_event events[2] = 
{
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
					K_POLL_MODE_NOTIFY_ONLY,
					&pvt_data_sem, 0),
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
					K_POLL_MODE_NOTIFY_ONLY,
					&nmea_queue, 0),
};

BUILD_ASSERT(IS_ENABLED(CONFIG_LTE_NETWORK_MODE_LTE_M_GPS) ||
	     IS_ENABLED(CONFIG_LTE_NETWORK_MODE_NBIOT_GPS) ||
	     IS_ENABLED(CONFIG_LTE_NETWORK_MODE_LTE_M_NBIOT_GPS),
	     "CONFIG_LTE_NETWORK_MODE_LTE_M_GPS, "
	     "CONFIG_LTE_NETWORK_MODE_NBIOT_GPS or "
	     "CONFIG_LTE_NETWORK_MODE_LTE_M_NBIOT_GPS must be enabled");

void nrf_modem_recoverable_error_handler(uint32_t error)
{
#ifdef AGPS_DEBUG
	LOGD("Modem library recoverable error: %u", error);
#endif
}

static void gnss_event_handler(int event)
{
	int retval;
	struct nrf_modem_gnss_nmea_data_frame *nmea_data;

	switch (event)
	{
	case NRF_MODEM_GNSS_EVT_PVT:
		retval = nrf_modem_gnss_read(&last_pvt, sizeof(last_pvt), NRF_MODEM_GNSS_DATA_PVT);
		if(retval == 0)
		{
			k_sem_give(&pvt_data_sem);
		}
		break;

	#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
	case NRF_MODEM_GNSS_EVT_FIX:
		/* Time to fix is calculated here, but it's printed from a delayed work to avoid
		 * messing up the NMEA output.
		 */
		time_to_fix = (k_uptime_get() - fix_timestamp) / 1000;
		k_work_schedule_for_queue(&gnss_work_q, &ttff_test_got_fix_work, K_MSEC(100));
		k_work_schedule_for_queue(&gnss_work_q, &ttff_test_prepare_work, K_SECONDS(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST_INTERVAL));
		break;
	#endif /* CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */

	case NRF_MODEM_GNSS_EVT_NMEA:
		nmea_data = k_malloc(sizeof(struct nrf_modem_gnss_nmea_data_frame));
		if(nmea_data == NULL)
		{
		#ifdef AGPS_DEBUG
			LOGD("Failed to allocate memory for NMEA");
		#endif
			break;
		}

		retval = nrf_modem_gnss_read(nmea_data,
					     sizeof(struct nrf_modem_gnss_nmea_data_frame),
					     NRF_MODEM_GNSS_DATA_NMEA);
		if(retval == 0)
		{
			retval = k_msgq_put(&nmea_queue, &nmea_data, K_NO_WAIT);
		}

		if(retval != 0)
		{
			k_free(nmea_data);
		}
		break;

	case NRF_MODEM_GNSS_EVT_AGPS_REQ:
	#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
		retval = nrf_modem_gnss_read(&last_agps,
					     sizeof(last_agps),
					     NRF_MODEM_GNSS_DATA_AGPS_REQ);
		if(retval == 0)
		{
			k_work_submit_to_queue(&gnss_work_q, &agps_data_get_work);
		}
	#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE */
		break;

	default:
		break;
	}
}

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
#if defined(CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND)
K_SEM_DEFINE(lte_ready, 0, 1);

static void lte_lc_event_handler(const struct lte_lc_evt *const evt)
{
	switch(evt->type)
	{
	case LTE_LC_EVT_NW_REG_STATUS:
		if((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME)
			|| (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING))
		{
		#ifdef AGPS_DEBUG
			LOGD("Connected to LTE network");
        #endif
			k_sem_give(&lte_ready);
		}
		break;

	default:
		break;
	}
}

void lte_connect(void)
{
	int err;

#ifdef AGPS_DEBUG
	LOGD("Connecting to LTE network");
#endif

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_LTE);
	if(err)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to activate LTE, error: %d", err);
	#endif
		return;
	}

	k_sem_take(&lte_ready, K_FOREVER);

	/* Wait for a while, because with IPv4v6 PDN the IPv6 activation takes a bit more time. */
	k_sleep(K_SECONDS(1));
}

void lte_disconnect(void)
{
	int err;

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE);
	if(err)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to deactivate LTE, error: %d", err);
	#endif
		return;
	}

#ifdef AGPS_DEBUG
	LOGD("LTE disconnected");
#endif
}
#endif /* CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND */

static void agps_data_get_work_fn(struct k_work *item)
{
	ARG_UNUSED(item);

	int err;

#if defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_SUPL)
	/* SUPL doesn't usually provide satellite real time integrity information. If GNSS asks
	 * only for satellite integrity, the request should be ignored.
	 */
	if (last_agps.sv_mask_ephe == 0 
		&& last_agps.sv_mask_alm == 0 
		&& last_agps.data_flags == NRF_MODEM_GNSS_AGPS_INTEGRITY_REQUEST)
	{
	#ifdef AGPS_DEBUG
		LOGD("Ignoring assistance request for only satellite integrity");
	#endif
		return;
	}
#endif/* CONFIG_GNSS_SAMPLE_ASSISTANCE_SUPL */

#if defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_MINIMAL)
	/* With minimal assistance, the request should be ignored if no GPS time or position
	 * is requested.
	 */
	if (!(last_agps.data_flags & NRF_MODEM_GNSS_AGPS_SYS_TIME_AND_SV_TOW_REQUEST)
		&& !(last_agps.data_flags & NRF_MODEM_GNSS_AGPS_POSITION_REQUEST))
	{
	#ifdef AGPS_DEBUG
		LOGD("Ignoring assistance request because no GPS time or position is requested");
	#endif
		return;
	}
#endif /* CONFIG_GNSS_SAMPLE_ASSISTANCE_MINIMAL */

	requesting_assistance = true;

#ifdef AGPS_DEBUG
	LOGD("Assistance data needed, ephe 0x%08x, alm 0x%08x, flags 0x%02x",
		last_agps.sv_mask_ephe,
		last_agps.sv_mask_alm,
		last_agps.data_flags);
#endif

#if defined(CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND)
	lte_connect();
#endif /* CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND */

	err = assistance_request(&last_agps);
	if(err)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to request assistance data");
	#endif
	}

#if defined(CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND)
	lte_disconnect();
#endif /* CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND */

	requesting_assistance = false;
}
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE */

#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
static void ttff_test_got_fix_work_fn(struct k_work *item)
{
#ifdef AGPS_DEBUG
	LOGD("Time to fix: %u", time_to_fix);
#endif
	if(time_blocked > 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Time GNSS was blocked by LTE: %u", time_blocked);
	#endif
	}

#ifdef AGPS_DEBUG
	LOGD("Sleeping for %u seconds", CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST_INTERVAL);
#endif
}

static int ttff_test_force_cold_start(void)
{
	int err;
	uint32_t delete_mask;

#ifdef AGPS_DEBUG
	LOGD("Deleting GNSS data");
#endif

	/* Delete everything else except the TCXO offset. */
	delete_mask = NRF_MODEM_GNSS_DELETE_EPHEMERIDES |
		      NRF_MODEM_GNSS_DELETE_ALMANACS |
		      NRF_MODEM_GNSS_DELETE_IONO_CORRECTION_DATA |
		      NRF_MODEM_GNSS_DELETE_LAST_GOOD_FIX |
		      NRF_MODEM_GNSS_DELETE_GPS_TOW |
		      NRF_MODEM_GNSS_DELETE_GPS_WEEK |
		      NRF_MODEM_GNSS_DELETE_UTC_DATA |
		      NRF_MODEM_GNSS_DELETE_GPS_TOW_PRECISION;

	/* With minimal assistance, we want to keep the factory almanac. */
	if(IS_ENABLED(CONFIG_GNSS_SAMPLE_ASSISTANCE_MINIMAL))
	{
		delete_mask &= ~NRF_MODEM_GNSS_DELETE_ALMANACS;
	}

	err = nrf_modem_gnss_nv_data_delete(delete_mask);
	if(err)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to delete GNSS data");
	#endif
		return -1;
	}

	return 0;
}

static void ttff_test_prepare_work_fn(struct k_work *item)
{
	/* Make sure GNSS is stopped before next start. */
	nrf_modem_gnss_stop();

	if(IS_ENABLED(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST_COLD_START))
	{
		if(ttff_test_force_cold_start() != 0)
		{
			return;
		}
	}

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
	if(IS_ENABLED(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST_COLD_START))
	{
		/* All A-GPS data is always requested before GNSS is started. */
		last_agps.sv_mask_ephe = 0xffffffff;
		last_agps.sv_mask_alm = 0xffffffff;
		last_agps.data_flags =
			NRF_MODEM_GNSS_AGPS_GPS_UTC_REQUEST |
			NRF_MODEM_GNSS_AGPS_KLOBUCHAR_REQUEST |
			NRF_MODEM_GNSS_AGPS_SYS_TIME_AND_SV_TOW_REQUEST |
			NRF_MODEM_GNSS_AGPS_POSITION_REQUEST |
			NRF_MODEM_GNSS_AGPS_INTEGRITY_REQUEST;

		k_work_submit_to_queue(&gnss_work_q, &agps_data_get_work);
	}
	else
	{
		/* Start and stop GNSS to trigger possible A-GPS data request. If new A-GPS
		 * data is needed it is fetched before GNSS is started.
		 */
		nrf_modem_gnss_start();
		nrf_modem_gnss_stop();
	}
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE */

	k_work_submit_to_queue(&gnss_work_q, &ttff_test_start_work);
}

static void ttff_test_start_work_fn(struct k_work *item)
{
#ifdef AGPS_DEBUG
	LOGD("Starting GNSS");
#endif

    if(nrf_modem_gnss_start() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to start GNSS");
	#endif
		return;
	}

	fix_timestamp = k_uptime_get();
	time_blocked = 0;
}
#endif /* CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */

static void date_time_evt_handler(const struct date_time_evt *evt)
{
	k_sem_give(&time_sem);
}

static int modem_init(void)
{
    uint8_t tmpbuf[256] = {0};

    if(nrf_modem_at_cmd(tmpbuf, sizeof(tmpbuf), "AT+CGMR") == 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("MODEM version:%s", &tmpbuf);
	#endif
    }

	if(nrf_modem_at_cmd(tmpbuf, sizeof(tmpbuf), "AT%%HWVERSION") == 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Chip version:%s", &tmpbuf);
	#endif
	}

	if(strlen(CONFIG_GNSS_SAMPLE_AT_MAGPIO) > 0)
	{
		if(nrf_modem_at_printf("%s", CONFIG_GNSS_SAMPLE_AT_MAGPIO) != 0)
		{
		#ifdef AGPS_DEBUG
			LOGD("Failed to set MAGPIO configuration");
		#endif
			return -1;
		}
	}

	if(strlen(CONFIG_GNSS_SAMPLE_AT_COEX0) > 0)
	{
		if(nrf_modem_at_printf("%s", CONFIG_GNSS_SAMPLE_AT_COEX0) != 0)
		{
		#ifdef AGPS_DEBUG
			LOGD("Failed to set COEX0 configuration");
		#endif
			return -1;
		}
	}

	if(IS_ENABLED(CONFIG_DATE_TIME))
	{
		date_time_register_handler(date_time_evt_handler);
	}

	if(lte_lc_init() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to initialize LTE link controller");
	#endif
		return -1;
	}

#if defined(CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND)
	lte_lc_register_handler(lte_lc_event_handler);
#elif !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
	lte_lc_psm_req(true);

#ifdef AGPS_DEBUG
	LOGD("Connecting to LTE network");
#endif

	if(lte_lc_connect() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to connect to LTE network");
	#endif
		return -1;
	}

#ifdef AGPS_DEBUG
	LOGD("Connected to LTE network");
#endif

	uint8_t str[128] = {0};

	if(nrf_modem_at_cmd(str, sizeof(str), "AT%%XSYSTEMMODE?") == 0)
	{
	#ifdef AGPS_DEBUG	
		LOGD("XSYSTEMMODE:%s", str);
	#endif
	}

	if(IS_ENABLED(CONFIG_DATE_TIME))
	{
	#ifdef AGPS_DEBUG
		LOGD("Waiting for current time");
	#endif

		/* Wait for an event from the Date Time library. */
		k_sem_take(&time_sem, K_MINUTES(10));

		if(!date_time_is_valid())
		{
		#ifdef AGPS_DEBUG
			LOGD("Failed to get current time, continuing anyway");
		#endif
		}
	}
#endif

	return 0;
}

static int sample_init(void)
{
	int err = 0;

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE) || defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
	struct k_work_queue_config cfg = {
		.name = "gnss_work_q",
		.no_yield = false
	};

	k_work_queue_start(
		&gnss_work_q,
		gnss_workq_stack_area,
		K_THREAD_STACK_SIZEOF(gnss_workq_stack_area),
		GNSS_WORKQ_THREAD_PRIORITY,
		&cfg);
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE || CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */

#if !defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE)
	k_work_init(&agps_data_get_work, agps_data_get_work_fn);

	err = assistance_init(&gnss_work_q);
#endif /* !CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE */

#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
	k_work_init_delayable(&ttff_test_got_fix_work, ttff_test_got_fix_work_fn);
	k_work_init_delayable(&ttff_test_prepare_work, ttff_test_prepare_work_fn);
	k_work_init(&ttff_test_start_work, ttff_test_start_work_fn);
#endif /* CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */

	return err;
}

static int gnss_init_and_start(void)
{
#if defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE) || defined(CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND)
	/* Enable GNSS. */
	if(lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to activate GNSS functional mode");
	#endif
		return -1;
	}
#endif /* CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE || CONFIG_GNSS_SAMPLE_LTE_ON_DEMAND */

	/* Configure GNSS. */
	if(nrf_modem_gnss_event_handler_set(gnss_event_handler) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS event handler");
	#endif
		return -1;
	}

	/* Enable all supported NMEA messages. */
	uint16_t nmea_mask = NRF_MODEM_GNSS_NMEA_RMC_MASK |
			     NRF_MODEM_GNSS_NMEA_GGA_MASK |
			     NRF_MODEM_GNSS_NMEA_GLL_MASK |
			     NRF_MODEM_GNSS_NMEA_GSA_MASK |
			     NRF_MODEM_GNSS_NMEA_GSV_MASK;

	if(nrf_modem_gnss_nmea_mask_set(nmea_mask) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS NMEA mask");
	#endif
		return -1;
	}

	/* This use case flag should always be set. */
	uint8_t use_case = NRF_MODEM_GNSS_USE_CASE_MULTIPLE_HOT_START;

	if(IS_ENABLED(CONFIG_GNSS_SAMPLE_MODE_PERIODIC) 
		&& !IS_ENABLED(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE))
	{
		/* Disable GNSS scheduled downloads when assistance is used. */
		use_case |= NRF_MODEM_GNSS_USE_CASE_SCHED_DOWNLOAD_DISABLE;
	}

	if(IS_ENABLED(CONFIG_GNSS_SAMPLE_LOW_ACCURACY))
	{
		use_case |= NRF_MODEM_GNSS_USE_CASE_LOW_ACCURACY;
	}

	if(nrf_modem_gnss_use_case_set(use_case) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS use case");
	#endif
	}

#if defined(CONFIG_NRF_CLOUD_AGPS_ELEVATION_MASK)
	if(nrf_modem_gnss_elevation_threshold_set(CONFIG_NRF_CLOUD_AGPS_ELEVATION_MASK) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set elevation threshold");
	#endif
		return -1;
	}
#ifdef AGPS_DEBUG
	LOGD("Set elevation threshold to %u", CONFIG_NRF_CLOUD_AGPS_ELEVATION_MASK);
#endif
#endif

#if defined(CONFIG_GNSS_SAMPLE_MODE_CONTINUOUS)
	/* Default to no power saving. */
	uint8_t power_mode = NRF_MODEM_GNSS_PSM_DISABLED;

#if defined(GNSS_SAMPLE_POWER_SAVING_MODERATE)
	power_mode = NRF_MODEM_GNSS_PSM_DUTY_CYCLING_PERFORMANCE;
#elif defined(GNSS_SAMPLE_POWER_SAVING_HIGH)
	power_mode = NRF_MODEM_GNSS_PSM_DUTY_CYCLING_POWER;
#endif

	if(nrf_modem_gnss_power_mode_set(power_mode) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS power saving mode");
	#endif
		return -1;
	}
#endif /* CONFIG_GNSS_SAMPLE_MODE_CONTINUOUS */

	/* Default to continuous tracking. */
	uint16_t fix_retry = 0;
	uint16_t fix_interval = 1;

#if defined(CONFIG_GNSS_SAMPLE_MODE_PERIODIC)
	fix_retry = CONFIG_GNSS_SAMPLE_PERIODIC_TIMEOUT;
	fix_interval = CONFIG_GNSS_SAMPLE_PERIODIC_INTERVAL;
#elif defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
	/* Single fix for TTFF test mode. */
	fix_retry = 0;
	fix_interval = 0;
#endif

	if(nrf_modem_gnss_fix_retry_set(fix_retry) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS fix retry");
	#endif
		return -1;
	}

	if(nrf_modem_gnss_fix_interval_set(fix_interval) != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to set GNSS fix interval");
	#endif
		return -1;
	}

#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
	k_work_schedule_for_queue(&gnss_work_q, &ttff_test_prepare_work, K_NO_WAIT);
#else /* !CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */
	if(nrf_modem_gnss_start() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to start GNSS");
	#endif
		return -1;
	}
#endif

	return 0;
}

static bool output_paused(void)
{
#if defined(CONFIG_GNSS_SAMPLE_ASSISTANCE_NONE) || defined(CONFIG_GNSS_SAMPLE_LOG_LEVEL_OFF)
	return false;
#else
	return (requesting_assistance || assistance_is_active()) ? true : false;
#endif
}

static void print_satellite_stats(struct nrf_modem_gnss_pvt_data_frame *pvt_data)
{
	uint8_t tracked   = 0;
	uint8_t in_fix    = 0;
	uint8_t unhealthy = 0;

	for(int i = 0; i < NRF_MODEM_GNSS_MAX_SATELLITES; ++i)
	{
		if(pvt_data->sv[i].sv > 0)
		{
			tracked++;

			if(pvt_data->sv[i].flags & NRF_MODEM_GNSS_SV_FLAG_USED_IN_FIX)
			{
				in_fix++;
			}

			if(pvt_data->sv[i].flags & NRF_MODEM_GNSS_SV_FLAG_UNHEALTHY)
			{
				unhealthy++;
			}
		}
	}
#ifdef AGPS_DEBUG
	LOGD("Tracking: %2d Using: %2d Unhealthy: %d\n", tracked, in_fix, unhealthy);
#endif
}

static void print_fix_data(struct nrf_modem_gnss_pvt_data_frame *pvt_data)
{
    uint8_t tmpbuf[256] = {0};
#ifdef AGPS_DEBUG
    int32_t lon, lat;

    lon = pvt_data->longitude*1000000;
    lat = pvt_data->latitude*1000000;
    sprintf(tmpbuf, "Longitude:%d.%06d, Latitude:%d.%06d", lon/1000000, lon%1000000, lat/1000000, lat%1000000);
    LOGD("%s",tmpbuf);

	//LOGD("Latitude:       %.06f\n", pvt_data->latitude);
	//LOGD("Longitude:      %.06f\n", pvt_data->longitude);
	//LOGD("Altitude:       %.01f m\n", pvt_data->altitude);
	//LOGD("Accuracy:       %.01f m\n", pvt_data->accuracy);
	//LOGD("Speed:          %.01f m/s\n", pvt_data->speed);
	//LOGD("Speed accuracy: %.01f m/s\n", pvt_data->speed_accuracy);
	//LOGD("Heading:        %.01f deg\n", pvt_data->heading);
	LOGD("Date:           %04u-%02u-%02u\n",
	       pvt_data->datetime.year,
	       pvt_data->datetime.month,
	       pvt_data->datetime.day);
	LOGD("Time (UTC):     %02u:%02u:%02u.%03u\n",
	       pvt_data->datetime.hour,
	       pvt_data->datetime.minute,
	       pvt_data->datetime.seconds,
	       pvt_data->datetime.ms);
	//LOGD("PDOP:           %.01f\n", pvt_data->pdop);
	//LOGD("HDOP:           %.01f\n", pvt_data->hdop);
	//LOGD("VDOP:           %.01f\n", pvt_data->vdop);
	//LOGD("TDOP:           %.01f\n", pvt_data->tdop);
#endif
}

//int main(void)
void agps_test(void)
{
	uint8_t cnt = 0;
	struct nrf_modem_gnss_nmea_data_frame *nmea_data;

#ifdef AGPS_DEBUG
	LOGD("Starting GNSS sample");
#endif

	if(modem_init() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to initialize modem");
	#endif
		//return -1;
	}

	if(sample_init() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to initialize sample");
	#endif
		//return -1;
	}

	if(gnss_init_and_start() != 0)
	{
	#ifdef AGPS_DEBUG
		LOGD("Failed to initialize and start GNSS");
	#endif
		//return -1;
	}

	fix_timestamp = k_uptime_get();

	for(;;)
	{
		(void)k_poll(events, 2, K_FOREVER);

		if(events[0].state == K_POLL_STATE_SEM_AVAILABLE 
			&& k_sem_take(events[0].sem, K_NO_WAIT) == 0)
		{
			/* New PVT data available */

			if(!IS_ENABLED(CONFIG_GNSS_SAMPLE_NMEA_ONLY) && !output_paused())
			{
			#ifdef AGPS_DEBUG
				LOGD("\033[1;1H");
				LOGD("\033[2J");
			#endif
				print_satellite_stats(&last_pvt);

				if(last_pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_DEADLINE_MISSED)
				{
				#ifdef AGPS_DEBUG
					LOGD("GNSS operation blocked by LTE");
				#endif
				}
				
				if(last_pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_NOT_ENOUGH_WINDOW_TIME)
				{
				#ifdef AGPS_DEBUG
					LOGD("Insufficient GNSS time windows");
				#endif
				}
				
				if(last_pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_SLEEP_BETWEEN_PVT)
				{
				#ifdef AGPS_DEBUG
					LOGD("Sleep period(s) between PVT notifications");
				#endif
				}

				if(last_pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_FIX_VALID)
				{
					fix_timestamp = k_uptime_get();
					print_fix_data(&last_pvt);
				}
				else
				{
				#ifdef AGPS_DEBUG
					LOGD("Seconds since last fix: %lld", (k_uptime_get() - fix_timestamp) / 1000);
				#endif
					cnt++;
				#ifdef AGPS_DEBUG
					LOGD("Searching [%c]", update_indicator[cnt%4]);
				#endif
				}
				
			#ifdef AGPS_DEBUG
				LOGD("NMEA strings:");
			#endif
			}
		#if defined(CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST)
			else
			{
				/* Calculate the time GNSS has been blocked by LTE. */
				if(last_pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_DEADLINE_MISSED)
				{
					time_blocked++;
				}
			}
		#endif /* CONFIG_GNSS_SAMPLE_MODE_TTFF_TEST */
		}
		if(events[1].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE 
			&& k_msgq_get(events[1].msgq, &nmea_data, K_NO_WAIT) == 0)
		{
			/* New NMEA data available */

			if(!output_paused())
			{
			#ifdef AGPS_DEBUG
				LOGD("%s", nmea_data->nmea_str);
			#endif
			}
			k_free(nmea_data);
		}

		events[0].state = K_POLL_STATE_NOT_READY;
		events[1].state = K_POLL_STATE_NOT_READY;
	}

	//return 0;
}
