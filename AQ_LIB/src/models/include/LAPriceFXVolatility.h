#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"


// ID for LAPriceFXVolatility
#define FN_FXVOLATILITY	2303 
// Function name for LAPriceFXVolatility
#define FN_FXVOLATILITY_STR		"fn_fxvolatility"





///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of fx volatility function class
*/
class LAPriceFXVolatility : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceFXVolatility(AQLFunctionBase* pVol_fx_and_t, AQLFunctionBase* pVol_t, const DoubleArray& timegrid);
	//	Copy constructor
	LAPriceFXVolatility(const LAPriceFXVolatility& v);
	// Destructor
	virtual ~LAPriceFXVolatility();

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
								// Return volatility function of t part
	const AQLFunctionBase*		get_t_part() const {return mpVol_t;}
								//======================================
								// Return volatility function of fx and t part
	const AQLFunctionBase*		get_fx_and_t_part() const {return mpVol_fx_and_t;}
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;

//	OPERATION

private:
	                            //==========================================
	                            // set up this function
	void						setUp();
	                            //==========================================
	                            // Return integral result
	double						integral(double t1, double t2, AQL1DIntegral* pIntegral) const;

protected:
	AQLFunctionBase*				mpVol_t;// < volatility of t function part
	AQLFunctionBase*				mpVol_fx_and_t;// < volatility of fx rate function part
	DoubleArray					mTimeGrid;		// < time grid
	DoubleArray					mVolData;		// volatility data (t part)
	DoubleArray					mIntegratedVolData;	// integral of volatility^2(t part) data
	mutable	unsigned int		mPos_old;		// tempolary variable	

};
