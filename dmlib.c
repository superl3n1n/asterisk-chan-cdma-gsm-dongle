 /**
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * File:   dmlib.c
 * Created on 6 August 2011 г., 10:33 by PC BEST
 * Copyright:  Copyright (c) 2008-2012 Best LLC & Oleg Zhabko 
 *              Oleg Zhabko, mailto:olegzhabko@gmail.com
 *              phone +380 (4143) 2-32-97
 *              Berdichev, Ukraine
 */

/* This is version adopted for chan_datacard. It includes generic coding/decoding
functions for qualcomm-based modems and some interface functions reverse-engineered from ZTE AC8710 modem.
I dont know if will it work with other ZTE modems, this lib was created only for AC8710. Also
I don't include any information about functions I use, if you want to know more - write 
 olegzhabko@gmail.com and I'll answer your questions			*/

#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
//#include "asist.h"
#include "dmlib.h"
#include "chan_dongle.h"
#include "at_response.h"
#include "cpvt.h"
#include "manager.h"
#include "channel.h"
#include <asterisk.h>
#include <asterisk/lock.h>
//------------------------------------ utility part of library -------------------------------------------

const unsigned int crc_table[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

int dm_crc16 (unsigned char *buffer, int len)
{
 
    int crc = 0xffff;
    while (len--)
            crc = crc_table[(crc ^ *buffer++) & 0xff] ^ (crc >> 8);
    return ~crc;
}

int dm_escape (unsigned char *inbuf, int inbuf_len, unsigned char *outbuf, int outbuf_len)
{
    unsigned char *src = inbuf;
    unsigned char *dst = outbuf;
    int i = inbuf_len;
    if (inbuf == NULL || inbuf_len < 0 || outbuf == NULL || outbuf_len < inbuf_len){
	return 0;
    }
    if (outbuf_len <= inbuf_len << 1) {
        int outbuf_required = inbuf_len + 1; // +1 for the trailing control char
        while (i--) {
            if (*src == DIAG_CONTROL_CHAR || *src == DIAG_ESC_CHAR)
                outbuf_required++;
            src++;
        }
        if (outbuf_len < outbuf_required)
            return 0;
    }
    src = inbuf;
    i = inbuf_len;
    while (i--) {
        if (*src == DIAG_CONTROL_CHAR || *src == DIAG_ESC_CHAR){
            *dst++ = DIAG_ESC_CHAR;
            *dst++ = *src ^ DIAG_ESC_MASK;
        } else
            *dst++ = *src;
        src++;
    }
    return (dst - outbuf);
}

int dm_unescape (unsigned char *inbuf, int inbuf_len, unsigned char *outbuf, int outbuf_len, int *escaping)
{
    int i, outsize;
    if (inbuf_len < 0 || outbuf_len <= inbuf_len || escaping == NULL){
	return 0;
        }
    for (i = 0, outsize = 0; i < inbuf_len; i++) {
        if (*escaping) {
            outbuf[outsize++] = inbuf[i] ^ DIAG_ESC_MASK;
            *escaping = 0;
        } else if (inbuf[i] == DIAG_ESC_CHAR){
    	    *escaping = 1;
        }else
            outbuf[outsize++] = inbuf[i];
	if (outsize >= outbuf_len)
            return 0;
    }
    return outsize;
}

int dm_encode_message (unsigned char *inbuf, int cmd_len, int inbuf_len, unsigned char *outbuf, int outbuf_len)
{
    int crc;
    int escaped_len;
    if (inbuf == NULL || cmd_len < 1 || inbuf_len < cmd_len || outbuf == NULL){
	return 0;
    }
    crc = dm_crc16 (inbuf, cmd_len);
    inbuf[cmd_len++] = crc & 0xFF;
    inbuf[cmd_len++] = (crc >> 8) & 0xFF;
    //int i;
    escaped_len = dm_escape (inbuf, cmd_len, outbuf, outbuf_len);
    if (outbuf_len < escaped_len){
	return 0;
    }else
    outbuf[escaped_len++] = DIAG_CONTROL_CHAR;
    return escaped_len;
}

int dm_decode_message (unsigned char *inbuf, int inbuf_len, unsigned char *outbuf, int outbuf_len, int *out_decap_len, int *out_used, int *out_need_more)
{
    int escaping = 0;
    int i, pkt_len = 0, unesc_len;
    int crc, pkt_crc;
    if (inbuf == NULL || outbuf == NULL || outbuf_len < 0 || out_decap_len == NULL || out_used == NULL || out_need_more == NULL){
	return 0;
    }
    *out_decap_len = 0;
    *out_used = 0;
    *out_need_more = 0;
    if (inbuf_len < 4) {
        *out_need_more = 1;
        return 1;
    }
    for (i = 0; i <= inbuf_len; i++) {
        if (inbuf[i] == DIAG_CONTROL_CHAR) {
            if (i < 3) {
                *out_used = i + 1;
                return 0;
            }
            pkt_len = i;
            break;
        }
    }
    if (!pkt_len) {
        *out_need_more = 1;
        return 1;
    }
    unesc_len = dm_unescape (inbuf, pkt_len, outbuf, outbuf_len, &escaping);
    if (!unesc_len) {
        *out_used = pkt_len + 1;
        return 0;
    }
    if (escaping) {
        *out_need_more = 1;
        return 1;
    }
    crc = dm_crc16 (outbuf, unesc_len - 2);
    pkt_crc = outbuf[unesc_len - 2] & 0xFF;
    pkt_crc |= (outbuf[unesc_len - 1] & 0xFF) << 8;
    crc=crc&0x00ffff;
    if (crc != pkt_crc) {
        *out_used = pkt_len + 1;
        return 0;
    }
    *out_used = pkt_len + 1;
    *out_decap_len = unesc_len - 2;
    return 1;
}

// --------------------------------------------------- IO part of library ------------------------------

EXPORT_DEF int dm_port_setup (char *port){
struct termios stbuf;
int fd;
fd = open (port, O_RDWR | O_EXCL | O_NONBLOCK | O_NOCTTY);
if (fd < 0){
    printf("Error: Cant open port\r\n");
    return -1;                                                                                                                                         
}
//errno = 0;
memset (&stbuf, 0, sizeof (stbuf));
if(tcgetattr(fd, &stbuf)!= 0) {
    printf("Error: Cant get attribute\r\n");
}
stbuf.c_cflag &= ~(CBAUD | CSIZE | CSTOPB | CLOCAL | PARENB);
stbuf.c_iflag &= ~(HUPCL | IUTF8 | IUCLC | ISTRIP | IXON | IXOFF | IXANY | ICRNL);
stbuf.c_oflag &= ~(OPOST | OCRNL | ONLCR | OLCUC | ONLRET);
stbuf.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO | ECHOE | ECHOK | ECHONL);
stbuf.c_lflag &= ~(NOFLSH | XCASE | TOSTOP | ECHOPRT | ECHOCTL | ECHOKE);
stbuf.c_cc[VMIN] = 1;
stbuf.c_cc[VTIME] = 0;
stbuf.c_cc[VEOF] = 1;
stbuf.c_cflag |= (B9600 | CS8 | CREAD | 0 | 0);  //No parity, 1 stop bit 
stbuf.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
//stbuf.c_ispeed = 9600;
//stbuf.c_ospeed = 9600;
//errno = 0;
if (tcsetattr (fd, TCSANOW, &stbuf) < 0) {
    printf("Error: Cant config serial port\r\n");
    return -1;
}
return fd;
}

