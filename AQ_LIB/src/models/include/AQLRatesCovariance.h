#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreTemplateType.h"
#include "AQLGaussLegendre.h"




class AQLFunctionBase;
class AQL1DDataSet;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of covariance class

*/
class AQLRatesCovariance
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesCovariance();
	// copy constructor
//	AQLRatesCovariance(const AQLRatesCovariance& v);
	// Destructor
	virtual ~AQLRatesCovariance();
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
    void						setVolaility(const std::vector<const AQLFunctionBase*>& vol) 
								{mVolatility = vol;}
								//======================================
								// set correlation
								/*!
									@param[in] cor correlation
									@note	this class is not pointer owner of input function
								*/
	void						setCorrelation(const std::vector<std::vector<const AQLFunctionBase*> >& cor)
								{mCorrelation = cor;}
								// calculate and store integral of covariance
	void						calcIntegratedCov(const DoubleArray& timegrid, const DoubleArray* pReset_timegrid);
private:
								// calculte integral of product of AQL1DDataSet functions
	double						integral(double t1, double t2, std::vector<const AQL1DDataSet*>& funcs) const;
protected:
	std::vector<const AQLFunctionBase*>	mVolatility;	// volatility
	std::vector<std::vector<const AQLFunctionBase*> >	mCorrelation;	// correlation
	DoubleArray									mTimeGrid;		// time grid
	std::vector<DoubleMatrix>					mIntegratedCovData;// calculated integral of covariance data
	AQLGaussLegendre								mGL;// integral method
	mutable DoubleArray							mXX;// tempolary variable
	mutable DoubleArray							mWW;// tempolary variable	
};
