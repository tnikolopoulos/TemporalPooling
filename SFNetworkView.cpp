#include "StdAfx.h"
#include "SFNetworkView.h"
#include "SFNetwork.h"
#include "SFLayer.h"
#include "SFUnit.h"

SFNetworkView::SFNetworkView(void) : PNetwork(NULL), UpdateDisplay(true) , BackBuffer(NULL) , Handle(NULL) , Scale(1.f) , SelectedLayer(-1)
{
}

SFNetworkView::~SFNetworkView(void)
{
}

void SFNetworkView::initialize( HWND H)
{
	Handle = H;
	handleResize();
}

void SFNetworkView::handleResize()
{
	GetClientRect( Handle , &ClientRect );

	if ( BackBuffer != NULL ) 
		DeleteObject( BackBuffer);
		
	HDC SDC = GetDC( Handle);
	BackBuffer = CreateCompatibleBitmap( SDC , ClientRect.right , ClientRect.bottom );
	ReleaseDC(Handle,SDC);
}	

void SFNetworkView::updateView()
{
	if ( !PNetwork ) 
		return;
	if ( !UpdateDisplay )
		return;
	
	SFRetina& Retina = PNetwork->getRetina();
	UINT32 RetinaWidth = Retina.getWidth();
	
	HDC DC = GetDC( Handle);

	HDC BackBufferDC = CreateCompatibleDC( DC);
	SelectObject( BackBufferDC , BackBuffer);

	PatBlt( BackBufferDC, 0,0, ClientRect.right , ClientRect.bottom, WHITENESS);

	HDC RetinaDC = Retina.startDraw(DC);
	BitBlt(  BackBufferDC , 0, 0, Retina.getWidth() , Retina.getWidth() , RetinaDC , 0, 0, SRCCOPY);
	Retina.endDraw();
	
	std::vector<SFLayer>& Layers = PNetwork->getLayers();
	
	UINT32 OffsetX = RetinaWidth + 2 ;
	UINT32 OffsetY = RetinaWidth + 2;
	for ( UINT32 I = 0 ; I < Layers.size() ; I++ )
	{
		displayOutput(BackBufferDC , Layers[I].getWidth() , Layers[I].getOutput(), Layers[I].getTargetOutput() == NULL ? Layers[I].getSnapshot():Layers[I].getTargetOutput()   , OffsetX, 0 );
		OffsetX += Layers[I].getWidth() + 2 ;
	}
	OffsetX *= 2;
	if ( SelectedLayer == -1 )
		displayReconstruction(BackBufferDC, Layers[0] , Layers[0].getOutput() , 0 , OffsetY);
	else
		displayWeights( BackBufferDC , Layers[SelectedLayer], 0, OffsetY);
	OffsetY += RetinaWidth + 2; 

	char Buf[256];
	sprintf_s( Buf , 256 , "%d" , PNetwork->getTrainingCount() );
	TextOutA( BackBufferDC , RetinaWidth + 4 , RetinaWidth +  5 , Buf, strlen(Buf) );

	StretchBlt( DC , 0, 0, ClientRect.right , (OffsetY *  ClientRect.right) / OffsetX , BackBufferDC, 0, 0, OffsetX , OffsetY ,SRCCOPY);
	
	DeleteDC(BackBufferDC);
	ReleaseDC(Handle,DC);
	Scale = (float)ClientRect.right / (float)OffsetX;
}

void SFNetworkView::displayOutput( HDC DC , UINT32 Width, UINT64* Bits, UINT64* SnapBits,  UINT32 SX , UINT32 SY )
{
	UINT32 Sz = 1;
	Rectangle(DC,SX-1, SY-1, SX + Width * Sz  +1 , SY + Width * Sz + 1);
	for ( UINT32 Y = 0; Y < Width ; Y++ )
	{
		for ( UINT32 X = 0; X < Width ; X++ )
		{
			UINT32 Index = Y * Width + X ; 

			bool B1 = ( Bits[Index/64] & ( 1ULL << (Index%64))) != 0 ;
			bool B2 = SnapBits == NULL ? false : SnapBits[Index/64] & ( 1ULL << (Index%64));
			
			if ( B1 | B2 ) 
			{
				DWORD Col = 0xFF0000; // neutral
				if ( B2 & B1 )
					Col = 0xFF00; // match
				if ( B2 & !B1)
					Col = 0xDDDDDD; // ghost

				SetPixel( DC, X + SX , Y + SY  , Col);
			}
			// PatBlt( DC , X * Sz + SX , Y * Sz + SY, Sz , Sz , BLACKNESS );
		}
	}
}

