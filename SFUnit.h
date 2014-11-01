#pragma once

class SFUnit
{
public:
	SFUnit(void);
	~SFUnit(void);
	
	void setWeight( UINT32 Index , UINT16 W );

	UINT16*	Weights;
	UINT64*	Connections;
	UINT32 Overlap;
};
