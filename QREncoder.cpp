#include "stdafx.h"
#include "QREncoder.h"


CQREncoder::CQREncoder()
	: m_pSymbleDC(NULL)
	, m_pSymbleBitmap(NULL)
	, m_pOldBitmap(NULL)
	, m_iModuleSize(0)
{

}


CQREncoder::~CQREncoder()
{
	if (m_pSymbleDC != NULL)
	{
		m_pSymbleDC->SelectObject(m_pOldBitmap);
		delete m_pSymbleDC;
	}
	if (m_pSymbleBitmap != NULL)
	{
		delete m_pSymbleBitmap;
	}
}

void CQREncoder::Generate(CString str, CString path, int nLevel, int nVersion, bool bAutoExtent, int nMask)
{
	CQR_Encode* pQR_Encode = new CQR_Encode;

	if (pQR_Encode->EncodeData(nLevel, nVersion, bAutoExtent, nMask, CT2A(str)))
	{
		if (m_pSymbleDC != NULL)
		{
			m_pSymbleDC->SelectObject(m_pOldBitmap);

			delete m_pSymbleBitmap;
			delete m_pSymbleDC;
		}

		m_nSymbleSize = pQR_Encode->m_nSymbleSize + (QR_MARGIN * 2);

		m_pSymbleBitmap = new CBitmap;
		m_pSymbleBitmap->CreateBitmap(m_nSymbleSize, m_nSymbleSize, 1, 1, NULL);

		m_pSymbleDC = new CDC;
		m_pSymbleDC->CreateCompatibleDC(NULL);

		m_pOldBitmap = m_pSymbleDC->SelectObject(m_pSymbleBitmap);

		m_pSymbleDC->PatBlt(0, 0, m_nSymbleSize, m_nSymbleSize, WHITENESS);

		for (int i = 0; i < pQR_Encode->m_nSymbleSize; ++i)
		{
			for (int j = 0; j < pQR_Encode->m_nSymbleSize; ++j)
			{
				if (pQR_Encode->m_byModuleData[i][j])
				{
					m_pSymbleDC->SetPixel(i + QR_MARGIN, j + QR_MARGIN, RGB(0, 0, 0));
				}
			}
		}

		SaveImage(path);
	}
	else
	{
		AfxMessageBox(_T("Error"),  MB_OK | MB_ICONINFORMATION);
	}

	delete pQR_Encode;

}

void CQREncoder::SaveImage(CString strSavePath)
{
	int nPixelSize = m_nSymbleSize * (m_iModuleSize + 1);
	if (nPixelSize < 200) { nPixelSize = 200; }

    int nBmpSize = ((nPixelSize + 31) / 32) * 32 / 8;
    nBmpSize *= nPixelSize;

    HANDLE hFile = CreateFile(strSavePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        strSavePath.Empty();
        AfxMessageBox(_T("Create File Failed"), MB_ICONSTOP);
            return;
    }

    HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE | SEC_COMMIT, 0, 
        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + nBmpSize, NULL);
    LPBYTE lpbyMapView = (LPBYTE)MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0, 0);

    ZeroMemory(lpbyMapView, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + nBmpSize);

    LPBITMAPFILEHEADER pbfh = (LPBITMAPFILEHEADER)lpbyMapView;
    LPBITMAPINFO       pbmi = (LPBITMAPINFO)(lpbyMapView + sizeof(BITMAPFILEHEADER));
    LPVOID             pbdt = lpbyMapView + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;

    // BITMAPFILEHEADER
    pbfh->bfType      = (WORD) (('M' << 8) | 'B'); // "BM"
    pbfh->bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + nBmpSize;
    pbfh->bfReserved1 = 0;
    pbfh->bfReserved2 = 0;
    pbfh->bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;

    // LPBITMAPINFO->BITMAPINFOHEADER
    pbmi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth			= nPixelSize;
    pbmi->bmiHeader.biHeight		= nPixelSize;
    pbmi->bmiHeader.biPlanes		= 1;
    pbmi->bmiHeader.biBitCount		= 1;
    pbmi->bmiHeader.biCompression	= BI_RGB;
    pbmi->bmiHeader.biSizeImage		= nBmpSize;
    pbmi->bmiHeader.biXPelsPerMeter = 3780;
    pbmi->bmiHeader.biYPelsPerMeter = 3780;
    pbmi->bmiHeader.biClrUsed		= 0;
    pbmi->bmiHeader.biClrImportant	= 0;

    CDC* pWorkDC = new CDC;
    pWorkDC->CreateCompatibleDC(NULL);

    CBitmap* pWorkBitmap = new CBitmap;
    pWorkBitmap->CreateBitmap(nPixelSize, nPixelSize, 1, 1, NULL);

    CBitmap* pOldBitmap = pWorkDC->SelectObject(pWorkBitmap);
    pWorkDC->StretchBlt(0, 0, nPixelSize, nPixelSize, m_pSymbleDC, 0, 0, m_nSymbleSize, m_nSymbleSize, SRCCOPY);

    GetDIBits(pWorkDC->m_hDC, (HBITMAP)*pWorkBitmap, 0, nPixelSize, pbdt, pbmi, DIB_RGB_COLORS);

    pWorkDC->SelectObject(pOldBitmap);
    delete pWorkBitmap;
    delete pWorkDC;

    UnmapViewOfFile(lpbyMapView);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);

}