#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// ID for LAPriceDriftHJM
#define FN_DRIFTHJM	2212
// Function name for LAPriceDriftHJM
#define FN_DRIFTHJM_STR	"fn_drift_hjm"


class LAMathPathEntity;
class LARatesNumeraireBase;
class LA1DDataSet;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of HJM sde

*/
class LAPriceDriftHJM : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceDriftHJM();
	// constructor
	LAPriceDriftHJM(const LAString& sdeAttrName, double T, const DoubleArray& grid_T);	
	//	Copy constructor
	LAPriceDriftHJM(const LAPriceDriftHJM& v);
	// Destructor
	virtual ~LAPriceDriftHJM();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
								//==========================================
	                            // Return drift value
	virtual double				operator()(double x) const;

								// return string representaion
    virtual LAString			convertToString(void) const;

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const LAString& str);

								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set volatility
								/*!
									@param[in] vol volatility
									@note this class is not pointer owner of input function
								*/
    void						setVolaility(const std::vector<std::vector<LAFunctionBase*> >& vol) 
								{mVolatility = vol;}
								//======================================
								// set correlation
								/*!
									@param[in] cor correlation
									@note	this class is pointer owner of input function for m_i = 0
											this class is not pointer owner of input function for m_i > 0								*/
	void						setCorrelation(const std::vector<std::vector<LAFunctionBase*> >& cor)
								{mCorrelation = cor;}
								//======================================
								// set tenor
//	void						setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor);

								//======================================
								// set T
								/*!
									@param[in] T maturity
								*/
	void						setT(double T) {mT = T;}
	
private:
								// get integral part
	double						getIntegratedpart (unsigned int i, unsigned int j,
												   double t1, double t2, unsigned int pos) const;
								// calculate integral of product of LA1DDataSet functions
	double						integral(double t1, double t2, std::vector<const LA1DDataSet*>& funcs) const;
								// calculate drift
	double						calcDrift(double x) const;

protected:
	//DoubleArray								mTenor;			// tenor
	//DoubleArray								mDeltaTenor;	// delta tenor
	//unsigned int								m_i;			// suffix
	unsigned int								mFactorNum;		// foctor number
	double										mT;				// T
	DoubleArray									mgrid_T;		// grid T
	std::vector<std::vector<LAFunctionBase*> >	mVolatility;	// volatility
	std::vector<std::vector<LAFunctionBase*> >	mCorrelation;	// correlation
	LAString									mSDEAttrName;	// data name of ir sde
	const DoubleArray*							mpTimes;		// correlation data time
	DoubleArray									mDrift;			// drift data
	//DoubleMatrix								mCorData;		// correlation data
	mutable	unsigned int						mPos_old;		// tempolary variable	
	//mutable DoubleArray*						mpCache;		// cache
};
