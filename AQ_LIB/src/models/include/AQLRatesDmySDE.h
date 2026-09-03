#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLRatesBM.h"
#include "AQLRatesSDEBase.h"
#include "AQLRatesNumeraireBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLAlgorithm.h"


// ID for AQLRatesDmySDE
#define FN_DMYSDE	11701 


class AQLRatesDmySDE : public AQLRatesSDEBase
{
public:
//  LIFECYCLE
	// constructor
	explicit AQLRatesDmySDE();
	// destructor
	virtual ~AQLRatesDmySDE(void);
	// copy constructor
	AQLRatesDmySDE(const AQLRatesDmySDE &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;
  								//==========================================
							    // get path element
	virtual const AQLRatesPathElementBase*
								getPathElement(unsigned int pos);

	                            //==========================================
	                            // set up for mc
	virtual	void				setUp();

protected:
	                            // calculate path
								/*!
							
									@param[in] pos position of sde integral grid				
								*/	
	virtual void				calcPath(unsigned int pos);

};
