#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "LARatesCovariance.h"
#include "AQLCoreTemplateType.h"

// ID for LAPriceDriftLMMBase
#define FN_DRIFTLMMBASE	2200


class LAMathPathEntity;
class LARatesNumeraireBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstruct base class of drift function of LMM sde

*/
class LAPriceDriftLMMBase : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftLMMBase(double Q = 1.0);
	// constructor
	LAPriceDriftLMMBase(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q = 1.0);	
	//	Copy constructor
	LAPriceDriftLMMBase(const LAPriceDriftLMMBase& v);
	// Destructor
	virtual ~LAPriceDriftLMMBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//==========================================
								// return string representaion
    virtual AQLString			convertToString(void) const;
//	OPERATION
								//==========================================
								// transform from string representaion
    virtual void				convertFromString(const AQLString& str);
								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set volatility
								/*!
									@param[in] vol volatility
									@note this class is not pointer owner of input function
								*/
    void						setVolaility(const std::vector<std::vector<AQLFunctionBase*> > & vol) 
								{mVolatility = vol;}
								//======================================
								// set correlation
								/*!
									@param[in] cor correlation
									@note	this class is pointer owner of input function for m_i = 0
											this class is not pointer owner of input function for m_i > 0
								*/
	void						setCorrelation(const std::vector<std::vector<AQLFunctionBase*> >& cor)
								{mCorrelation = cor;}
								//======================================
								// set tenor
	virtual void				setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor);

								//======================================
								// set suffix
								/*!
									@param[in] i suffix
								*/
	void						setSuffix(unsigned int i) {m_i = i;}
								//======================================
								// set covariance
								/*!
									@param[in] pcov covariance
									@note	this class is pointer owner of input function for m_i = 0
											this class is not pointer owner of input function for m_i > 0
								*/
	void						setCovariance(LARatesCovariance* pcov) {mpCovariance = pcov;}
								// set up covariance
	void						setUpCovariance() const;
								// set logQ
	void						setLogQ(double logq){mLogQ = logq;};
private:
protected:
	virtual inline	double		getLpart(double x, double delta) const
								{
									double deltaL = delta * x;
									return deltaL / (1.0 + deltaL - mSpread * delta);	
								}
	double										mLogQ;			// LogQ for mSpread;
	double										mL_i;			// initialLibor for mSpread;
	DoubleArray									mTenor;			// tenor
	DoubleArray									mDeltaTenor;	// delta tenor
	unsigned int								m_i;			// suffix
	std::vector<std::vector<AQLFunctionBase*> >	mVolatility;	// volatility
	std::vector<std::vector<AQLFunctionBase*> >	mCorrelation;	// correlation
	AQLString									mSDEAttrName;	// data name of ir sde
	const DoubleArray*							mpTimes;		// correlation data time
	DoubleMatrix								mCorData;		// correlation data
	mutable	unsigned int						mPos_old;		// tempolary variable	
	mutable DoubleArray*						mpCache;		// cache 
	mutable DoubleArray*						mpCache2;		// cache 
	mutable DoubleArray*						mpCache_before;	// cache
	mutable double*								mpCacheTime;	// time of cache data
	mutable double*								mpCacheTime_before;// time of cache data
	LARatesCovariance*								mpCovariance;	// covariance class
	double										mSpread;		// spread added to L(for displace diffusion case)
	mutable bool								mIsCovSetuped;	// covariance is setuped or not
	mutable DoubleArray							mX;				// tempolary variable
};

