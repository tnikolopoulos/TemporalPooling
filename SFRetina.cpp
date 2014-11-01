#include "StdAfx.h"
#include "SFRetina.h"

SFRetina::SFRetina(void) : Bitmap(NULL) , PBits(NULL),Output(NULL)
{
}

SFRetina::~SFRetina(void)
{
	if ( Bitmap )
		DeleteObject(Bitmap);
	if ( PBits )
		delete[] PBits;
	if ( Output)
		delete[] Output;
}


void SFRetina::initialize( HWND Handle, UINT32 PWidth )
{
	Width = PWidth;
	HDC DC = GetDC(Handle);
	Bitmap = CreateCompatibleBitmap(DC,Width,Width);
	ReleaseDC( Handle, DC);
	PBits = new RGBQUAD[Width*Width];
	Output = new UINT64[(Width * Width) / 64  + 1];
}

UINT64* SFRetina::getOutput( HDC DC)
{
	BITMAPINFO bmi = {0}; 
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader); 
	bmi.bmiHeader.biWidth = Width ; 
	bmi.bmiHeader.biHeight = Width ; 
	bmi.bmiHeader.biPlanes = 1; 
	bmi.bmiHeader.biBitCount = 32; 
	bmi.bmiHeader.biCompression = BI_RGB; 

	GetDIBits(
		DrawDC,
		Bitmap, 
		0,  
		Width,  
		PBits,
		&bmi,  
		DIB_RGB_COLORS
		);  
	

	
	/*
	for ( UINT32 Y = 0; Y < Width ; Y++ )
	{
		for ( UINT32 X = 0 ; X < Width ; X++ )
		{
			RGBQUAD Val = PBits[Y * Width + X];
			SetPixel(DC, X ,Y , Val.rgbBlue);
		}
	}*/
	
	memset( Output, 0, ((Width*Width) / 64 + 1) * 8 );
	for ( UINT32 I = 0 ; I < Width * Width; I++ )
	{
		if ( PBits[I].rgbBlue == 0 )
			Output[I / 64] |= 1ULL << (I % 64);
	}
	return Output;
}

HDC SFRetina::startDraw(HDC DC )
{
	DrawDC = CreateCompatibleDC(DC);
	SelectObject( DrawDC, Bitmap);
	return DrawDC;
}

void SFRetina::endDraw()
{
	DeleteDC(DrawDC);
	DrawDC = NULL;
}

void SFRetina::moveTo( int X ,int Y )
{
	MoveToEx( DrawDC , X , Y, NULL);
}

void SFRetina::lineTo( int X , int Y)
{
	LineTo( DrawDC , X , Y);
}

