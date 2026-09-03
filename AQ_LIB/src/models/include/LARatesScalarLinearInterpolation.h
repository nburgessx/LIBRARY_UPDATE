#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsScalar.h"


// ID for LARatesScalarLinearInterpolation
#define FN_SCALARLINEARINTERPOLATION	1902 
// Function name for LARatesScalarLinearInterpolation
#define FN_SCALARLINEARINTERPOLATION_STR	"fn_scalar_linearinterpolation"






///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of linear interpolation of scalar path element
*/
class LARatesScalarLinearInterpolation : public LARatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesScalarLinearInterpolation();
	//	Copy constructor
	LARatesScalarLinearInterpolation(const LARatesScalarLinearInterpolation& v);
	// Destructor
	virtual ~LARatesScalarLinearInterpolation();

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
	virtual const LARatesPathElementBase&
								value(double t,	double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2);


// OPERATION

private:
	LARatesPathElementScalar		mValue;
protected:


};

