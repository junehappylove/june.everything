// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

/* Data structure for MD5 (Message-Digest) computation */
typedef struct{
	ULONG i[2];
	ULONG buf[4];
	unsigned char in[64];
	unsigned char digest[16];
}MD5_CTX;

#define MD5_DIGEST_LEN 16
#define PROTO_LIST(list) list

/*
* MTS: Each of these assumes MD5_CTX is locked against simultaneous use.
*/
typedef void (WINAPI* PMD5Init) PROTO_LIST ((MD5_CTX *));
typedef void (WINAPI* PMD5Update) PROTO_LIST ((MD5_CTX *, const unsigned char *, unsigned int));
typedef void (WINAPI* PMD5Final ) PROTO_LIST ((MD5_CTX *));

PMD5Init MD5Init = NULL;
PMD5Update MD5Update = NULL;
PMD5Final MD5Final = NULL;

const char *Hex2ASC(const BYTE *Hex, int Len){
	static char  ASC[4096 * 2];
	int    i; 
 
	for (i = 0; i < Len; i++){
		ASC[i * 2] = "0123456789abcdef"[Hex[i] >> 4];
		ASC[i * 2 + 1] = "0123456789abcdef"[Hex[i] & 0x0F];
	}
	ASC[i * 2] = '/0';
 
	return ASC;
}

int main(int argc, char* argv[]){
	//printf("Hello World!\n");
	MD5_CTX ctx;
	//unsigned char* str = "/9j/4AAQSkZJRgABAQEBLAEsAAD=";
	unsigned char buf[] = "/9j/4AAQSkZJRgABAQEBLAEsAAD=";//598367

	HINSTANCE hDLL;

	if ( (hDLL = LoadLibrary("advapi32.dll")) > 0 ){
		MD5Init = (PMD5Init)GetProcAddress(hDLL,"MD5Init");
		MD5Update = (PMD5Update)GetProcAddress(hDLL,"MD5Update");
		MD5Final = (PMD5Final)GetProcAddress(hDLL,"MD5Final");

		MD5Init(&ctx);
		MD5Update(&ctx,buf,28);
		MD5Final(&ctx);

		printf(Hex2ASC(ctx.digest,16));
		printf("\n");
		printf("bul size:%d\n",sizeof(buf)/sizeof(char));
	}//E034FB6B 66AACC1D 48F445DD FB08DA98 0
	return 0;
}

