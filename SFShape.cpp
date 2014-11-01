#include "StdAfx.h"
#include "SFShape.h"

SFShape::SFShape(void) : X(0) , Y(0) , Name(NULL)
{
}

SFShape::~SFShape(void)
{
	if ( Name != NULL ) 
		delete[] Name;
}


void SFShape::draw(HDC dc)
{
	for ( UINT32 I = 0 ; I < Lines.size() ; I++)
	{
		MoveToEx(dc , (DWORD) ( Lines[I].left + X), (DWORD) ( Lines[I].top + Y ),NULL);
		LineTo( dc , (DWORD) ( Lines[I].right + X) , (DWORD)( Lines[I].bottom + Y));
	}
}

void SFShape::generateName( UINT32 Bits, UINT32 TotalBits )
{
	Name = new UINT64[TotalBits/64+1];
	for (UINT32 I = 0; I < TotalBits / 64 + 1 ; I++ ) 
		Name[I] = 0;

	for (UINT32 I = 0; I < Bits ; I++ ) 
	{
		UINT32 Pos = ( TotalBits * rand() ) / RAND_MAX;
		Name[Pos/64] |= ( 1ULL << (Pos % 64));  
	}
}

void SFShape::setPos( float PX , float PY )
{
	X = PX ;
	Y = PY ;
	
	int QX = X / 8;
	int QY = Y / 8;
	UINT64 PosCode = 0 ;
	if ( QX == 0 )
		PosCode |= 1;
	else if ( QX == 1 )
		PosCode |= 2;
	
	if ( QY == 0 )
		PosCode |= 4;
	else if ( QY == 1 )
		PosCode |= 8;
		
	Name[0] = (Name[0] & ~0xFULL) | PosCode;
}