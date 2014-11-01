#include "StdAfx.h"
#include "SFUnit.h"

SFUnit::SFUnit(void) : Weights(NULL) , Connections(NULL)
{
}

SFUnit::~SFUnit(void)
{
	if ( Weights )
		delete[] Weights;
	if ( Connections )
		delete[] Connections;
}


void SFUnit::setWeight( UINT32 Index , UINT16 W )
{
	Weights[Index] = W;

	UINT32 Q = Index/64;
	UINT64 R = Index%64;
	if ( W > CONNECTION_THRESHOLD )
		Connections[Q] |= (1ULL << R);
	else
		Connections[Q] &= ~(1ULL << R);
}

