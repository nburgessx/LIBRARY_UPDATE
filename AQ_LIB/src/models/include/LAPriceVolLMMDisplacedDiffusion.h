#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathVolFuncBase.h"


// ID for LAPriceVolLMMDisplacedDiffusion
#define FN_VOLDISPLACEDDIFFUSION	2302 
// Function name for LAPriceVolLMMDisplacedDiffusion
#define FN_VOLDISPLACEDDIFFUSION_STR	"fn_voldisplaceddiffusion"



class LAMathPathEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of volatility function class
*/
class LAPriceVolLMMDisplacedDiffusion : public LAMathVolFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceVolLMMDisplacedDiffusion(bool isMultiVariables = false);
	// constructor
	LAPriceVolLMMDisplacedDiffusion(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double beta, bool isMultiVariables = false);
	//	Copy constructor
//	LAPriceVolLMMDisplacedDiffusion(const LAPriceVolLMMDisplacedDiffusion& v);
	// Destructor
	virtual ~LAPriceVolLMMDisplacedDiffusion();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
								//==========================================
								// return string representaion
    virtual AQLString			convertToString(void) const;

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);
								//======================================
								// set up this class
								/*!
									@param[in] path path object 
								*/
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set beta
								/*!
									@param[in] beta beta 
								*/	
	void						setBeta(double beta) {mBeta = beta;}
								//======================================
								// set tenor
	void						setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor);

private:

protected:
	double						mBeta;// < beta
	double						mL0;// < initial value of L
	DoubleArray					mTenor;// < tenor
	DoubleArray					mDeltaTenor;// < delta tenor

};
