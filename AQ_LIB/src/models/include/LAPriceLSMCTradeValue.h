/*! @file
    @brief Class declaration to evaluate trade.

*/
//  2007, AlgoQuantHub..
#ifndef LAPriceLSMCTradeValue_h
#define LAPriceLSMCTradeValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceTradeValue.h"
#include "AQLFunctionBase.h"


// LAPriceLSMCTradeValue's function id
#define FN_IR_LSMCTRADEVALUE				2013
// LAPriceLSMCTradeValue's function name
#define FN_IR_LSMCTRADEVALUE_STR			"fn_ir_lsmctradevalue"

//// FROTOTYPE ////
class LAPolyFitBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates portfolio

*/
class LAPriceLSMCTradeValue : public LAPriceTradeValue
{
public:
    // constructor	
	explicit LAPriceLSMCTradeValue(LAPolyFitBase* polyfit, LAPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~LAPriceLSMCTradeValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object,
									const AQLDataValuation& att) const;

	//Remove warning:C4512
	LAPriceLSMCTradeValue & operator=( const LAPriceLSMCTradeValue & ) { return *this; }
protected:
    // copy constructor
    LAPriceLSMCTradeValue(const LAPriceLSMCTradeValue& v);	
	
	// value by backward
	virtual double				value_backward(const std::vector<DoubleMatrix>& time,
											const std::vector<DoubleMatrix>& cf,
											const std::vector<DoubleMatrix>& numeraire_cftime,
											const DoubleMatrix& rebate,
											const DoubleMatrix& numeraire_expirytime,
											const std::vector<DoubleMatrix>& explanatory,
											const DoubleArray& numeraire_base,
											const AQLDataProvider* dataProvider,
											DoubleMatrix& coefficient,
											DoubleMatrix& coefficient_rebate,
											BoolVector &isconvert_xy,
											std::vector<DoubleArray>& standardization_y,
											std::vector<DoubleMatrix>& standardization_x,
											BoolVector &isconvert_xy_rebate,
											std::vector<DoubleArray>& standardization_y_rebate,
											std::vector<DoubleMatrix>& standardization_x_rebate,
											IntArray& calltiming,
											bool isstartable,
											bool iscalcrisk = true,											
											DoubleArray* pvs = 0,
											BoolVector* afterjudge = 0,
											BoolVector* judge = 0,
											DoubleMatrix* lsmcparam = 0,
											DoubleMatrix* explainedvar = 0,
											DoubleMatrix* lsmcparam_rebate = 0,
											DoubleMatrix* transformed_explainedvar = 0,
											std::vector<DoubleMatrix>* transformed_explanatory = 0
											) const;

	// set standardization param
	void	setStandardizationParam(LAPriceTradeValueDataProvider &dataProvider, const BoolVector &isconvert_xy, const std::vector<DoubleArray>& standardization_y, const std::vector<DoubleMatrix>& standardization_x,
											const BoolVector &isconvert_xy_rebate, const std::vector<DoubleArray>& standardization_y_rebate, const std::vector<DoubleMatrix>& standardization_x_rebate) const;

	// set up dataProvider
//	AQLDataProvider*								setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
//												const AQLDataValuation& att) const;	

	/*!
		@brief cache class for performance up
	*/
/*	class LAPriceLSMCTradeValueDataProvider : public LAPriceTradeValueDataProvider
	{
	public:
		int								lsmcnum;		// lsmc num
		DoubleArray						expirytimes;	// call expiry times 
		DoubleArray						actiontimes;	// call action times
		LAPolynomialBase*				poly;			// polynomial function
		AQLDataDoubleMatrix*				coefficient;	// coefficient
	};
*/
	class LAPriceLSMCOperator : public AQLFunctionBase
	{
	public:
									//======================================
									// Make copy(clone) of this class
		virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
									//==========================================
									// return function value
		virtual double				operator()(const DoubleArray& x) const;		


		LAPolynomialBase*			mPoly;		// polynomial function
	};

	mutable LAPolyFitBase*	mpPolyFit;// polynomial fitting class

private:
	// create new cache class
//	virtual	AQLDataProvider*				createNewDataProvider() const;

};
#endif
