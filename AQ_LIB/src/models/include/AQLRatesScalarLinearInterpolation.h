#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsScalar.h"


// ID for AQLRatesScalarLinearInterpolation
#define FN_SCALARLINEARINTERPOLATION	1902 
// Function name for AQLRatesScalarLinearInterpolation
#define FN_SCALARLINEARINTERPOLATION_STR	"fn_scalar_linearinterpolation"






///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of linear interpolation of scalar path element
*/
class AQLRatesScalarLinearInterpolation : public AQLRatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesScalarLinearInterpolation();
	//	Copy constructor
	AQLRatesScalarLinearInterpolation(const AQLRatesScalarLinearInterpolation& v);
	// Destructor
	virtual ~AQLRatesScalarLinearInterpolation();

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
								// Return interpolated value
	virtual const AQLRatesPathElementBase&
								value(double t,	double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2);


// OPERATION

private:
	AQLRatesPathElementScalar		mValue;
protected:


};

