#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSpotSDE.h"


// ID for AQLRatesLJSpotSDE
#define FN_LJSPOTSDE	1704 
// Function name for AQLRatesLJSpotSDE
#define FN_LJSPOTSDE_STR	"fn_lj_spot_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of long-jump version of spot sde class

*/
class AQLRatesLJSpotSDE : public AQLRatesSpotSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesLJSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
								AQLFunctionBase* pAdjuster = 0);
	//	Copy constructor
	AQLRatesLJSpotSDE(const AQLRatesLJSpotSDE& v);
	// Destructor
	virtual ~AQLRatesLJSpotSDE();
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
	virtual const	std::vector<AQLRatesPathElementBase*>&
								getPath(void);
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual AQLRatesNumeraireBase*	getNumeraire(void) {return mpNumeraire_LJ;}
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual const AQLRatesNumeraireBase*
								getNumeraire(void) const {return mpNumeraire_LJ;}
//  OPERATION
								//==========================================
	                            // set numeraire		
	virtual void				setNumeraire(AQLRatesNumeraireBase* pnumeraire);
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

	std::vector<AQLRatesPathElementBase*>		mPath_LJ;	// path of Long Jump
	double									mWt;		// W(t):Brownian Motion Value
	std::vector<std::pair<double, double> >	mIntegralRegion;// Variable for Integral Region
	AQLRatesNumeraireBase*						mpNumeraire_LJ;		// numeraire

};
