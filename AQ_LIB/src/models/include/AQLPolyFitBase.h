#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLPolyFitBase
#define FN_POLYFITBASE	2501 

class AQLPolynomialBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of polynomial fitting class

*/
class AQLPolyFitBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPolyFitBase();
	//	Copy constructor
//	AQLPolyFitBase(const AQLPolyFitBase& v);
	// Destructor
	virtual ~AQLPolyFitBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// execute regression
								/*!
									@param[in] y explained variables
									@param[in] x explanatory variables
									@param[in, out] poly polynomial
									@param[out] y transformed variables
									@param[out] x transformed variables
								*/
    virtual void				regress(const DoubleArray& y, const DoubleMatrix& x, AQLPolynomialBase& poly, DoubleArray *p_y_t = 0, DoubleMatrix *p_x_t = 0) const = 0;

private:

protected:


};
