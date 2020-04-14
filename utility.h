#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <QString>
using namespace std;

#define  Getbit(x,n)  ((x>>n) & 0x01)

int HexChar( char src );
int StrToHex( string str,unsigned char* Data );
int Dec2Hex(int input);
void kmp_init( const unsigned char *pattern, int pattern_size,int *pi );
int kmp( const unsigned char *matcher, int mlen, const unsigned char *pattern, int plen );
unsigned short CRC16_A001( unsigned char * ptr, int len );
unsigned short CRC16_CCITT( unsigned char * ptr, int len );
void string_replace(string&s1,const string&s2,const string&s3);

void Char2Hex(unsigned char ch, char* szHex);
void CharStr2HexStr(unsigned char const* pucCharStr, string &pszHexStr, int iSize);
unsigned short CalcCRC16_KT(unsigned char *pdest, int len);
unsigned short CalcCRC16_USB(unsigned char *pdest, int len);
unsigned short CRC16_XMODEM(unsigned char *pdest, int usDataLen);
unsigned short CRC16_X25(unsigned char *pdest, int usDataLen);
void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf);
void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf);
void StringTrim(string &s);
unsigned int CRC32(unsigned char *pdest, int len);

int BCD2OCT(unsigned char src);
unsigned char AsciiToInt(unsigned char src);
void IntToAscii(int src,unsigned char * cDsc);
int SF_checkSum(unsigned char* src,int len);
//subneedstr---功能：原始字符串截取stDes与edDes之间的字符串
//参数1：原始字符串;参数2：查找的起始字符串;参数3：查找的结束字符串;
string subneedstr(const string strSrc,const string staDes,const string endDes);

void utf8ToGb2312(std::string& strUtf8);
//QString utf8ToGb2312(const char *strUtf8);
void gb2312ToUtf8(std::string& strGb2312);
#ifdef Q_OS_WIN
void UTF8_to_GB2312_Win(const char* utf8, string & gb2312_str);
void GB2312_to_UTF8_Win(const char* gb2312, string & utf8_str);
#endif

#endif // UTILITY_H
