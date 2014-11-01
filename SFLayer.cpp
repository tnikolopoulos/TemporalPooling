#include "StdAfx.h"
#include "SFLayer.h"
#include "SFUnit.h"

SFLayer::SFLayer(void) : Units(NULL) , Output(NULL) , LearnEnabled(false) , Current(0) , SparseSize(7),  
InputUnion(NULL) , Width(5) , InputWidth(5) , Snapshot(NULL) , SelectedUnit(-1)  , Input(NULL), InputError(NULL),TargetOutput(NULL)
{
}

SFLayer::~SFLayer(void)
{
	if ( Units )
		delete []Units;
	if ( Output )
		delete[]Output;
	if ( Snapshot != NULL )
		delete[] Snapshot;
	if ( InputUnion != NULL) 
		delete[] InputUnion;
	for ( UINT32 I = 0 ; I < InputHistory.size() ; I++ )
		delete[] InputHistory[I];
	if ( InputError != NULL)
		delete[] InputError;
}

BYTE BitCounts[0xFFFF];
bool GInitializedBitCounts = false;

UINT32 countBits( const UINT64& V)
{
	if ( !GInitializedBitCounts )
	{
		for ( UINT16 I = 0 ; I < 0xFFFF ; I++ )
		{
			BitCounts[I] = 0;
			for  ( UINT32 S = 0 ; S < 16 ;S++) 
				if ( (1 << S) & I )
					BitCounts[I]++;			
		}
		GInitializedBitCounts = true;
	}

	UINT16* SV = (UINT16*)&V; 
	return BitCounts[SV[0]] + BitCounts[SV[1]] + BitCounts[SV[2]] + BitCounts[SV[3]];
}



void SFLayer::initialize( UINT32 PWidth , UINT32 PInputWidth ) 
{
	Width = PWidth;
	InputWidth = PInputWidth;

	Output = new UINT64[(Width * Width)/64 + 1];
	memset(Output,0, (Width * Width / 64 + 1) * 8 );
	Units = new SFUnit[Width * Width];
	for ( UINT32 I = 0; I < Width * Width ; I++ )		
	{
		Units[I].Connections = new UINT64[ (InputWidth * InputWidth) / 64 + 1];
		Units[I].Weights = new UINT16[InputWidth * InputWidth ];
		for ( UINT32 J = 0 ; J < InputWidth * InputWidth ; J++ )
			Units[I].setWeight( J , 0x7FFF + (10000 * rand())/RAND_MAX - 5000);
	}
	InputError = new INT16[InputWidth * InputWidth];
}

void SFLayer::feed2(UINT64* PInput , bool First) 
{
	Input = PInput;
	memset( Output,0, ((Width* Width)/64 + 1) * 8 );

	for ( UINT32 I = 0 ; I < Width * Width ; I++ )
	{
		SFUnit& Unit = Units[I];

		UINT32 InputCount = 0;
		Unit.Overlap = 0;		
		for ( UINT32 J = 0 ; J < InputWidth * InputWidth / 64 + 1 ; J++) 
		{
			Unit.Overlap += countBits(Unit.Connections[J] & Input[J]);
			InputCount += countBits(Input[J]);
		}
		
		UINT32 Q = I / 64;
		UINT32 R = I % 64;
		UINT64 Mask = 1ULL << R ; 

		if ( First )
		{
			if ( Unit.Overlap >  (90 * InputCount ) / 100 )
				Output[Q] |= Mask;
		}
		else
		{
			if ( Unit.Overlap >  8) // (90 * InputCount ) / 100 )
				Output[Q] |= Mask;
		}
	}
}

void SFLayer::calcError( UINT64* PTargetOutput , INT16* Errors) 
{
	TargetOutput = PTargetOutput;

	for ( UINT32 I = 0 ; I < Width * Width ; I++ )
	{
		UINT32 Q = I / 64;
		UINT32 R = I % 64;
		UINT64 Mask = 1ULL << R;
		int T = (PTargetOutput[Q] & Mask) ? 1 : 0;
		int Y = (Output[Q] & Mask ) ? 1 : 0;
		Errors[I]= T - Y;
	}
}

