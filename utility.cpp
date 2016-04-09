#include "utility.h"

int HexChar( char src )
{
    if((src>='0')&&(src<='9'))
        return src-0x30;
    else if((src>='A')&&(src<='F'))
        return src-'A'+10;
    else if((src>='a')&&(src<='f'))
        return src-'a'+10;
    else
        return 0x10;
}

int Dec2Hex(int input) //10进制转换成相同数字的进制
    {
        int hi = (input / 10);
        int low = input % 10;
        return hi * 16 + low;
    }

int StrToHex( string str,unsigned char* Data )
{
    int t,t1;
    int rlen=0,len=str.length();
    for(int i=0;i<len;)
    {
        char l,h=str[i];
        if(h==' ')
        {
            i++;
            continue;
        }
        i++;
        if(i>=len)
            break;
        l=str[i];
        t=HexChar(h);
        t1=HexChar(l);
        if((t==16)||(t1==16))
            break;
        else
            t=t*16+t1;
        i++;
        Data[rlen]=(unsigned char)t;
        rlen++;
    }
    return rlen;
}