EXPORT_DEF int dm_send_command (int fd, unsigned char *buf, int comlen ,int buflen,int encode){
int status=-1;
int i=0;                                                                                                 
unsigned char outbuf[1024],end=(unsigned char)0x7e;
int outlen,errno;
if(encode){
    outlen = dm_encode_message (buf, comlen, buflen, &outbuf[0], sizeof(outbuf));
	if (outlen < comlen){
	    printf("Encode error.\r\n");
	    return 0;
	}
    status = write (fd,&end, 1);
    if (status < 0){
        return 0;
    }
    usleep (9000);
}else{
    outlen = comlen;
}
while (i < outlen){
    status = write (fd, &outbuf[i], 1);
    if(status < 0){
        return 0;
    } else{
        i++;
    }
    usleep (1000);
}
return 1;
}


EXPORT_DEF int dm_wait_reply(int fd, unsigned char *buf, int len, int encode){
//fd_set in;
int success;
//struct timeval timeout={1,0};
unsigned char readbuf[1024];
ssize_t bytes_read;
int total=0,retries=0;
int decap_len=0;
int used=0;
int more=0;
//int wait=0;
/*FD_ZERO(&in);
FD_SET(fd, &in);
result=select(fd+1, &in, NULL, NULL, &timeout);
if(result!=1 || !FD_ISSET(fd, &in)){
    return 0;
} */
do{
    bytes_read=read(fd,&readbuf[total],1);
    if(bytes_read==0){
        if(retries>10){
            return 0; // 2 seconds, give up 
        }
        usleep(1000);
        retries++;
        continue;
    }else{ 
	if(bytes_read == 1){
    	    if ((int)readbuf[total]==0x7e && total>1){
    		if(encode){
   		    success = dm_decode_message(readbuf, total, buf, len, &decap_len, &used, &more);
		    if (success) {
        		break;
    		    }
		}else{
		    break;
		}
	    }else{
    		total++;
    	    }
	}else{
	    usleep (1000);
	    retries++;
	}
    }
}while((unsigned int)total<sizeof(readbuf) || retries<20);
return decap_len;
}

