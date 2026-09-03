/*! @file
    @brief Declaration of drift function of SZDD volatility factor (similar to HW sde with spot measure)



*/
//  2016, AlgoQuantHub SC Co., Ltd.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPriceDriftSZDDVolFactor.cpp
//
//  SYNOPSIS    :       AQLPriceDriftSZDDVolFactor
//  DESCRIPTION :       Source code of drift function of HW sde
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ INCLUDE +++++
#include "AQLPriceDriftSZDDVolFactor.h"
#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathAttrSDE.h"
#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLMathCorrelation.h"
#include "AQLMathVolFuncBase.h"
#include "AQLAlgorithm.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLMathHWFuncSigma.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceDriftQuantAdjustment.h"
#include "AQLCombinationFunc.h"
#include <algorithm>
#include "AQLMathVolFuncSZDDVolFactor.h"
#include "AQLStepInterpolation.h"

using namespace std;
#define COR "COR"
//
//------------------------------ AQLPriceDriftSZDDVolFactor ------------------------------
//

/*!
	@brief default constructor
	
*/
AQLPriceDriftSZDDVolFactor::AQLPriceDriftSZDDVolFactor()
: 
AQLPriceDriftHW(),
mpThetaFunc(0),
mInitialValue(0.)
{
}

/*!
	@brief default constructor
	@param[in] sdeAttrName
  
*/
AQLPriceDriftSZDDVolFactor::AQLPriceDriftSZDDVolFactor( const AQLString& sdeAttrName)
:
AQLPriceDriftHW(sdeAttrName),
mpThetaFunc(0),
mInitialValue(0.)
{   
	/*mpHWtool = new AQLMathHWFuncToolForVar(HW_a,HW_s);
	mpHWtool2 = new AQLMathHWFuncToolForMR(HW_a,HW_s);
	mpHWtool3 = new AQLMathHWFuncToolForMRIntegral(HW_a,HW_s);*/
}

/*!
	@brief copy constructor
*/
AQLPriceDriftSZDDVolFactor::AQLPriceDriftSZDDVolFactor(const AQLPriceDriftSZDDVolFactor& rhs) 
:
AQLPriceDriftHW(rhs),
mpThetaFunc(rhs.mpThetaFunc),
mInitialValue(rhs.mInitialValue)
{

}

