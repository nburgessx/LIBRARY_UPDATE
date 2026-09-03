#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftLMMTerminal.h"
#include "AQLBasic.h"
#include <float.h>

// ID for AQLPriceDriftLogLMMTerminal
#define FN_DRIFTLOGLMMTERMINAL	2207
// Function name for AQLPriceDriftLogLMMTerminal
#define FN_DRIFTLOGLMMTERMINAL_STR	"fn_drift_loglmmterminal"



static const double MAXIMUM_2207 = AQLMath::log(DBL_MAX) - 1.0;//

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (terminal measure) (log type)

*/
class AQLPriceDriftLogLMMTerminal : public AQLPriceDriftLMMTerminal
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftLogLMMTerminal(double Q = 1.0);
	// constructor
	AQLPriceDriftLogLMMTerminal(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
//	AQLPriceDriftLogLMMTerminal(const AQLPriceDriftLogLMMTerminal& v);
	// Destructor
	virtual ~AQLPriceDriftLogLMMTerminal();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;

private:
protected:
	virtual inline	double		getLpart(double x, double delta) const
								{
									if (x > MAXIMUM_2207) return 1.0;
									double deltaL = delta * AQLMath::exp(x);
									return deltaL / (1.0 + deltaL - mSpread * delta);	
								}	


};
