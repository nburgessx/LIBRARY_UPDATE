#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDriftSMMBase.h"


// ID for AQLPriceDriftSMMAnnuity
#define FN_DRIFTSMMANNUITY	2211
// Function name for AQLPriceDriftSMMAnnuity
#define FN_DRIFTSMMANNUITY_STR	"fn_drift_smmannuity"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of SMM sde (annuity measure)

*/
class AQLPriceDriftSMMAnnuity : public AQLPriceDriftSMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftSMMAnnuity(double s);
	// constructor
	AQLPriceDriftSMMAnnuity(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double s);	
	//	Copy constructor
	AQLPriceDriftSMMAnnuity(const AQLPriceDriftSMMAnnuity& v);
	// Destructor
	virtual ~AQLPriceDriftSMMAnnuity();

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

//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);

private:
protected:

};

