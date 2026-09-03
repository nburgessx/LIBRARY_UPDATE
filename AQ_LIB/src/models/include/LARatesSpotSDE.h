#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEBase.h"
#include "LAModelDynamicsBase.h"


// ID for LARatesSpotSDE
#define FN_SPOTSDE	1702 
// Function name for LARatesSpotSDE
#define FN_SPOTSDE_STR	"fn_spot_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of spot sde class

*/
class LARatesSpotSDE : public LARatesSDEBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesSpotSDE(SDE_TYPE type, LAFunctionBase* pTransformer = 0, LAFunctionBase* pInvTransformer = 0,
										LAFunctionBase* pAdjuster = 0);
	//	Copy constructor
	LARatesSpotSDE(const LARatesSpotSDE& v);
	// Destructor
	virtual ~LARatesSpotSDE();
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
							    // get path element
	virtual const LARatesPathElementBase*
								getPathElement(unsigned int pos);
   								//==========================================
							    // get transform function
	const LAFunctionBase*		getTransfromer() const {return mpTransformer;}
   								//==========================================
							    // get inverse of transform function
	const LAFunctionBase*		getInvTransfromer() const {return mpInvTransformer;}
								//==========================================
								// set up this class for path calculation
	virtual	void				setUp();
								//==========================================
								// set adjuster
	void						setAdjuster(LAFunctionBase* p){mpAdjuster = p;}
private:

protected:
								//==========================================
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	SCALARARRAY	mVar;			// variable for integral
	LARatesPathElementBase*		mpPathElement;		// current path element
	LAFunctionBase*				mpTransformer;		// transform function
	LAFunctionBase*				mpInvTransformer;	// inverse of transform function
	LAFunctionBase*				mpAdjuster;	        // adjust function

};