void SFLayer::backPropagate( INT16* Errors ) 
{
	memset( InputError, 0 , InputWidth * InputWidth * 2);
	for ( UINT32 I = 0 ; I < Width * Width ; I++ )
	{
		INT16 Error = Errors[I];
		for ( UINT32 J = 0 ; J < InputWidth * InputWidth ; J++ )
		{
			UINT32 Q = J / 64;
			UINT32 R = J % 64;
			UINT64 Mask = 1ULL << R;
			int X = (Input[Q] & Mask ) ? 1 : 0;
			Units[I].setWeight( J , Units[I].Weights[J] + 4 * X * Error );
			InputError[J] += Error;
		}
	}
}

UINT64* SFLayer::feed(UINT64* Input) 
{	
	memcpy( InputHistory[Current]  , Input , ((InputWidth * InputWidth) / 64 + 1) * 8 );
	Current++;
	if ( Current == InputHistory.size() )
		Current = 0;
	
	for (UINT32 I = 0 ; I < InputWidth * InputWidth / 64 + 1 ; I++ )
	{
		InputUnion[I] = 0;
		for ( UINT32 J = 0 ; J < InputHistory.size() ; J++ )	
			InputUnion[I] |= InputHistory[J][I];
	}
	Input = InputUnion;

	UINT32 Sz = ( InputWidth * InputWidth ) / 64 + 1;

	for ( UINT32 I = 0 ; I < Width * Width; I++ )
	{
		SFUnit& Unit = Units[I];
		Unit.Overlap = 0;		
		for ( UINT32 J = 0 ; J < Sz ; J++) 
			Unit.Overlap += countBits(Unit.Connections[J] & Input[J]);
	}
	memset( Output,0, ((Width* Width)/64 + 1) * 8 );

	for ( UINT32 Y = 0 ; Y < Width ; Y++ )
	{
		for ( UINT32 X = 0 ; X < Width ; X++ ) 
		{
			UINT32 Index = Y * Width + X;
			SFUnit& Unit1 = Units[Index];
			if (Unit1.Overlap < OVERLAP_THRESHOLD )
				continue;
				
			UINT32 NY = 0 , NX = 0 ;
		
			for ( NY = 0 ; NY < SparseSize ; NY++ )
			{
				INT32 PY = Y + NY - SparseSize / 2;
				if ( PY >= (INT32)Width || PY < 0 )
					continue;
				for ( NX = 0 ; NX < SparseSize ; NX++ )
				{
					INT32 PX = X + NX - SparseSize / 2;
					if ( PX >= (INT32)Width || PX < 0)
						continue;
					SFUnit& Unit2 = Units[PY * Width + PX ];
					if  ( Unit2.Overlap > Unit1.Overlap )
						break;
				}
				if ( NX != SparseSize )
					break;
			}

			if ( NY != SparseSize )
				continue;
			
			if ( LearnEnabled )
				learn(Unit1,Input);

			UINT32 Q = Index / 64;
			UINT64 Mask = 1ULL << (Index % 64);
			Output[Q] |= Mask;
		}
	}
	return Output;
}

void SFLayer::learn(SFUnit& Unit , UINT64* Input)
{
	for ( UINT32 I = 0 ; I < InputWidth * InputWidth ; I++ )
	{
		INT32 W = Unit.Weights[I];
		if ( Input[I / 64] & ( 1ULL << (I % 64)  )  )
			W+=10;
		else
			W-=3;
		if ( W < 0 ) 
			W = 0;
		if ( W > 0xFFFF )
			W = 0xFFFF;
		Unit.setWeight(I,W);
	}
}

void SFLayer::setInputDelay(UINT32 ID ) 
{
	for ( UINT32 I = 0 ; I < ID ; I++ )
		InputHistory.push_back( new UINT64[InputWidth * InputWidth / 64 + 1]);
	InputUnion = new UINT64[InputWidth * InputWidth / 64 + 1];
}

void SFLayer::takeSnapshot()
{
	UINT32 Size = Width * Width / 64 + 1;
	if ( Snapshot == NULL)
		Snapshot = new UINT64[Size]; 
	memcpy( Snapshot , Output , Size * 8 );
}