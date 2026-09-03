#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesNumeraireBase.h"


// ID for AQLRatesNumeraireAnnuity
#define FN_NUMERAIREANNUITY	2104 
// Function name for AQLRatesNumeraireAnnuity
#define FN_NUMERAIREANNUITY_STR	"fn_numeraire_annuity"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of annuity numeraire class
*/
class AQLRatesNumeraireAnnuity : public AQLRatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesNumeraireAnnuity(bool isStochasticIR = true);
	//	Copy constructor
//	AQLRatesNumeraireAnnuity(const AQLRatesNumeraireAnnuity& v);
	// Destructor
	virtual ~AQLRatesNumeraireAnnuity();

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

