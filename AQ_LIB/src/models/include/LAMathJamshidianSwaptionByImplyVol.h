#ifndef LAMathJamshidianSwaptionByImplyVol_h
#define LAMathJamshidianSwaptionByImplyVol_h

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
#include "LAMathJamshidianRStarFinder.h"
#include "AQLGaussLegendre.h"
#include "LAMathJamshidianSwaption.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"



// Funciton ID of AQLShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL	10044
// Function Name of AQLShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR	"fn_jamshidianswaptionbyimplyvol"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;


class LAMathJamshidianSwaptionByImplyVol : public LAMathJamshidianSwaption
{
public:
	// Default constructor
	LAMathJamshidianSwaptionByImplyVol();
	// Copy constructor
	//LAMathJamshidianSwaptionByImplyVol(LAMathJamshidianSwaptionByImplyVol& v);
	// Destructor
	~LAMathJamshidianSwaptionByImplyVol();
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
	class LAMathJamshidianSwaptionByImplyVolDataProvider : public LAMathJamshidianSwaptionDataProvider
	{
	public:
		LAMathJamshidianSwaptionByImplyVolDataProvider();
		
		virtual ~LAMathJamshidianSwaptionByImplyVolDataProvider(); 
		
		AnalyticBKParam mBKParam;
		LABlackScholesBase* mAnalyticMethod2;
		LABlackScholesBase* mAnalyticMethod3;
	};

		// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
	

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
#endif

