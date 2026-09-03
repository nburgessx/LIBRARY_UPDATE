#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAModelDynamicsBase.h"


// ID for LARatesPEInterpolationBase
#define FN_PEINTERPOLATIONBASE	1901 


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of path element interpolation class

*/
class LARatesPEInterpolationBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPEInterpolationBase();
	//	Copy constructor
	LARatesPEInterpolationBase(const LARatesPEInterpolationBase& v);
	// Destructor
	virtual ~LARatesPEInterpolationBase();

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
								// Return interpolated value
	virtual const LARatesPathElementBase&
								value(double t,	double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2) = 0;
								//======================================
								// initialize this class
	virtual void				init() {;}
// OPERATION

private:

protected:


};

