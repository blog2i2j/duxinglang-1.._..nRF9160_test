/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <stdio.h>
#include <drivers/uart.h>
#include <string.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <dk_buttons_and_leds.h>
#include "lcd.h"
#include "font.h"
#include "settings.h"
#include "Max20353.h"

#include <net/mqtt.h>
#include <net/socket.h>
#include <modem/lte_lc.h>
#if defined(CONFIG_LWM2M_CARRIER)
#include <lwm2m_carrier.h>
#endif

//add by liming
#if defined(CONFIG_MQTT_LIB_TLS)
static sec_tag_t sec_tag_list[] = { CONFIG_SEC_TAG };
#endif /* defined(CONFIG_MQTT_LIB_TLS) */ 

/* Buffers for MQTT client. */
static u8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

/* The mqtt client struct */
static struct mqtt_client client;

/* MQTT Broker details. */
static struct sockaddr_storage broker;

/* Connected flag */
static bool connected;

/* File descriptor */
static struct pollfd fds;

static u8_t tmpbuf[128] = {0};

bool lcd_sleep_in = false;
bool lcd_sleep_out = false;
bool sys_pwr_off = false;
bool sys_time_count = false;
bool show_date_time_first = true;
bool update_time = false;
bool update_date = false;
bool update_week = false;
bool update_date_time = false;

//#define TEST_BNT_LED   //liming

void show_infor(u8_t *strbuf)
{
	LCD_Fill(20,90,200,50,BLACK);
	LCD_ShowStringInRect(20,90,200,50,strbuf);
}

#if defined(CONFIG_BSD_LIBRARY)

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
	printk("bsdlib recoverable error: %u\n", (unsigned int)err);
	sprintf(tmpbuf, "bsdlib recoverable error: %u\n", (unsigned int)err);
	show_infor(tmpbuf);
}

#endif /* defined(CONFIG_BSD_LIBRARY) */

#if defined(CONFIG_LWM2M_CARRIER)
K_SEM_DEFINE(carrier_registered, 0, 1);

void lwm2m_carrier_event_handler(const lwm2m_carrier_event_t *event)
{
	switch (event->type) {
	case LWM2M_CARRIER_EVENT_BSDLIB_INIT:
		printk("LWM2M_CARRIER_EVENT_BSDLIB_INIT\n");
		show_infor("LWM2M_CARRIER_EVENT_BSDLIB_INIT");
		break;
	case LWM2M_CARRIER_EVENT_CONNECT:
		printk("LWM2M_CARRIER_EVENT_CONNECT\n");
		show_infor("LWM2M_CARRIER_EVENT_CONNECT");
		break;
	case LWM2M_CARRIER_EVENT_DISCONNECT:
		printk("LWM2M_CARRIER_EVENT_DISCONNECT\n");
		show_infor("LWM2M_CARRIER_EVENT_DISCONNECT");
		break;
	case LWM2M_CARRIER_EVENT_READY:
		printk("LWM2M_CARRIER_EVENT_READY\n");
		show_infor("LWM2M_CARRIER_EVENT_READY");
		k_sem_give(&carrier_registered);
		break;
	case LWM2M_CARRIER_EVENT_FOTA_START:
		printk("LWM2M_CARRIER_EVENT_FOTA_START\n");
		show_infor("LWM2M_CARRIER_EVENT_FOTA_START");
		break;
	case LWM2M_CARRIER_EVENT_REBOOT:
		printk("LWM2M_CARRIER_EVENT_REBOOT\n");
		show_infor("LWM2M_CARRIER_EVENT_REBOOT");
		break;
	}
}
#endif /* defined(CONFIG_LWM2M_CARRIER) */

/**@brief Function to print strings without null-termination
 */
static void data_print(u8_t *prefix, u8_t *data, size_t len)
{
	char buf[len + 1];

	memcpy(buf, data, len);
	buf[len] = 0;
	printk("%s%s\n", prefix, buf);
}

