#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsBase.h"


// ID for AQLRatesSpotSDE
#define FN_SPOTSDE	1702 
// Function name for AQLRatesSpotSDE
#define FN_SPOTSDE_STR	"fn_spot_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of spot sde class

*/
class AQLRatesSpotSDE : public AQLRatesSDEBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
										AQLFunctionBase* pAdjuster = 0);
	//	Copy constructor
	AQLRatesSpotSDE(const AQLRatesSpotSDE& v);
	// Destructor
	virtual ~AQLRatesSpotSDE();
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
	virtual const AQLRatesPathElementBase*
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
	AQLRatesPathElementBase*		mpPathElement;		// current path element
	AQLFunctionBase*				mpTransformer;		// transform function
	AQLFunctionBase*				mpInvTransformer;	// inverse of transform function
	AQLFunctionBase*				mpAdjuster;	        // adjust function

};

