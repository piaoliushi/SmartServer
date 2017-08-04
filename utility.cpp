#include "utility.h"
const unsigned short mtab[256] = {
    0x0000,0xc1c0,0x81c1,0x4001,0x01c3,0xc003,0x8002,0x41c2,
    0x01c6,0xc006,0x8007,0x41c7,0x0005,0xc1c5,0x81c4,0x4004,
    0x01cc,0xc00c,0x800d,0x41cd,0x000f,0xc1cf,0x81ce,0x400e,
    0x000a,0xc1ca,0x81cb,0x400b,0x01c9,0xc009,0x8008,0x41c8,
    0x01d8,0xc018,0x8019,0x41d9,0x001b,0xc1db,0x81da,0x401a,
    0x001e,0xc1de,0x81df,0x401f,0x01dd,0xc01d,0x801c,0x41dc,
    0x0014,0xc1d4,0x81d5,0x4015,0x01d7,0xc017,0x8016,0x41d6,
    0x01d2,0xc012,0x8013,0x41d3,0x0011,0xc1d1,0x81d0,0x4010,
    0x01f0,0xc030,0x8031,0x41f1,0x0033,0xc1f3,0x81f2,0x4032,
    0x0036,0xc1f6,0x81f7,0x4037,0x01f5,0xc035,0x8034,0x41f4,
    0x003c,0xc1fc,0x81fd,0x403d,0x01ff,0xc03f,0x803e,0x41fe,
    0x01fa,0xc03a,0x803b,0x41fb,0x0039,0xc1f9,0x81f8,0x4038,
    0x0028,0xc1e8,0x81e9,0x4029,0x01eb,0xc02b,0x802a,0x41ea,
    0x01ee,0xc02e,0x802f,0x41ef,0x002d,0xc1ed,0x81ec,0x402c,
    0x01e4,0xc024,0x8025,0x41e5,0x0027,0xc1e7,0x81e6,0x4026,
    0x0022,0xc1e2,0x81e3,0x4023,0x01e1,0xc021,0x8020,0x41e0,
    0x01a0,0xc060,0x8061,0x41a1,0x0063,0xc1a3,0x81a2,0x4062,
    0x0066,0xc1a6,0x81a7,0x4067,0x01a5,0xc065,0x8064,0x41a4,
    0x006c,0xc1ac,0x81ad,0x406d,0x01af,0xc06f,0x806e,0x41ae,
    0x01aa,0xc06a,0x806b,0x41ab,0x0069,0xc1a9,0x81a8,0x4068,
    0x0078,0xc1b8,0x81b9,0x4079,0x01bb,0xc07b,0x807a,0x41ba,
    0x01be,0xc07e,0x807f,0x41bf,0x007d,0xc1bd,0x81bc,0x407c,
    0x01b4,0xc074,0x8075,0x41b5,0x0077,0xc1b7,0x81b6,0x4076,
    0x0072,0xc1b2,0x81b3,0x4073,0x01b1,0xc071,0x8070,0x41b0,
    0x0050,0xc190,0x8191,0x4051,0x0193,0xc053,0x8052,0x4192,
    0x0196,0xc056,0x8057,0x4197,0x0055,0xc195,0x8194,0x4054,
    0x019c,0xc05c,0x805d,0x419d,0x005f,0xc19f,0x819e,0x405e,
    0x005a,0xc19a,0x819b,0x405b,0x0199,0xc059,0x8058,0x4198,
    0x0188,0xc048,0x8049,0x4189,0x004b,0xc18b,0x818a,0x404a,
    0x004e,0xc18e,0x818f,0x404f,0x018d,0xc04d,0x804c,0x418c,
    0x0044,0xc184,0x8185,0x4045,0x0187,0xc047,0x8046,0x4186,
    0x0182,0xc042,0x8043,0x4183,0x0041,0xc181,0x8180,0x4040
};

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


void Char2Hex(unsigned char ch, char* szHex)
{
    unsigned char byte[2];
    byte[0] = ch/16;
    byte[1] = ch%16;
    for(int i=0; i<2; i++)
    {
        if(byte[i] >= 0 && byte[i] <= 9)
            szHex[i] = '0' + byte[i];
        else
            szHex[i] = 'A' + byte[i] - 10;
    }
    szHex[2] = 0;
}

void CharStr2HexStr(unsigned char const* pucCharStr, string &pszHexStr, int iSize)
{

    int i;
    char szHex[3];
    for(i=0; i<iSize; i++)
    {
        Char2Hex(pucCharStr[i], szHex);
        pszHexStr.append(szHex);
        pszHexStr.append(" ");
    }
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

void string_replace(string&s1,const string&s2,const string&s3)
{
    string::size_type pos=0;
    string::size_type a=s2.size();
    string::size_type b=s3.size();
    while((pos=s1.find(s2,pos))!=string::npos)
    {
        s1.replace(pos,a,s3);
        pos+=b;
    }
}

unsigned short CalcCRC16_KT(unsigned char *pdest, int len)
{
    unsigned char tmp;
    unsigned short ret = 0;
    static const unsigned short crc_table [] = {
            0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
            0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF};
    while (len-- > 0)
    {
      tmp   = ret >> 8;
      tmp  ^= *pdest;
      ret <<= 4;
      ret  ^= crc_table[tmp >> 4];
      tmp   = ret >> 8;
      tmp >>= 4;
      tmp  ^= *pdest;
      ret <<= 4;
      ret  ^= crc_table[tmp & 0x0F];
      pdest++;
     }
    return (ret);
}



unsigned short CalcCRC16_USB(unsigned char *pdest, int len)
{
    unsigned short fcs;        /* frame check sequence */
    unsigned short q;          /* calculation register */

    fcs = 0xffff;             /* fcs initialised with all ones */
    while (len--) {
        q = *(mtab + (*pdest++ ^ (fcs >> 8)));
        fcs = ((q & 0xff00) ^ (fcs << 8)) | (q & 0x00ff);
    }
    return (fcs ^ 0xffff);

}

//去掉string首尾空格
void StringTrim(string &s)
{

    if( !s.empty() )
    {
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
}