// ---------------------------------------- interface part of library ------------------------------------

void dm_check6a08(int port){
unsigned char command[10] = {0x4b,0x6a,0x08,0x00};
dm_send_command(port,command,4,sizeof(command),1);
}

void dm_check65(int port){
unsigned char command[10] = {0x4b,0x65,0x00,0x00};                                                                                 
dm_send_command(port,command,4,sizeof(command),1);
}

EXPORT_DEF int dm_wait(pvt_t *pvt){
unsigned char message[1024];
int len = 0;
int port = pvt->dm_fd;
do{
len = dm_wait_reply(port, message, sizeof(message),1);
if(len>0)
    dm_response(pvt,message,len);
else
    break;
}while(len>0);
return 0;
}

EXPORT_DEF void dm_modem_check(pvt_t *modem){
int port = modem->dm_fd;
dm_check65(port);
usleep(1000);
dm_check6a08(port);
usleep(1000);
}

EXPORT_DEF int dm_modem_init(pvt_t *modem){
int anslen,i,q,status=1;
int port = modem->dm_fd;
int st=1;
unsigned char message[1024];
unsigned char answer[10][10]={
{0x4b,0x6a,0x11,0x00,0x00,0x01,0x02,0x01,0x01},
{0x4b,0x6a,0x10,0x00,0x00,0x01,0x01},
{0x4b,0x6a,0x07,0x00,0x00,0x00,0x00},
{0x4b,0x6a,0x14,0x00,0x00,0x03},
{0x4b,0x6a,0x16,0x00,0x00,0x02},
{0x4b,0x6a,0x0c,0x00,0x00},
{0x4b,0x6a,0x01,0x00,0x00,0x00},
{0x4b,0x6a,0x08,0x00,0x00,0x00},
{0x4b,0x65,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00}
};
unsigned char command[10][10]={
{0x4b,0x6a,0x11,0x00},
{0x4b,0x6a,0x10,0x00},
{0x4b,0x6a,0x07,0x00},
{0x4b,0x6a,0x14,0x00},
{0x4b,0x6a,0x16,0x00},
{0x4b,0x6a,0x0c,0x00},
{0x4b,0x6a,0x01,0x00},
{0x4b,0x6a,0x08,0x00},
{0x4b,0x65,0x00,0x00},
{0x00}
};


ast_verb(3,"dm_modem_init: Starting init of CDMA modem\n");

int seq[] = {0,1,1,2,3,3,10,0,0,1,0,10,10,1,4,3,2,10,9,10,10,10,10,5,1,8,8,6,7,8};
for(q=0;q<30;q++){
    ast_verb(6,"Sending init command %d....... ",q+1); //6
    status=1;
    if(seq[q]==10){
	sleep(1);
	ast_verb(6,"[sleep]\r\n"); //6
	continue;
    }else{
	dm_send_command(port,command[seq[q]],(seq[q]==9) ? 1 : 4,sizeof(command[seq[q]]),1);
	anslen = dm_wait_reply(port,message,sizeof(message),1);
	
	if(anslen>0){				
	    if(0){
		/*if(seq[q]!=9 && seq[q]!=6 && seq[q]!=8){ // return this string to enable init check- */
		for(i=0;i<anslen;i++)		
		    if(message[i]==answer[seq[q]][i])
			status=status*1;
		    else
			status=status*0;
		if(status)
		    ast_verb(6,"[OK]\r\n\r\n"); //6
		else
		    ast_verb(6,"[FAIL]\r\n\r\n"); //6
	    }else{
		if(seq[q]==6)
		    for(i=0;i<anslen;i++)
			printf("%02x ",(unsigned int)message[i]);
	    ast_verb(6,"[UNKNOWN]\r\n"); //6
	    }
	}else{
	    ast_verb(6,"[ERROR]\r\n"); //6
	    status=status*0;
	}
    }
    st=st*status;
    if(!st)
	break;
}
ast_verb(3,"dm_modem_init: Init status = %d\n",st);
if(st){
    return st;
}
return 0;
}

