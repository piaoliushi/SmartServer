#ifndef UTILITY_H
#define UTILITY_H

#include <string>
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
void StringTrim(string &s);
unsigned int CRC32(unsigned char *pdest, int len);

int BCD2OCT(unsigned char src);
unsigned char AsciiToInt(unsigned char src);

#endif // UTILITY_H
