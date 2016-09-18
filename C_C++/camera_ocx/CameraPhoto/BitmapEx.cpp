// BitmapEx.cpp: implementation of the CBitmapEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitmapEx.h"
#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitmapEx::CBitmapEx()
{
	// Init members
	m_lpData = NULL;
	m_dwSize = 0;
	m_iPaletteEntries = 0;
	m_ResampleMode = RM_NEARESTNEIGHBOUR;
}

CBitmapEx::~CBitmapEx()
{
	// Deinit members
	if (m_lpData != NULL)
	{
		free(m_lpData);
		m_lpData = NULL;
	}
}

void CBitmapEx::Create(long width, long height)
{
	// Deinit members
	if (m_lpData != NULL)
	{
		free(m_lpData);
		m_lpData = NULL;
	}

	// Format BITMAPINFOHEADER info
	m_iBpp = _BITS_PER_PIXEL_32 >> 3;
	memset(&m_bih, 0, sizeof(BITMAPINFOHEADER));
	m_bih.biSize = sizeof(BITMAPINFOHEADER);
	m_bih.biWidth = max(0, width);
	m_bih.biHeight = max(0, height);
	m_bih.biPlanes = 1;
	m_bih.biBitCount = (WORD)(m_iBpp << 3);
	m_iPaletteEntries = 0;
	m_iPitch = m_bih.biWidth * m_iBpp;
	m_dwSize = m_iPitch * m_bih.biHeight;
	m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));

	// Format BITMAPFILEHEADER info
	memset(&m_bfh, 0, sizeof(BITMAPFILEHEADER));
	m_bfh.bfType = 'MB';
	m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
	m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}

void CBitmapEx::Create(CBitmapEx& bitmapEx)
{
	// Check for valid bitmap
	if (bitmapEx.IsValid())
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		// Calculate bitmap params
		m_iBpp = bitmapEx.GetBpp();
		m_iPitch = bitmapEx.GetPitch();
		memcpy(&m_bfh, bitmapEx.GetFileInfo(), sizeof(BITMAPFILEHEADER));
		memcpy(&m_bih, bitmapEx.GetInfo(), sizeof(BITMAPINFOHEADER));
		m_dwSize = bitmapEx.GetSize();
		m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
		memcpy(m_lpData, bitmapEx.GetData(), m_dwSize*sizeof(BYTE));
		m_iPaletteEntries = bitmapEx.GetPaletteEntries();
		memcpy(m_lpPalette, bitmapEx.GetPalette(), m_iPaletteEntries*sizeof(RGBQUAD));
		m_ResampleMode = bitmapEx.GetResampleMode();
	}
}

void CBitmapEx::Load(LPTSTR lpszBitmapFile)
{
	// Check for valid .BMP file path
	if (lpszBitmapFile != NULL)
	{
		// Open .BMP file
		FILE* file = _tfopen(lpszBitmapFile, _T("rb"));
		if (file != NULL)
		{
			// Deinit members
			if (m_lpData != NULL)
			{
				free(m_lpData);
				m_lpData = NULL;
			}

			// Read BITMAPFILEHEADER info
			fread(&m_bfh, 1, sizeof(BITMAPFILEHEADER), file);

			// Read BITMAPINFOHEADER info
			fread(&m_bih, 1, sizeof(BITMAPINFOHEADER), file);

			// Calculate pitch
			m_iBpp = m_bih.biBitCount >> 3;
			m_iPitch = m_iBpp * m_bih.biWidth;
			while ((m_iPitch & 3) != 0)
				m_iPitch++;

			// Check for bit-depth (8, 16, 24 and 32bpp only)
			if (m_bih.biBitCount >= 8)
			{
				if (m_bih.biBitCount == 8)
				{
					// Calculate palette entries
					m_iPaletteEntries = m_bih.biClrUsed;
					if (m_iPaletteEntries == 0)
						m_iPaletteEntries = 256;

					// Read palette info
					fread(m_lpPalette, m_iPaletteEntries, sizeof(RGBQUAD), file);
				}

				// Read image data
				m_dwSize = m_iPitch * m_bih.biHeight;
				m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
				fread(m_lpData, m_dwSize, sizeof(BYTE), file);

				// Convert to 32bpp bitmap
				_ConvertTo32Bpp();
			}

			// Close .BMP file
			fclose(file);
		}
	}
}

void CBitmapEx::Load(LPBYTE lpBitmapData)
{
	// Check for valid bitmap data buffer
	if (lpBitmapData != NULL)
	{
		// Deinit members
		if (m_lpData != NULL)
		{
			free(m_lpData);
			m_lpData = NULL;
		}

		/*
		// Read BITMAPFILEHEADER info
		memcpy(&m_bfh, lpBitmapData, sizeof(BITMAPFILEHEADER));

		// Read BITMAPINFOHEADER info
		memcpy(&m_bih, lpBitmapData+sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
		*/
		memcpy(&m_bih, lpBitmapData, sizeof(BITMAPINFOHEADER));

		// Calculate pitch
		m_iBpp = m_bih.biBitCount >> 3;
		m_iPitch = m_iBpp * m_bih.biWidth;
		while ((m_iPitch & 3) != 0)
			m_iPitch++;

		// Check for bit-depth (8, 16, 24 and 32bpp only)
		if (m_bih.biBitCount >= 8)
		{
			if (m_bih.biBitCount == 8)
			{
				// Calculate palette entries
				m_iPaletteEntries = m_bih.biClrUsed;
				if (m_iPaletteEntries == 0)
					m_iPaletteEntries = 256;

				// Read palette info
				memcpy(m_lpPalette, lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), m_iPaletteEntries*sizeof(RGBQUAD));
			}

			// Read image data
			m_dwSize = m_iPitch * m_bih.biHeight;
			m_lpData = (LPBYTE)malloc(m_dwSize*sizeof(BYTE));
			//memcpy(m_lpData, lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+m_iPaletteEntries*sizeof(RGBQUAD), m_dwSize*sizeof(BYTE));
			memcpy(m_lpData, lpBitmapData+sizeof(BITMAPINFOHEADER)+m_iPaletteEntries*sizeof(RGBQUAD), m_dwSize*sizeof(BYTE));

			// Convert to 32bpp bitmap
			_ConvertTo32Bpp();
		}
	}
}

void CBitmapEx::Save(LPTSTR lpszBitmapFile)
{
	// Check for valid .BMP file path
	if (lpszBitmapFile != NULL)
	{
		// Convert to 24bpp bitmap
		_ConvertTo24Bpp();

		// Open .BMP file
		FILE* file = _tfopen(lpszBitmapFile, _T("wb"));
		if (file != NULL)
		{
			// Write BITMAPFILEHEADER info
			fwrite(&m_bfh, 1, sizeof(BITMAPFILEHEADER), file);

			// Write BITMAPINFOHEADER info
			fwrite(&m_bih, 1, sizeof(BITMAPINFOHEADER), file);

			// Check for palettized bitmap
			if (m_bih.biBitCount == 8)
			{
				// Write palette info
				fwrite(m_lpPalette, m_iPaletteEntries, sizeof(RGBQUAD), file);
			}

			// Write image data
			fwrite(m_lpData, m_dwSize, sizeof(BYTE), file);

			// Close .BMP file
			fclose(file);
		}

		// Convert to 32bpp bitmap
		_ConvertTo32Bpp();
	}
}

void CBitmapEx::Save(LPBYTE lpBitmapData)
{
	// Check for valid bitmap data buffer
	if (lpBitmapData != NULL)
	{
		// Convert to 24bpp bitmap
		_ConvertTo24Bpp();

		// Write BITMAPFILEHEADER info
		memcpy(lpBitmapData, &m_bfh, sizeof(BITMAPFILEHEADER));

		// Write BITMAPINFOHEADER info
		memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER), &m_bih, sizeof(BITMAPINFOHEADER));

		// Check for palettized bitmap
		if (m_bih.biBitCount == 8)
		{
			// Write palette info
			memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), m_lpPalette, m_iPaletteEntries*sizeof(RGBQUAD));
		}

		// Write image data
		memcpy(lpBitmapData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+m_iPaletteEntries*sizeof(RGBQUAD), m_lpData, m_dwSize*sizeof(BYTE));

		// Convert to 32bpp bitmap
		_ConvertTo32Bpp();
	}
}

void CBitmapEx::_ConvertTo32Bpp()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate new params
		long _bpp = (_BITS_PER_PIXEL_32 >> 3);
		long _pitch = m_bih.biWidth * _bpp;

		// Create temporary bitmap
		DWORD dwSize = _pitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Convert bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = 0;
			dwSrcHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				switch (m_bih.biBitCount)
				{
					case 8:
						{
							BYTE red = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbRed;
							BYTE green = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbGreen;
							BYTE blue = m_lpPalette[m_lpData[dwSrcTotalOffset]].rgbBlue;
							lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
						}
						break;

					case 16:
						{
							LPWORD lpSrcData = (LPWORD)m_lpData;
							BYTE red = (lpSrcData[dwSrcTotalOffset>>1] & 0x7C00) >> 10;
							BYTE green = (lpSrcData[dwSrcTotalOffset>>1] & 0x03E0) >> 5;
							BYTE blue = lpSrcData[dwSrcTotalOffset>>1] & 0x001F;
							lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
						}
						break;

					case 24:
						{
							lpDstData[dwDstTotalOffset>>2] = _RGB(m_lpData[dwSrcTotalOffset+2], m_lpData[dwSrcTotalOffset+1], m_lpData[dwSrcTotalOffset]);
						}
						break;

					case 32:
						{
							LPDWORD lpSrcData = (LPDWORD)m_lpData;
							lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
						}
						break;
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += _bpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_iBpp = _bpp;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
		m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
		m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		m_bih.biBitCount = _BITS_PER_PIXEL_32;
	}
}

void CBitmapEx::_ConvertTo24Bpp()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate new params
		long _bpp = (_BITS_PER_PIXEL_24 >> 3);
		long _pitch = m_bih.biWidth * _bpp;
		while ((_pitch & 3) != 0)
			_pitch++;

		// Create temporary bitmap
		DWORD dwSize = _pitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Convert bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = 0;
			dwSrcHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpData[dwDstTotalOffset+2] = _GetRValue(lpSrcData[dwSrcTotalOffset>>2]);
				lpData[dwDstTotalOffset+1] = _GetGValue(lpSrcData[dwSrcTotalOffset>>2]);
				lpData[dwDstTotalOffset] = _GetBValue(lpSrcData[dwSrcTotalOffset>>2]);

				// Update destination horizontal offset
				dwDstHorizontalOffset += _bpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_iBpp = _bpp;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
		m_bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_dwSize;
		m_bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		m_bih.biBitCount = _BITS_PER_PIXEL_24;
	}
}