EXPORT_DEF int dm_modem_preinit(pvt_t *pvt){
if(pvt->cdma){
    pvt->timeout = 2;
    pvt->has_voice = 1;
    strcpy(pvt->provider_name,"CDMA");
    strcpy(pvt->manufacturer,"ZTE");
    strcpy(pvt->model,"AC8710");
    strcpy(pvt->firmware,"hidden by Qualcomm");
    strcpy(pvt->imei,"1111111111111111");
    strcpy(pvt->imsi,"1111111111111111");
    //strcpy(pvt->number,"47575");
    strcpy(pvt->location_area_code,"3804143");
    pvt->dialing=0;
    pvt->ring=0;
    pvt->gsm_registered=0;
}
//pvt->audio_fd = dm_port_setup(pvt->audio_tty);
//pvt->data_fd = dm_port_setup(pvt->data_tty);
//pvt->dm_fd = dm_port_setup(pvt->dm_tty);
if(pvt->data_fd>0 && pvt->audio_fd>0 && pvt->dm_fd>0)
    pvt->connected=1;
else
    pvt->connected=0;
return 0;
}

EXPORT_DEF int dm_dial_possible(const struct pvt *pvt){
int res = 0;
if(pvt->dialing || pvt->ring)
    res = 0;
else
    res = 1;
ast_verb(6,"[%s] Test device: %s\n",PVT_ID(pvt),res ? "In use" : "Not in use");
return res;
}

EXPORT_DEF int dm_call(struct cpvt *cpvt, const char *num){
int i,len;
struct pvt *pvt = cpvt->pvt;
int port = pvt->dm_fd;
unsigned char *message,mes[1024];
struct templ6c02 strmessage;
if(!pvt->dialing && !pvt->ring)
{
len = strlen(num);
for(i=0;i<10;i++)
    if(i<len)
	strmessage.number[i]=num[i];
    else
	strmessage.number[i]=0x00;
strmessage.len=(unsigned char)len;
for(i=0;i<22;i++)
    strmessage.nulls[i]=0x00;
strmessage.header[0]=0x4b;
strmessage.header[1]=0x6c;
strmessage.header[2]=0x02;
strmessage.header[3]=0x00;
strmessage.tail[0]=0x0b;
strmessage.tail[1]=0x00;
message = (unsigned char*)&strmessage;
memcpy(mes,message,39);
ast_verb(3, "[%s] dm_call: Starting call to %s.\n",PVT_ID(pvt), num); 
if(dm_send_command(port,mes,39,sizeof(mes),1))
{
    CPVT_SET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);
    return 0;
}
}
else
ast_verb(3, "[%s] dm_call: Cannot call, already dialing\n", PVT_ID(pvt));

return 1;
}

EXPORT_DEF int dm_answer(pvt_t *pvt){
int port = pvt->dm_fd;
unsigned char command[10] = {0x4b,0x6c,0x03,0x00};                                                                                 
ast_verb(3, "[%s] dm_answer: Performing answer.\n",PVT_ID(pvt));
if(dm_send_command(port,command,4,sizeof(command),1))
    return 0;
return 1;
}

EXPORT_DEF int dm_hang(pvt_t *pvt){
int port = pvt->dm_fd;
unsigned char command[10] = {0x4b,0x6c,0x01,0x00};                                                                                 
ast_verb(3, "[%s] dm_hang: Hanging up.\n",PVT_ID(pvt));
if(dm_send_command(port,command,4,sizeof(command),1))
    return 0;
return 1;
}