/*!
	@brief destructor
*/
AQLPriceDriftSZDDVolFactor::~AQLPriceDriftSZDDVolFactor() 
{
	if (mpThetaFunc != 0) delete mpThetaFunc;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftSZDDVolFactor::clone() const
{
    try 
	{
		return new AQLPriceDriftSZDDVolFactor(*this);
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
AQLPriceDriftSZDDVolFactor::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTSZDDVOLAFACTOR ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftSZDDVolFactor::getType() const
{
	return FN_DRIFTSZDDVOLAFACTOR;
}

/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix : tenor : deltatenor)
*/
AQLString
AQLPriceDriftSZDDVolFactor::convertToString(void) const
{
	AQLString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += AQLDataInt(1).convertToString();
	ret += ":";

    return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (sde attr name : suffix : tenor : deltatenor)
*/
void
AQLPriceDriftSZDDVolFactor::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 5 || tmp.getSize() % 2 == 0)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrName = tmp.get()[0];
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftSZDDVolFactor::setUp(AQLMathPathEntity& path)
{
	AQLPriceDriftHW::setUp(path);

	AQLDataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	AQLMathAttrSDE* pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	const std::vector<std::vector<AQLFunctionBase* > > mVolatility = pattrsde->getSDE().getVolatility();

	mpSimuTimeGrid = &(pattrsde->getSDE().getBM()->getTimeGrid());
	const AQLMathVolFuncSZDDVolFactor* pVolSZDDVolFactor=0;
	//in case of calculating vega
	if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCSZDDVOLFACTOR))
		pVolSZDDVolFactor = dynamic_cast<AQLMathVolFuncSZDDVolFactor*>(mVolatility[0][0]);
	else//otherwise
	{
		const AQLFunctionBase* pbase = dynamic_cast<AQLMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
		pVolSZDDVolFactor = dynamic_cast<const AQLMathVolFuncSZDDVolFactor*>(pbase);
	}
	if (pVolSZDDVolFactor == 0)
	{
        throw AQLCoreInvalidData("Error Casting volatility object to AQLMathVolFuncSZDD was failed.", __FILE__, __LINE__);
	}
	
	////get MR and Sigma Datas
	//const AQL1DDataSet &kappaFunc = dynamic_cast<const AQL1DDataSet &>(mpvolSZDD->getKappa());
	//AQLMathHWFuncMRTMDPT MR(kappaFunc.getGrids(), kappaFunc.getParam(), *(new AQLStepInterpolation()));
	//const AQL1DDataSet &epsilonFunc = dynamic_cast<const AQL1DDataSet &>(mpvolSZDD->getEpsilon());
	//AQLMathHWFuncSigmaTMDPT Sigma(epsilonFunc.getGrids(), epsilonFunc.getParam(), *(new AQLStepInterpolation()));

	////set MR and Sigma Datas into Drift Class
	//delete mpHWtool;
	//mpHWtool =  new AQLMathHWFuncToolForVar(*dynamic_cast<AQLMathHWFuncMR*>(MR.clone()),
	//									*dynamic_cast<AQLMathHWFuncSigma*>(Sigma.clone()));
	//delete mpHWtool2;
	//mpHWtool2 =  new AQLMathHWFuncToolForMR(*dynamic_cast<AQLMathHWFuncMR*>(MR.clone()),
	//									*dynamic_cast<AQLMathHWFuncSigma*>(Sigma.clone()));
	//delete mpHWtool3;
	//mpHWtool3 = new AQLMathHWFuncToolForMRIntegral(*dynamic_cast<AQLMathHWFuncMR*>(MR.clone()),
	//									*dynamic_cast<AQLMathHWFuncSigma*>(Sigma.clone()));

	//mFuncExpInt_a_inv.SetFunc(*this, &AQLPriceDriftSZDDVolFactor::getExpInt_a_inv);
	//mFuncExpInt_a_sigma_SQ.SetFunc(*this, &AQLPriceDriftSZDDVolFactor::getExpInt_a_sigma_SQ);
	//mFuncExpInt_a_sigma_SQ_B.SetFunc(*this, &AQLPriceDriftSZDDVolFactor::getExpInt_a_sigma_SQ_B);

	//set long term volatility
	mpThetaFunc = dynamic_cast<const AQL1DDataSet *>(pVolSZDDVolFactor->getTheta()->clone());
	mFuncExpInt_a_theta.SetFunc(*this, &AQLPriceDriftSZDDVolFactor::getExpInt_a_theta);

	////set Cachesize
	////initialize cachesize
	//delete mpCache1;
	//delete mpCache2;
	//delete mpCache3;
	//mpCache1 = new DoubleVector(1, 0.0);
	//mpCache2 = new DoubleVector(1, 0.0);
	//mpCache3 = new DoubleVector(1, 0.0);

	//mCacheAlpha.clear();
	//mCacheFuncExpInt_a_inv.clear();
	//mCacheFuncExpInt_a_sigma_SQ.clear();
	//mCacheFuncExpInt_a_sigma_SQ_B.clear();
	//mCacheAlpha.insert(make_pair(0.0, 0.0));
	//mCacheFuncExpInt_a_inv.insert(make_pair(0.0, 0.0));
	//mCacheFuncExpInt_a_sigma_SQ.insert(make_pair(0.0, 0.0));
	//mCacheFuncExpInt_a_sigma_SQ_B.insert(make_pair(0.0, 0.0));
	mCacheFuncExpInt_a_theta.insert(make_pair(0.0,0.0));

	//delete mpIntegrate_cache;
	//delete mpTimeGrid;
	//mpIntegrate_cache = new DoubleVector(1, 0.0);
	//mpTimeGrid = new DoubleVector(1, 0.0);


	////set MR and Sigma Datas into Pathelement Class
	//if(pattrsde->getSDE().getTemplate()->isTypeOf(PE_HWCURVETMDPT))
	//{

	//	const AQLRatesPathElementHW1FCurveTMDPT* tmppcurve =dynamic_cast<const AQLRatesPathElementHW1FCurveTMDPT*>(pattrsde->getSDE().getTemplate()); 
	//	AQLRatesPathElementHW1FCurveTMDPT* pcurve = const_cast<AQLRatesPathElementHW1FCurveTMDPT*>(tmppcurve);
	//	pcurve->clear_cache();

	//	delete pcurve->mpHWtoolMR;
	//	pcurve->mpHWtoolMR = new AQLMathHWFuncToolForMR(*dynamic_cast<AQLMathHWFuncMR*>(MR.clone()),
	//												*dynamic_cast<AQLMathHWFuncSigma*>(Sigma.clone()));

	//	delete pcurve->mpHWtoolVar; 
	//	pcurve->mpHWtoolVar = new AQLMathHWFuncToolForVar(*dynamic_cast<AQLMathHWFuncMR*>(MR.clone()),
	//												*dynamic_cast<AQLMathHWFuncSigma*>(Sigma.clone()));
	//}

	mInitialValue = pattrsde->getSDE().getInitialValue()->get()[0];

}
/*!
    @brief Return integral result
	@param[in] x {ts, te, r} 
    @return integral result
*/
double
AQLPriceDriftSZDDVolFactor::operator()(const DoubleArray& x) const
{	
    if ( x.size() != 3 )
	{
        throw AQLCoreInvalidData(" x.size() != 3 : x[0] = ts, x[1] = te, x[1] = r ", __FILE__, __LINE__);
	}
    
	double te = x[1];

	unsigned int pos = 0;
	if (!AQLAlgorithm::find<DoubleArray, double>(*mpTimeGrid, te, 0, mpTimeGrid->size() - 1, pos))
	{
        double ts = x[0];
		//double x0 = mpHWtool->ExpIntegralMR(te);
		//double x1 = dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(te);
		//double x2 = mpHWtool->ExpIntegralMR(ts);
		//double x3 = dynamic_cast<const AQLMathPathYieldCurve*>(mpInitialCurve)->getForward(ts);

		//double s_cache1 = mpCache1->back();
		//double s_cache2 = mpCache2->back();
		//double e_cache1 = s_cache1 + mGL.integrate((*mpHWtool), ts, te);
		//double e_cache2 = s_cache2 + mGL.integrate((*mpHWtool2), ts, te);
		//mpCache1->push_back(e_cache1);
		//mpCache2->push_back(e_cache2);
		//AQLCombinationMethod pi = (*mpHWtool).operator *(*mpHWtool3);
		//mpCache3->push_back(mGL.integrate(pi, ts, te));

		//double val = x0 * x1 - x2* x3 + e_cache2 * e_cache1 - 
		//				s_cache2 * s_cache1 - mpCache3->back();
		const double val = mFuncExpInt_a_theta.Integrate_GL_piecewise(ts, te, *mpSimuTimeGrid, HWINTEGRATEDNUM);
		
		mpTimeGrid->push_back(te);
		mpIntegrate_cache->push_back(val);
		return val;
	}
	else
	{
		return (*mpIntegrate_cache)[pos];
	}
}

