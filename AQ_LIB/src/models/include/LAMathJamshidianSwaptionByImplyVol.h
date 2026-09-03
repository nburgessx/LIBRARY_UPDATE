#ifndef LAMathJamshidianSwaptionByImplyVol_h
#define LAMathJamshidianSwaptionByImplyVol_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABasic.h"
#include "LADist.h"
#include "LADataValuation.h"
#include "LAFindRootBrent.h"
#include "LAMathJamshidianRStarFinder.h"
#include "LAGaussLegendre.h"
#include "LAMathJamshidianSwaption.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"



// Funciton ID of LAShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL	10044
// Function Name of LAShiftMethod
#define FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR	"fn_jamshidianswaptionbyimplyvol"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;


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
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	

	virtual double              value(const LADate& basedate, LAObject& inst, const LADataValuation& att) const;

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
	LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;
	

private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
};
#endif

