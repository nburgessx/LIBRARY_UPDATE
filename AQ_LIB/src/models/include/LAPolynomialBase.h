#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for LAPolynomialBase
#define FN_POLYNOMIALBASE	2401 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of polynomial class

*/
class LAPolynomialBase : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPolynomialBase(unsigned int order = 1, unsigned int varnum = 1, bool checkflag = false);
	//	Copy constructor
	LAPolynomialBase(const LAPolynomialBase& v);
	// Destructor
	virtual ~LAPolynomialBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const;
								//======================================
								// calculate basis function
								/*!
									@param[in] x variables
									@param[out] basis_func values of basis functions
								*/
	virtual void				calcBasisFunc(const DoubleArray& x, DoubleArray& basis_func) const = 0;										
								//======================================
								// set order of basis functinos
								/*!
									@param[in] num order of basis functinos
								*/
	void						setOrderBF(unsigned int num) {mOrderBF = num;}
								//======================================
								// set number of variables
								/*!
									@param[in] num number of variables
								*/
	void						setNumVar(unsigned int num) {mNumVar = num;} 
								//======================================
								// combination function(return the number of ways to uniquely choose n items from a set of size m)
	unsigned int				combin(unsigned int m, unsigned int n) const;
	//Remove warning:C4512
	LAPolynomialBase & operator=( const LAPolynomialBase & ) { return *this; }

								//======================================
								// calc transformed value
	void                       calcTransformedValue(const DoubleArray& y, const DoubleMatrix& x , bool isShift_Y, bool isScale_Y, bool isShift_X, bool isScale_X, DoubleArray& y_t, DoubleMatrix& x_t);
								//======================================
								// set standardization param
	void                       setStandardizationParam(const double shift_y, const double scale_y, const DoubleArray &shift_x, const DoubleArray &scale_x);
								//======================================
								// get standardization param
	void                       getStandardizationParam(double &shift_y, double &scale_y, DoubleArray &shift_x, DoubleArray &scale_x) const;
								//======================================
								// set convert flag
	void                       setConvertFlag(const bool isConvert) {mIsConvert = isConvert;}
								//======================================
								// isconvert
	bool                       isConvert() const {return mIsConvert;}


private:

protected:
	unsigned int mOrderBF;// number of basis functinos
	unsigned int mNumVar;// number of variables
	bool            mIsConvert;
	double		    mShift_Y;	// value of shift y value
	double		    mScale_Y;	// value of scale y value
	DoubleArray	    mShift_X;	// value array of shift x value
	DoubleArray	    mScale_X;	// value array of scale x value

};