/*!
    @brief calc alpha
	@param[in] t
    @return alpha
*/
double
AQLPriceDriftSZDDVolFactor::getAlpha(const double t) const
{
	if (mCacheAlpha.end() == mCacheAlpha.find(t))
	{
		double alpha = 0.;
		if (t != 0.0)
		{
			const double int_E_theta = mFuncExpInt_a_theta.Integrate_GL_piecewise(0, t, *mpSimuTimeGrid, HWINTEGRATEDNUM);
			const double bar_E = getExpInt_a_inv(t);
			alpha = (mInitialValue + int_E_theta) * bar_E;
		}
		mCacheAlpha.insert(make_pair(t, alpha));
		return alpha;
	}
	else
	{
		return mCacheAlpha[t];
	}
}

/*!
    @brief calc suquared result of sigma multiplied by exponential integral result using parameter a  
	@param[in] t
*/
double
AQLPriceDriftSZDDVolFactor::getExpInt_a_theta(const double t) const
{
	if (mCacheFuncExpInt_a_theta.end() == mCacheFuncExpInt_a_theta.find(t))
	{
		if (!mpHWtool2)
		{
			throw AQLCoreInvalidData("mpHWTool2 is Null", __FILE__, __LINE__);
		}
		//const double val = mpHWtool->ExpIntegralMR(t) * mpHWtool->getHWSigma()->operator ()(t);
		//return val * val;
		double val = mpHWtool->ExpIntegralMR(t);
		val *= (*mpThetaFunc)(t);
		mCacheFuncExpInt_a_theta.insert(make_pair(t, val));
		return val;
	}
	else
	{
		return mCacheFuncExpInt_a_theta[t];
	}
}