void CBitmapEx::_ScaleNearestNeighbour(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(0, horizontalPercent);
		long _verticalPercent = max(0, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBilinear(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(0, horizontalPercent);
		long _verticalPercent = max(0, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_ScaleBicubic(long horizontalPercent, long verticalPercent)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate scaling params
		long _horizontalPercent = max(0, horizontalPercent);
		long _verticalPercent = max(0, verticalPercent);
		float dx = 100.0f / (float)_horizontalPercent;
		float dy = 100.0f / (float)_verticalPercent;
		long _width = (long)((1.0f/dx) * (float)m_bih.biWidth + 0.5f);
		long _height = (long)((1.0f/dy) * (float)m_bih.biHeight + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Scale bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
				if ((m < 1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
				if (n >= m_bih.biWidth-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
				if (n >= m_bih.biWidth-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
				if (m >= m_bih.biHeight-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
				if (m >= m_bih.biHeight-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
				if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::Scale(long horizontalPercent, long verticalPercent)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Scale bitmap using nearest neighbour interpolation algorithm
				_ScaleNearestNeighbour(horizontalPercent, verticalPercent);
			}
			break;

		case RM_BILINEAR:
			{
				// Scale bitmap using bilinear interpolation algorithm
				_ScaleBilinear(horizontalPercent, verticalPercent);
			}
			break;

		case RM_BICUBIC:
			{
				// Scale bitmap using bicubic interpolation algorithm
				_ScaleBicubic(horizontalPercent, verticalPercent);
			}
			break;
	}
}

void CBitmapEx::_RotateNearestNeighbour(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)m_bih.biWidth*cos(_angle)) + abs((float)m_bih.biHeight*sin(_angle)) + 0.5f);
		long _height = (long)(abs((float)m_bih.biWidth*sin(_angle)) + abs((float)m_bih.biHeight*cos(_angle)) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_RotateBilinear(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)m_bih.biWidth*cos(_angle)) + abs((float)m_bih.biHeight*sin(_angle)) + 0.5f);
		long _height = (long)(abs((float)m_bih.biWidth*sin(_angle)) + abs((float)m_bih.biHeight*cos(_angle)) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));
		fixed f_1 = itofx(1);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					fixed f_f = f_m - itofx(m);
					fixed f_g = f_n - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((n >= m_bih.biWidth-1) || (m >= m_bih.biHeight-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_red = Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4);
					fixed f_green = Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4);
					fixed f_blue = Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4);
					BYTE red = (BYTE)max(0, min(255, fxtoi(f_red)));
					BYTE green = (BYTE)max(0, min(255, fxtoi(f_green)));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(f_blue)));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::_RotateBicubic(long degrees, _PIXEL bgColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate rotating params
		float _angle = ((float)-degrees/180.0f) * _PI;
		long _width = (long)(abs((float)m_bih.biWidth*cos(_angle)) + abs((float)m_bih.biHeight*sin(_angle)) + 0.5f);
		long _height = (long)(abs((float)m_bih.biWidth*sin(_angle)) + abs((float)m_bih.biHeight*cos(_angle)) + 0.5f);
		long _pitch = m_iBpp * _width;
		while ((_pitch & 3) != 0)
			_pitch++;
		fixed f_0_5 = ftofx(0.5f);
		fixed f_H = itofx(m_bih.biHeight/2);
		fixed f_W = itofx(m_bih.biWidth/2);
		fixed f_cos = ftofx(cos(-_angle));
		fixed f_sin = ftofx(sin(-_angle));
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Create temporary bitmap
		DWORD dwSize = _pitch * _height;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Rotate bitmap
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<_height; i++)
		{
			dwDstHorizontalOffset = 0;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i-_height/2);
				fixed f_j = itofx(j-_width/2);
				fixed f_m = Mulfx(f_j,f_sin) + Mulfx(f_i,f_cos) + f_0_5 + f_H;
				fixed f_n = Mulfx(f_j,f_cos) - Mulfx(f_i,f_sin) + f_0_5 + f_W;
				long m = fxtoi(f_m);
				long n = fxtoi(f_n);
				if ((m > 0) && (m < m_bih.biHeight-1) && (n > 0) && (n < m_bih.biWidth-1))
				{
					fixed f_f = f_m - itofx(m);
					fixed f_g = f_n - itofx(n);
					dwSrcTotalOffset = m*m_iPitch + n*m_iBpp;
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset - m_iPitch - m_iBpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset - m_iPitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset - m_iPitch + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset - m_iPitch + m_iBpp + m_iBpp;
					if ((m < 1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - m_iBpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + m_iBpp;
					if (n >= m_bih.biWidth-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + m_iBpp + m_iBpp;
					if (n >= m_bih.biWidth-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset + m_iPitch;
					if (m >= m_bih.biHeight-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-1) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset + m_iPitch + m_iPitch - m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset + m_iPitch + m_iPitch;
					if (m >= m_bih.biHeight-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset + m_iPitch + m_iPitch + m_iBpp + m_iBpp;
					if ((m >= m_bih.biHeight-2) || (n >= m_bih.biWidth-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}
				else
					lpDstData[dwDstTotalOffset>>2] = bgColor;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += _pitch;
		}

		// Update bitmap info
		m_iPitch = _pitch;
		m_bih.biWidth = _width;
		m_bih.biHeight = _height;
		free(m_lpData);
		m_dwSize = dwSize;
		m_lpData = lpData;
	}
}

void CBitmapEx::Rotate(long degrees, _PIXEL bgColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Rotate bitmap using nearest neighbour interpolation algorithm
				_RotateNearestNeighbour(degrees, bgColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Rotate bitmap using bilinear interpolation algorithm
				_RotateBilinear(degrees, bgColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Rotate bitmap using bicubic interpolation algorithm
				_RotateBicubic(degrees, bgColor);
			}
			break;
	}
}

void CBitmapEx::FlipHorizontal()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate flipping params
		long _width = m_bih.biWidth / 2;

		// Flip bitmap horizontaly
		DWORD dwDstHorizontalStartOffset = 0;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixelTmp = lpDstData[dwDstTotalOffset>>2];
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];
				lpSrcData[dwSrcTotalOffset>>2] = pixelTmp;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset -= m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::FlipVertical()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate flipping params
		long _height = m_bih.biHeight / 2;

		// Flip bitmap verticaly
		DWORD dwDstTotalOffset = 0;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpTmpData = (LPDWORD)malloc(m_iPitch*sizeof(BYTE));
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpTmpData, lpDstData+(dwDstTotalOffset>>2), m_iPitch*sizeof(BYTE));
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));
			memcpy(lpSrcData+(dwSrcTotalOffset>>2), lpTmpData, m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset += m_iPitch;

			// Update source total offset
			dwSrcTotalOffset -= m_iPitch;
		}
		free(lpTmpData);
	}
}

void CBitmapEx::MirrorLeft()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _width = m_bih.biWidth / 2;

		// Mirror bitmap left
		DWORD dwDstHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = 0;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset -= m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::MirrorRight()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _width = m_bih.biWidth / 2;

		// Mirror bitmap right
		DWORD dwDstHorizontalStartOffset = 0;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = 0;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = (m_bih.biWidth-1) * m_iBpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = 0;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset -= m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset += m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::MirrorTop()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _height = m_bih.biHeight / 2;

		// Mirror bitmap top
		DWORD dwDstTotalOffset = 0;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset += m_iPitch;

			// Update source total offset
			dwSrcTotalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::MirrorBottom()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate mirroring params
		long _height = m_bih.biHeight / 2;

		// Mirror bitmap bottom
		DWORD dwDstTotalOffset = (m_bih.biHeight-1) * m_iPitch;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		for (long i=0; i<_height; i++)
		{
			// Update bitmap
			memcpy(lpDstData+(dwDstTotalOffset>>2), lpSrcData+(dwSrcTotalOffset>>2), m_iPitch*sizeof(BYTE));

			// Update destination total offset
			dwDstTotalOffset -= m_iPitch;

			// Update source total offset
			dwSrcTotalOffset += m_iPitch;
		}
	}
}

void CBitmapEx::Clear(_PIXEL clearColor)
{
	// Check for valid bitmap
	if (IsValid())
	{
/*		// Clear bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				lpDstData[dwTotalOffset>>2] = clearColor;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Clear bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov eax, clearColor
			mov ecx, dwSize
			rep stosd
		}
	}
}

void CBitmapEx::Negative()
{
	// Check for valid bitmap
	if (IsValid())
	{
/*		// Negative bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				_PIXEL oldPixel = lpDstData[dwTotalOffset>>2];
				_PIXEL newPixel = _RGB(255-_GetRValue(oldPixel),255-_GetGValue(oldPixel),255-_GetBValue(oldPixel));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Negative bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov eax, [edi]
			xor eax, 0x00FFFFFF
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Grayscale()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate grayscale params
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);

/*		// Grayscale bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Grayscale bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov ebx, [edi]
			mov eax, ebx
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			push eax
			xor edx, edx
			pop eax
			mov edx, eax
			pop eax
			add edx, eax
			pop eax
			add edx, eax
			shr edx, 8
			and edx, 0x000000FF
			mov eax, edx
			shl eax, 8
			or eax, edx
			shl eax, 8
			or eax, edx
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Sepia(long depth)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate sepia params
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		long _depth = max(0, min(100, depth));
		fixed f_depth = itofx(_depth);
		fixed f_depth2 = Mulfx(itofx(2),f_depth);

/*		// Sepia bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				fixed f_red = itofx(_GetRValue(lpDstData[dwTotalOffset>>2]));
				fixed f_green = itofx(_GetGValue(lpDstData[dwTotalOffset>>2]));
				fixed f_blue = itofx(_GetBValue(lpDstData[dwTotalOffset>>2]));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				BYTE red = (BYTE)fxtoi(f_value+f_depth2);
				if (red < ((_depth<<1)-1))
					red = 255;
				BYTE green = (BYTE)fxtoi(f_value+f_depth);
				if (green < (_depth-1))
					green = 255;
				BYTE blue = (BYTE)fxtoi(f_value);
				lpDstData[dwTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}*/

		// Sepia bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSize = m_dwSize >> 2;
		DWORD _depthm1 = _depth - 1;
		DWORD _depth2 = _depth << 1;
		DWORD _depth2m1 = _depth2 - 1;
		__asm {
			mov edi, lpDstData
			mov ecx, 0
label_loop:
			mov ebx, [edi]
			mov eax, ebx
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			push eax
			mov eax, ebx
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			push eax
			xor edx, edx
			pop eax
			mov edx, eax
			pop eax
			add edx, eax
			pop eax
			add edx, eax
			shr edx, 8
			and edx, 0x000000FF
			xor eax, eax
			mov ebx, edx
			add ebx, _depth2
			and ebx, 0x000000FF
			cmp ebx, _depth2m1
			jge next1
			mov ebx, 255
next1:		mov eax, ebx
			mov ebx, edx
			add ebx, _depth
			and ebx, 0x000000FF
			cmp ebx, _depthm1
			jge next2
			mov ebx, 255
next2:		shl eax, 8
			or eax, ebx
			shl eax, 8
			or eax, edx
			mov [edi], eax
			add edi, 4
			inc ecx
			cmp ecx, dwSize
			jl label_loop
		}
	}
}

