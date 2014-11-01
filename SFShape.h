#pragma once

class SFShape
{
public:
	SFShape(void);
	~SFShape(void);

	void draw(HDC dc );
	void addLine( UINT32 X1 , UINT32 Y1 , UINT32 X2 , UINT32 Y2 ) { RECT R ={X1,Y1,X2,Y2}; Lines.push_back(R); }
		

	void generateName( UINT32 Bits , UINT32 TotalBits );
	UINT64*					getName() { return Name;}
	
	void setPos( float PX , float PY );

private:
	float					X,Y;
	std::vector<RECT>		Lines;
	UINT64*					Name;
};
