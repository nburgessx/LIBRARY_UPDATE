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
	explicit LARatesSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
										AQLFunctionBase* pAdjuster = 0);
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
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
   								//==========================================
							    // get path element
	virtual const LARatesPathElementBase*
								getPathElement(unsigned int pos);
   								//==========================================
							    // get transform function
	const AQLFunctionBase*		getTransfromer() const {return mpTransformer;}
   								//==========================================
							    // get inverse of transform function
	const AQLFunctionBase*		getInvTransfromer() const {return mpInvTransformer;}
								//==========================================
								// set up this class for path calculation
	virtual	void				setUp();
								//==========================================
								// set adjuster
	void						setAdjuster(AQLFunctionBase* p){mpAdjuster = p;}
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
	AQLFunctionBase*				mpTransformer;		// transform function
	AQLFunctionBase*				mpInvTransformer;	// inverse of transform function
	AQLFunctionBase*				mpAdjuster;	        // adjust function

};

