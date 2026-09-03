#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLRatesCovariance.h"
#include "AQLCoreTemplateType.h"

// ID for AQLPriceDriftSMMBase
#define FN_DRIFTSMMBASE	2210


class AQLMathPathEntity;
class AQLRatesNumeraireBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstruct base class of drift function of SMM sde 

*/
class AQLPriceDriftSMMBase : public AQLMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPriceDriftSMMBase(double s = 0.0);
	// constructor
	AQLPriceDriftSMMBase(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double s = 0.0);	
	//	Copy constructor
	AQLPriceDriftSMMBase(const AQLPriceDriftSMMBase& v);
	// Destructor
	virtual ~AQLPriceDriftSMMBase();

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
	virtual void				setUp(AQLMathPathEntity& path);
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
//	void						setCovariance(AQLRatesCovariance* pcov) {mpCovariance = pcov;}
								// set up covariance
//	void						setUpCovariance() const; 
private:
protected:
	virtual inline	double		getLpart(double x, double delta) const
								{
									double deltaL = delta * x;
									return deltaL / (1.0 + deltaL - mSpread * delta);	
								}
	virtual inline	double		get_s(int m_i, int j, DoubleArray delta_tenor, DoubleArray x) const
								{
									double s = 0.0;
									double _s = 0.0;
									int k = x.size() - 1;
									//for (int n = delta_tenor.size() - 1; n >= m_i; n--, k--)
									for (int n = delta_tenor.size() - 1; n >= j; n--, k--)
									{
										s = delta_tenor[n];
										for (int l = n; l >= m_i + 1; l--)
											s *= 1 + delta_tenor[l] * x[k + l - n];
										_s += s;
									}
									return _s;
								}
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
	double										mSpread;		// spread added to L(for displace diffusion case)
};

