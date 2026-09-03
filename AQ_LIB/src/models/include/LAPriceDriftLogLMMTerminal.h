#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDriftLMMTerminal.h"
#include "AQLBasic.h"
#include <float.h>

// ID for LAPriceDriftLogLMMTerminal
#define FN_DRIFTLOGLMMTERMINAL	2207
// Function name for LAPriceDriftLogLMMTerminal
#define FN_DRIFTLOGLMMTERMINAL_STR	"fn_drift_loglmmterminal"



static const double MAXIMUM_2207 = AQLMath::log(DBL_MAX) - 1.0;//

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (terminal measure) (log type)

*/
class LAPriceDriftLogLMMTerminal : public LAPriceDriftLMMTerminal
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftLogLMMTerminal(double Q = 1.0);
	// constructor
	LAPriceDriftLogLMMTerminal(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
//	LAPriceDriftLogLMMTerminal(const LAPriceDriftLogLMMTerminal& v);
	// Destructor
	virtual ~LAPriceDriftLogLMMTerminal();

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
