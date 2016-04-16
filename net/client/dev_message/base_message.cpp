#include "base_message.h"

namespace hx_net
{

	base_message::base_message(void)
        //:dev_agent_(new HDevAgent)
	{
	}
	base_message::~base_message(void)
	{
	}
    /*unsigned long base_message::CRC16_A001( unsigned char * ptr, int len )
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
    }*/

	void base_message::Char2Hex(unsigned char ch, char* szHex)
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
    void base_message::CharStr2HexStr(unsigned char const* pucCharStr, string &pszHexStr, int iSize)
	{
		
		int i;
		char szHex[3];
		pszHexStr[0] = 0;
		for(i=0; i<iSize; i++)
		{
			Char2Hex(pucCharStr[i], szHex);
			//strcat(pszHexStr, szHex);
			//string xx;
			pszHexStr.append(szHex);
			pszHexStr.append(" ");
        }
	}
}