//int dm_response_6a(pvt_t *pvt,unsigned char *ans){
//struct ans6a *struct6a;
//struct6a = (struct ans6a*)&ans;
//return 0;
//}

int dm_response_6c(pvt_t *pvt,unsigned char *ans){
//struct ans6c *struct6c;
cpvt_t *cpvt;
cpvt = pvt_find_cpvt(pvt, 1);
//struct6c = (struct ans6c*)&ans;
switch((unsigned int)ans[2]){
    case 0x01:
	/* Hanging up call: some unset of flags, need to investigate */
	//CPVT_RESET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);
	//change_channel_state(cpvt, CALL_STATE_RELEASED, 0);
	return 0;
    case 0x02:
	/* Starting dial: need to move here some of acts from dm_call() */
	//change_channel_state(cpvt, CALL_STATE_DIALING, 0);
	//CPVT_SET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);
	//pvt->dialing = 1;
	return 0;
    case 0x03:
	/* Answering call: need to move here some of acts from dm_answer() */
	//change_channel_state(cpvt, CALL_STATE_ACTIVE, 0);
	//pvt->dialing = 1;
	//CPVT_SET_FLAGS(pvt->cpvt, CALL_FLAG_NEED_HANGUP);
	return 0;
    default:
	break;
}
return 1;
}

int dm_create_answer(pvt_t *pvt, unsigned char *ans){
cpvt_t *cpvt = NULL;
unsigned char num[11];
int j,i = 4;
int numlen = (unsigned int)ans[3];
for(j = 0; j < numlen; j++)
    num[j] = ans[i+j];
num[j] = '\0';
cpvt = pvt_find_cpvt(pvt, 1);
ast_verb(6,"[%s] dm_create_answer: starting incoming call from %s\n",PVT_ID(pvt),pvt->last_number);
if(!cpvt && pvt->ring){
    if(start_pbx(pvt,(const char*)&pvt->last_number, 1,CALL_STATE_INCOMING)<0){
	ast_verb(6,"[%s] some error in creating answer\n",PVT_ID(pvt));
	return 0;
    }
}else
    ast_verb(6,"[%s] dm_create_answer: cpvt exists or !pvt->ring\n",PVT_ID(pvt));
return 1;
}

int dm_response_d2(pvt_t *pvt,unsigned char *ans){
//struct ansd2 *structd2;
int j,i=4;
int numlen;
cpvt_t *cpvt;
cpvt = pvt_find_cpvt(pvt, 1);
if((unsigned int)ans[1] == 0x06){
    numlen = (unsigned int)ans[3];
    for(j=0;j<numlen;j++)
	pvt->last_number[j] = ans[i+j];
    pvt->last_number[j] = '\0';
    ast_verb(9,"[%s] dm_response_d2: number set to %s\n",PVT_ID(pvt),pvt->last_number);
}
if((unsigned int)ans[1] != 0x08)
    dm_modem_check(pvt);
//structd2 = (struct ansd2*)&ans;
/*switch((unsigned int)ans[1]){
    case 0x0a:
	//dm_check_65(pvt->dm_fd);
	return 0;
    case 0x07:
	change_channel_state(cpvt, CALL_STATE_ACTIVE, 0);
	//dm_check_65(pvt->dm_fd);
	return 0;
    case 0x08:
	if(!pvt->dialing)
	    pvt->ring = 1;
	    //dm_create_answer(pvt);
	return 0;
    case 0x06:
	if(pvt->ring)
	    dm_create_answer(pvt, ans);
	return 0;
    case 0x09:
	//change_channel_state(cpvt, CALL_STATE_RELEASED, 16);
	//CPVT_RESET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);
	//manager_event_cend(PVT_ID(pvt), 1, 1, 0, 1);
	return 0;
    default:
	break;
}*/
return 1;
}

