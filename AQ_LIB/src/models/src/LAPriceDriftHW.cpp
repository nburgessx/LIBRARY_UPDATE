/*! @file
    @brief Source code of drift function of HW sde



*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftHW.cpp
//
//  SYNOPSIS    :       LAPriceDriftHW
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
#include "LAPriceDriftHW.h"
#include "LAMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LAMathCorrelation.h"
#include "LAMathVolFuncBase.h"
#include "AQLAlgorithm.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LAMathHWFuncSigma.h"
#include "LAMathYieldCurve.h"
#include "LAPriceDriftQuantAdjustment.h"
#include "AQLCombinationFunc.h"
#include <algorithm>
#include "LAMathVolFuncHW.h"

using namespace std;
#define COR "COR"
//
//------------------------------ LAPriceDriftHW ------------------------------
//

/*!
	@brief default constructor
	
*/
LAPriceDriftHW::LAPriceDriftHW()
: 
LAMathDriftFuncBase(),
mSDEAttrName(0), 
mpInitialCurve(0),
is_cloned(false),
mGL(HWGAUSSLEGENDRENUM),
mpHWtool(0),
mpHWtool2(0),
mpHWtool3(0),
mpCache1(0),mpCache2(0),mpCache3(0),mpIntegrate_cache(0),mpTimeGrid(0)
{
}

/*!
	@brief default constructor
	@param[in] sdeAttrName
  
*/
LAPriceDriftHW::LAPriceDriftHW( const AQLString& sdeAttrName)
:
LAMathDriftFuncBase(),
mSDEAttrName(sdeAttrName), 
mpInitialCurve(0),
is_cloned(false),
mGL(HWGAUSSLEGENDRENUM),
mpHWtool(0),
mpHWtool2(0),
mpHWtool3(0),
mpCache1(0),mpCache2(0),mpCache3(0),mpIntegrate_cache(0),mpTimeGrid(0)
{   
	/*mpHWtool = new LAMathHWFuncToolForVar(HW_a,HW_s);
	mpHWtool2 = new LAMathHWFuncToolForMR(HW_a,HW_s);
	mpHWtool3 = new LAMathHWFuncToolForMRIntegral(HW_a,HW_s);*/
}

/*!
	@brief copy constructor
*/
LAPriceDriftHW::LAPriceDriftHW(const LAPriceDriftHW& rhs) 
:
LAMathDriftFuncBase(rhs),
mSDEAttrName(rhs.mSDEAttrName),
is_cloned(true),
mGL(rhs.mGL),
mpHWtool(rhs.mpHWtool !=0 ? dynamic_cast<LAMathHWFuncToolForVar*>(rhs.mpHWtool->clone()) : 0),
mpHWtool2(rhs.mpHWtool2 !=0 ? dynamic_cast<LAMathHWFuncToolForMR*>(rhs.mpHWtool2->clone()) : 0),
mpHWtool3(rhs.mpHWtool3 !=0 ? dynamic_cast<LAMathHWFuncToolForMRIntegral*>(rhs.mpHWtool3->clone()) : 0),
mpCache1(rhs.mpCache1),	
mpCache2(rhs.mpCache2),		
mpCache3(rhs.mpCache3),
mpIntegrate_cache(rhs.mpIntegrate_cache),
mpTimeGrid(rhs.mpTimeGrid)
{
    mpInitialCurve = rhs.mpInitialCurve != 0 ? dynamic_cast<const LARatesPathElementCurve*>(rhs.mpInitialCurve->clone()) : 0;
}