/**@brief Function to publish data on the configured topic
 */
static int data_publish(struct mqtt_client *c, enum mqtt_qos qos,
	u8_t *data, size_t len)
{
	struct mqtt_publish_param param;

	param.message.topic.qos = qos;
	param.message.topic.topic.utf8 = CONFIG_MQTT_PUB_TOPIC;
	param.message.topic.topic.size = strlen(CONFIG_MQTT_PUB_TOPIC);
	param.message.payload.data = data;
	param.message.payload.len = len;
	param.message_id = sys_rand32_get();
	param.dup_flag = 0;
	param.retain_flag = 0;

	data_print("Publishing: ", data, len);
	printk("to topic: %s len: %u\n",
		CONFIG_MQTT_PUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_PUB_TOPIC));
	sprintf(tmpbuf, "to topic: %s len: %u",
		CONFIG_MQTT_PUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_PUB_TOPIC));
	show_infor(tmpbuf);

	return mqtt_publish(c, &param);
}

/**@brief Function to subscribe to the configured topic
 */
static int subscribe(void)
{
	struct mqtt_topic subscribe_topic = {
		.topic = {
			.utf8 = CONFIG_MQTT_SUB_TOPIC,
			.size = strlen(CONFIG_MQTT_SUB_TOPIC)
		},
		.qos = MQTT_QOS_1_AT_LEAST_ONCE
	};

	const struct mqtt_subscription_list subscription_list = {
		.list = &subscribe_topic,
		.list_count = 1,
		.message_id = 1234
	};

	printk("Subscribing to: %s len %u\n", CONFIG_MQTT_SUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_SUB_TOPIC));
	sprintf(tmpbuf, "Subscribing to: %s len %u", CONFIG_MQTT_SUB_TOPIC,
		(unsigned int)strlen(CONFIG_MQTT_SUB_TOPIC));
	show_infor(tmpbuf);

	return mqtt_subscribe(&client, &subscription_list);
}

/**@brief Function to read the published payload.
 */
static int publish_get_payload(struct mqtt_client *c, size_t length)
{
	u8_t *buf = payload_buf;
	u8_t *end = buf + length;

	if (length > sizeof(payload_buf)) {
		return -EMSGSIZE;
	}

	while (buf < end) {
		int ret = mqtt_read_publish_payload(c, buf, end - buf);

		if (ret < 0) {
			int err;

			if (ret != -EAGAIN) {
				return ret;
			}

			printk("mqtt_read_publish_payload: EAGAIN\n");
			show_infor("mqtt_read_publish_payload: EAGAIN");

			err = poll(&fds, 1,
				   CONFIG_MQTT_KEEPALIVE * MSEC_PER_SEC);
			if (err > 0 && (fds.revents & POLLIN) == POLLIN) {
				continue;
			} else {
				return -EIO;
			}
		}

		if (ret == 0) {
			return -EIO;
		}

		buf += ret;
	}

	return 0;
}

/**@brief MQTT client event handler
 */