void CBitmapEx::Emboss()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate emboss params
		fixed f_w[9] = {itofx(-1), itofx(-1), itofx(0),
						itofx(-1), itofx(0), itofx(1),
						itofx(0), itofx(1), itofx(1)
		};
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		fixed f_128 = itofx(128);
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Emboss bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffsets[9];
				dwSrcOffsets[0] = dwTotalOffset - m_iPitch - m_iBpp;
				if ((i < 1) || (j < 1))
					dwSrcOffsets[0] = dwTotalOffset;
				dwSrcOffsets[1] = dwTotalOffset - m_iPitch;
				if (i < 1)
					dwSrcOffsets[1] = dwTotalOffset;
				dwSrcOffsets[2] = dwTotalOffset - m_iPitch + m_iBpp;
				if ((i < 1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwTotalOffset;
				dwSrcOffsets[3] = dwTotalOffset - m_iBpp;
				if (j < 1)
					dwSrcOffsets[3] = dwTotalOffset;
				dwSrcOffsets[4] = dwTotalOffset;
				dwSrcOffsets[5] = dwTotalOffset + m_iBpp;
				if (j >= m_bih.biWidth-1)
					dwSrcOffsets[5] = dwTotalOffset;
				dwSrcOffsets[6] = dwTotalOffset + m_iPitch - m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j < 1))
					dwSrcOffsets[6] = dwTotalOffset;
				dwSrcOffsets[7] = dwTotalOffset + m_iPitch;
				if (i >= m_bih.biHeight-1)
					dwSrcOffsets[7] = dwTotalOffset;
				dwSrcOffsets[8] = dwTotalOffset + m_iPitch + m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[8] = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<9; k++)
				{
					fixed f_r = itofx(_GetRValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_g = itofx(_GetGValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_b = itofx(_GetBValue(lpSrcData[dwSrcOffsets[k]>>2]));
					f_red += Mulfx(f_r,f_w[k]);
					f_green += Mulfx(f_g,f_w[k]);
					f_blue += Mulfx(f_b,f_w[k]);
				}
				f_red = max(0, min(f_255, f_red+f_128));
				f_green = max(0, min(f_255, f_green+f_128));
				f_blue = max(0, min(f_255, f_blue+f_128));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Engrave()
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate engrave params
		fixed f_w[9] = {itofx(1), itofx(1), itofx(0),
						itofx(1), itofx(0), itofx(-1),
						itofx(0), itofx(-1), itofx(-1)
		};
		fixed f_w1 = ftofx(0.299f);
		fixed f_w2 = ftofx(0.587f);
		fixed f_w3 = ftofx(0.114f);
		fixed f_128 = itofx(128);
		fixed f_255 = itofx(255);

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Engrave bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i++)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j++)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffsets[9];
				dwSrcOffsets[0] = dwTotalOffset - m_iPitch - m_iBpp;
				if ((i < 1) || (j < 1))
					dwSrcOffsets[0] = dwTotalOffset;
				dwSrcOffsets[1] = dwTotalOffset - m_iPitch;
				if (i < 1)
					dwSrcOffsets[1] = dwTotalOffset;
				dwSrcOffsets[2] = dwTotalOffset - m_iPitch + m_iBpp;
				if ((i < 1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[2] = dwTotalOffset;
				dwSrcOffsets[3] = dwTotalOffset - m_iBpp;
				if (j < 1)
					dwSrcOffsets[3] = dwTotalOffset;
				dwSrcOffsets[4] = dwTotalOffset;
				dwSrcOffsets[5] = dwTotalOffset + m_iBpp;
				if (j >= m_bih.biWidth-1)
					dwSrcOffsets[5] = dwTotalOffset;
				dwSrcOffsets[6] = dwTotalOffset + m_iPitch - m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j < 1))
					dwSrcOffsets[6] = dwTotalOffset;
				dwSrcOffsets[7] = dwTotalOffset + m_iPitch;
				if (i >= m_bih.biHeight-1)
					dwSrcOffsets[7] = dwTotalOffset;
				dwSrcOffsets[8] = dwTotalOffset + m_iPitch + m_iBpp;
				if ((i >= m_bih.biHeight-1) || (j >= m_bih.biWidth-1))
					dwSrcOffsets[8] = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<9; k++)
				{
					fixed f_r = itofx(_GetRValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_g = itofx(_GetGValue(lpSrcData[dwSrcOffsets[k]>>2]));
					fixed f_b = itofx(_GetBValue(lpSrcData[dwSrcOffsets[k]>>2]));
					f_red += Mulfx(f_r,f_w[k]);
					f_green += Mulfx(f_g,f_w[k]);
					f_blue += Mulfx(f_b,f_w[k]);
				}
				f_red = max(0, min(f_255, f_red+f_128));
				f_green = max(0, min(f_255, f_green+f_128));
				f_blue = max(0, min(f_255, f_blue+f_128));
				fixed f_value = Mulfx(f_w1,f_red) + Mulfx(f_w2,f_green) + Mulfx(f_w3,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_value),fxtoi(f_value),fxtoi(f_value));
				lpDstData[dwTotalOffset>>2] = newPixel;

				// Update horizontal offset
				dwHorizontalOffset += m_iBpp;
			}

			// Update vertical offset
			dwVerticalOffset += m_iPitch;
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Pixelize(long size)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Calculate pixelize params
		long _size = max(1, min((m_bih.biWidth>>4), size));
		fixed f_size = Divfx(itofx(1),itofx(_size*_size));

		// Create temporary bitmap
		DWORD dwSize = m_iPitch * m_bih.biHeight;
		LPBYTE lpData = (LPBYTE)malloc(dwSize*sizeof(BYTE));

		// Pixelize bitmap
		DWORD dwHorizontalOffset;
		DWORD dwVerticalOffset = 0;
		DWORD dwTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)m_lpData;
		LPDWORD lpDstData = (LPDWORD)lpData;
		for (long i=0; i<m_bih.biHeight; i+=_size)
		{
			dwHorizontalOffset = 0;
			for (long j=0; j<m_bih.biWidth; j+=_size)
			{
				// Update total offset
				dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

				// Update bitmap
				DWORD dwSrcOffset = dwTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=0; k<_size; k++)
				{
					long m = i + k;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=0; l<_size; l++)
					{
						long n = j + l;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
						f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
						f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
					}
				}
				f_red = Mulfx(f_size,f_red);
				f_green = Mulfx(f_size,f_green);
				f_blue = Mulfx(f_size,f_blue);
				_PIXEL newPixel = _RGB(fxtoi(f_red),fxtoi(f_green),fxtoi(f_blue));
				for (long k=0; k<_size; k++)
				{
					long m = i + k;
					if (m >= m_bih.biHeight-1)
						m = m_bih.biHeight - 1;
					for (long l=0; l<_size; l++)
					{
						long n = j + l;
						if (n >= m_bih.biWidth-1)
							n = m_bih.biWidth - 1;
						dwSrcOffset = m*m_iPitch + n*m_iBpp;
						lpDstData[dwSrcOffset>>2] = newPixel;
					}
				}

				// Update horizontal offset
				dwHorizontalOffset += (_size*m_iBpp);
			}

			// Update vertical offset
			dwVerticalOffset += (_size*m_iPitch);
		}

		// Update bitmap info
		free(m_lpData);
		m_lpData = lpData;
	}
}

void CBitmapEx::Draw(HDC hDC)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Draw bitmap
		::SetDIBitsToDevice(hDC, 0, 0, m_bih.biWidth, m_bih.biHeight, 0, 0, 0, m_bih.biHeight, m_lpData, (LPBITMAPINFO)&m_bih, DIB_RGB_COLORS);
	}
}

