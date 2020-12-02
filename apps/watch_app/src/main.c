/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <assert.h>
#include <compile_time.h>//macro for build time as unix time, for setting clock,optional

#include "sysinit/sysinit.h"
#include <console/console.h>
#include "os/os.h"
#include "os/os_time.h"
#include "datetime/datetime.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#include "hal/hal_spi.h"
#include "ssd1306/ssd1306.h"

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

struct os_timeval os_time;
struct clocktime clocktime;
static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin,g_front_button_pin,g_side_button_pin;

static int bleprph_gap_event(struct ble_gap_event *event, void *arg);

static void
bleprph_advertise(void)
{
    uint8_t own_addr_type;
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        //MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assiging the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    // fields.uuids16 = (ble_uuid16_t[]){
    //     BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    // };
    // fields.num_uuids16 = 1;
    // fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        //MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL);
    if (rc != 0) {
        //MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            console_printf("Connected\n");
        }else{
            console_printf("Connection fail\n");
            /* Connection failed; resume advertising. */
            bleprph_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        console_printf("Disconnect\n");
        /* Connection terminated; resume advertising. */
        bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        bleprph_advertise();
        return 0;   

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */

        /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    return 0;
}

static void
bleprph_on_sync(void)
{
    int rc;

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Begin advertising. */
    bleprph_advertise();
}

/* The timer callout */
static struct os_callout blinky_callout;

/*
 * Event callback function for timer events. It toggles the led pin.
 */
static void
timer_ev_cb(struct os_event *ev)
{
    assert(ev != NULL);

 ++g_task1_loops;

        ssd1306_Fill(Black);
        char str[12];
        sprintf(str, "Time:");
        ssd1306_SetCursor(0,0);
        ssd1306_WriteString(str,Font_7x10,White);

        os_gettimeofday(&os_time,NULL);        
        timeval_to_clocktime(&os_time, NULL, &clocktime);
        
        
        sprintf(str,"%02d:%02d:%02d",clocktime.hour,clocktime.min,clocktime.sec);
        ssd1306_SetCursor(0,11);
        ssd1306_WriteString(str,Font_7x10,White);   
        
        sprintf(str,"%02d.%02d.%02d", clocktime.day,clocktime.mon,clocktime.year%100);   
        ssd1306_SetCursor(0,21);
        ssd1306_WriteString(str,Font_7x10,White);

        ssd1306_UpdateScreen();
        os_time_delay(OS_TICKS_PER_SEC/10);
 
  
        
        if (hal_gpio_read(g_front_button_pin) == 1)
        {
			/* Pulse the motor */
			hal_gpio_write(g_led_pin,1);
			os_time_delay(OS_TICKS_PER_SEC/20);
			hal_gpio_write(g_led_pin,0);
		}
		
		if (hal_gpio_read(g_side_button_pin) == 0)
        {
			/* Pulse the motor */
			hal_gpio_write(g_led_pin,1);
			os_time_delay(OS_TICKS_PER_SEC/10);
			hal_gpio_write(g_led_pin,0);
		}

    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC/100);
}

static void
init_timer(void)
{
    /*
     * Initialize the callout for a timer event.
     */
    os_callout_init(&blinky_callout, os_eventq_dflt_get(),
                    timer_ev_cb, NULL);

    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC/100);
}

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    console_printf("oledtest sysinit done\n");

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.sync_cb = bleprph_on_sync;

    rc = ble_svc_gap_device_name_set("nimble-bleprph");
    assert(rc == 0);

    console_printf("oledtest ble setup done\n");

	//init pins
    g_led_pin = LED_BLINK_PIN;
    g_front_button_pin = FRONT_BUTTON_PIN;
    g_side_button_pin = SIDE_BUTTON_PIN;
    
    hal_gpio_init_out(g_led_pin, 0);
    hal_gpio_init_in(g_front_button_pin,HAL_GPIO_PULL_NONE);
    hal_gpio_init_in(g_side_button_pin,HAL_GPIO_PULL_UP);

    //init timer callback
    init_timer();

    //set time
    struct os_timeval time_now = { __TIME_UNIX__, 0 }; 
    struct os_timezone my_timezone = { 60, 0 };
    os_settimeofday(&time_now,&my_timezone);
    console_printf("time set done\n");

    //Init Oled display
    ssd1306_Init();

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0,0);
    ssd1306_WriteString("OLED Test",Font_7x10,White);
    ssd1306_UpdateScreen();
    os_time_delay(OS_TICKS_PER_SEC/2);

    console_printf("oledtest setup done\n");

    console_printf("Starting default event queue\n");

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    return 0;
}

