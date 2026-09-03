#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LACoreTemplateType.h"
#include "LAGaussLegendre.h"




class LAFunctionBase;
class LA1DDataSet;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of covariance class

*/
class LARatesCovariance
{
public:
//  LIFECYCLE
	// constructor
	LARatesCovariance();
	// copy constructor
//	LARatesCovariance(const LARatesCovariance& v);
	// Destructor
	virtual ~LARatesCovariance();
	//  QUERY
	// get covariance
	double						getCov (unsigned int i, unsigned int j, double t) const;	
	// get integral of covariance
	double						getIntegratedCov (unsigned int i, unsigned int j, unsigned int pos1, unsigned int pos2) const;
	// get integral of covariance
	double						getIntegratedCov (unsigned int i, unsigned int j, double t1, double t2) const;
	// get integral of covariance
	const std::vector<DoubleMatrix>&
								getIntegratedCovData () const {return mIntegratedCovData;}
								//======================================
								// set volatility
								/*!
									@param[in] vol volatility
									@note this class is not pointer owner of input function
								*/
    void						setVolaility(const std::vector<const LAFunctionBase*>& vol) 
								{mVolatility = vol;}
								//======================================
								// set correlation
								/*!
									@param[in] cor correlation
									@note	this class is not pointer owner of input function
								*/
	void						setCorrelation(const std::vector<std::vector<const LAFunctionBase*> >& cor)
								{mCorrelation = cor;}
								// calculate and store integral of covariance
	void						calcIntegratedCov(const DoubleArray& timegrid, const DoubleArray* pReset_timegrid);
private:
								// calculte integral of product of LA1DDataSet functions
	double						integral(double t1, double t2, std::vector<const LA1DDataSet*>& funcs) const;
protected:
	std::vector<const LAFunctionBase*>	mVolatility;	// volatility
	std::vector<std::vector<const LAFunctionBase*> >	mCorrelation;	// correlation
	DoubleArray									mTimeGrid;		// time grid
	std::vector<DoubleMatrix>					mIntegratedCovData;// calculated integral of covariance data
	LAGaussLegendre								mGL;// integral method
	mutable DoubleArray							mXX;// tempolary variable
	mutable DoubleArray							mWW;// tempolary variable	
};
