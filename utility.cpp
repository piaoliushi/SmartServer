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


void kmp_init( const unsigned char *pattern, int pattern_size,int *pi )
{
    if(pi==NULL)
        return;
    pi[0] = 0;  // pi[0] always equals to 0 by defination
    int k = 0;  // an important pointer
    for(int q = 1; q < pattern_size; q++)  // find each pi[q] for pattern[q]
    {
        while(k>0 && pattern[k+1]!=pattern[q])
            k = pi[k];  // use previous prefixes to match pattern[0..q]

        if(pattern[k+1] == pattern[q]) // if pattern[0..(k+1)] is a prefix
            k++;             // let k = k + 1

        pi[q] = k;   // be ware, (0 <= k <= q), and (pi[k] < k)
    }
}

int kmp( const unsigned char *matcher, int mlen, const unsigned char *pattern, int plen )
{
    if(!mlen || !plen || mlen < plen) // take care of illegal parameters
        return -1;
    int *pi = new int[plen];
    kmp_init(pattern, plen,pi);  // prefix-function

    int i=0, j=0;
    while(i < mlen && j < plen)  // don't increase i and j at this level
    {
        if(matcher[i+j] == pattern[j])
            j++;
        else if(j == 0)  // dismatch: matcher[i] vs pattern[0]
            i++;
        else      // dismatch: matcher[i+j] vs pattern[j], and j>0
        {
            i = i + j - pi[j-1];  // i: jump forward by (j - pi[j-1])
            j = pi[j-1];          // j: reset to the proper position
        }
    }
    delete pi,pi=NULL;
    if(j == plen) // found a match!!
        return i;
    else          // if no match was found
        return -1;
}

unsigned short CRC16_A001( unsigned char * ptr, int len )
{
    unsigned constnum=0xA001;
    unsigned short nAccum=0xFFFF;
    int i,j;
    for ( i = 0; i < len; i++ )
    {
        nAccum=(nAccum&0xff00)+(*(ptr+i))^(nAccum&0x00ff);

        for (j = 0; j < 8; j++ )
        {
            if (nAccum & 0x1 )
                nAccum = ( nAccum >> 1 ) ^ constnum;
            else
                nAccum = nAccum >> 1;
        }
    }
    return  nAccum;
}

unsigned short CRC16_CCITT( unsigned char * ptr, int len )
{
    unsigned constnum = 0x8408;
    unsigned short nAccum=0xFFFF;
    int i,j;
    for ( i = 0; i < len; i++ )
    {
        unsigned char input = *(ptr+i);
        for (j = 0; j < 8; j++ )
        {
            if( (nAccum & 0x0001) ^ (input & 0x01) )
                constnum = 0x8408;
            else
                constnum = 0x0000;
            nAccum = nAccum >> 1;
            // XOR in the x16 value
            nAccum ^= constnum;
            // shift input for next iteration
            input = input >> 1;
        }
    }
    return  nAccum;
}
