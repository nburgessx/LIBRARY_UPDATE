#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesNumeraireBase.h"


// ID for LARatesNumeraireAnnuity
#define FN_NUMERAIREANNUITY	2104 
// Function name for LARatesNumeraireAnnuity
#define FN_NUMERAIREANNUITY_STR	"fn_numeraire_annuity"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of annuity numeraire class
*/
class LARatesNumeraireAnnuity : public LARatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesNumeraireAnnuity(bool isStochasticIR = true);
	//	Copy constructor
//	LARatesNumeraireAnnuity(const LARatesNumeraireAnnuity& v);
	// Destructor
	virtual ~LARatesNumeraireAnnuity();

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
								//======================================
								// get numeraire value at t
	virtual double				operator()(double t) const;
//	OPERATION
								//======================================
								// set annuity info(grid, term)
	void						setAnnuityInfo(const DoubleArray& grid, const DoubleArray& term);
private:

protected:
	DoubleArray		mTerms;		// term array
	DoubleArray		mTimeGrid;	// time grid

};

