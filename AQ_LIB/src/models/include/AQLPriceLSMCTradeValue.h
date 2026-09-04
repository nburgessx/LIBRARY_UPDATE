/*! @file
    @brief Class declaration to evaluate trade.
*/
#ifndef AQLPriceLSMCTradeValue_h
#define AQLPriceLSMCTradeValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceTradeValue.h"
#include "AQLFunctionBase.h"


// AQLPriceLSMCTradeValue's function id
#define FN_IR_LSMCTRADEVALUE				2013
// AQLPriceLSMCTradeValue's function name
#define FN_IR_LSMCTRADEVALUE_STR			"fn_ir_lsmctradevalue"

//// FROTOTYPE ////
class AQLPolyFitBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates portfolio

*/
class AQLPriceLSMCTradeValue : public AQLPriceTradeValue
{
public:
    // constructor	
	explicit AQLPriceLSMCTradeValue(AQLPolyFitBase* polyfit, AQLPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~AQLPriceLSMCTradeValue();
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
	AQLPriceLSMCTradeValue & operator=( const AQLPriceLSMCTradeValue & ) { return *this; }
protected:
    // copy constructor
    AQLPriceLSMCTradeValue(const AQLPriceLSMCTradeValue& v);	
	
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
	void	setStandardizationParam(AQLPriceTradeValueDataProvider &dataProvider, const BoolVector &isconvert_xy, const std::vector<DoubleArray>& standardization_y, const std::vector<DoubleMatrix>& standardization_x,
											const BoolVector &isconvert_xy_rebate, const std::vector<DoubleArray>& standardization_y_rebate, const std::vector<DoubleMatrix>& standardization_x_rebate) const;

	// set up dataProvider
//	AQLDataProvider*								setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
//												const AQLDataValuation& att) const;	

	/*!
		@brief cache class for performance up
	*/
/*	class AQLPriceLSMCTradeValueDataProvider : public AQLPriceTradeValueDataProvider
	{
	public:
		int								lsmcnum;		// lsmc num
		DoubleArray						expirytimes;	// call expiry times 
		DoubleArray						actiontimes;	// call action times
		AQLPolynomialBase*				poly;			// polynomial function
		AQLDataDoubleMatrix*				coefficient;	// coefficient
	};
*/
	class AQLPriceLSMCOperator : public AQLFunctionBase
	{
	public:
									//======================================
									// Make copy(clone) of this class
		virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
									//==========================================
									// return function value
		virtual double				operator()(const DoubleArray& x) const;		


		AQLPolynomialBase*			mPoly;		// polynomial function
	};

	mutable AQLPolyFitBase*	mpPolyFit;// polynomial fitting class

private:
	// create new cache class
//	virtual	AQLDataProvider*				createNewDataProvider() const;

};
#endif
