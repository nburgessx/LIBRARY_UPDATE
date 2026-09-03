#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDriftLMMForward.h"
#include "AQLBasic.h"
#include <float.h>

// ID for LAPriceDriftLogLMMForward
#define FN_DRIFTLOGLMMFORWARD	2209
// Function name for LAPriceDriftLogLMMForward
#define FN_DRIFTLOGLMMFORWARD_STR	"fn_drift_loglmmforward"



static const double MAXIMUM_2206 = AQLMath::log(DBL_MAX) - 1.0;//

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (forward measure) (log type)

*/
class LAPriceDriftLogLMMForward : public LAPriceDriftLMMForward
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftLogLMMForward(unsigned int point, double Q = 1.0);
	// constructor
	LAPriceDriftLogLMMForward(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, unsigned int point, double Q = 1.0);	
	//	Copy constructor
//	LAPriceDriftLogLMMForward(const LAPriceDriftLogLMMForward& v);
	// Destructor
	virtual ~LAPriceDriftLogLMMForward();

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
									if (x > MAXIMUM_2206) return 1.0;
									double deltaL = delta * AQLMath::exp(x);
									return deltaL / (1.0 + deltaL - mSpread * delta);	
								}


};