void CBitmapEx::Draw(HDC hDC, long dstX, long dstY)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Draw bitmap
		::SetDIBitsToDevice(hDC, dstX, dstY, m_bih.biWidth, m_bih.biHeight, 0, 0, 0, m_bih.biHeight, m_lpData, (LPBITMAPINFO)&m_bih, DIB_RGB_COLORS);
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			mov eax, [esi]
			mov [edi], eax
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);
		fixed f_1malpha = f_1 - f_alpha;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			push edx
			push ebx
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop ebx
			pop edx
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									SetPixel(j, i, bitmapEx.GetPixel(n, m));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									pixel1 =  _RGB(red, green, blue);
									pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									fixed f_f = ftofx(delta*_height) - itofx(m);
									fixed f_g = ftofx(mu*_width) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									_PIXEL pixel1 =  _RGB(red, green, blue);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									SetPixel(j, i, bitmapEx.GetPixel(n, m));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::Draw(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcTopLeft = dwSrcTotalOffset;
									DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcTopRight = dwSrcTotalOffset;
									DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcBottomLeft = dwSrcTotalOffset;
									DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
									if ((n >= _width-1) || (m >= _height-1))
										dwSrcBottomRight = dwSrcTotalOffset;
									fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
									fixed f_w2 = Mulfx(f_1-f_f, f_g);
									fixed f_w3 = Mulfx(f_f, f_1-f_g);
									fixed f_w4 = Mulfx(f_f, f_g);
									_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
									_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
									_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
									_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
									fixed f_r1 = itofx(_GetRValue(pixel1));
									fixed f_r2 = itofx(_GetRValue(pixel2));
									fixed f_r3 = itofx(_GetRValue(pixel3));
									fixed f_r4 = itofx(_GetRValue(pixel4));
									fixed f_g1 = itofx(_GetGValue(pixel1));
									fixed f_g2 = itofx(_GetGValue(pixel2));
									fixed f_g3 = itofx(_GetGValue(pixel3));
									fixed f_g4 = itofx(_GetGValue(pixel4));
									fixed f_b1 = itofx(_GetBValue(pixel1));
									fixed f_b2 = itofx(_GetBValue(pixel2));
									fixed f_b3 = itofx(_GetBValue(pixel3));
									fixed f_b4 = itofx(_GetBValue(pixel4));
									BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
									BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
									BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
									pixel1 =  _RGB(red, green, blue);
									pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
									fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
									fixed f_1 = itofx(1);
									DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
									DWORD dwSrcOffsets[16];
									dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
									if ((m < 1) || (n < 1))
										dwSrcOffsets[0] = dwSrcTotalOffset;
									dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
									if (m < 1)
										dwSrcOffsets[1] = dwSrcTotalOffset;
									dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
									if ((m < 1) || (n >= _width-1))
										dwSrcOffsets[2] = dwSrcTotalOffset;
									dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
									if ((m < 1) || (n >= _width-2))
										dwSrcOffsets[3] = dwSrcTotalOffset;
									dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
									if (n < 1)
										dwSrcOffsets[4] = dwSrcTotalOffset;
									dwSrcOffsets[5] = dwSrcTotalOffset;
									dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
									if (n >= _width-1)
										dwSrcOffsets[6] = dwSrcTotalOffset;
									dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
									if (n >= _width-2)
										dwSrcOffsets[7] = dwSrcTotalOffset;
									dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
									if ((m >= _height-1) || (n < 1))
										dwSrcOffsets[8] = dwSrcTotalOffset;
									dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
									if (m >= _height-1)
										dwSrcOffsets[9] = dwSrcTotalOffset;
									dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
									if ((m >= _height-1) || (n >= _width-1))
										dwSrcOffsets[10] = dwSrcTotalOffset;
									dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
									if ((m >= _height-1) || (n >= _width-2))
										dwSrcOffsets[11] = dwSrcTotalOffset;
									dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
									if ((m >= _height-2) || (n < 1))
										dwSrcOffsets[12] = dwSrcTotalOffset;
									dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
									if (m >= _height-2)
										dwSrcOffsets[13] = dwSrcTotalOffset;
									dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
									if ((m >= _height-2) || (n >= _width-1))
										dwSrcOffsets[14] = dwSrcTotalOffset;
									dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
									if ((m >= _height-2) || (n >= _width-2))
										dwSrcOffsets[15] = dwSrcTotalOffset;
									fixed f_red=0, f_green=0, f_blue=0;
									for (long k=-1; k<3; k++)
									{
										fixed f = itofx(k)-f_f;
										fixed f_fm1 = f - f_1;
										fixed f_fp1 = f + f_1;
										fixed f_fp2 = f + f_2;
										fixed f_a = 0;
										if (f_fp2 > 0)
											f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
										fixed f_b = 0;
										if (f_fp1 > 0)
											f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
										fixed f_c = 0;
										if (f > 0)
											f_c = Mulfx(f,Mulfx(f,f));
										fixed f_d = 0;
										if (f_fm1 > 0)
											f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
										fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
										for (long l=-1; l<3; l++)
										{
											fixed f = itofx(l)-f_g;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											fixed f_R = Mulfx(f_RY,f_RX);
											long _k = ((k+1)*4) + (l+1);
											fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
											f_red += Mulfx(f_rs,f_R);
											f_green += Mulfx(f_gs,f_R);
											f_blue += Mulfx(f_bs,f_R);
										}
									}
									BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
									BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
									BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
									_PIXEL pixel1 =  _RGB(red, green, blue);
									_PIXEL pixel2 =  GetPixel(j, i);
									fixed f_sred = itofx(_GetRValue(pixel1));
									fixed f_sgreen = itofx(_GetGValue(pixel1));
									fixed f_sblue = itofx(_GetBValue(pixel1));
									fixed f_dred = itofx(_GetRValue(pixel2));
									fixed f_dgreen = itofx(_GetGValue(pixel2));
									fixed f_dblue = itofx(_GetBValue(pixel2));
									red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
									green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
									blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
									SetPixel(j, i, _RGB(red, green, blue));
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				lpDstData[dwDstTotalOffset>>2] = lpSrcData[dwSrcTotalOffset>>2];

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
				BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
				BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight);
			}
			break;
	}
}

void CBitmapEx::_DrawNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4, f_srcPixel;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov f_srcPixel, eax
			mov eax, f_srcPixel
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
			pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		fixed f_alpha = ftofx(_alphaPercent);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
				fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
				fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::Draw(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
					lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			mov [edi], eax
next1:		add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
										SetPixel(j, i, pixel);
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel = bitmapEx.GetPixel(n, m);
									if (pixel != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);
		fixed f_1malpha = f_1 - f_alpha;
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel1 != transparentColor)
				{
					_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
					fixed f_sred = itofx(_GetRValue(pixel1));
					fixed f_sgreen = itofx(_GetGValue(pixel1));
					fixed f_sblue = itofx(_GetBValue(pixel1));
					fixed f_dred = itofx(_GetRValue(pixel2));
					fixed f_dgreen = itofx(_GetGValue(pixel2));
					fixed f_dblue = itofx(_GetBValue(pixel2));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwSrcPitch = _pitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov esi, lpSrcData
			mov edi, lpDstData
			add esi, dwSrcVerticalOffset
			add esi, dwSrcHorizontalStartOffset
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			mov edx, 0
outer_loop:	mov ecx, 0
			xor ebx, ebx
inner_loop:	inc ecx
			push edx
			push ebx
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next1:		pop ebx
			pop edx
			add esi, 4
			add edi, 4
			add ebx, 4
			cmp ecx, _width
			jle inner_loop
			sub esi, ebx
			sub esi, dwSrcPitch
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _height
			jle outer_loop
		}
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height+0.5f);
									long n = (long)(mu*_width+0.5f);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										pixel1 =  _RGB(red, green, blue);
										pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_height);
									long n = (long)(mu*_width);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_height) - itofx(m);
										fixed f_g = ftofx(mu*_width) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										_PIXEL pixel1 =  _RGB(red, green, blue);
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
					lpDstData[dwDstTotalOffset>>2] = pixel;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			mov [edi], eax
