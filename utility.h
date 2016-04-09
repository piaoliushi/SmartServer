#ifndef UTILITY_H
#define UTILITY_H

#include <string>
using namespace std;

#define  Getbit(x,n)  ((x>>n) & 0x01)

int HexChar( char src );
int StrToHex( string str,unsigned char* Data );
int Dec2Hex(int input);
#endif // UTILITY_H
