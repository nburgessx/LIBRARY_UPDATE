#ifndef AQLMathJamshidianSwaptionByImplyVol_h
#define AQLMathJamshidianSwaptionByImplyVol_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQLMathJamshidianRStarFinder.h"
#include "AQLGaussLegendre.h"
#include "AQLMathJamshidianSwaption.h"

#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"



// Funciton ID of AQLShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL	10044
// Function Name of AQLShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR	"fn_jamshidianswaptionbyimplyvol"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;


class AQLMathJamshidianSwaptionByImplyVol : public AQLMathJamshidianSwaption
{
public:
	// Default constructor
	AQLMathJamshidianSwaptionByImplyVol();
	// Copy constructor
	//AQLMathJamshidianSwaptionByImplyVol(AQLMathJamshidianSwaptionByImplyVol& v);
	// Destructor
	~AQLMathJamshidianSwaptionByImplyVol();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	

	virtual double              value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const;

	/*!
		@brief cache class for performance up
	*/
	class AQLMathJamshidianSwaptionByImplyVolDataProvider : public AQLMathJamshidianSwaptionDataProvider
	{
	public:
		AQLMathJamshidianSwaptionByImplyVolDataProvider();
		
		virtual ~AQLMathJamshidianSwaptionByImplyVolDataProvider(); 
		
		AnalyticBKParam mBKParam;
		AQLBlackScholesBase* mAnalyticMethod2;
		AQLBlackScholesBase* mAnalyticMethod3;
	};

		// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
	

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
#endif