int dm_response_65(pvt_t *pvt,unsigned char *ans){
//struct ans65 *struct65;
cpvt_t *cpvt = NULL;
cpvt = pvt_find_cpvt(pvt, 1);
int i=0;
//struct65=(struct ans65*)&ans;
//if((unsigned int)ans[5]==0x04 || (unsigned int)ans[5]==0x02){ //old
if((unsigned int)ans[5]==0x04 || (unsigned int)ans[5]==0x02 || (unsigned int)ans[5]==0x00){
    ast_verb(9, "[%s] dm_response_65: initialized and ready\n",PVT_ID(pvt));
    pvt->initialized=1;
    pvt->gsm_registered=1;
}else{
	ast_verb(3, "dm_response_65: initialized error ans[5] = %d\n", ans[5]);
    pvt->initialized=0;
    pvt->gsm_registered=0;
    pvt->dialing = 0;
    pvt->ring = 0;
    return 1;
}
if(cpvt)
	ast_verb(9,"[%s] dm_response_65: cpvt is active\n",PVT_ID(pvt));
    else
	ast_verb(9,"[%s] dm_response_65: cpvt is inactive\n",PVT_ID(pvt));
switch((unsigned int)ans[6]){
    case 0x05:
    pvt->dialing=1;
    pvt->ring=0;
    if(!cpvt){
	ast_verb(6,"[%s] dm_response_65 error: cpvt absent on active call\n",PVT_ID(pvt));
	dm_hang(pvt);
    }else
    change_channel_state(cpvt, CALL_STATE_ACTIVE, 0);
    return 0;
    case 0x01:
    pvt->ring=0;
    pvt->dialing=1;
    if(cpvt){
	change_channel_state(cpvt, CALL_STATE_DIALING, 0);
	CPVT_SET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);
	cpvt->call_idx = 1;
    }else{
	ast_verb(6,"[%s] dm_response_65 error: call without cpvt\n",PVT_ID(pvt));
	dm_hang(pvt);
    }
    return 0;
    case 0x03:
    pvt->dialing=0;
    pvt->ring=1;
    if(!cpvt)
	dm_create_answer(pvt,ans);
    return 0;
    case 0x00:
    default:
    pvt->dialing=0;
    pvt->ring=0;
    for(i=0;i<14;i++)
        pvt->last_number[i] = '0';
    pvt->last_number[i]='\0';
    ast_verb(9,"[%s] dm_response_65: number set to %s\n",PVT_ID(pvt),pvt->last_number);
    if(cpvt)
	change_channel_state(cpvt, CALL_STATE_RELEASED, 0);
    return 0;
}
return 1;
}

EXPORT_DEF int dm_response(pvt_t *pvt,unsigned char *ans,int ans_len){
int i;
switch((unsigned int)ans[0]){
    case 0x00:
	//dm_response_00(pvt,ans,ans_len); /* response:00 ... - Taking general information from modem */
	break;
    case 0x4b:
	switch((unsigned int)ans[1]){
	    case 0x6a:
		ast_verb(9,"[%s] dm_response: responsing to 0x6a - doing nothing\n",PVT_ID(pvt));
		//dm_response_6a(pvt,ans); /* response:4b 6a ... - Status answer 1 */
		break;
	    case 0x65:
	    //for(i=0;i<ans_len;i++) ast_verb(3,"ans[%d] = %d ",i ,(unsigned int)ans[i]); //6	
	    //ast_verb(3,"\n");
		ast_verb(9,"[%s] dm_response: responsing to 0x65\n",PVT_ID(pvt));
		if(dm_response_65(pvt,ans)) /* response:4b 65 ... - Status answer 2 */
		    ast_verb(9,"[%s] Error in 65 answer\n",PVT_ID(pvt));
		break;
	    case 0x6c:
		ast_verb(9,"[%s] dm_response: responsing to 0x6c\n",PVT_ID(pvt));
		dm_response_6c(pvt,ans); /* response:46 6c ... - Dial control commands answer */
		break;
	    default:
		break;
	}
	break;
    case 0xd2:
	ast_verb(6, "[%s] Proceeding event %02x\n",PVT_ID(pvt),(unsigned int)ans[1]);
	dm_response_d2(pvt,ans); /* response:d2 ... - Events processing */
	break;
    default:
	break;
}
return 0;
}

EXPORT_DEF void* dm_monitor_phone (void* data){
	struct pvt*	pvt = (struct pvt*) data;
	if(pvt->connected){
	    ast_verb(6, "[%s] dm_monitor_phone: starting loop\n",PVT_ID(pvt));
	    while(!pvt->terminate_monitor){
		dm_wait(pvt);
		usleep(10000);
	    }
	}
return NULL;
}
