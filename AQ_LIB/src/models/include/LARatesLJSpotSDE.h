#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSpotSDE.h"


// ID for LARatesLJSpotSDE
#define FN_LJSPOTSDE	1704 
// Function name for LARatesLJSpotSDE
#define FN_LJSPOTSDE_STR	"fn_lj_spot_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of long-jump version of spot sde class

*/
class LARatesLJSpotSDE : public LARatesSpotSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesLJSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
								AQLFunctionBase* pAdjuster = 0);
	//	Copy constructor
	LARatesLJSpotSDE(const LARatesLJSpotSDE& v);
	// Destructor
	virtual ~LARatesLJSpotSDE();
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
	                            // get path
	virtual const	std::vector<LARatesPathElementBase*>&
								getPath(void);
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual LARatesNumeraireBase*	getNumeraire(void) {return mpNumeraire_LJ;}
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual const LARatesNumeraireBase*
								getNumeraire(void) const {return mpNumeraire_LJ;}
//  OPERATION
								//==========================================
	                            // set numeraire		
	virtual void				setNumeraire(LARatesNumeraireBase* pnumeraire);
								//==========================================
								// set up this class for path calculation
	virtual	void				setUp();
private:

protected:

	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	std::vector<LARatesPathElementBase*>		mPath_LJ;	// path of Long Jump
	double									mWt;		// W(t):Brownian Motion Value
	std::vector<std::pair<double, double> >	mIntegralRegion;// Variable for Integral Region
	LARatesNumeraireBase*						mpNumeraire_LJ;		// numeraire

};
