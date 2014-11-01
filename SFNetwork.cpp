#include "StdAfx.h"
#include "SFNetwork.h"
#include "SFUnit.h"

SFNetwork::SFNetwork(void) : Training(false) , PShape(NULL) , Speed(1.f)  , TrainingCount(0)
{
}

SFNetwork::~SFNetwork(void)
{
}

void SFNetwork::initialize()
{
	HWND Handle = GetDesktopWindow();
	Retina.initialize(Handle,20);

	Layers.resize(2);
	Layers[0].initialize(24,Retina.getWidth() );
	Layers[1].initialize(22,24);
	//Layers[2].initialize(16,22);
	//Layers[3].initialize(6,14);

	Layers[0].setSparseSize(19);
	//Layers[1].setSparseSize(19);
	//Layers[2].setSparseSize(15);
	//Layers[3].setSparseSize(4);

	Layers[0].setInputDelay(1);
	//Layers[1].setInputDelay(100);
	//Layers[2].setInputDelay(1000);
	//Layers[3].setInputDelay(1000);

/*
	Layers.resize(5);
	Layers[0].initialize(24,Retina.getWidth());
	Layers[1].initialize(22,24);
	Layers[2].initialize(20,22);
	Layers[3].initialize(18,20);
	Layers[4].initialize(14,18);

	Layers[0].setSparseSize(17);
	Layers[1].setSparseSize(15);
	Layers[2].setSparseSize(15);
	Layers[3].setSparseSize(13);
	Layers[4].setSparseSize(7);

	Layers[0].setInputDelay(2);
	Layers[1].setInputDelay(30);
	Layers[2].setInputDelay(60);
	Layers[3].setInputDelay(120);
	Layers[4].setInputDelay(240);
*/

	ShapeX.addLine( 0,0,8,8);
	ShapeX.addLine( 8,0,0,8);
	ShapeX.generateName(8 , Layers[1].getWidth() * Layers[1].getWidth() );

	ShapeTri.addLine( 0,0,8,0);
	ShapeTri.addLine( 8,0,8,8);
	ShapeTri.addLine( 8,8,0,8);
	ShapeTri.addLine( 0,8,0,0);
	ShapeTri.generateName(8 , Layers[1].getWidth() * Layers[1].getWidth() );
	
	PShape = &ShapeX;
	
}

void SFNetwork::setTraining(bool T )
{
	Training = T;
}

void SFNetwork::setLearning(bool T )
{
	for ( UINT32 I = 0 ; I < Layers.size() ; I++)
		Layers[I].setLearnEnabled(T);
}


float U = 1.f;
float P1X = 0 , P1Y = 0;
float P2X = 0 , P2Y = 0;

void SFNetwork::trainShape2()
{
	float X = ((Retina.getWidth() - 8 ) * rand()) / RAND_MAX;
	float Y = ((Retina.getWidth() - 8 ) * rand()) / RAND_MAX;
	PShape->setPos( X , Y);
	if ( (1000 * rand()) / RAND_MAX <= 100 )
		changeShape();
}

void SFNetwork::trainShape()
{
	U += 0.01f * Speed;
	if ( U >= 1.f ) 
	{
		P1X = P2X;
		P1Y = P2Y;
		P2X = (float) ( (Retina.getWidth() - 5)* rand()) / RAND_MAX;
		P2Y = (float) ( (Retina.getWidth() - 5)* rand()) / RAND_MAX;
		U = 0;
		if ( (1000 * rand()) / RAND_MAX <= 500 )
			changeShape();
	}
	float X = P1X + U * ( P2X - P1X);	
	float Y = P1Y + U * ( P2Y - P1Y);	
	PShape->setPos( X , Y);
}

void SFNetwork::run()
{
	HWND DesktopHandle = GetDesktopWindow();
	HDC DC = GetDC( DesktopHandle);

	if ( Layers[0].getLearnEnabled() )
		TrainingCount++;
	// draw the training
	HDC RetinaDC = Retina.startDraw(DC);
	PatBlt(RetinaDC , 0, 0, Retina.getWidth(), Retina.getWidth(), WHITENESS);
	if  (Training )
		trainShape2();
	PShape->draw(RetinaDC);
	UINT64* Input = Retina.getOutput(NULL);
	
	// process network
	Layers[0].feed2(Input,true);
	Layers[1].feed2(Layers[0].getOutput(),false);

	if ( Layers[0].getLearnEnabled() )
	{
		INT16 Error[2000];
		Layers[1].calcError( PShape->getName(), Error);
		
		Layers[1].backPropagate(Error);
		Layers[0].backPropagate(Layers[1].getInputError());
	}
	
	Retina.endDraw();
	ReleaseDC(DesktopHandle,DC);
}

void SFNetwork::changeShape()
{
	if ( PShape == &ShapeX )
		PShape = &ShapeTri;
	else
		PShape = &ShapeX;

};

void SFNetwork::save( const char* FileName)
{
	FILE* F = NULL;
	fopen_s( &F , FileName , "wb");
	UINT32 NumLayers = Layers.size();
	fwrite( &NumLayers , 4,  1 , F);
	
	fwrite( &TrainingCount , 4,  1 , F);

	for ( UINT32 I = 0 ; I < NumLayers ; I++ )
	{
		UINT32 Width = Layers[I].getWidth();
		fwrite( &Width , 4,  1 , F);
		UINT32 InputWidth = Layers[I].getInputWidth();
		fwrite( &InputWidth , 4,  1 , F);
		UINT32 InputDelay = Layers[I].getInputDelay();
		fwrite( &InputDelay , 4,  1 , F);
		UINT32 SparseSize = Layers[I].getSparseSize();
		fwrite( &SparseSize , 4,  1 , F);

		for ( UINT32 J = 0 ; J < Width * Width ; J++ )
		{
			SFUnit& Unit = Layers[I].Units[J];
			fwrite( Unit.Weights , 2 , InputWidth * InputWidth, F);
		}
	}
	fclose(F);
}

void SFNetwork::load(const char* FileName)
{
	FILE* F = NULL;
	fopen_s(&F , FileName , "rb");
	UINT32 NumLayers = 0;
	fread( &NumLayers , 4, 1, F);
	Layers.clear();
	Layers.resize(NumLayers);
	fread( &TrainingCount , 4,  1 , F);


	for ( UINT32 I = 0 ; I < NumLayers ; I++ )
	{
		UINT32 Width = 0;
		fread( &Width , 4,  1 , F);
		
		UINT32 InputWidth = 0;
		fread( &InputWidth , 4,  1 , F);

		UINT32 InputDelay = 0; 
		fread( &InputDelay , 4,  1 , F);
		
		UINT32 SparseSize = 0;
		fread( &SparseSize , 4,  1 , F);

		Layers[I].initialize( Width, InputWidth );
		Layers[I].setInputDelay(InputDelay);
		Layers[I].setSparseSize(SparseSize);

		for ( UINT32 J = 0 ; J < Width * Width ; J++ )
		{
			SFUnit& Unit = Layers[I].Units[J];
			fread( Unit.Weights , 2 , InputWidth * InputWidth, F);
			for ( UINT32 K = 0 ; K < InputWidth * InputWidth ; K++ )
				Unit.setWeight( K , Unit.Weights[K]);
		}
	}
	fclose(F);
}


void SFNetwork::takeSnapshot()
{
	for ( UINT32 I = 0 ; I < Layers.size() ; I++ )
		Layers[I].takeSnapshot();
}