/*
 * Copyright (C) 2005 by Grigoriy A. Sitkarev                            
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

#ifndef __IEC104_TYPES_H
#define __IEC104_TYPES_H

#pragma pack(push,1)


#define M_SP_NA_1	1   //单点遥信变位
#define M_BO_NA_1	7   //32位比特串
#define M_ME_NA_1	9   //带品质描述的规一化值
#define M_ME_NB_1	11 //带品质描述的比例系数
#define M_ME_NC_1	13 //短浮点数
#define M_SP_TB_1	    30 //带 CP56Time2a 时标的单点信息
#define M_BO_TB_1	33 //带 CP56Time2a 时标的 32 比特串
#define M_ME_TD_1	34 //带 CP56Time2a 时标的测量值规一化值
#define M_ME_TE_1	35 //带 CP56Time2a 时标的测量值标度化值
#define M_ME_TF_1	36 //带 CP56Time2a 时标的测量值短浮点数

/* 32-bit string state and change data unit */
struct iec_stcd {
	unsigned char		st1	:1;
	unsigned char		st2	:1;
	unsigned char		st3	:1;
	unsigned char		st4	:1;
	unsigned char		st5	:1;
	unsigned char		st6	:1;
	unsigned char		st7	:1;
	unsigned char		st8	:1;
	unsigned char		st9	:1;
	unsigned char		st10	:1;
	unsigned char		st11	:1;
	unsigned char		st12	:1;
	unsigned char		st13	:1;
	unsigned char		st14	:1;
	unsigned char		st15	:1;
	unsigned char		st16	:1;

	unsigned char		cd1	:1;
	unsigned char		cd2	:1;
	unsigned char		cd3	:1;
	unsigned char		cd4	:1;
	unsigned char		cd5	:1;
	unsigned char		cd6	:1;
	unsigned char		cd7	:1;
	unsigned char		cd8	:1;
	unsigned char		cd9	:1;
	unsigned char		cd10	:1;
	unsigned char		cd11	:1;
	unsigned char		cd12	:1;
	unsigned char		cd13	:1;
	unsigned char		cd14	:1;
	unsigned char		cd15	:1;
	unsigned char		cd16	:1;
};

/* CP56Time2a timestamp */
typedef struct cp56time2a {
	unsigned short		msec;
	unsigned char		min	:6;
	unsigned char		res1	:1;
	unsigned char		iv	:1;
	unsigned char		hour	:5;
	unsigned char		res2	:2;
	unsigned char		su	:1;
	unsigned char		mday	:5;
	unsigned char		wday	:3;
	unsigned char		month	:4;
	unsigned char		res3	:4;
	unsigned char		year	:7;
	unsigned char		res4	:1;	
} cp56time2a ;

/* M_SP_NA_1 - single point information with quality description */
//单点信息
struct iec_type1 {
	unsigned char		sp	:1; /* single point information */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
} ;

/* M_BO_NA_1 - state and change information bit string */
//状态改变bit串信息
struct iec_type7 {
	struct iec_stcd	stcd;
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
};

/* M_ME_NA_1 - normalized measured value */
//测量值----归一化值
struct iec_type9 {
	unsigned short		mv;	/* normalized value */
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
};

/* M_ME_NB_1 - scaled measured value */
//测量值----标度化值
struct iec_type11 {
	unsigned short		mv;	/* scaled value */
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
};

/* M_ME_NC_1 - short floating point measured value */
//测量值----短浮点数
struct iec_type13 {
	float		mv;
	unsigned char		ov:1; /* overflow/no overflow */
	unsigned char		res:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
};

/* M_SP_TB_1 - single point information with quality description and time tag */
//带时标CP56Time2a的单点信息
struct iec_type30 {
	unsigned char		sp	:1; /* single point information */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
	cp56time2a	time;
};

/* M_BO_TB_1 - state and change information bit string and time tag  */
//带时标的32比特串
struct iec_type33 {
	struct iec_stcd	stcd;
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
	cp56time2a	time;
};

/* M_ME_TD_1 - scaled measured value with time tag */
//带时标的测量值----归一化值
struct iec_type34 {
	unsigned short		mv;	/* scaled value */
	unsigned char		ov:1; /* overflow/no overflow */
	unsigned char		res:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
	cp56time2a	time;
};

/* M_ME_TE_1 - scaled measured value with time tag*/
//带时标的测量值----标度化值
struct iec_type35 {
	unsigned short		mv;	/* scaled value */
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
	cp56time2a	time;
};

/* M_ME_TF_1 - short floating point measurement value and time tag */
//带时标的测量值----短浮点数
struct iec_type36 {
	float		mv;
	unsigned char		ov	:1; /* overflow/no overflow */
	unsigned char		res	:3;
	unsigned char		bl	:1; /* blocked/not blocked */
	unsigned char		sb	:1; /* substituted/not substituted */
	unsigned char		nt	:1; /* not topical/topical */
	unsigned char		iv	:1; /* valid/invalid */
	cp56time2a	time;
};

/* Data unit identifier block - ASDU header */
struct iec_unit_id {
	unsigned char		type;	/* type identification */
	unsigned char		num	:7; /* number of information objects *///可变字节数
	unsigned char		sq	:1; /* sequenced/not sequenced address *///0：寻址统一类型的信息体的单独信息,1:寻址一个信息体的顺序元素
	unsigned char		cause	:6; /* cause of transmission *///传输原因
	unsigned char		pn	:1; /* positive/negative app. confirmation *///0:肯定认可，1：否定认可
	unsigned char		t	:1; /* test *///0:未试验，1：已试验
	unsigned char		oa;         	/* originator addres */
	unsigned short	    ca;	        	/* common address of ASDU */
};

#pragma pack(pop)
#endif	/* __IEC104_TYPES_H */