/*!
	@brief destructor
*/
LAPriceDriftHW::~LAPriceDriftHW() 
{
    if(is_cloned)
    {
        if (mpHWtool != 0) delete mpHWtool;
		if (mpHWtool2 != 0) delete mpHWtool2;
		if (mpHWtool3 != 0) delete mpHWtool3;
		if (mpInitialCurve != 0) delete mpInitialCurve;
		if (mpCache1 != 0) delete mpCache1;
		if (mpCache2 != 0) delete mpCache2;
		if (mpCache3 != 0) delete mpCache3;
		if (mpIntegrate_cache != 0) delete mpIntegrate_cache;
		if (mpTimeGrid != 0) delete mpTimeGrid;
	}
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAPriceDriftHW::clone() const
{
    try 
	{
		return new LAPriceDriftHW(*this);
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
LAPriceDriftHW::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHW ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftHW::getType() const
{
	return FN_DRIFTHW;
}

/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix : tenor : deltatenor)
*/
AQLString
LAPriceDriftHW::convertToString(void) const
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
LAPriceDriftHW::convertFromString(const AQLString& str)
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
LAPriceDriftHW::setUp(LAMathPathEntity& path)
{
	AQLDataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	const std::vector<std::vector<AQLFunctionBase* > > mVolatility = pattrsde->getSDE().getVolatility();

	mpSimuTimeGrid = &(pattrsde->getSDE().getBM()->getTimeGrid());
	const LAMathVolFuncHW* mpvolHW=0;
	//in case of calculating vega
	if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCHW))
		mpvolHW = dynamic_cast<LAMathVolFuncHW*>(mVolatility[0][0]);
	else//otherwise
	{
		const AQLFunctionBase* pbase = dynamic_cast<LAMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
		mpvolHW = dynamic_cast<const LAMathVolFuncHW*>(pbase);
	}
	
	//get MR and Sigma Datas
	const LAMathHWFuncToolForVar* ptool =  mpvolHW->getHWFuncTool();
	LAMathHWFuncMR* pMR = ptool->getHWMR();
	LAMathHWFuncSigma* pSigma = ptool->getHWSigma();

	//set MR and Sigma Datas into Drift Class
	delete mpHWtool;
	mpHWtool =  new LAMathHWFuncToolForVar(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
										*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));
	delete mpHWtool2;
	mpHWtool2 =  new LAMathHWFuncToolForMR(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
										*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));
	delete mpHWtool3;
	mpHWtool3 = new LAMathHWFuncToolForMRIntegral(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
										*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));

	mFuncExpInt_a_inv.SetFunc(*this, &LAPriceDriftHW::getExpInt_a_inv);
	mFuncExpInt_a_sigma_SQ.SetFunc(*this, &LAPriceDriftHW::getExpInt_a_sigma_SQ);
	mFuncExpInt_a_sigma_SQ_B.SetFunc(*this, &LAPriceDriftHW::getExpInt_a_sigma_SQ_B);
	//set Cachesize
	//initialize cachesize
	delete mpCache1;
	delete mpCache2;
	delete mpCache3;
	mpCache1 = new DoubleVector(1, 0.0);
	mpCache2 = new DoubleVector(1, 0.0);
	mpCache3 = new DoubleVector(1, 0.0);

	mCacheAlpha.clear();
	mCacheFuncExpInt_a_inv.clear();
	mCacheFuncExpInt_a_sigma_SQ.clear();
	mCacheFuncExpInt_a_sigma_SQ_B.clear();
	mCacheAlpha.insert(make_pair(0.0, 0.0));
	mCacheFuncExpInt_a_inv.insert(make_pair(0.0, 0.0));
	mCacheFuncExpInt_a_sigma_SQ.insert(make_pair(0.0, 0.0));
	mCacheFuncExpInt_a_sigma_SQ_B.insert(make_pair(0.0, 0.0));

	delete mpIntegrate_cache;
	delete mpTimeGrid;
	mpIntegrate_cache = new DoubleVector(1, 0.0);
	mpTimeGrid = new DoubleVector(1, 0.0);


	//set MR and Sigma Datas into Pathelement Class
	if(pattrsde->getSDE().getTemplate()->isTypeOf(PE_HWCURVETMDPT))
	{

		const LARatesPathElementHW1FCurveTMDPT* tmppcurve =dynamic_cast<const LARatesPathElementHW1FCurveTMDPT*>(pattrsde->getSDE().getTemplate()); 
		LARatesPathElementHW1FCurveTMDPT* pcurve = const_cast<LARatesPathElementHW1FCurveTMDPT*>(tmppcurve);
		pcurve->clear_cache();

		delete pcurve->mpHWtoolMR;
		pcurve->mpHWtoolMR = new LAMathHWFuncToolForMR(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
													*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));

		delete pcurve->mpHWtoolVar; 
		pcurve->mpHWtoolVar = new LAMathHWFuncToolForVar(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
													*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));
	}
	//set InitialCurve into Drift Class
	delete mpInitialCurve;
	mpInitialCurve = dynamic_cast<LAMathPathYieldCurve*>(pattrsde->getSDE().getInitialValue()->clone());

	

}
/*!
    @brief Return integral result
	@param[in] x {ts, te, r} 
    @return integral result
*/
double
LAPriceDriftHW::operator()(const DoubleArray& x) const
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
		double x0 = mpHWtool->ExpIntegralMR(te);
		double x1 = dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(te);
		double x2 = mpHWtool->ExpIntegralMR(ts);
		double x3 = dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getForward(ts);

		double s_cache1 = mpCache1->back();
		double s_cache2 = mpCache2->back();
		double e_cache1 = s_cache1 + mGL.integrate((*mpHWtool), ts, te);
		double e_cache2 = s_cache2 + mGL.integrate((*mpHWtool2), ts, te);
		mpCache1->push_back(e_cache1);
		mpCache2->push_back(e_cache2);
		AQLCombinationMethod pi = (*mpHWtool).operator *(*mpHWtool3);
		mpCache3->push_back(mGL.integrate(pi, ts, te));

		double val = x0 * x1 - x2* x3 + e_cache2 * e_cache1 - 
						s_cache2 * s_cache1 - mpCache3->back();
		
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
LAPriceDriftHW::getAlpha(const double t) const
{
	if (mCacheAlpha.end() == mCacheAlpha.find(t))
	{
		double alpha = dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getF(t);
		if (t != 0.0)
		{
			const double x = mFuncExpInt_a_inv.Integrate_GL_piecewise(0, t, *mpSimuTimeGrid, HWINTEGRATEDNUM);
			const double x2 = mFuncExpInt_a_sigma_SQ.Integrate_GL_piecewise(0, t, *mpSimuTimeGrid, HWINTEGRATEDNUM);
			const double x3 = mFuncExpInt_a_sigma_SQ_B.Integrate_GL_piecewise(0, t, *mpSimuTimeGrid, HWINTEGRATEDNUM);
			const double x4 = mpHWtool->ExpIntegralMR(t);
			alpha += (x * x2 - x3) / x4;
		}
		mCacheAlpha.insert(make_pair(t, alpha));
		return alpha;
	}
	else
	{
		return mCacheAlpha[t];
	}
}

