#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesTermStructureSDE.h"


// ID for LARatesLJTermStructureSDE
#define FN_LJTERMSTRUCTURESDE	1705 
// Function name for LARatesLJTermStructureSDE
#define FN_LJTERMSTRUCTURESDE_STR	"fn_lj_termstructure_sde"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of long-jump version of term structure sde class

*/
class LARatesLJTermStructureSDE : public LARatesTermStructureSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesLJTermStructureSDE(SDE_TYPE type);
	//	Copy constructor
	LARatesLJTermStructureSDE(const LARatesLJTermStructureSDE& v);
	// Destructor
	virtual ~LARatesLJTermStructureSDE();
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
								//==========================================
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	std::vector<LARatesPathElementBase*>		mPath_LJ;	// path of Long Jump
	DoubleArray								mWt;		// W(t):Brownian Motion Value	
	SCALARARRAY								mVar_LJ;	// tempolary variable for calculation
	std::vector<std::pair<double, double> >	mIntegralRegion;// Variable for Integral Region
	LARatesNumeraireBase*						mpNumeraire_LJ;	// numeraire
	
};
