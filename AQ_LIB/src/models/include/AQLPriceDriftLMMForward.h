#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftLMMBase.h"

// ID for AQLPriceDriftLMMForward
#define FN_DRIFTLMMFORWARD	2208
// Function name for AQLPriceDriftLMMForward
#define FN_DRIFTLMMFORWARD_STR	"fn_drift_lmmforward"


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of LMM sde (forward measure)

*/
class AQLPriceDriftLMMForward : public AQLPriceDriftLMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftLMMForward(unsigned int point, double Q = 1.0);
	// constructor
	AQLPriceDriftLMMForward(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, unsigned int point, double Q = 1.0);	
	//	Copy constructor
	AQLPriceDriftLMMForward(const AQLPriceDriftLMMForward& v);
	// Destructor
	virtual ~AQLPriceDriftLMMForward();

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
								//==========================================
								// transform from string representaion
    virtual void				convertFromString(const AQLString& str);
								//======================================
								// set tenor
	virtual void				setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor);
								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);
private:

protected:
	unsigned int				mPoint;			// forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
};

