/*! @file
    @brief Source code of Euler-Maruyama sde integral class



*/
//  2007, AlgoQuantHub.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesPCIntegral.h"
#include "AQLBasic.h"
#include "AQLPriceDriftFX.h"
#include "AQLFunctionBase.h"

using namespace std;
//================ AQLRatesPCIntegral ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
AQLRatesPCIntegral::AQLRatesPCIntegral(SDEINTEGRAL_TYPE type)
: AQLRatesSDEIntegralBase(type)
{

}
/*!
	@brief copy constructor
*/
AQLRatesPCIntegral::AQLRatesPCIntegral(const AQLRatesPCIntegral& v) 
: AQLRatesSDEIntegralBase(v)
{

}

/*!
	@brief destructor
*/
AQLRatesPCIntegral::~AQLRatesPCIntegral() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesPCIntegral::clone() const
{
    try 
	{
		return new AQLRatesPCIntegral(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPCIntegral::isTypeOf(function_t id) const
{
	return (id==FN_PCINTEGRAL ? true : AQLRatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesPCIntegral::getType() const
{
	return FN_PCINTEGRAL;
}

/*!
    @brief excecute integral
    @param[in] ts starttime
    @param[in] te endtime
    @param[in] drift drift
    @param[in] vol volatility
    @param[in] bm brownian motion
    @param[in, out] x_in_out input and output 
    @param[in] varnum number of input(output)
*/
void
AQLRatesPCIntegral::integral(double ts, double te, 
							vector<AQLFunctionBase*>::const_iterator drift,										
							vector<vector<AQLFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
	if ((mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT) 
		&& vol->size() > 1)
	{
		//error
		throw AQLCoreInvalidData("Multi volatility is not support when SDE IntegralType is LOG", __FILE__, __LINE__);
	}

	// setup
	SCALARARRAY::const_iterator it = x_in_out;

	mVar.resize(varnum + 1); mVar[0] = ts;
	//vector<double> Var2(varnum + 1); Var2[0] = ts;

	vector<AQLFunctionBase*> drift_(varnum);
	vector<vector<AQLFunctionBase*> > vol_(varnum);
	vector<double> bm_(varnum);

	for (unsigned int i = 0 ; i < varnum; i++)
	{
		// drift
		drift_[i] = *(drift+i);
		bm_[i] = *(bm+i);

		// volatility
		size_t num_vol = (*vol).size();
		vol_[i].resize(num_vol);
		for(size_t j = 0; j < num_vol; j++)
		{
			vol_[i][j] = (*(vol+i))[j];
		}

		//
		mVar[i + 1] = (*it++);
		//Var2[i + 1] = mVar[i + 1];
	}

	if ((*drift)->isTypeOf(FN_DRIFTFX)) mVar.push_back(te);

	////stochastic integral by Eular integral
	//vector<double> integral_drift_Eular(varnum);
	//vector<double> integral_vol(varnum);
	//for (unsigned int i = 0 ; i < varnum; i++)
	//{
	//	integral_drift_Eular[i] = IntegralDrift(ts,te,drift_,vol_,mVar,i);
	//	integral_vol[i] = IntegralVol(ts,te,drift_,vol_,bm_,mVar,i);

	//	double tmp = integral_drift_Eular[i] + integral_vol[i];

	//	Var2[i+1] = integral_helper(Var2[i+1],tmp);
	//}

	//// stochastic integral by PC(predictor corrector) integral
	//for (unsigned int i = 0 ; i < varnum; i++)
	//{
	//	double tmp = 0.5 * (integral_drift_Eular[i] + IntegralDrift(ts,te,drift_,vol_,Var2,i)) + integral_vol[i];
	//	
	//	(*x_in_out) = integral_helper((*x_in_out),tmp);
	//	x_in_out++;
	//}

	//stochastic integral by PC(predictor corrector) integral
	vector<double> integral_drift_Eular(varnum);
	vector<double> integral_vol(varnum);
	for (unsigned int i = 0 ; i < varnum; i++)
	{
		// stochastic integral by Eular
		integral_drift_Eular[i] = IntegralDrift(ts,te,drift_,vol_,mVar,i);
		integral_vol[i] = IntegralVol(ts,te,drift_,vol_,bm_,mVar,i);

		double tmp = integral_drift_Eular[i] + integral_vol[i];

		// stochastic integral by PC
		mVar[i+1] = integral_helper(mVar[i+1],tmp);
		tmp = 0.5 * (integral_drift_Eular[i] + IntegralDrift(ts,te,drift_,vol_,mVar,i)) + integral_vol[i];

		(*x_in_out) = integral_helper((*x_in_out),tmp);
		x_in_out++;
	}
}

/*!
    @brief caluculate drift
    @param[in] vector of time and underlying stream
*/
double
AQLRatesPCIntegral::IntegralDrift(double ts,
							  double te,
							  std::vector<AQLFunctionBase*>& drift_,
							  std::vector<std::vector<AQLFunctionBase*> >& vol_,
							  const DoubleArray& mVar_,
							  size_t i) const
{
		double tmp = 0.;
		if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
			tmp += drift_[i]->operator()(mVar_) * (te - ts);	
		else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
			tmp += drift_[i]->operator()(mVar_) * mVar_[i + 1] * (te - ts);	
		else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL)
		{
			double v = vol_[i][0]->operator()(mVar_) / mVar_[i + 1];
			tmp += (drift_[i]->operator()(mVar_) / mVar_[i + 1] - 0.5 * v * v) * (te - ts);		
		}
		else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
		{
			double v = vol_[i][0]->operator()(mVar_) / AQLMath::exp(mVar_[i + 1]);
			tmp += (drift_[i]->operator()(mVar_) / AQLMath::exp(mVar_[i + 1]) - 0.5 * v * v) * (te - ts);		
		}
		else if (mSdeType == DIVIDEdXbyX && (mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT))
		{
			double v = vol_[i][0]->operator()(mVar_);
			tmp += (drift_[i]->operator()(mVar_) - 0.5 * v * v) * (te - ts);
		}
		return tmp;
}

/*!
    @brief caluculate volatility
    @param[in] vector of time and underlying stream
*/
double
AQLRatesPCIntegral::IntegralVol(double ts,
							double te,
							std::vector<AQLFunctionBase*>& drift_,
							std::vector<std::vector<AQLFunctionBase*> >& vol_,
							const DoubleArray& bm_,
							const DoubleArray& mVar_,
							size_t i) const
{
		double tmp = 0.;
		if (vol_[i].size() == 1)
		{
			if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
				tmp += vol_[i][0]->operator()(mVar_) * bm_[i];	
			else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
				tmp += vol_[i][0]->operator()(mVar_) * mVar_[i + 1] * bm_[i];		
			else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL)
				tmp += vol_[i][0]->operator()(mVar_) / mVar_[i + 1] * bm_[i];
			else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
				tmp += vol_[i][0]->operator()(mVar_) / AQLMath::exp(mVar_[i + 1]) * bm_[i];
			else if (mSdeType == DIVIDEdXbyX && (mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT))
				tmp += vol_[i][0]->operator()(mVar_) * bm_[i];	
		}
		else 
		{
			for (unsigned int j = 0 ; j < vol_[i].size(); j++)
			{
				if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
					tmp += vol_[i][j]->operator()(mVar_) * bm_[i];
				else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
					tmp += vol_[i][j]->operator()(mVar_) * mVar_[i + 1] * bm_[i];
			}
		}
		return tmp;
}