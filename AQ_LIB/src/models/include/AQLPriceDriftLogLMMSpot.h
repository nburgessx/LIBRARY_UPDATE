#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftLMMSpot.h"
#include "AQLBasic.h"
#include <float.h>

// ID for AQLPriceDriftLogLMMSpot
#define FN_DRIFTLOGLMMSPOT	2206
// Function name for AQLPriceDriftLogLMMSpot
#define FN_DRIFTLOGLMMSPOT_STR	"fn_drift_loglmmspot"



static const double MAXIMUM_2206 = AQLMath::log(DBL_MAX) - 1.0;//

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (spot measure) (log type)

*/
class AQLPriceDriftLogLMMSpot : public AQLPriceDriftLMMSpot
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftLogLMMSpot(double Q = 1.0);
	// constructor
	AQLPriceDriftLogLMMSpot(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
//	AQLPriceDriftLogLMMSpot(const AQLPriceDriftLogLMMSpot& v);
	// Destructor
	virtual ~AQLPriceDriftLogLMMSpot();

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

