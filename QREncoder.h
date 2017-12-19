#pragma once
#include "QR_Encode.h"

class CQREncoder
{
public:
	CQREncoder();
	~CQREncoder();


	void Generate(CString str, CString path, int nLevel = 0, int nVersion = 0, bool bAutoExtent = true, int nMask = -1);
	// inlcude file name in path e.g E:/test.bmp and only .bmp format avaliable

	void SaveImage(CString strSavePath);


private:
	CDC* m_pSymbleDC;
	CBitmap* m_pSymbleBitmap;
	CBitmap* m_pOldBitmap;
	int m_nSymbleSize;
	int m_iModuleSize;

};

