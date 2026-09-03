#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDriftLMMBase.h"


// ID for LAPriceDriftLMMSpot
#define FN_DRIFTLMMSPOT	2203
// Function name for LAPriceDriftLMMSpot
#define FN_DRIFTLMMSPOT_STR	"fn_drift_lmmspot"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (spot measure)

*/
class LAPriceDriftLMMSpot : public LAPriceDriftLMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftLMMSpot(double Q = 1.0);
	// constructor
	LAPriceDriftLMMSpot(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
	LAPriceDriftLMMSpot(const LAPriceDriftLMMSpot& v);
	// Destructor
	virtual ~LAPriceDriftLMMSpot();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);

private:
protected:
};
