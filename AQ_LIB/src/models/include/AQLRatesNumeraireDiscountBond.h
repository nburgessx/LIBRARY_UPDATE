#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesNumeraireBase.h"


// ID for AQLRatesNumeraireDiscountBond
#define FN_NUMERAIREDISCOUNTBOND	2103 
// Function name for AQLRatesNumeraireDiscountBond
#define FN_NUMERAIREDISCOUNTBOND_STR	"fn_numeraire_discountbond"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of discount bond numeraire class

*/
class AQLRatesNumeraireDiscountBond : public AQLRatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesNumeraireDiscountBond(double T, bool isStochasticIR = true);
	//	Copy constructor
//	AQLRatesNumeraireDiscountBond(const AQLRatesNumeraireDiscountBond& v);
	// Destructor
	virtual ~AQLRatesNumeraireDiscountBond();

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
								// set terminal
	void						setTerminal(double T);
private:

protected:
	double						mTerminal;// terminal

};