void mqtt_evt_handler(struct mqtt_client *const c,
		      const struct mqtt_evt *evt)
{
	int err;

	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result != 0) {
			printk("MQTT connect failed %d\n", evt->result);
			sprintf(tmpbuf, "MQTT connect failed %d", evt->result);
			show_infor(tmpbuf);
			break;
		}

		connected = true;
		printk("[%s:%d] MQTT client connected!\n", __func__, __LINE__);
		sprintf(tmpbuf, "[%s:%d] MQTT client connected!", __func__, __LINE__);
		show_infor(tmpbuf);
		subscribe();
		break;

	case MQTT_EVT_DISCONNECT:
		printk("[%s:%d] MQTT client disconnected %d\n", __func__,
		       __LINE__, evt->result);
		sprintf(tmpbuf, "[%s:%d] MQTT client disconnected %d", __func__,
		       __LINE__, evt->result);
		show_infor(tmpbuf);

		connected = false;
		break;

	case MQTT_EVT_PUBLISH: {
		const struct mqtt_publish_param *p = &evt->param.publish;

		printk("[%s:%d] MQTT PUBLISH result=%d len=%d\n", __func__,
		       __LINE__, evt->result, p->message.payload.len);
		sprintf(tmpbuf, "[%s:%d] MQTT PUBLISH result=%d len=%d", __func__,
		       __LINE__, evt->result, p->message.payload.len);
		show_infor(tmpbuf);
		
		err = publish_get_payload(c, p->message.payload.len);
		if (err >= 0) {
			data_print("Received: ", payload_buf,
				p->message.payload.len);
			/* Echo back received data */
			data_publish(&client, MQTT_QOS_1_AT_LEAST_ONCE,
				payload_buf, p->message.payload.len);
		} else {
			printk("mqtt_read_publish_payload: Failed! %d\n", err);
			printk("Disconnecting MQTT client...\n");
			sprintf(tmpbuf, "mqtt_read_publish_payload: Failed! %d", err);
			show_infor(tmpbuf);
			show_infor("Disconnecting MQTT client...");

			err = mqtt_disconnect(c);
			if (err) {
				printk("Could not disconnect: %d\n", err);
				sprintf(tmpbuf, "Could not disconnect: %d", err);
				show_infor(tmpbuf);	
			}
		}
	} break;

	case MQTT_EVT_PUBACK:
		if (evt->result != 0) {
			printk("MQTT PUBACK error %d\n", evt->result);
			sprintf(tmpbuf, "MQTT PUBACK error %d", evt->result);
			show_infor(tmpbuf);			
			break;
		}

		printk("[%s:%d] PUBACK packet id: %u\n", __func__, __LINE__,
				evt->param.puback.message_id);
		sprintf(tmpbuf, "[%s:%d] PUBACK packet id: %u", __func__, __LINE__,
				evt->param.puback.message_id);
		show_infor(tmpbuf);
		break;

	case MQTT_EVT_SUBACK:
		if (evt->result != 0) {
			printk("MQTT SUBACK error %d\n", evt->result);
			sprintf(tmpbuf, "MQTT SUBACK error %d", evt->result);
			show_infor(tmpbuf);	
			break;
		}

		printk("[%s:%d] SUBACK packet id: %u\n", __func__, __LINE__,
				evt->param.suback.message_id);
		sprintf(tmpbuf, "[%s:%d] SUBACK packet id: %u", __func__, __LINE__,
				evt->param.suback.message_id);
		show_infor(tmpbuf);			
		break;

	default:
		printk("[%s:%d] default: %d\n", __func__, __LINE__,
				evt->type);
		sprintf(tmpbuf, "[%s:%d] default: %d", __func__, __LINE__,
				evt->type);
		show_infor(tmpbuf);			
		break;
	}
}

/**@brief Resolves the configured hostname and
 * initializes the MQTT broker structure
 */
