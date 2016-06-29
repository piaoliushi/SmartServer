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
#endif // UTILITY_H
