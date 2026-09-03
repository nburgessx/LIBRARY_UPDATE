#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesNumeraireBase.h"
#include <map>


// ID for LARatesNumeraireBankAccount
#define FN_NUMERAIREBANKACCOUNT	2102 
// Function name for LARatesNumeraireBankAccount
#define FN_NUMERAIREBANKACCOUNT_STR	"fn_numeraire_bankaccount"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of bank account numeraire class
*/
class LARatesNumeraireBankAccount : public LARatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesNumeraireBankAccount(double T, bool isStochasticIR = true);
	//	Copy constructor
//	LARatesNumeraireBankAccount(const LARatesNumeraireBankAccount& v);
	// Destructor
	virtual ~LARatesNumeraireBankAccount();

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
								//======================================
								// set curve
	virtual void				setCurve(double t, const LARatesPathElementCurve* pcurve);
								//======================================
								// set terminal	
	void						setTerminal(double T);
								//======================================
								// set df ratio for numeraire	
	void						setDFRatioForNumeraire() const;
								//======================================
								// set df ratio
	void						setDFRatio(double t) const;
 
	void						setTimeGrid(const DoubleArray &timeGrid) const;

//	OPERATION
private:
								//======================================
								// calc nuemraire	
	void						calcNumeraire(void) const;

	//mutable bool	mUpdateFlag;		// update flag
	mutable	DoubleArray	mNumeraireArray;// numerarie values
	mutable	DoubleArray	mTimeGrid;		// time grid
	mutable DoubleArray	mDFRatioNumeArray;   // df ratio for numeraire
	mutable	std::map<double, double>	mDFRatio;   // df ratio 

	
	double						mTerminal;// terminal

protected:

};

