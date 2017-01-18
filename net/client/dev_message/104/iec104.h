/*
 * Copyright (C) 2005, Grigoriy Sitkarev                                 
 * sitkarev@komi.tgk-9.ru                                                
 *                                                                       
 * This program is free software; you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation; either version 2 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program; if not, write to the                         
 * Free Software Foundation, Inc.,                                       
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             
 */
#ifndef __IEC104_H
#define __IEC104_H

#include "iec104_types.h"

#define IEC_OBJECT_MAX	127
#define IEC_TYPEID_LEN	4

#define IEC_APDU_MAX	253
#define IEC_APDU_MIN	4
#define IEC_APCI_LEN    6

#define  START  0x68//起始标志
#define  RESET   0x69

#define  STARTDTACT     0x07//启动数据传输请求
#define  STARTDTCON     0x0B//启动数据传输应答
#define  STOPDTACT      0x13//停止数据传输请求
#define  STOPDTCON      0x23//停止数据传输应答
#define  TESTFRACT      0x43//测试请求
#define  TESTFRCON      0x83//测试应答
#define  INTERROGATION  0x64//总召唤

#define CYCLIC         1  //周期循环
#define BGSCAN         2  //背景扫描
#define SPONTANEOUS    3  //突发spont
#define REQUEST        5  //请求或被请求
#define ACTIVATION     6  //激活
#define ACTCONFIRM     7  //激活确认
#define DEACTIVATION   8  //停止激活
#define ACTTERM        10 //激活结束


#define COM_ADDRLEN	  2 //公共字节长度
#define IOA_ADDRLEN	  3 //信息体字节长度

#pragma pack(push,1)
/* Information object */
struct iec_object {
	unsigned short		ioa;	/* information object address */
	union {
		struct iec_type1   	type1;
        struct iec_type7	type7;
        struct iec_type9	type9;
        struct iec_type11	type11;
		struct iec_type13 	type13;
        struct iec_type30	type30;
		struct iec_type33 	type33;
		struct iec_type34 	type34;
		struct iec_type35 	type35;
		struct iec_type36 	type36;
	} o;	
};

struct iec_i {
	unsigned 	ft:1;
	unsigned 	ns:15;
	unsigned 	res:1;
	unsigned 	nr:15;
};

struct iec_s {
	unsigned 	ft:1;
	unsigned 	res1:15;
	unsigned 	res2:1;
	unsigned 	nr:15;
};

struct iec_u {
	unsigned char	ft:2;
	unsigned char	start_act:1;
	unsigned char	start_con:1;
	unsigned char	stop_act:1;
	unsigned char	stop_con:1;
	unsigned char	test_act:1;
	unsigned char	test_con:1;
	unsigned char	res1;
	unsigned short res2;
};

struct iechdr {
	union {
	unsigned char  raw[1];
	struct iec_i ic;
	struct iec_s sc;
	struct iec_u uc;
	};
	//iec_unit_id asdu_h;
};

struct iec_buf {
	unsigned char	start;
	unsigned char	length;
	struct iechdr      h;	
	iec_unit_id			asdu_h;
	unsigned char  data[255];
};

enum frame_type {
	FRAME_TYPE_I,
	FRAME_TYPE_S,
	FRAME_TYPE_U
};

enum uframe_func {
	STARTACT,
	STARTCON,
	STOPACT,
	STOPCON,
	TESTACT,
	TESTCON
};
#pragma pack(pop)


enum frame_type frame_type(struct iechdr *h);

enum uframe_func uframe_func(struct iechdr *h);

char * uframe_func_to_string(enum uframe_func func);

char * frame_to_string(struct iechdr *h);

int iecasdu_parse(struct  iec_object *obj, unsigned char *type, unsigned short *com_addr, 
	int *cnt, unsigned char *cause, unsigned char *test, unsigned char *pn, size_t ioa_len, 
	size_t ca_len, unsigned char *buf, size_t buflen);


#endif	/* __IEC104_H */
