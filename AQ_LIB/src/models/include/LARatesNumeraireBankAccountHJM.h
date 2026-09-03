#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesNumeraireBase.h"
#include "LAInterpolationBase.h"
#include "LA1DDataSet.h"
#include "LABasic.h"


// ID for LARatesNumeraireBankAccountHJM
#define FN_NUMERAIREBANKACCOUNTHJM	2105
// Function name for LARatesNumeraireBankAccountHJM
#define FN_NUMERAIREBANKACCOUNTHJM_STR	"fn_numeraire_bankaccounthjm"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of bank account numeraire for HJM class
*/
class LARatesNumeraireBankAccountHJM : public LARatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesNumeraireBankAccountHJM(double T, bool isStochasticIR = true);
	//	Copy constructor
//	LARatesNumeraireBankAccountHJM(const LARatesNumeraireBankAccountHJM& v);
	// Destructor
	virtual ~LARatesNumeraireBankAccountHJM();

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
	void						setInterpolationMethod_hjm(LAInterpolationBase* pinter); 

//	OPERATION
private:
								//======================================
								// calc nuemraire	
	//void						calcNumeraire(void) const;

	//mutable bool	mUpdateFlag;		// update flag
	mutable	DoubleArray	mNumeraireArray;// numerarie values
	mutable	DoubleArray	mTimeGrid;		// time grid
	
	double						mTerminal;// terminal
	LAInterpolationBase*		mpInter_hjm;	// pointer to interpolation function
	LA1DDataSet					mfunc;

protected:

};







#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAShiftMethod
#define FN_CONTIBANKACCOUNTFUNC	2702
// Function Name of LAShiftMethod
#define FN_CONTIBANKACCOUNTFUNC_STR	"fn_contibankaccountfunc"


class LARatesContiBankAccountFunction : public LAFunctionBase
{
public:
	// Default constructor
	LARatesContiBankAccountFunction();
	// Destructor
	~LARatesContiBankAccountFunction();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	// 
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								}
	//
	LARatesContiBankAccountFunction & operator=( const LARatesContiBankAccountFunction & ) { return *this; }
	virtual void				setCurve(const LARatesPathElementCurve* curve);

private:
	const LARatesPathElementCurve* pCurve;
	

};

