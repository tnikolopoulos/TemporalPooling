#pragma once

#include "SFLayer.h"
#include "SFRetina.h"
#include "SFShape.h"

class SFNetwork
{
public:
	SFNetwork(void);
	~SFNetwork(void);

	void initialize();
	void run();
	void changeShape();
	
	void setTraining( bool L ) ;
	bool getTraining() { return Training;}
	
	void setLearning( bool L ) ;
	bool getLearning() { return Layers[0].getLearnEnabled() ;}

	void setSpeed( float S ) { Speed = S; if ( Speed < 0.05f) Speed = 0.05f;}
	float getSpeed() { return Speed;}

	void save( const char* FileName);
	void load( const char* FileName);

	SFRetina&	getRetina() { return Retina; }
	std::vector<SFLayer>& getLayers() { return Layers; }

	SFShape*	getShape() { return PShape;}
	
	void takeSnapshot();
	
	UINT32 getTrainingCount() const { return TrainingCount; }

protected:
	void trainShape();
	void trainShape2();

	bool Training;
	std::vector<SFLayer>	Layers;
	SFRetina Retina;
	SFShape* PShape;
	SFShape ShapeX ,ShapeTri;
	float Speed ;
	UINT32 TrainingCount;
};


