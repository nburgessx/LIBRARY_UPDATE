///////////////////////////////////////////////////////////this is the start sorce of Calibration Func Base////
#ifndef LAMathRStarFinder_h
#define LAMathRStarFinder_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAShiftMethod
#define FN_JAMSHIDIANRSTARFINDER	10042
// Function Name of LAShiftMethod
#define FN_JAMSHIDIANRSTARFINDER_STR	"fn_jamshidianrstarfinder"


class LARatesPathElementCurve;

class LAMathJamshidianRStarFinder : public LAFunctionBase
{
public:
	// constructor
	LAMathJamshidianRStarFinder(double strike, double premium, const DoubleVector& grids, 
													  const DoubleVector& deltas, LARatesPathElementCurve* pcurve);
	// copy constructor
	LAMathJamshidianRStarFinder(const LAMathJamshidianRStarFinder &rhs) ;
	// Destructor
	virtual ~LAMathJamshidianRStarFinder();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	
	LAMathJamshidianRStarFinder & operator=( const LAMathJamshidianRStarFinder & ) { return *this; }
private:
	double mStrike;
	double mPrem;
	DoubleVector mGrids;
	DoubleVector mDelta;
	LARatesPathElementCurve* mpCurve;
	bool is_cloned;
};

#endif