next1:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
					if ((n >= _width-1) || (m >= _height-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
					BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next4
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov [edi], eax
next4:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
					if ((m < 1) || (n >= _width-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
					if ((m < 1) || (n >= _width-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
					if (n >= _width-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
					if ((m >= _height-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
					if ((m >= _height-1) || (n >= _width-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
					if ((m >= _height-1) || (n >= _width-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset - _pitch + _pitch - _bpp;
					if ((m >= _height-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset - _pitch + _pitch;
					if (m >= _height-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset - _pitch + _pitch + _bpp;
					if ((m >= _height-2) || (n >= _width-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset - _pitch + _pitch + _bpp + _bpp;
					if ((m >= _height-2) || (n >= _width-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
					BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
					BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawTransparentNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawTransparentBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawTransparentBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, transparentColor);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
										SetPixel(j, i, pixel1);
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::_DrawTransparentNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel1 != transparentColor)
				{
					_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
					fixed f_sred = itofx(_GetRValue(pixel1));
					fixed f_sgreen = itofx(_GetGValue(pixel1));
					fixed f_sblue = itofx(_GetBValue(pixel1));
					fixed f_dred = itofx(_GetRValue(pixel2));
					fixed f_dgreen = itofx(_GetGValue(pixel2));
					fixed f_dblue = itofx(_GetBValue(pixel2));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			mov ebx, _height
			sub ebx, _srcStartY
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 16
			sub ebx, eax
			dec ebx
			mov eax, ebx
			mul _pitch
			push eax
			mov ebx, _srcStartX
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 16
			add eax, ebx
			mul _bpp
			pop ebx
			add eax, ebx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next1
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next1:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1malpha = f_1 - f_alpha;

/*		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcTopLeft = dwSrcTotalOffset;
					DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcTopRight = dwSrcTotalOffset;
					DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcBottomLeft = dwSrcTotalOffset;
					DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
					if ((n >= _width-1) || (m >= _height-1))
						dwSrcBottomRight = dwSrcTotalOffset;
					fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
					fixed f_w2 = Mulfx(f_1-f_f, f_g);
					fixed f_w3 = Mulfx(f_f, f_1-f_g);
					fixed f_w4 = Mulfx(f_f, f_g);
					_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
					_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
					_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
					_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
					fixed f_r1 = itofx(_GetRValue(pixel1));
					fixed f_r2 = itofx(_GetRValue(pixel2));
					fixed f_r3 = itofx(_GetRValue(pixel3));
					fixed f_r4 = itofx(_GetRValue(pixel4));
					fixed f_g1 = itofx(_GetGValue(pixel1));
					fixed f_g2 = itofx(_GetGValue(pixel2));
					fixed f_g3 = itofx(_GetGValue(pixel3));
					fixed f_g4 = itofx(_GetGValue(pixel4));
					fixed f_b1 = itofx(_GetBValue(pixel1));
					fixed f_b2 = itofx(_GetBValue(pixel2));
					fixed f_b3 = itofx(_GetBValue(pixel3));
					fixed f_b4 = itofx(_GetBValue(pixel4));
					fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
					fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
					fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
					fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}*/

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset = dwDstHorizontalStartOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset = 0;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		DWORD dwDstPitch = m_iPitch;
		DWORD m, n, dwSrcVerticalOffset, dwSrcHorizontalOffset;
		DWORD dwSrcTopLeft, dwSrcTopRight, dwSrcBottomLeft, dwSrcBottomRight;
		fixed f_f, f_g, f_w1, f_w2, f_w3, f_w4, f_srcPixel;
		DWORD dwTransparentColor = transparentColor;
		__asm {
			mov edi, lpDstData
			add edi, dwDstVerticalOffset
			add edi, dwDstHorizontalStartOffset
			xor edx, edx
outer_loop:	
			xor ecx, ecx
			xor ebx, ebx
inner_loop:	
			push ebx
			push edx
			push edx
			mov eax, edx
			shl eax, 8
			mul f_dy
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov m, ebx
			mov eax, ecx
			shl eax, 8
			mul f_dx
			shr eax, 8
			push eax
			mov ebx, eax
			shr ebx, 8
			mov n, ebx
			pop eax
			and eax, 0x000000FF
			mov f_g, eax
			pop eax
			and eax, 0x000000FF
			mov f_f, eax
			mov eax, _height
			sub eax, _srcStartY
			sub eax, m
			dec eax
			mul _pitch
			mov dwSrcVerticalOffset, eax
			mov eax, _srcStartX
			add eax, n
			mul _bpp
			mov dwSrcHorizontalOffset, eax
			mov eax, dwSrcVerticalOffset
			add eax, dwSrcHorizontalOffset
			mov dwSrcTotalOffset, eax
			pop edx
			mov esi, lpSrcData
			add esi, eax
			mov eax, [esi]
			cmp eax, dwTransparentColor
			je next4
			mov eax, dwSrcTotalOffset
			mov dwSrcTopLeft, eax
			mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next1
			add eax, _bpp
			mov dwSrcTopRight, eax
			sub eax, _bpp
			jmp next11
next1:		mov dwSrcTopRight, eax
next11:		mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next2
			sub eax, _pitch
			mov dwSrcBottomLeft, eax
			add eax, _pitch
			jmp next22
next2:		mov dwSrcBottomLeft, eax
next22:		mov ebx, _width
			dec ebx
			cmp ecx, ebx
			jge next3
			mov ebx, _height
			dec ebx
			cmp edx, ebx
			jge next3
			sub eax, _pitch
			add eax, _bpp
next3:		mov dwSrcBottomRight, eax
			mov eax, f_1
			sub eax, f_f
			mov ebx, f_1
			sub ebx, f_g
			mul ebx
			shr eax, 8
			mov f_w1, eax
			mov eax, f_1
			sub eax, f_f
			mul f_g
			shr eax, 8
			mov f_w2, eax
			mov eax, f_1
			sub eax, f_g
			mul f_f
			shr eax, 8
			mov f_w3, eax
			mov eax, f_f
			mul f_g
			shr eax, 8
			mov f_w4, eax
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x0000FF00
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			mov esi, lpSrcData
			add esi, dwSrcTopLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w1
			shr eax, 8
			mov ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcTopRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w2
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomLeft
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w3
			shr eax, 8
			add ebx, eax
			mov esi, lpSrcData
			add esi, dwSrcBottomRight
			mov eax, [esi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_w4
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			push ebx
			pop ebx
			mov eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			pop ebx
			shl eax, 8
			or eax, ebx
			mov f_srcPixel, eax
			mov eax, f_srcPixel
			and eax, 0x000000FF
			shl eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x000000FF
			shl eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x0000FF00
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x0000FF00
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			mov eax, f_srcPixel
			and eax, 0x00FF0000
			shr eax, 8
			mul f_alpha
			shr eax, 8
			mov ebx, eax
			mov eax, [edi]
			and eax, 0x00FF0000
			shr eax, 8
			mul f_1malpha
			shr eax, 8
			add ebx, eax
			shr ebx, 8
			and ebx, 0x000000FF
			push ebx
			pop ebx
			mov eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			shl eax, 8
			pop ebx
			or eax, ebx
			mov [edi], eax
next4:		pop edx
			pop ebx
			add edi, 4
			add ebx, 4
			inc ecx
			cmp ecx, _dstWidth
			jl inner_loop
			sub edi, ebx
			sub edi, dwDstPitch
			inc edx
			cmp edx, _dstHeight
			jl outer_loop
		}
	}
}

void CBitmapEx::_DrawTransparentBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		fixed f_alpha = ftofx(_alphaPercent);

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel = lpSrcData[dwSrcTotalOffset>>2];
				if (pixel != transparentColor)
				{
					DWORD dwSrcOffsets[16];
					dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
					if ((m < 1) || (n < 1))
						dwSrcOffsets[0] = dwSrcTotalOffset;
					dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
					if (m < 1)
						dwSrcOffsets[1] = dwSrcTotalOffset;
					dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
					if ((m < 1) || (n >= _width-1))
						dwSrcOffsets[2] = dwSrcTotalOffset;
					dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
					if ((m < 1) || (n >= _width-2))
						dwSrcOffsets[3] = dwSrcTotalOffset;
					dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
					if (n < 1)
						dwSrcOffsets[4] = dwSrcTotalOffset;
					dwSrcOffsets[5] = dwSrcTotalOffset;
					dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
					if (n >= _width-1)
						dwSrcOffsets[6] = dwSrcTotalOffset;
					dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
					if (n >= _width-2)
						dwSrcOffsets[7] = dwSrcTotalOffset;
					dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
					if ((m >= _height-1) || (n < 1))
						dwSrcOffsets[8] = dwSrcTotalOffset;
					dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
					if (m >= _height-1)
						dwSrcOffsets[9] = dwSrcTotalOffset;
					dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
					if ((m >= _height-1) || (n >= _width-1))
						dwSrcOffsets[10] = dwSrcTotalOffset;
					dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
					if ((m >= _height-1) || (n >= _width-2))
						dwSrcOffsets[11] = dwSrcTotalOffset;
					dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
					if ((m >= _height-2) || (n < 1))
						dwSrcOffsets[12] = dwSrcTotalOffset;
					dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
					if (m >= _height-2)
						dwSrcOffsets[13] = dwSrcTotalOffset;
					dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
					if ((m >= _height-2) || (n >= _width-1))
						dwSrcOffsets[14] = dwSrcTotalOffset;
					dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
					if ((m >= _height-2) || (n >= _width-2))
						dwSrcOffsets[15] = dwSrcTotalOffset;
					fixed f_red=0, f_green=0, f_blue=0;
					for (long k=-1; k<3; k++)
					{
						fixed f = itofx(k)-f_f;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						for (long l=-1; l<3; l++)
						{
							fixed f = itofx(l)-f_g;
							fixed f_fm1 = f - f_1;
							fixed f_fp1 = f + f_1;
							fixed f_fp2 = f + f_2;
							fixed f_a = 0;
							if (f_fp2 > 0)
								f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
							fixed f_b = 0;
							if (f_fp1 > 0)
								f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
							fixed f_c = 0;
							if (f > 0)
								f_c = Mulfx(f,Mulfx(f,f));
							fixed f_d = 0;
							if (f_fm1 > 0)
								f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
							fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
							fixed f_R = Mulfx(f_RY,f_RX);
							long _k = ((k+1)*4) + (l+1);
							fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
							f_red += Mulfx(f_rs,f_R);
							f_green += Mulfx(f_gs,f_R);
							f_blue += Mulfx(f_bs,f_R);
						}
					}
					fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
					fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
					fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
					fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
					fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
					BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
					BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
					BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
					lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);
				}

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawTransparent(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawTransparentNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawTransparentBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawTransparentBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, alpha, transparentColor);
			}
			break;
	}
}

void CBitmapEx::DrawTransparent(_QUAD dstQuad, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long alpha, _PIXEL transparentColor)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		_QUAD _dstQuad = dstQuad;
		_dstQuad.p1.x = max(0, min(m_bih.biWidth-1, _dstQuad.p1.x));
		_dstQuad.p1.y = max(0, min(m_bih.biHeight-1, _dstQuad.p1.y));
		_dstQuad.p2.x = max(0, min(m_bih.biWidth-1, _dstQuad.p2.x));
		_dstQuad.p2.y = max(0, min(m_bih.biHeight-1, _dstQuad.p2.y));
		_dstQuad.p3.x = max(0, min(m_bih.biWidth-1, _dstQuad.p3.x));
		_dstQuad.p3.y = max(0, min(m_bih.biHeight-1, _dstQuad.p3.y));
		_dstQuad.p4.x = max(0, min(m_bih.biWidth-1, _dstQuad.p4.x));
		_dstQuad.p4.y = max(0, min(m_bih.biHeight-1, _dstQuad.p4.y));

		// Calculate destination params
		long _dstStartX = max(0, min(min(_dstQuad.p1.x, _dstQuad.p2.x), min(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstStartY = max(0, min(min(_dstQuad.p1.y, _dstQuad.p2.y), min(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstEndX = min(m_bih.biWidth-1, max(max(_dstQuad.p1.x, _dstQuad.p2.x), max(_dstQuad.p3.x, _dstQuad.p4.x)));
		long _dstEndY = min(m_bih.biHeight-1, max(max(_dstQuad.p1.y, _dstQuad.p2.y), max(_dstQuad.p3.y, _dstQuad.p4.y)));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;
		float _dstInvHeight = 1.0f / (float)_dstHeight;
		float _dstInvWidth = 1.0f / (float)_dstWidth;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		long _srcEndX = max(0, min(bitmapEx.GetWidth()-1, _srcStartX+srcWidth));
		long _srcEndY = max(0, min(bitmapEx.GetHeight()-1, _srcStartY+srcHeight));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Check drawing region
		long _height = bitmapEx.GetHeight();
		long _width = bitmapEx.GetWidth();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		float f_x0 = (float)_dstQuad.p1.x / (float)_dstWidth;
		float f_y0 = (float)_dstQuad.p1.y / (float)_dstHeight;
		float f_x1 = (float)_dstQuad.p2.x / (float)_dstWidth;
		float f_y1 = (float)_dstQuad.p2.y / (float)_dstHeight;
		float f_x2 = (float)_dstQuad.p3.x / (float)_dstWidth;
		float f_y2 = (float)_dstQuad.p3.y / (float)_dstHeight;
		float f_x3 = (float)_dstQuad.p4.x / (float)_dstWidth;
		float f_y3 = (float)_dstQuad.p4.y / (float)_dstHeight;
		float a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		if ((long)(a*1000000) == 0)
		{
			f_y2 += _dstInvHeight;
			a = (f_y1-f_y0)*(f_x2-f_x3-f_x1+f_x0) - (f_x1-f_x0)*(f_y2-f_y3-f_y1+f_y0);
		}
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float f_inv2a = 1.0f / (2*a);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);
		long _alpha = max(0, min(alpha, 100));
		float _alphaPercent = (float)_alpha / 100.0f;
		fixed f_alpha = ftofx(_alphaPercent);
		fixed f_1 = itofx(1);

		// Draw bitmap
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		switch (m_ResampleMode)
		{
			case RM_NEARESTNEIGHBOUR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+0.5f) + _srcStartY;
									long n = (long)(mu*_srcWidth+0.5f) + _srcStartX;
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BILINEAR:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcTopLeft = dwSrcTotalOffset;
										DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcTopRight = dwSrcTotalOffset;
										DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcBottomLeft = dwSrcTotalOffset;
										DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
										if ((n >= _width-1) || (m >= _height-1))
											dwSrcBottomRight = dwSrcTotalOffset;
										fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
										fixed f_w2 = Mulfx(f_1-f_f, f_g);
										fixed f_w3 = Mulfx(f_f, f_1-f_g);
										fixed f_w4 = Mulfx(f_f, f_g);
										_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
										_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
										_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
										_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
										fixed f_r1 = itofx(_GetRValue(pixel1));
										fixed f_r2 = itofx(_GetRValue(pixel2));
										fixed f_r3 = itofx(_GetRValue(pixel3));
										fixed f_r4 = itofx(_GetRValue(pixel4));
										fixed f_g1 = itofx(_GetGValue(pixel1));
										fixed f_g2 = itofx(_GetGValue(pixel2));
										fixed f_g3 = itofx(_GetGValue(pixel3));
										fixed f_g4 = itofx(_GetGValue(pixel4));
										fixed f_b1 = itofx(_GetBValue(pixel1));
										fixed f_b2 = itofx(_GetBValue(pixel2));
										fixed f_b3 = itofx(_GetBValue(pixel3));
										fixed f_b4 = itofx(_GetBValue(pixel4));
										BYTE red = (BYTE)fxtoi(Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
										BYTE green = (BYTE)fxtoi(Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
										BYTE blue = (BYTE)fxtoi(Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
										pixel1 =  _RGB(red, green, blue);
										pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;

			case RM_BICUBIC:
				{
					for (long i=_dstStartY; i<=_dstEndY; i++)
					{
						for (long j=_dstStartX; j<=_dstEndX; j++)
						{
							// Update bitmap
							float f_y = (float)i * _dstInvHeight;
							float f_x = (float)j * _dstInvWidth;
							float b = (f_x-f_x0)*(f_y2-f_y3-f_y1+f_y0) - (f_x1-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x2-f_x3-f_x1+f_x0) + (f_y1-f_y0)*(f_x3-f_x0);
							float c = (f_x-f_x0)*(f_y3-f_y0) - (f_y-f_y0)*(f_x3-f_x0);
							float det = b*b - 4*a*c;
							if (det >= 0.0f)
							{
								float mu = (-b - sqrt(det)) * f_inv2a;
								float delta = (f_y-f_y0-mu*(f_y1-f_y0)) / (f_y3 + mu*(f_y2-f_y3)-f_y0-mu*(f_y1-f_y0));
								if ((mu >= 0.0f) && (mu < 1.0f) && (delta >= 0.0f) && (delta < 1.0f))
								{
									long m = (long)(delta*_srcHeight+_srcStartY);
									long n = (long)(mu*_srcWidth+_srcStartX);
									_PIXEL pixel1 =  bitmapEx.GetPixel(n, m);
									if (pixel1 != transparentColor)
									{
										fixed f_f = ftofx(delta*_srcHeight+_srcStartY) - itofx(m);
										fixed f_g = ftofx(mu*_srcWidth+_srcStartX) - itofx(n);
										fixed f_1 = itofx(1);
										DWORD dwSrcTotalOffset = (_height-m-1)*_pitch + n*_bpp;
										DWORD dwSrcOffsets[16];
										dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
										if ((m < 1) || (n < 1))
											dwSrcOffsets[0] = dwSrcTotalOffset;
										dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
										if (m < 1)
											dwSrcOffsets[1] = dwSrcTotalOffset;
										dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
										if ((m < 1) || (n >= _width-1))
											dwSrcOffsets[2] = dwSrcTotalOffset;
										dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
										if ((m < 1) || (n >= _width-2))
											dwSrcOffsets[3] = dwSrcTotalOffset;
										dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
										if (n < 1)
											dwSrcOffsets[4] = dwSrcTotalOffset;
										dwSrcOffsets[5] = dwSrcTotalOffset;
										dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
										if (n >= _width-1)
											dwSrcOffsets[6] = dwSrcTotalOffset;
										dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
										if (n >= _width-2)
											dwSrcOffsets[7] = dwSrcTotalOffset;
										dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
										if ((m >= _height-1) || (n < 1))
											dwSrcOffsets[8] = dwSrcTotalOffset;
										dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
										if (m >= _height-1)
											dwSrcOffsets[9] = dwSrcTotalOffset;
										dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
										if ((m >= _height-1) || (n >= _width-1))
											dwSrcOffsets[10] = dwSrcTotalOffset;
										dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
										if ((m >= _height-1) || (n >= _width-2))
											dwSrcOffsets[11] = dwSrcTotalOffset;
										dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
										if ((m >= _height-2) || (n < 1))
											dwSrcOffsets[12] = dwSrcTotalOffset;
										dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
										if (m >= _height-2)
											dwSrcOffsets[13] = dwSrcTotalOffset;
										dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
										if ((m >= _height-2) || (n >= _width-1))
											dwSrcOffsets[14] = dwSrcTotalOffset;
										dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
										if ((m >= _height-2) || (n >= _width-2))
											dwSrcOffsets[15] = dwSrcTotalOffset;
										fixed f_red=0, f_green=0, f_blue=0;
										for (long k=-1; k<3; k++)
										{
											fixed f = itofx(k)-f_f;
											fixed f_fm1 = f - f_1;
											fixed f_fp1 = f + f_1;
											fixed f_fp2 = f + f_2;
											fixed f_a = 0;
											if (f_fp2 > 0)
												f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
											fixed f_b = 0;
											if (f_fp1 > 0)
												f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
											fixed f_c = 0;
											if (f > 0)
												f_c = Mulfx(f,Mulfx(f,f));
											fixed f_d = 0;
											if (f_fm1 > 0)
												f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
											fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
											for (long l=-1; l<3; l++)
											{
												fixed f = itofx(l)-f_g;
												fixed f_fm1 = f - f_1;
												fixed f_fp1 = f + f_1;
												fixed f_fp2 = f + f_2;
												fixed f_a = 0;
												if (f_fp2 > 0)
													f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
												fixed f_b = 0;
												if (f_fp1 > 0)
													f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
												fixed f_c = 0;
												if (f > 0)
													f_c = Mulfx(f,Mulfx(f,f));
												fixed f_d = 0;
												if (f_fm1 > 0)
													f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
												fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
												fixed f_R = Mulfx(f_RY,f_RX);
												long _k = ((k+1)*4) + (l+1);
												fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
												f_red += Mulfx(f_rs,f_R);
												f_green += Mulfx(f_gs,f_R);
												f_blue += Mulfx(f_bs,f_R);
											}
										}
										BYTE red = (BYTE)max(0, min(255, fxtoi(Mulfx(f_red,f_gama))));
										BYTE green = (BYTE)max(0, min(255, fxtoi(Mulfx(f_green,f_gama))));
										BYTE blue = (BYTE)max(0, min(255, fxtoi(Mulfx(f_blue,f_gama))));
										_PIXEL pixel1 =  _RGB(red, green, blue);
										_PIXEL pixel2 =  GetPixel(j, i);
										fixed f_sred = itofx(_GetRValue(pixel1));
										fixed f_sgreen = itofx(_GetGValue(pixel1));
										fixed f_sblue = itofx(_GetBValue(pixel1));
										fixed f_dred = itofx(_GetRValue(pixel2));
										fixed f_dgreen = itofx(_GetGValue(pixel2));
										fixed f_dblue = itofx(_GetBValue(pixel2));
										red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
										green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
										blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
										SetPixel(j, i, _RGB(red, green, blue));
									}
								}
							}
						}
					}
				}
				break;
		}
	}
}

void CBitmapEx::DrawBlended(long dstX, long dstY, long width, long height, CBitmapEx& bitmapEx, long srcX, long srcY, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+width, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+height, m_bih.biHeight-1));

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+width, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+height, bitmapEx.GetHeight()-1));

		// Check drawing region
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		fixed f_1 = itofx(1);
		long _width = min(_dstEndX-_dstStartX, _srcEndX-_srcStartX);
		long _height = min(_dstEndY-_dstStartY, _srcEndY-_srcStartY);
		if ((_width == 0) || (_height == 0))
			return;
		float _heightHalf = (float)_height / 2;
		float _widthHalf = (float)_width / 2;
		float _heightSquare = (float)(_height*_height);
		float _widthSquare = (float)(_width*_width);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_width;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_height;
		float _gamma = 2.2f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcHorizontalStartOffset = _srcStartX * _bpp;
		DWORD dwSrcHorizontalOffset;
		DWORD dwSrcVerticalOffset = (bitmapEx.GetHeight()-_srcStartY-1) * _pitch;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD) bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<=_height; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			dwSrcHorizontalOffset = dwSrcHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<=_width; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update source total offset
				dwSrcTotalOffset = dwSrcVerticalOffset + dwSrcHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if (mode == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, min(_alphaV*_gamma, _alphaH*_gamma));
				else if (mode == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if (mode == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if (mode == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(1.0f, 1.0f-_alpha));
					else
						_alpha = min(1.0f, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;

				// Update source horizontal offset
				dwSrcHorizontalOffset += _bpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;

			// Update source vertical offset
			dwSrcVerticalOffset -= _pitch;
		}
	}
}

void CBitmapEx::_DrawBlendedNearestNeighbour(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if (mode == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, min(_alphaV*_gamma, _alphaH*_gamma));
				else if (mode == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if (mode == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if (mode == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(1.0f, 1.0f-_alpha));
					else
						_alpha = min(1.0f, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				_PIXEL pixel1 = lpSrcData[dwSrcTotalOffset>>2];
				_PIXEL pixel2 = lpDstData[dwDstTotalOffset>>2];
				fixed f_sred = itofx(_GetRValue(pixel1));
				fixed f_sgreen = itofx(_GetGValue(pixel1));
				fixed f_sblue = itofx(_GetBValue(pixel1));
				fixed f_dred = itofx(_GetRValue(pixel2));
				fixed f_dgreen = itofx(_GetGValue(pixel2));
				fixed f_dblue = itofx(_GetBValue(pixel2));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawBlendedBilinear(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if (mode == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, min(_alphaV*_gamma, _alphaH*_gamma));
				else if (mode == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if (mode == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if (mode == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(1.0f, 1.0f-_alpha));
					else
						_alpha = min(1.0f, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcTopLeft = dwSrcTotalOffset;
				DWORD dwSrcTopRight = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcTopRight = dwSrcTotalOffset;
				DWORD dwSrcBottomLeft = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcBottomLeft = dwSrcTotalOffset;
				DWORD dwSrcBottomRight = dwSrcTotalOffset - _pitch + _bpp;
				if ((n >= _width-1) || (m >= _height-1))
					dwSrcBottomRight = dwSrcTotalOffset;
				fixed f_w1 = Mulfx(f_1-f_f, f_1-f_g);
				fixed f_w2 = Mulfx(f_1-f_f, f_g);
				fixed f_w3 = Mulfx(f_f, f_1-f_g);
				fixed f_w4 = Mulfx(f_f, f_g);
				_PIXEL pixel1 = lpSrcData[dwSrcTopLeft>>2];
				_PIXEL pixel2 = lpSrcData[dwSrcTopRight>>2];
				_PIXEL pixel3 = lpSrcData[dwSrcBottomLeft>>2];
				_PIXEL pixel4 = lpSrcData[dwSrcBottomRight>>2];
				fixed f_r1 = itofx(_GetRValue(pixel1));
				fixed f_r2 = itofx(_GetRValue(pixel2));
				fixed f_r3 = itofx(_GetRValue(pixel3));
				fixed f_r4 = itofx(_GetRValue(pixel4));
				fixed f_g1 = itofx(_GetGValue(pixel1));
				fixed f_g2 = itofx(_GetGValue(pixel2));
				fixed f_g3 = itofx(_GetGValue(pixel3));
				fixed f_g4 = itofx(_GetGValue(pixel4));
				fixed f_b1 = itofx(_GetBValue(pixel1));
				fixed f_b2 = itofx(_GetBValue(pixel2));
				fixed f_b3 = itofx(_GetBValue(pixel3));
				fixed f_b4 = itofx(_GetBValue(pixel4));
				fixed f_sred = (Mulfx(f_w1, f_r1) + Mulfx(f_w2, f_r2) + Mulfx(f_w3, f_r3) + Mulfx(f_w4, f_r4));
				fixed f_sgreen = (Mulfx(f_w1, f_g1) + Mulfx(f_w2, f_g2) + Mulfx(f_w3, f_g3) + Mulfx(f_w4, f_g4));
				fixed f_sblue = (Mulfx(f_w1, f_b1) + Mulfx(f_w2, f_b2) + Mulfx(f_w3, f_b3) + Mulfx(f_w4, f_b4));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::_DrawBlendedBicubic(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for valid bitmap
	if (IsValid() && bitmapEx.IsValid())
	{
		// Calculate destination params
		long _dstStartX = max(0, min(m_bih.biWidth-1, dstX));
		if (srcX < 0)
			_dstStartX = max(0, min(m_bih.biWidth-1, dstX-srcX));
		long _dstStartY = max(0, min(m_bih.biHeight-1, dstY));
		if (srcY < 0)
			_dstStartY = max(0, min(m_bih.biHeight-1, dstY-srcY));
		long _dstEndX = max(0, min(dstX+dstWidth, m_bih.biWidth-1));
		long _dstEndY = max(0, min(dstY+dstHeight, m_bih.biHeight-1));
		long _dstWidth = _dstEndX - _dstStartX + 1;
		long _dstHeight = _dstEndY - _dstStartY + 1;

		// Calculate source params
		long _srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX));
		if (dstX < 0)
			_srcStartX = max(0, min(bitmapEx.GetWidth()-1, srcX-dstX));
		long _srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY));
		if (dstY < 0)
			_srcStartY = max(0, min(bitmapEx.GetHeight()-1, srcY-dstY));
		long _srcEndX = max(0, min(srcX+srcWidth, bitmapEx.GetWidth()-1));
		long _srcEndY = max(0, min(srcY+srcHeight, bitmapEx.GetHeight()-1));
		long _srcWidth = _srcEndX - _srcStartX + 1;
		long _srcHeight = _srcEndY - _srcStartY + 1;

		// Calculate scaling params
		long _width = bitmapEx.GetWidth();
		long _height = bitmapEx.GetHeight();
		long _pitch = bitmapEx.GetPitch();
		long _bpp = bitmapEx.GetBpp();
		long _horizontalPercent = (long)(((float)_dstWidth / (float)_srcWidth) * 100.0f);
		long _verticalPercent = (long)(((float)_dstHeight / (float)_srcHeight) * 100.0f);
		float dx = (float)_srcWidth / (float)_dstWidth;
		float dy = (float)_srcHeight / (float)_dstHeight;
		fixed f_dx = ftofx(dx);
		fixed f_dy = ftofx(dy);
		fixed f_1 = itofx(1);
		fixed f_2 = itofx(2);
		fixed f_4 = itofx(4);
		fixed f_6 = itofx(6);
		fixed f_gama = ftofx(1.04f);

		// Check drawing region
		long _startAlpha = max(0, min(startAlpha, 100));
		float _startAlphaPercent = (float)_startAlpha / 100.0f;
		long _endAlpha = max(0, min(endAlpha, 100));
		float _endAlphaPercent = (float)_endAlpha / 100.0f;
		if ((_dstWidth == 0) || (_dstHeight == 0) || (_srcWidth == 0) || (_srcHeight == 0))
			return;
		float _heightHalf = (float)_dstHeight / 2;
		float _widthHalf = (float)_dstWidth / 2;
		float _heightSquare = (float)(_dstHeight*_dstHeight);
		float _widthSquare = (float)(_dstWidth*_dstWidth);
		float _alphaStepH = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstWidth;
		float _alphaStepV = (float)(_endAlphaPercent-_startAlphaPercent) / (float)_dstHeight;
		float _gamma = 2.2f;

		// Draw bitmap
		DWORD dwDstHorizontalStartOffset = _dstStartX * m_iBpp;
		DWORD dwDstHorizontalOffset;
		DWORD dwDstVerticalOffset = (m_bih.biHeight-_dstStartY-1) * m_iPitch;
		DWORD dwDstTotalOffset;
		LPDWORD lpDstData = (LPDWORD)m_lpData;
		DWORD dwSrcTotalOffset;
		LPDWORD lpSrcData = (LPDWORD)bitmapEx.GetData();
		float _alphaV = _startAlphaPercent;
		for (long i=0; i<_dstHeight; i++)
		{
			dwDstHorizontalOffset = dwDstHorizontalStartOffset;
			float _alphaH = _startAlphaPercent;
			for (long j=0; j<_dstWidth; j++)
			{
				// Update destination total offset
				dwDstTotalOffset = dwDstVerticalOffset + dwDstHorizontalOffset;

				// Update alpha
				float _alpha = 0.0f;
				if (mode == (GM_HORIZONTAL|GM_VERTICAL))
					_alpha = min(1.0f, min(_alphaV*_gamma, _alphaH*_gamma));
				else if (mode == GM_HORIZONTAL)
					_alpha = min(1.0f, _alphaH*_gamma);
				else if (mode == GM_VERTICAL)
					_alpha = min(1.0f, _alphaV*_gamma);
				else if (mode == GM_RADIAL)
				{
					float _ySquare = (float)((i - _heightHalf)*(i - _heightHalf));
					float _xSquare = (float)((j - _widthHalf)*(j - _widthHalf));
					_alpha = sqrt((_ySquare/_heightSquare) + (_xSquare/_widthSquare)) * _gamma;
					if (_startAlphaPercent <= _endAlphaPercent)
						_alpha = max(0.0f, min(1.0f, 1.0f-_alpha));
					else
						_alpha = min(1.0f, _alpha);
				}
				fixed f_alpha = ftofx(_alpha);

				// Update bitmap
				fixed f_i = itofx(i);
				fixed f_j = itofx(j);
				fixed f_a = Mulfx(f_i, f_dy);
				fixed f_b = Mulfx(f_j, f_dx);
				long m = fxtoi(f_a);
				long n = fxtoi(f_b);
				fixed f_f = f_a - itofx(m);
				fixed f_g = f_b - itofx(n);
				dwSrcTotalOffset = (_height-_srcStartY-m-1)*_pitch + (n+_srcStartX)*_bpp;
				DWORD dwSrcOffsets[16];
				dwSrcOffsets[0] = dwSrcTotalOffset + _pitch - _bpp;
				if ((m < 1) || (n < 1))
					dwSrcOffsets[0] = dwSrcTotalOffset;
				dwSrcOffsets[1] = dwSrcTotalOffset + _pitch;
				if (m < 1)
					dwSrcOffsets[1] = dwSrcTotalOffset;
				dwSrcOffsets[2] = dwSrcTotalOffset + _pitch + _bpp;
				if ((m < 1) || (n >= _width-1))
					dwSrcOffsets[2] = dwSrcTotalOffset;
				dwSrcOffsets[3] = dwSrcTotalOffset + _pitch + _bpp + _bpp;
				if ((m < 1) || (n >= _width-2))
					dwSrcOffsets[3] = dwSrcTotalOffset;
				dwSrcOffsets[4] = dwSrcTotalOffset - _bpp;
				if (n < 1)
					dwSrcOffsets[4] = dwSrcTotalOffset;
				dwSrcOffsets[5] = dwSrcTotalOffset;
				dwSrcOffsets[6] = dwSrcTotalOffset + _bpp;
				if (n >= _width-1)
					dwSrcOffsets[6] = dwSrcTotalOffset;
				dwSrcOffsets[7] = dwSrcTotalOffset + _bpp + _bpp;
				if (n >= _width-2)
					dwSrcOffsets[7] = dwSrcTotalOffset;
				dwSrcOffsets[8] = dwSrcTotalOffset - _pitch - _bpp;
				if ((m >= _height-1) || (n < 1))
					dwSrcOffsets[8] = dwSrcTotalOffset;
				dwSrcOffsets[9] = dwSrcTotalOffset - _pitch;
				if (m >= _height-1)
					dwSrcOffsets[9] = dwSrcTotalOffset;
				dwSrcOffsets[10] = dwSrcTotalOffset - _pitch + _bpp;
				if ((m >= _height-1) || (n >= _width-1))
					dwSrcOffsets[10] = dwSrcTotalOffset;
				dwSrcOffsets[11] = dwSrcTotalOffset - _pitch + _bpp + _bpp;
				if ((m >= _height-1) || (n >= _width-2))
					dwSrcOffsets[11] = dwSrcTotalOffset;
				dwSrcOffsets[12] = dwSrcTotalOffset - _pitch - _pitch - _bpp;
				if ((m >= _height-2) || (n < 1))
					dwSrcOffsets[12] = dwSrcTotalOffset;
				dwSrcOffsets[13] = dwSrcTotalOffset - _pitch - _pitch;
				if (m >= _height-2)
					dwSrcOffsets[13] = dwSrcTotalOffset;
				dwSrcOffsets[14] = dwSrcTotalOffset - _pitch - _pitch + _bpp;
				if ((m >= _height-2) || (n >= _width-1))
					dwSrcOffsets[14] = dwSrcTotalOffset;
				dwSrcOffsets[15] = dwSrcTotalOffset - _pitch - _pitch + _bpp + _bpp;
				if ((m >= _height-2) || (n >= _width-2))
					dwSrcOffsets[15] = dwSrcTotalOffset;
				fixed f_red=0, f_green=0, f_blue=0;
				for (long k=-1; k<3; k++)
				{
					fixed f = itofx(k)-f_f;
					fixed f_fm1 = f - f_1;
					fixed f_fp1 = f + f_1;
					fixed f_fp2 = f + f_2;
					fixed f_a = 0;
					if (f_fp2 > 0)
						f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
					fixed f_b = 0;
					if (f_fp1 > 0)
						f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
					fixed f_c = 0;
					if (f > 0)
						f_c = Mulfx(f,Mulfx(f,f));
					fixed f_d = 0;
					if (f_fm1 > 0)
						f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
					fixed f_RY = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
					for (long l=-1; l<3; l++)
					{
						fixed f = itofx(l)-f_g;
						fixed f_fm1 = f - f_1;
						fixed f_fp1 = f + f_1;
						fixed f_fp2 = f + f_2;
						fixed f_a = 0;
						if (f_fp2 > 0)
							f_a = Mulfx(f_fp2,Mulfx(f_fp2,f_fp2));
						fixed f_b = 0;
						if (f_fp1 > 0)
							f_b = Mulfx(f_fp1,Mulfx(f_fp1,f_fp1));
						fixed f_c = 0;
						if (f > 0)
							f_c = Mulfx(f,Mulfx(f,f));
						fixed f_d = 0;
						if (f_fm1 > 0)
							f_d = Mulfx(f_fm1,Mulfx(f_fm1,f_fm1));
						fixed f_RX = Divfx((f_a-Mulfx(f_4,f_b)+Mulfx(f_6,f_c)-Mulfx(f_4,f_d)),f_6);
						fixed f_R = Mulfx(f_RY,f_RX);
						long _k = ((k+1)*4) + (l+1);
						fixed f_rs = itofx(_GetRValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_gs = itofx(_GetGValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						fixed f_bs = itofx(_GetBValue(lpSrcData[dwSrcOffsets[_k]>>2]));
						f_red += Mulfx(f_rs,f_R);
						f_green += Mulfx(f_gs,f_R);
						f_blue += Mulfx(f_bs,f_R);
					}
				}
				fixed f_sred = itofx(max(0, min(255, fxtoi(Mulfx(f_red,f_gama)))));
				fixed f_sgreen = itofx(max(0, min(255, fxtoi(Mulfx(f_green,f_gama)))));
				fixed f_sblue = itofx(max(0, min(255, fxtoi(Mulfx(f_blue,f_gama)))));
				fixed f_dred = itofx(_GetRValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dgreen = itofx(_GetGValue(lpDstData[dwDstTotalOffset>>2]));
				fixed f_dblue = itofx(_GetBValue(lpDstData[dwDstTotalOffset>>2]));
				BYTE red = (BYTE)fxtoi(Mulfx(f_sred, f_alpha) + Mulfx(f_dred, f_1-f_alpha));
				BYTE green = (BYTE)fxtoi(Mulfx(f_sgreen, f_alpha) + Mulfx(f_dgreen, f_1-f_alpha));
				BYTE blue = (BYTE)fxtoi(Mulfx(f_sblue, f_alpha) + Mulfx(f_dblue, f_1-f_alpha));
				lpDstData[dwDstTotalOffset>>2] = _RGB(red, green, blue);

				// Update horizontal alpha
				_alphaH += _alphaStepH;

				// Update destination horizontal offset
				dwDstHorizontalOffset += m_iBpp;
			}

			// Update vertical alpha
			_alphaV += _alphaStepV;

			// Update destination vertical offset
			dwDstVerticalOffset -= m_iPitch;
		}
	}
}

void CBitmapEx::DrawBlended(long dstX, long dstY, long dstWidth, long dstHeight, CBitmapEx& bitmapEx, long srcX, long srcY, long srcWidth, long srcHeight, long startAlpha, long endAlpha, DWORD mode)
{
	// Check for resampling mode
	switch (m_ResampleMode)
	{
		case RM_NEARESTNEIGHBOUR:
			{
				// Draw bitmap using nearest neighbour interpolation algorithm
				_DrawBlendedNearestNeighbour(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;

		case RM_BILINEAR:
			{
				// Draw bitmap using bilinear interpolation algorithm
				_DrawBlendedBilinear(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;

		case RM_BICUBIC:
			{
				// Draw bitmap using bicubic interpolation algorithm
				_DrawBlendedBicubic(dstX, dstY, dstWidth, dstHeight, bitmapEx, srcX, srcY, srcWidth, srcHeight, startAlpha, endAlpha, mode);
			}
			break;
	}
}

_PIXEL CBitmapEx::GetPixel(long x, long y)
{
	_PIXEL pixel = _RGB(0,0,0);

	// Check for valid bitmap
	if (IsValid())
	{
		// Get pixel color
		long _x = max(0, min(m_bih.biWidth-1, x));
		long _y = max(0, min(m_bih.biHeight-1, y));
		DWORD dwTotalOffset = (m_bih.biHeight-1-_y)*m_iPitch + _x*m_iBpp;
		LPDWORD lpData = (LPDWORD)m_lpData;
		pixel = lpData[dwTotalOffset>>2];
	}

	return pixel;
}

void CBitmapEx::SetPixel(long x, long y, _PIXEL pixel)
{
	// Check for valid bitmap
	if (IsValid())
	{
		// Set pixel color
		long _x = max(0, min(m_bih.biWidth-1, x));
		long _y = max(0, min(m_bih.biHeight-1, y));
		DWORD dwTotalOffset = (m_bih.biHeight-1-_y)*m_iPitch + _x*m_iBpp;
		LPDWORD lpData = (LPDWORD)m_lpData;
		lpData[dwTotalOffset>>2] = pixel;
	}
}

//wangjun

#define WIDTHBYTES(bits)        (((bits) + 31) / 32 * 4)
#define PALETTESIZE(lpbi)		(_DIBNumColors((LPBITMAPINFOHEADER) lpbi)* sizeof (RGBQUAD))
const int HIMETRIC_INCH	= 2540;

void CBitmapEx::LoadImageFile(LPTSTR lpszImageFile)
{
	IPicture* pPicture = NULL;

	HBITMAP hbm = NULL;
	HRESULT hr = OleLoadPicturePath(T2OLE(lpszImageFile), NULL, 0, -1, IID_IPicture, (LPVOID*)&pPicture);

	if (pPicture)
	{
		hbm = _extractBitmap(pPicture);
		pPicture->Release();
	}

	HANDLE hDIB = NULL;
	LPBITMAPINFOHEADER  lpbi;
	if (hbm)
	{
		hDIB = _dibFromBitmap(hbm);
		if (hDIB)
		{
			lpbi = (LPBITMAPINFOHEADER)GlobalLock (hDIB);
			Load((LPBYTE)lpbi);
			GlobalUnlock (hDIB);
			GlobalFree (hDIB);
		}
	}

	hDIB = NULL;
	hbm = NULL;
	pPicture = NULL;

}

// DDB->DIB
HANDLE CBitmapEx::_dibFromBitmap(HBITMAP hBitmap)
{
   HDC                  hDC;
   WORD			wBits;
   HPALETTE 		hPal=NULL;
   BITMAP               bm;
   BITMAPINFOHEADER     bi;
   BITMAPINFOHEADER     *lpbi;
   DWORD                dwLen;
   HANDLE               hDIB;

   if (!hBitmap)
      return NULL;

   ::GetObject (hBitmap, sizeof (bm), (LPSTR)&bm);

    wBits =  bm.bmBitsPixel;

   bi.biSize               = sizeof (BITMAPINFOHEADER);
   bi.biWidth              = bm.bmWidth;
   bi.biHeight             = bm.bmHeight;
   bi.biPlanes             = 1;
   bi.biBitCount           = wBits;
   bi.biCompression        = BI_RGB;
   bi.biSizeImage          = 0;
   bi.biXPelsPerMeter      = 0;
   bi.biYPelsPerMeter      = 0;
   bi.biClrUsed            = 0;
   bi.biClrImportant       = 0;

   dwLen  = bi.biSize + PALETTESIZE ((LPSTR) &bi);
   hDIB	  = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,dwLen);

   if (!hDIB)
      return NULL;

   lpbi   = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
   *lpbi  = bi;
   
   HWND hWnd= GetFocus();
   
   if(!hWnd)
   {//load default system palette
   	hPal   = (HPALETTE)GetStockObject (DEFAULT_PALETTE);
   	hDC    = GetDC (NULL);
   	hPal   = SelectPalette (hDC, hPal, FALSE);
   	RealizePalette(hDC);
	}
   else
   	{
	//or get palette from focused window
	HDC hDCw    = GetDC (hWnd);
	hPal   = SelectPalette (hDCw, (HPALETTE)GetStockObject (DEFAULT_PALETTE), FALSE);
   	hDC    = GetDC (NULL);
   	hPal   = SelectPalette (hDC, hPal, FALSE);
	RealizePalette(hDC);
	SelectPalette (hDCw, hPal, FALSE);
	ReleaseDC(hWnd,hDCw);
	}

   // calculate the biSizeImage
   GetDIBits (hDC,
              hBitmap,
              0,
              (WORD) bi.biHeight,
              NULL,
              (LPBITMAPINFO) lpbi,
              DIB_RGB_COLORS);

   bi = *lpbi;
   GlobalUnlock(hDIB);

   // If the driver did not fill in the biSizeImage field, 
   // make one up

   if (bi.biSizeImage == 0)
      {
      bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * wBits) * bm.bmHeight;

      if (bi.biCompression != BI_RGB)
         bi.biSizeImage = (bi.biSizeImage * 3) / 2;
      }


   DWORD sl =dwLen;
   HGLOBAL hPtr =hDIB;
   dwLen = bi.biSize + PALETTESIZE((LPSTR)&bi) + bi.biSizeImage;

	// if you have plans to use DDE or clipboard, you have
    // to allocate memory with GMEM_DDESHARE flag
   if(!(hDIB = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,dwLen))) 
   {
      SelectPalette(hDC,hPal,FALSE);
      ReleaseDC(NULL,hDC);
	  GlobalFree(hPtr);
	  return NULL;}

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
	LPBITMAPINFOHEADER lpS=(LPBITMAPINFOHEADER)GlobalLock(hPtr);
	
	CopyMemory(lpbi,lpS,sl);
	GlobalUnlock(hPtr);
	GlobalFree(hPtr);


    // actually fill lpBits
   if (GetDIBits( hDC,
         hBitmap,
         0,
         (WORD) bi.biHeight,
         (LPSTR) lpbi + (WORD) lpbi->biSize + PALETTESIZE((LPSTR) lpbi),
         (LPBITMAPINFO) lpbi, DIB_RGB_COLORS) == 0)
      {
      GlobalUnlock (hDIB);
      hDIB = NULL;
      SelectPalette (hDC, hPal, FALSE);
      ReleaseDC (NULL, hDC);
      return NULL;
      }

   bi = *lpbi;
   GlobalUnlock (hDIB);
   SelectPalette (hDC, hPal, FALSE);
   ReleaseDC (NULL, hDC);
   //OK
   return hDIB;
}

int CBitmapEx::_DIBNumColors (LPBITMAPINFOHEADER lpbi)
{
      if (lpbi->biClrUsed)
         return (int)lpbi->biClrUsed;
   switch (lpbi->biBitCount)
      {
      case 1:
         return 2;
      case 4:
         return 16;
      case 8:
         return 256;
      default:
         return 0;	//16,24,32 bits bitmap has no color table
      }
}

HBITMAP CBitmapEx::_extractBitmap(IPicture* pPicture)
{
	ASSERT(pPicture);

	if (!pPicture)
		return NULL;

	CBitmap bmMem;
	CDC dcMem;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();

	if (dcMem.CreateCompatibleDC(pDC))
	{
		long hmWidth;
		long hmHeight;

		pPicture->get_Width(&hmWidth);
		pPicture->get_Height(&hmHeight);
		
		int nWidth	= MulDiv(hmWidth, pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		int nHeight	= MulDiv(hmHeight, pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);

		if (bmMem.CreateCompatibleBitmap(pDC, nWidth, nHeight))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
			
			HRESULT hr = pPicture->Render(dcMem, 0, 0, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, NULL);
			dcMem.SelectObject(pOldBM);
		}
	}

	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	return (HBITMAP)bmMem.Detach();
}

void CBitmapEx::SaveJPGFile(LPTSTR lpszImageFile)
{
	if (!IsValid())
		return;

	if (lpszImageFile == NULL)
		return;

	ULONG_PTR m_gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap* myGdiBitmap;
	myGdiBitmap = Bitmap::FromBITMAPINFO((BITMAPINFO*)&m_bih,m_lpData);
	CLSID  Clsid;
	_GetCodecClsid(L"image/jpeg", &Clsid);
	myGdiBitmap->Save(lpszImageFile, &Clsid, NULL);

	myGdiBitmap = NULL;

	Gdiplus::GdiplusShutdown(m_gdiplusToken);	
}

int CBitmapEx::_GetCodecClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         return j;  // Success
      }    
   } // for

   return -1;  // Failure

} 