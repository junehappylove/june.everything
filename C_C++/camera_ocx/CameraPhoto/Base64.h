
const BYTE Base64IdxTab[128] =
{
	255,255,255,255,  255,255,255,255,  255,255,255,255,  255,255,255,255,
	255,255,255,255,  255,255,255,255,  255,255,255,255,  255,255,255,255,
	255,255,255,255,  255,255,255,255,  255,255,255,62,   255,255,255,63,
	52,53,54,55,      56,57,58,59,      60,61,255,255,    255,255,255,255,
	255,0,1,2,        3,4,5,6,          7,8,9,10,         11,12,13,14,
	15,16,17,18,      19,20,21,22,      23,24,25,255,     255,255,255,255,
	255,26,27,28,     29,30,31,32,      33,34,35,36,      37,38,39,40,
	41,42,43,44,      45,46,47,48,      49,50,51,255,     255,255,255,255
};

#define BVal(x) Base64IdxTab[x]

int DecodeBase64(char * pInput, char * pOutput);

const BYTE Base64ValTab[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define AVal(x) Base64ValTab[x]

int EncodeBase64(BYTE * pInput, int iInputLen, BYTE * pOutput);

#define DCD_ONCE_LEN	400*1024
#define CDC_ONCE_LEN	300*1024