#include "stdafx.h"
#include "Base64.h"

int DecodeBase64(char * pInput, char * pOutput)
{
	int i = 0;
	int iCnt = 0;
	int iSrcLen = (int)strlen(pInput);

	char * p = pOutput;

	for (i=0; i<iSrcLen; i++)
	{
		if (pInput[i] > 127) continue;
		if (pInput[i] == '=') return p-pOutput+1;

		BYTE a = BVal(pInput[i]);
		if (a == 255) continue;
		
		switch (iCnt)
		{
		case 0:
			{
				*p = a << 2;
				iCnt++;
			}
			break;

		case 1:
			{
				*p++ |= a >> 4;
				*p = a << 4;
				iCnt++;
			}
			break;

		case 2:
			{
				*p++ |= a >> 2;
				*p = a << 6;
				iCnt++;
			}
			break;

		case 3:
			{
				*p++ |= a;
				iCnt = 0;
			}
			break;
		} 
	}

	*p = 0x00;
	return p-pOutput;
}

int EncodeBase64(BYTE * pInput,int iInputLen,BYTE * pOutput)
{
	int i = 0;
	int loop = 0;
	int remain = 0;
	int iDstLen = 0;
	int iSrcLen = iInputLen;
	
	loop = iSrcLen/3;
	remain = iSrcLen%3;

	// also can encode native char one by one as decode method
	// but because all of char in native string  is to be encoded so encode 3-chars one time is easier.

	for (i=0; i<loop; i++)
	{
		BYTE a1 = (pInput[i*3] >> 2);
		BYTE a2 = ( ((pInput[i*3] & 0x03) << 4) | (pInput[i*3+1] >> 4) );
		BYTE a3 = ( ((pInput[i*3+1] & 0x0F) << 2) | ((pInput[i*3+2] & 0xC0) >> 6) );
		BYTE a4 = (pInput[i*3+2] & 0x3F);

		pOutput[i*4] = AVal(a1);
		pOutput[i*4+1] = AVal(a2);
		pOutput[i*4+2] = AVal(a3);
		pOutput[i*4+3] = AVal(a4);
	}

	iDstLen = i*4;

	if (remain == 1)
	{
		// should pad two equal sign
		i = iSrcLen-1;
		BYTE a1 = (pInput[i] >> 2);
		BYTE a2 = ((pInput[i] & 0x03) << 4);
		
		pOutput[iDstLen++] = AVal(a1);
		pOutput[iDstLen++] = AVal(a2);
		pOutput[iDstLen++] = '=';
		pOutput[iDstLen++] = '=';
		pOutput[iDstLen] = 0x00;
	}
	else if (remain == 2)
	{
		// should pad one equal sign
		i = iSrcLen-2;
		BYTE a1 = (pInput[i] >> 2);
		BYTE a2 = ( ((pInput[i] & 0x03) << 4) | (pInput[i+1] >> 4));
		BYTE a3 = ( (pInput[i+1] & 0x0F) << 2);

		pOutput[iDstLen++] = AVal(a1);
		pOutput[iDstLen++] = AVal(a2);
		pOutput[iDstLen++] = AVal(a3);
		pOutput[iDstLen++] = '=';
		pOutput[iDstLen] = 0x00;
	}
	else
	{
		// just division by 3
		pOutput[iDstLen] = 0x00;
	}

	return iDstLen;
}