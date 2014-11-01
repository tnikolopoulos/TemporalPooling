#pragma once

class SFRetina
{
public:
	SFRetina(void);
	~SFRetina(void);

	void	initialize( HWND Handle, UINT32 Width );
	UINT64* getOutput(HDC DC);

	HDC startDraw( HDC DC );
	void endDraw();
	void moveTo(int X , int Y );
	void lineTo(int X , int Y );

	UINT32 getWidth() const { return Width; }
	
private:
	HBITMAP Bitmap;
	UINT32 Width;
	RGBQUAD* PBits;
	UINT64* Output;
	HDC DrawDC;
};
