#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesNumeraireBase.h"


// ID for LARatesNumeraireDiscountBond
#define FN_NUMERAIREDISCOUNTBOND	2103 
// Function name for LARatesNumeraireDiscountBond
#define FN_NUMERAIREDISCOUNTBOND_STR	"fn_numeraire_discountbond"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of discount bond numeraire class

*/
class LARatesNumeraireDiscountBond : public LARatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesNumeraireDiscountBond(double T, bool isStochasticIR = true);
	//	Copy constructor
//	LARatesNumeraireDiscountBond(const LARatesNumeraireDiscountBond& v);
	// Destructor
	virtual ~LARatesNumeraireDiscountBond();

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
								// set terminal
	void						setTerminal(double T);
private:

protected:
	double						mTerminal;// terminal

};
