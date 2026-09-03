///////////////////////////////////////////////////////////this is the start sorce of Calibration Func Base////
#ifndef AQLMathRStarFinder_h
#define AQLMathRStarFinder_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLShiftMethod
#define FN_JAMSHIDIANRSTARFINDER	10042
// Function Name of AQLShiftMethod
#define FN_JAMSHIDIANRSTARFINDER_STR	"fn_jamshidianrstarfinder"


class AQLRatesPathElementCurve;

class AQLMathJamshidianRStarFinder : public AQLFunctionBase
{
public:
	// constructor
	AQLMathJamshidianRStarFinder(double strike, double premium, const DoubleVector& grids, 
													  const DoubleVector& deltas, AQLRatesPathElementCurve* pcurve);
	// copy constructor
	AQLMathJamshidianRStarFinder(const AQLMathJamshidianRStarFinder &rhs) ;
	// Destructor
	virtual ~AQLMathJamshidianRStarFinder();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	
	AQLMathJamshidianRStarFinder & operator=( const AQLMathJamshidianRStarFinder & ) { return *this; }
private:
	double mStrike;
	double mPrem;
	DoubleVector mGrids;
	DoubleVector mDelta;
	AQLRatesPathElementCurve* mpCurve;
	bool is_cloned;
};

#endif
