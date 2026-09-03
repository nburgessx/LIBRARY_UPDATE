#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// ID for LARatesPCIntegral
#define FN_PCINTEGRAL	10802
// Function name for LARatesPCIntegral
#define FN_PCINTEGRAL_STR	"fn_pcintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class LARatesPCIntegral : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesPCIntegral(SDEINTEGRAL_TYPE type);
	//	Copy constructor
	LARatesPCIntegral(const LARatesPCIntegral& v);
	// Destructor
	virtual ~LARatesPCIntegral();

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
								//======================================
								// excecute integral
	inline 	virtual void		integral(double ts, double te, 
										 std::vector<LAFunctionBase*>::const_iterator drift,										
										 std::vector<std::vector<LAFunctionBase*> >::const_iterator vol,
										 DoubleArray::const_iterator	bm,
										 SCALARARRAY::iterator	x_in_out,	
										 unsigned int varnum
										) const;
								
	
private:

	//
	double IntegralDrift(double ts,
						 double te,
						 std::vector<LAFunctionBase*>& drift_,
						 std::vector<std::vector<LAFunctionBase*> >& vol_,
						 const DoubleArray& mVar_,
						 size_t i) const;

	//
	double IntegralVol(double ts,
					   double te,
					   std::vector<LAFunctionBase*>& drift_,
					   std::vector<std::vector<LAFunctionBase*> >& vol_,
					   const DoubleArray& bm_,
					   const DoubleArray& mVar_,
					   size_t i) const;

	//
	double integral_helper(double x_, double deviation_) const
	{
		if (mIntegralType == NORMAL_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
		{
			return x_ + deviation_; 
		}
		else
		{
			 return x_ * LAMath::exp(deviation_);
		}
	}

	mutable DoubleArray			mVar;// tempolary variable for calculation
protected:
};

