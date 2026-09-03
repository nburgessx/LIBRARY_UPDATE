#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLRatesPCIntegral
#define FN_PCINTEGRAL	10802
// Function name for AQLRatesPCIntegral
#define FN_PCINTEGRAL_STR	"fn_pcintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class AQLRatesPCIntegral : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesPCIntegral(SDEINTEGRAL_TYPE type);
	//	Copy constructor
	AQLRatesPCIntegral(const AQLRatesPCIntegral& v);
	// Destructor
	virtual ~AQLRatesPCIntegral();

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
								//======================================
								// excecute integral
	inline 	virtual void		integral(double ts, double te, 
										 std::vector<AQLFunctionBase*>::const_iterator drift,										
										 std::vector<std::vector<AQLFunctionBase*> >::const_iterator vol,
										 DoubleArray::const_iterator	bm,
										 SCALARARRAY::iterator	x_in_out,	
										 unsigned int varnum
										) const;
								
	
private:

	//
	double IntegralDrift(double ts,
						 double te,
						 std::vector<AQLFunctionBase*>& drift_,
						 std::vector<std::vector<AQLFunctionBase*> >& vol_,
						 const DoubleArray& mVar_,
						 size_t i) const;

	//
	double IntegralVol(double ts,
					   double te,
					   std::vector<AQLFunctionBase*>& drift_,
					   std::vector<std::vector<AQLFunctionBase*> >& vol_,
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
			 return x_ * AQLMath::exp(deviation_);
		}
	}

	mutable DoubleArray			mVar;// tempolary variable for calculation
protected:
};

