
/**
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * File:   dmlib.h
 * Created on 6 August 2011 г., 11:33 by PC BEST
 * Copyright:  Copyright (c) 2008-2012 Best LLC & Oleg Zhabko 
 *              Oleg Zhabko, mailto:olegzhabko@gmail.com
 *              phone +380 (4143) 2-32-97
 *              Berdichev, Ukraine
 */

#include "export.h"
#include "chan_dongle.h"
#include "cpvt.h"

#define DIAG_CONTROL_CHAR 	0x7E
#define DIAG_TRAILER_LEN  	3
#define USB_CTRL_GET_TIMEOUT 	5000
#define USB_DIR_OUT 		0x00
#define DIAG_ESC_CHAR     	0x7D
#define DIAG_ESC_MASK     	0x20
#define AST_MODULE		"chan_dongle"

//int dm_crc16 (const char *buffer, int len);
//int  dm_escape (const char *inbuf, int inbuf_len, char *outbuf, int outbuf_len);
//int dm_unescape (const char *inbuf, int inbuf_len, char *outbuf, int outbuf_len, int *escaping);
//int dm_encode_message (char *inbuf, int cmd_len, int inbuf_len, char *outbuf, int outbuf_len);
//int dm_decode_message (const char *inbuf, int inbuf_len, char *outbuf, int outbuf_len, int *out_decap_len, int *out_used, int *out_need_more);
EXPORT_DECL int dm_port_setup (char *port);
EXPORT_DECL int dm_send_command (int fd, unsigned char *buf, int comlen, int buflen,int encode);
EXPORT_DECL int dm_wait_reply (int fd, unsigned char *buf, int len, int encode);
EXPORT_DECL int dm_modem_preinit(pvt_t *pvt);
EXPORT_DECL void dm_modem_check(pvt_t *modem);
EXPORT_DECL int dm_modem_init(pvt_t *modem);
EXPORT_DECL int dm_dial_possible(const struct pvt *pvt);
EXPORT_DECL int dm_call(struct cpvt *cpvt, const char *num);
EXPORT_DECL int dm_answer(pvt_t *pvt);
EXPORT_DECL int dm_hang(pvt_t *pvt);
EXPORT_DECL int dm_wait(pvt_t *pvt);
EXPORT_DECL int dm_response(pvt_t *pvt,unsigned char *ans, int ans_len);
EXPORT_DECL void* dm_monitor_phone(void* data);

/*typedef struct ans00{
unsigned char header;
unsigned char date[11];
unsigned char time[8];
unsigned char f_date[11];
unsigned char f_time[8];
unsigned char sn[10];
unsigned char unknown[8];
} __attribute__ ((packed));

typedef struct ans65{
unsigned char header[4];
unsigned char unknown;
unsigned char network_state;
unsigned char call_state;
unsigned char unknown2[7];
} __attribute__ ((packed));
*/
typedef struct templ6c02{
unsigned char header[4];// = {0x4b,0x6c,0x02,0x00};
unsigned char len;
unsigned char number[10];
unsigned char nulls[22];// = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char tail[2];// = {0x0b,0x00};
} __attribute__ ((packed));