void SFNetworkView::displayWeights(HDC DC , SFLayer& Layer , UINT32 SX , UINT32 SY )
{
	if ( Layer.getSelectedUnit() == -1 )
		return;
	UINT32 InputWidth = Layer.getInputWidth();
	SFUnit& Unit = Layer.Units[Layer.getSelectedUnit()];
	for ( UINT32 Y = 0 ; Y < InputWidth ; Y++ )
	{
		for ( UINT32 X = 0 ; X < InputWidth ; X++ )
		{
			UINT32 J = Y * InputWidth + X;
			INT32 C = Unit.Weights[J]/0xFF ;
			C -= 128;
			C *= 20;
			//C += 128;
			if ( C < 0 ) C = 0;
			if ( C > 0xFF ) C = 0xFF;
			SetPixel( DC , X + SX , Y + SY , C );
		}
	}
}

void SFNetworkView::displayReconstruction(HDC DC , SFLayer& Layer , UINT64* Output , UINT32 SX , UINT32 SY )
{
	UINT32 RetinaWidth = PNetwork->getRetina().getWidth();
	UINT32* PX = new UINT32[RetinaWidth*RetinaWidth];	

	for ( UINT32 I = 0 ; I < RetinaWidth * RetinaWidth ; I++ )	
		PX[I] = 0;

	UINT32 Count = 0 ;
	for ( UINT32 I = 0 ; I < Layer.getWidth() * Layer.getWidth() ; I++ )
	{
		UINT64 Mask = 1ULL << ( I % 64 );
		if (  ( Output[I / 64] & Mask ) == 0)
			continue;

		Count++;
		SFUnit& Unit = Layer.Units[I];
		for ( UINT32 Y = 0 ; Y < RetinaWidth ; Y++ )
		{
			for ( UINT32 X = 0 ; X < RetinaWidth ; X++ )
			{
				UINT32 J = Y * RetinaWidth + X;
				if ( Unit.Connections[J / 64] & (1ULL << (J%64)) )
					PX[J] = PX[J] + 1;
			}
		}
	}
	for ( UINT32 Y = 0;  Y < RetinaWidth ; Y++ )
	{
		for ( UINT32 X = 0 ; X  < RetinaWidth ; X++ )
		{
			UINT32 P = PX[Y * RetinaWidth + X ] * 5 ;
			if ( P > 255 ) 
				P = 255;
			SetPixel( DC , X + SX , Y + SY , P);
		}
	}

	delete[] PX;
}

void SFNetworkView::handleLeftButtonDown(INT32 X , INT32 Y )
{
	float 	MX = X / Scale; 
	float	MY = Y / Scale;
	if ( MX < PNetwork->getRetina().getWidth() ) 
	{
		PNetwork->getShape()->setPos( MX , MY );
	}
	else if ( MX < PNetwork->getRetina().getWidth() + 2 + PNetwork->getLayers()[0].getWidth() )
	{
		INT32 CX = MX - PNetwork->getRetina().getWidth() - 2;
		INT32 CY = MY ;
		PNetwork->getLayers()[0].setSelectedUnit( CY * PNetwork->getLayers()[0].getWidth() + CX);				
		SelectedLayer = 0;
	}
	else if ( MX < PNetwork->getRetina().getWidth() + 4 + PNetwork->getLayers()[0].getWidth() + PNetwork->getLayers()[1].getWidth() )
	{
		INT32 CX = MX - PNetwork->getRetina().getWidth() - 4 - PNetwork->getLayers()[0].getWidth();
		INT32 CY = MY ;
		PNetwork->getLayers()[1].setSelectedUnit( CY * PNetwork->getLayers()[1].getWidth() + CX);
		SelectedLayer = 1;
	}
}