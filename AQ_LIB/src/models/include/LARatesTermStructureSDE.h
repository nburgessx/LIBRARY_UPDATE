#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEBase.h"
#include "LAModelDynamicsBase.h"
#include "AQLCoreTemplateType.h"


// ID for LARatesTermStructureSDE
#define FN_TERMSTRUCTURESDE	1703 
// Function name for LARatesTermStructureSDE
#define FN_TERMSTRUCTURESDE_STR	"fn_termstructure_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of term structure sde class

*/
class LARatesTermStructureSDE : public LARatesSDEBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesTermStructureSDE(SDE_TYPE type);
	//	Copy constructor
//	LARatesTermStructureSDE(const LARatesTermStructureSDE& v);
	// Destructor
	virtual ~LARatesTermStructureSDE();
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
//  OPERATION
//	void						setResetTimes(const DoubleArray& resettimes); 
								//==========================================
								// set up this class for path calculation
	virtual	void				setUp();
								// set up capratio for path calculation
	void						setCapRatio(double ratio){mCapRatio = ratio;}

private:

protected:
								//==========================================
	                            // calculate path
//	virtual	void				calcPath(void);
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	

	SCALARARRAY					mVar; // tempolary variable for calculation
	SCALARARRAY					mCAP; // mCap[i] = f(i) * CapRatio
	double						mCapRatio;// capratio for pathcalculation

};

