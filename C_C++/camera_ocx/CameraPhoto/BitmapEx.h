// BitmapEx.h: interface for the CBitmapEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_)
#define AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define _PI	3.1415926f											// Value of PI
#define _BITS_PER_PIXEL_32	32									// 32-bit color depth
#define _BITS_PER_PIXEL_24	24									// 24-bit color depth
#define _PIXEL	DWORD											// Pixel
#define _RGB(r,g,b)	(((r) << 16) | ((g) << 8) | (b))			// Convert to RGB
#define _GetRValue(c)	((BYTE)(((c) & 0x00FF0000) >> 16))		// Red color component
#define _GetGValue(c)	((BYTE)(((c) & 0x0000FF00) >> 8))		// Green color component
#define _GetBValue(c)	((BYTE)((c) & 0x000000FF))				// Blue color component


typedef long fixed;												// Our new fixed point type
#define itofx(x) ((x) << 8)										// Integer to fixed point
#define ftofx(x) (long)((x) * 256)								// Float to fixed point
#define dtofx(x) (long)((x) * 256)								// Double to fixed point
#define fxtoi(x) ((x) >> 8)										// Fixed point to integer
#define fxtof(x) ((float) (x) / 256)							// Fixed point to float
#define fxtod(x) ((double)(x) / 256)							// Fixed point to double
#define Mulfx(x,y) (((x) * (y)) >> 8)							// Multiply a fixed by a fixed
#define Divfx(x,y) (((x) << 8) / (y))							// Divide a fixed by a fixed


typedef struct __POINT
{
	long x;
	long y;

} _POINT, *_LPPOINT;

typedef struct __QUAD
{
	_POINT p1;
	_POINT p2;
	_POINT p3;
	_POINT p4;

} _QUAD, *_LPQUAD;

typedef enum __RESAMPLE_MODE
{
	RM_NEARESTNEIGHBOUR = 0,
	RM_BILINEAR,
	RM_BICUBIC,

} _RESAMPLE_MODE;

typedef enum __GRADIENT_MODE
{
	GM_NONE = 0x00,
	GM_HORIZONTAL = 0x01,
	GM_VERTICAL = 0x02,
	GM_RADIAL = 0x04

} _GRADIENT_MODE;


class CBitmapEx
{
public:
	// Public methods
	CBitmapEx();
	virtual ~CBitmapEx();
	void Create(long width, long height);
	void Create(CBitmapEx& bitmapEx);
	void Load(LPTSTR lpszBitmapFile);
	void Load(LPBYTE lpBitmapData);
	void Save(LPTSTR lpszBitmapFile);
	void Save(LPBYTE lpBitmapData);
	void Scale(long horizontalPercent=100, long verticalPercent=100);
	void Rotate(long degrees=0, _PIXEL bgColor=_RGB(0,0,0));
	void FlipHorizontal();
	void FlipVertical();
	void MirrorLeft();
	void MirrorRight();
	void MirrorTop();
	void MirrorBottom();
	void Clear(_PIXEL clearColor=_RGB(0,0,0));
	void Negative();
	void Grayscale();
	void Sepia(long depth=34);
	void Emboss();
	void Engrave();
	void Pixelize(long size=4);
	void Draw(HDC hDC);
	void Draw(HDC hDC, long dstX, long dstY);
	void Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY);
	void Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void DrawBlended(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void DrawBlended(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	LPBITMAPFILEHEADER GetFileInfo() {return &m_bfh;}
	LPBITMAPINFOHEADER GetInfo() {return &m_bih;}
	long GetWidth() {return m_bih.biWidth;}
	long GetHeight() {return m_bih.biHeight;}
	long GetPitch() {return m_iPitch;}
	long GetBpp() {return m_iBpp;}
	long GetPaletteEntries() {return m_iPaletteEntries;}
	LPRGBQUAD GetPalette() {return m_lpPalette;}
	DWORD GetSize() {return m_dwSize;}
	LPBYTE GetData() {return m_lpData;}
	void SetResampleMode(_RESAMPLE_MODE mode=RM_NEARESTNEIGHBOUR) {m_ResampleMode = mode;}
	_RESAMPLE_MODE GetResampleMode() {return m_ResampleMode;}
	BOOL IsValid() {return (m_dwSize > 0);}
	_PIXEL GetPixel(long x, long y);
	void SetPixel(long x, long y, _PIXEL pixel);

	//wangjun
	void LoadImageFile(LPTSTR lpszImageFile);
	void SaveJPGFile(LPTSTR lpszImageFile);

private:
	// Private methods
	void _ConvertTo32Bpp();
	void _ConvertTo24Bpp();
	void _ScaleNearestNeighbour(long horizontalPercent, long verticalPercent);
	void _ScaleBilinear(long horizontalPercent, long verticalPercent);
	void _ScaleBicubic(long horizontalPercent, long verticalPercent);
	void _RotateNearestNeighbour(long degrees, _PIXEL bgColor);
	void _RotateBilinear(long degrees, _PIXEL bgColor);
	void _RotateBicubic(long degrees, _PIXEL bgColor);
	void _DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight);
	void _DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha);
	void _DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor=_RGB(0,0,0));
	void _DrawBlendedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void _DrawBlendedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);
	void _DrawBlendedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode=GM_NONE);

	// wanhjun
	HANDLE _dibFromBitmap(HBITMAP hBitmap);	//DDB->DIB
	int _DIBNumColors (LPBITMAPINFOHEADER lpbi);
	HBITMAP _extractBitmap(IPicture* pPicture);
	int _GetCodecClsid(const WCHAR* format, CLSID* pClsid);

private:
	// Private members
	BITMAPFILEHEADER m_bfh;
	BITMAPINFOHEADER m_bih;
	long m_iPaletteEntries;
	RGBQUAD m_lpPalette[256];
	long m_iPitch;
	long m_iBpp;
	DWORD m_dwSize;
	LPBYTE m_lpData;
	_RESAMPLE_MODE m_ResampleMode;

};

#endif // !defined(AFX_BITMAPEX_H__80F20A52_B43F_42C5_B182_AC8D27BF5C0E__INCLUDED_)