static void broker_init(void)
{
	int err;
	struct addrinfo *result;
	struct addrinfo *addr;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM
	};

	err = getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, NULL, &hints, &result);
	if (err) {
		printk("ERROR: getaddrinfo failed %d\n", err);
		sprintf(tmpbuf, "ERROR: getaddrinfo failed %d", err);
		show_infor(tmpbuf);	

		return;
	}

	addr = result;
	err = -ENOENT;

	/* Look for address of the broker. */
	while (addr != NULL) {
		/* IPv4 Address. */
		if (addr->ai_addrlen == sizeof(struct sockaddr_in)) {
			struct sockaddr_in *broker4 =
				((struct sockaddr_in *)&broker);
			char ipv4_addr[NET_IPV4_ADDR_LEN];

			broker4->sin_addr.s_addr =
				((struct sockaddr_in *)addr->ai_addr)
				->sin_addr.s_addr;
			broker4->sin_family = AF_INET;
			broker4->sin_port = htons(CONFIG_MQTT_BROKER_PORT);

			inet_ntop(AF_INET, &broker4->sin_addr.s_addr,
				  ipv4_addr, sizeof(ipv4_addr));
			printk("IPv4 Address found %s\n", ipv4_addr);
			sprintf(tmpbuf, "IPv4 Address found %s", ipv4_addr);
			show_infor(tmpbuf);		

			break;
		} else {
			printk("ai_addrlen = %u should be %u or %u\n",
				(unsigned int)addr->ai_addrlen,
				(unsigned int)sizeof(struct sockaddr_in),
				(unsigned int)sizeof(struct sockaddr_in6));
			sprintf(tmpbuf, "ai_addrlen = %u should be %u or %u",
				(unsigned int)addr->ai_addrlen,
				(unsigned int)sizeof(struct sockaddr_in),
				(unsigned int)sizeof(struct sockaddr_in6));
			show_infor(tmpbuf);	
		}

		addr = addr->ai_next;
		break;
	}

	/* Free the address. */
	freeaddrinfo(result);
}

/**@brief Initialize the MQTT client structure
 */
static void client_init(struct mqtt_client *client)
{
	mqtt_client_init(client);

	broker_init();

	/* MQTT client configuration */
	client->broker = &broker;
	client->evt_cb = mqtt_evt_handler;
	client->client_id.utf8 = (u8_t *)CONFIG_MQTT_CLIENT_ID;
	client->client_id.size = strlen(CONFIG_MQTT_CLIENT_ID);
	client->password = NULL;
	client->user_name = NULL;
	client->protocol_version = MQTT_VERSION_3_1_1;

	/* MQTT buffers configuration */
	client->rx_buf = rx_buffer;
	client->rx_buf_size = sizeof(rx_buffer);
	client->tx_buf = tx_buffer;
	client->tx_buf_size = sizeof(tx_buffer);
/* MQTT transport configuration */  //add by liming
#if defined(CONFIG_MQTT_LIB_TLS)
    struct mqtt_sec_config *tls_config = &client->transport.tls.config;
    
    client->transport.type = MQTT_TRANSPORT_SECURE;
    
    tls_config->peer_verify = CONFIG_PEER_VERIFY;
    tls_config->cipher_count = 0;
    tls_config->cipher_list = NULL;
    tls_config->sec_tag_count = ARRAY_SIZE(sec_tag_list);
    tls_config->sec_tag_list = sec_tag_list;
    tls_config->hostname = CONFIG_MQTT_BROKER_HOSTNAME;
#else /* MQTT transport configuration */
    client->transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif /* defined(CONFIG_MQTT_LIB_TLS) */

}

/**@brief Initialize the file descriptor structure used by poll.
 */
static int fds_init(struct mqtt_client *c)
{
	if (c->transport.type == MQTT_TRANSPORT_NON_SECURE) {
		fds.fd = c->transport.tcp.sock;
	} else {
#if defined(CONFIG_MQTT_LIB_TLS)
		fds.fd = c->transport.tls.sock;
#else
		return -ENOTSUP;
#endif
	}

	fds.events = POLLIN;

	return 0;
}

/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
static void modem_configure(void)
{
	show_infor("modem_configure");
#if defined(CONFIG_LTE_LINK_CONTROL)
	if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT)) {
		/* Do nothing, modem is already turned on
		 * and connected.
		 */
	} else {
#if defined(CONFIG_LWM2M_CARRIER)
		/* Wait for the LWM2M_CARRIER to configure the modem and
		 * start the connection.
		 */
		printk("Waitng for carrier registration...\n");
		show_infor("Waitng for carrier registration...");
		k_sem_take(&carrier_registered, K_FOREVER);
		printk("Registered!\n");
		show_infor("Registered!");
#else /* defined(CONFIG_LWM2M_CARRIER) */
		int err;

		printk("LTE Link Connecting ...\n");
		show_infor("LTE Link Connecting ...");
		err = lte_lc_init_and_connect();
		__ASSERT(err == 0, "LTE link could not be established.");
		printk("LTE Link Connected!\n");
		show_infor("LTE Link Connected!");
#endif /* defined(CONFIG_LWM2M_CARRIER) */
	}