//furuya
/*!
    @brief calc Fwd
	@param[in] t
    @return Fwd
*/
double
LAPriceDriftHW::getFwd(const double t) const
{
	return dynamic_cast<const LAMathPathYieldCurve*>(mpInitialCurve)->getF(t);
}

/*!
    @brief calc inverse of exponential integral result using parameter a 
	@param[in] t
*/
double
LAPriceDriftHW::getExpInt_a_inv(const double t) const
{
	if (mCacheFuncExpInt_a_inv.end() == mCacheFuncExpInt_a_inv.find(t))
	{
		if (!mpHWtool2)
		{
			throw AQLCoreInvalidData("mpHWTool2 is Null", __FILE__, __LINE__);
		}
		const double val = mpHWtool2->operator()(t);
		mCacheFuncExpInt_a_inv.insert(make_pair(t, val));
		return val;
	}
	else
	{
		return mCacheFuncExpInt_a_inv[t];
	}
}

/*!
    @brief calc suquared result of sigma multiplied by exponential integral result using parameter a  
	@param[in] t
*/
double
LAPriceDriftHW::getExpInt_a_sigma_SQ(const double t) const
{
	if (mCacheFuncExpInt_a_sigma_SQ.end() == mCacheFuncExpInt_a_sigma_SQ.find(t))
	{
		if (!mpHWtool)
		{
			throw AQLCoreInvalidData("mpHWTool is Null", __FILE__, __LINE__);
		}
		//const double val = mpHWtool->ExpIntegralMR(t) * mpHWtool->getHWSigma()->operator ()(t);
		//return val * val;
		const double val = mpHWtool->operator()(t);
		mCacheFuncExpInt_a_sigma_SQ.insert(make_pair(t, val));
		return val;
	}
	else
	{
		return mCacheFuncExpInt_a_sigma_SQ[t];
	}
}

/*!
    @brief calc 
	@param[in] t
*/
double
LAPriceDriftHW::getExpInt_a_sigma_SQ_B(const double t) const
{
	if (mCacheFuncExpInt_a_sigma_SQ_B.end() == mCacheFuncExpInt_a_sigma_SQ_B.find(t))
	{
		const double b = mFuncExpInt_a_inv.Integrate_GL_piecewise(0, t, *mpSimuTimeGrid, HWINTEGRATEDNUM);
		const double val = b * getExpInt_a_sigma_SQ(t);
		mCacheFuncExpInt_a_sigma_SQ_B.insert(make_pair(t, val));
		return val;
	}
	else
	{
		return mCacheFuncExpInt_a_sigma_SQ_B[t];
	}
}