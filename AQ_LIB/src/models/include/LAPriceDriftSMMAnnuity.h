#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDriftSMMBase.h"


// ID for LAPriceDriftSMMAnnuity
#define FN_DRIFTSMMANNUITY	2211
// Function name for LAPriceDriftSMMAnnuity
#define FN_DRIFTSMMANNUITY_STR	"fn_drift_smmannuity"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of SMM sde (annuity measure)

*/
class LAPriceDriftSMMAnnuity : public LAPriceDriftSMMBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftSMMAnnuity(double s);
	// constructor
	LAPriceDriftSMMAnnuity(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double s);	
	//	Copy constructor
	LAPriceDriftSMMAnnuity(const LAPriceDriftSMMAnnuity& v);
	// Destructor
	virtual ~LAPriceDriftSMMAnnuity();

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

//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);

private:
protected:

};