#endif /* defined(CONFIG_LTE_LINK_CONTROL) */
}

static void buttons_leds_init(void)
{
	int err;

	err = dk_leds_init();
	if (err)
	{
		printk("Could not initialize leds, err code: %d\n", err);
	}

	err = dk_set_leds_state(0x00, DK_ALL_LEDS_MSK);
	if (err)
	{
		printk("Could not set leds state, err code: %d\n", err);
	}
}

void system_init(void)
{
	InitSystemSettings();
	buttons_leds_init();
	key_init();
	pmu_init();
	LCD_Init();
}

void main(void)
{
	int err;

	printk("The MQTT simple sample started\n");

	system_init();

	show_infor("NB-Test start!");
	
	modem_configure();

	client_init(&client);

	err = mqtt_connect(&client);
	if (err != 0) {
		printk("ERROR: mqtt_connect %d\n", err);
		sprintf(tmpbuf, "ERROR: mqtt_connect %d", err);
		show_infor(tmpbuf);	
		return;
	}

	err = fds_init(&client);
	if (err != 0) {
		printk("ERROR: fds_init %d\n", err);
		sprintf(tmpbuf, "ERROR: fds_init %d", err);
		show_infor(tmpbuf);	
		return;
	}

	while (1)
	{
		err = poll(&fds, 1, mqtt_keepalive_time_left(&client));
		if(err < 0)
		{
			printk("ERROR: poll %d\n", errno);
			sprintf(tmpbuf, "ERROR: poll %d", errno);
			show_infor(tmpbuf);
			break;
		}

		err = mqtt_live(&client);
		if((err != 0) && (err != -EAGAIN))
		{
			printk("ERROR: mqtt_live %d\n", err);
			sprintf(tmpbuf, "ERROR: mqtt_live %d", err);
			show_infor(tmpbuf);
			break;
		}

		if((fds.revents & POLLIN) == POLLIN)
		{
			err = mqtt_input(&client);
			if (err != 0)
			{
				printk("ERROR: mqtt_input %d\n", err);
				sprintf(tmpbuf, "ERROR: mqtt_input %d", err);
				show_infor(tmpbuf);
				break;
			}
		}

		if((fds.revents & POLLERR) == POLLERR)
		{
			printk("POLLERR\n");
			show_infor("POLLERR");
			break;
		}

		if((fds.revents & POLLNVAL) == POLLNVAL)
		{
			printk("POLLNVAL\n");
			show_infor("POLLNVAL");
			break;
		}
	}

	printk("Disconnecting MQTT client...\n");
	show_infor("Disconnecting MQTT client...");

	err = mqtt_disconnect(&client);
	if (err)
	{
		printk("Could not disconnect MQTT client. Error: %d\n", err);
		sprintf(tmpbuf, "Could not disconnect MQTT client. Error: %d", err);
		show_infor(tmpbuf);
	}

	while(1)
	{
		if(pmu_trige_flag)
		{
			pmu_trige_flag = false;
			pmu_interrupt_proc();
		}
		if(pmu_alert_flag)
		{
			pmu_alert_flag = false;
			pmu_alert_proc();
		}
		if(sys_pwr_off)
		{
			sys_pwr_off = false;
			SystemShutDown();
		}	
		if(lcd_sleep_in)
		{
			lcd_sleep_in = false;
			LCD_SleepIn();
		}
		if(lcd_sleep_out)
		{
			lcd_sleep_out = false;
			LCD_SleepOut();
		}
	
		k_cpu_idle();
	}
}
