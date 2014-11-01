#pragma once

class SFUnit;

class SFLayer
{
public:
	SFLayer(void);
	~SFLayer(void);

	void	initialize( UINT32 Width , UINT32 InputWidth);
	UINT64* feed(UINT64* Input);

	void	setLearnEnabled( bool LE )  { LearnEnabled = LE;}
	bool	getLearnEnabled() { return LearnEnabled;}

	void	learn(SFUnit& Unit , UINT64* Input);

	UINT32	getWidth() { return Width;}
	UINT64* getOutput() { return Output;}
	UINT32	getInputWidth() { return InputWidth;}

	void	setInputDelay(UINT32 ID);
	UINT32	getInputDelay() { return InputHistory.size();}

	void	setSparseSize( UINT32 SS ) {SparseSize = SS;}
	UINT32	getSparseSize() {return SparseSize;}
	
	void	takeSnapshot();
	UINT64* getSnapshot() const { return Snapshot; }

	void	setTargetOutput( UINT64* PTO ) { TargetOutput = PTO;}
	UINT64* getTargetOutput() { return TargetOutput;}
	
	void	feed2(UINT64* PInput , bool First) ;
	void	calcError( UINT64* PTargetOutput , INT16* Errors);
	void	backPropagate( INT16* Errors );
	INT16*	getInputError() { return InputError;}

	SFUnit*		Units;

	INT32 getSelectedUnit() const { return SelectedUnit; }
	void setSelectedUnit(const INT32& val) { SelectedUnit = val; }
private:
	UINT32		SparseSize;
	
	INT16*		InputError;
	UINT64*		Output, *Snapshot, *Input;
	UINT32		Width;
	UINT32		InputWidth;
	bool		LearnEnabled;
	
	UINT64*		InputUnion;
	std::vector<UINT64*>	InputHistory;
	UINT32		Current;
	UINT64*		TargetOutput;
	INT32		SelectedUnit;
};
