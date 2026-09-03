#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesTermStructureSDE.h"


// ID for AQLRatesLJTermStructureSDE
#define FN_LJTERMSTRUCTURESDE	1705 
// Function name for AQLRatesLJTermStructureSDE
#define FN_LJTERMSTRUCTURESDE_STR	"fn_lj_termstructure_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of long-jump version of term structure sde class

*/
class AQLRatesLJTermStructureSDE : public AQLRatesTermStructureSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesLJTermStructureSDE(SDE_TYPE type);
	//	Copy constructor
	AQLRatesLJTermStructureSDE(const AQLRatesLJTermStructureSDE& v);
	// Destructor
	virtual ~AQLRatesLJTermStructureSDE();
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
								//==========================================
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	std::vector<AQLRatesPathElementBase*>		mPath_LJ;	// path of Long Jump
	DoubleArray								mWt;		// W(t):Brownian Motion Value	
	SCALARARRAY								mVar_LJ;	// tempolary variable for calculation
	std::vector<std::pair<double, double> >	mIntegralRegion;// Variable for Integral Region
	AQLRatesNumeraireBase*						mpNumeraire_LJ;	// numeraire
	
};
