/*
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
#include <stdio.h>
#include <string.h>
#include "bsp/bsp.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "bleprph.h"
#include <console/console.h>

#include "os/os.h"
#include "os/os_time.h"
#include "datetime/datetime.h"

/* c3f27a93-ee5f-4bca-af73-dd1115f5be15 */
static const ble_uuid128_t gatt_svr_svc_smartwatch_uuid =
    BLE_UUID128_INIT(0xc3, 0xf2, 0x7a, 0x93, 0xee, 0x5f, 0x4b, 0xca,
                     0xaf, 0x73, 0xdd, 0x11, 0x15, 0xf5, 0xbe, 0x15);

/*e4 02 f2 59-f9 d2-44 a3-89 d5-f9 9d 5b 2b 35 ce*/
static const ble_uuid128_t gatt_svr_chr_time_uuid =
    BLE_UUID128_INIT(0xe4, 0x02, 0xf2, 0x59, 0xf9, 0xd2, 0x44, 0xa3,
                     0x89, 0xd5, 0xf9, 0x9d, 0x5b, 0x2b, 0x35, 0xce);

struct os_timeval time_val = { 0, 0 }; 
struct os_timezone my_timezone = { 60, 0 };

static int
gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service: Smartwatch. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_smartwatch_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {{
            /*** Characteristic: Set/read time. */
            .uuid = &gatt_svr_chr_time_uuid.u,
            .access_cb = gatt_svr_chr_access,
            .flags = BLE_GATT_CHR_F_READ |
                     BLE_GATT_CHR_F_WRITE ,
        }, {
            0, /* No more characteristics in this service. */
        } },
    },

    {
        0, /* No more services. */
    },
};

static int
gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                   void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

static int
gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    const ble_uuid_t *uuid;
    int rc;


    uuid = ctxt->chr->uuid;

    /* Determine which characteristic is being accessed by examining its
     * 128-bit UUID.
     */
    if (ble_uuid_cmp(uuid, &gatt_svr_chr_time_uuid.u) == 0) {
        switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            os_gettimeofday(&time_val,&my_timezone);
            rc = os_mbuf_append(ctxt->om, &(time_val.tv_sec),
                                sizeof (time_val.tv_sec));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = gatt_svr_chr_write(ctxt->om,
                                    4,
                                    sizeof (time_val.tv_sec),
                                    &(time_val.tv_sec), NULL);
            os_settimeofday(&time_val,&my_timezone);
            return rc;

        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
        }
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

int
gatt_svr_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
