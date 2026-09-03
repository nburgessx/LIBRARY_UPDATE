#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftLMMBase.h"


// ID for AQLPriceDriftLMMSpot
#define FN_DRIFTLMMSPOT	2203
// Function name for AQLPriceDriftLMMSpot
#define FN_DRIFTLMMSPOT_STR	"fn_drift_lmmspot"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (spot measure)

*/
class AQLPriceDriftLMMSpot : public AQLPriceDriftLMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftLMMSpot(double Q = 1.0);
	// constructor
	AQLPriceDriftLMMSpot(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
	AQLPriceDriftLMMSpot(const AQLPriceDriftLMMSpot& v);
	// Destructor
	virtual ~AQLPriceDriftLMMSpot();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);

private:
protected:
};
