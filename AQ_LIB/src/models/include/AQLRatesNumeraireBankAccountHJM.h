#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesNumeraireBase.h"
#include "AQLInterpolationBase.h"
#include "AQL1DDataSet.h"
#include "AQLBasic.h"


// ID for AQLRatesNumeraireBankAccountHJM
#define FN_NUMERAIREBANKACCOUNTHJM	2105
// Function name for AQLRatesNumeraireBankAccountHJM
#define FN_NUMERAIREBANKACCOUNTHJM_STR	"fn_numeraire_bankaccounthjm"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of bank account numeraire for HJM class
*/
class AQLRatesNumeraireBankAccountHJM : public AQLRatesNumeraireBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesNumeraireBankAccountHJM(double T, bool isStochasticIR = true);
	//	Copy constructor
//	AQLRatesNumeraireBankAccountHJM(const AQLRatesNumeraireBankAccountHJM& v);
	// Destructor
	virtual ~AQLRatesNumeraireBankAccountHJM();

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
								//======================================
								// set curve
	virtual void				setCurve(double t, const AQLRatesPathElementCurve* pcurve);
								//======================================
								// set terminal	
	void						setTerminal(double T);
	void						setInterpolationMethod_hjm(AQLInterpolationBase* pinter); 

//	OPERATION
private:
								//======================================
								// calc nuemraire	
	//void						calcNumeraire(void) const;

	//mutable bool	mUpdateFlag;		// update flag
	mutable	DoubleArray	mNumeraireArray;// numerarie values
	mutable	DoubleArray	mTimeGrid;		// time grid
	
	double						mTerminal;// terminal
	AQLInterpolationBase*		mpInter_hjm;	// pointer to interpolation function
	AQL1DDataSet					mfunc;

protected:

};







#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLShiftMethod
#define FN_CONTIBANKACCOUNTFUNC	2702
// Function Name of AQLShiftMethod
#define FN_CONTIBANKACCOUNTFUNC_STR	"fn_contibankaccountfunc"


class AQLRatesContiBankAccountFunction : public AQLFunctionBase
{
public:
	// Default constructor
	AQLRatesContiBankAccountFunction();
	// Destructor
	~AQLRatesContiBankAccountFunction();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	// 
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								}
	//
	AQLRatesContiBankAccountFunction & operator=( const AQLRatesContiBankAccountFunction & ) { return *this; }
	virtual void				setCurve(const AQLRatesPathElementCurve* curve);

private:
	const AQLRatesPathElementCurve* pCurve;
	

};

