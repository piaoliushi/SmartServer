#include "utility.h"
#include <QTextCodec>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

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

const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
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
    int rLen = mlen-plen+1;
    while(i <rLen  && j < plen)  // don't increase i and j at this level
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

unsigned int CRC32(unsigned char *pdest, int len)
{
    unsigned int i, crc;
    crc = 0xFFFFFFFF;


    for (i = 0; i < len; i++)
        crc = crc32tab[(crc ^ pdest[i]) & 0xff] ^ (crc >> 8);

    return crc^0xFFFFFFFF;
}


int BCD2OCT(unsigned char src)
{
    return (((src&0xF0)>>4)*10+(src&0x0F));
}


unsigned char AsciiToInt(unsigned char src)
{
    if(src>0x40 && src<0x47)
    {
        return (src-55);
    }
    else if(src>0x29 && src<0x3a)
    {
        return (src-48);
    }
    else if(src>0x60 && src<0x67)
    {
        return (src-87);
    }
    else
        return 0;
}



void IntToAscii(int src, unsigned char *cDsc)
{
    int rtnum = 0;
    int irem = 0;
   // irem = src%16;
    do
    {
       irem = src%16;
       if(irem<10)
       {
           cDsc[rtnum] = irem+48;
       }
       else
       {
           cDsc[rtnum] = irem+55;
       }
       src /=16;
       ++rtnum;
    }while(src>0);
}


int SF_checkSum(unsigned char *src, int len)
{
    unsigned int sums = 0;
    for(int i=0;i<len;++i)
    {
        sums += src[i];
    }
    return ((~(sums%65536))&0xFFFF)+1;
}

string subneedstr(const string strSrc, const string staDes, const string endDes)
{
    string strResult="";
    int dtstartpos=-1,dtlen=-1;
    dtstartpos = strSrc.find(staDes);
    if(dtstartpos>=0)
    {
        dtlen = strSrc.find(endDes,dtstartpos+staDes.length());
        if(dtlen>0)
        {
            strResult = strSrc.substr(dtstartpos+staDes.length(),dtlen-(dtstartpos+staDes.length()));
        }
    }
    return strResult;
}


unsigned short CRC16_XMODEM(unsigned char *pdest, int usDataLen)
{
    unsigned short wCRCin = 0x0000;
        unsigned short wCPoly = 0x1021;
        unsigned char wChar = 0;

        while (usDataLen--)
        {
            wChar = *(pdest++);
            wCRCin ^= (wChar << 8);
            for(int i = 0;i < 8;i++)
            {
                if(wCRCin & 0x8000)
                    wCRCin = (wCRCin << 1) ^ wCPoly;
                else
                    wCRCin = wCRCin << 1;
            }
        }
        return (wCRCin) ;
}


void InvertUint8(unsigned char *dBuf, unsigned char *srcBuf)
{
    int i;
    unsigned char tmp[4];
    tmp[0] = 0;
    for(i=0;i< 8;i++)
    {
        if(srcBuf[0]& (1 << i))
            tmp[0]|=1<<(7-i);
    }
    dBuf[0] = tmp[0];
}


void InvertUint16(unsigned short *dBuf, unsigned short *srcBuf)
{
    int i;
    unsigned short tmp[4];
    tmp[0] = 0;
    for(i=0;i< 16;i++)
    {
        if(srcBuf[0]& (1 << i))
            tmp[0]|=1<<(15 - i);
    }
    dBuf[0] = tmp[0];
}


unsigned short CRC16_X25(unsigned char *pdest, int usDataLen)
{
    unsigned short wCRCin = 0xFFFF;
    unsigned short wCPoly = 0x1021;
    unsigned char wChar = 0;

    while (usDataLen--)
    {
        wChar = *(pdest++);
        InvertUint8(&wChar,&wChar);
        wCRCin ^= (wChar << 8);
        for(int i = 0;i < 8;i++)
        {
            if(wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    InvertUint16(&wCRCin,&wCRCin);
    return (wCRCin^0xFFFF) ;
}


void utf8ToGb2312(std::string& strUtf8)
{
    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
    QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

    QString strUnicode= utf8Codec->toUnicode(strUtf8.c_str());
    QByteArray ByteGb2312= gb2312Codec->fromUnicode(strUtf8.c_str());

    strUtf8= ByteGb2312.data();
}

/*QString utf8ToGb2312(const char *strUtf8)
{
    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
    QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

    QString strUnicode= utf8Codec ->toUnicode(strUtf8);
    QByteArray ByteGb2312= gb2312Codec ->fromUnicode(strUnicode);

    strUtf8= ByteGb2312.data();
    return QString::fromLocal8Bit(strUtf8);
}*/

void gb2312ToUtf8(std::string& strGb2312)
{

    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
    QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

    QString strUnicode= gb2312Codec->toUnicode(strGb2312.c_str());
    QByteArray ByteUtf8= utf8Codec->fromUnicode(strUnicode);

    strGb2312= ByteUtf8.data();
}
#ifdef Q_OS_WIN
void UTF8_to_GB2312_Win(const char* utf8, string &gb2312_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len+1];
    memset(wstr, 0, len+1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len+1];
    memset(str, 0, len+1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    delete[] wstr;
    gb2312_str = str;
    delete[] str;
    return;
}

void GB2312_to_UTF8_Win(const char* gb2312, string& utf8_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len+1];
    memset(wstr, 0, len+1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len+1];
    memset(str, 0, len+1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    delete[] wstr;
    utf8_str = str;
    delete[] str;
    return;
}
#endif
