#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LARatesBM.h"
#include "LARatesSDEBase.h"
#include "LARatesNumeraireBase.h"
#include "LAModelDynamicsCurve.h"
#include "AQLAlgorithm.h"


// ID for LARatesDmySDE
#define FN_DMYSDE	11701 


class LARatesDmySDE : public LARatesSDEBase
{
public:
//  LIFECYCLE
	// constructor
	explicit LARatesDmySDE();
	// destructor
	virtual ~LARatesDmySDE(void);
	// copy constructor
	LARatesDmySDE(const LARatesDmySDE &rhs);

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
	virtual const LARatesPathElementBase*
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
