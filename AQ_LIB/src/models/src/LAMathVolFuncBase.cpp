/*! @file
    @brief Source code of base class of volatility function class



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncBase.cpp
//
//  SYNOPSIS    :       LAMathVolFuncBase
//  DESCRIPTION :       Source code of base class of volatility function class
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


#include "LAMathVolFuncBase.h"
#include "LAMathVolatility.h"
#include "LAMathPathEntity.h"

#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LARatesSDEBase.h"
#include "LAAlgorithm.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"
#include "LACombinationFunc.h"
#include "LALinearInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAGaussLegendre.h"
#include "LACombinationFunc.h"
#include "LABasic.h"

using namespace std;

#define VOL "VOL"
#define GAUSSLEGENDREPOINTNUM 20

//================ LAMathVolFuncBase ===================================
/*!
	@brief default constructor
	@param[in] isMultiVariables volatility function depends sde variables(L,FX,...) or not(only t depend)
*/
LAMathVolFuncBase::LAMathVolFuncBase(bool isMultiVariables)
: LAFunctionBase(), m_i(0), m_j(0), mpVolatility(NULL), mIsMultiVariables(isMultiVariables), mpTimes(0), mPos_old(0)
{

}
/*!
	@brief constructor
	@param[in] isMultiVariables volatility function depends sde variables(L,FX,...) or not(only t depend)
*/
LAMathVolFuncBase::LAMathVolFuncBase(const LAString& sdeAttrName, unsigned int i, unsigned int j, bool isMultiVariables)
: LAFunctionBase(), m_i(i), m_j(j), mpVolatility(NULL), mSDEAttrName(sdeAttrName), mIsMultiVariables(isMultiVariables)
, mpTimes(0), mPos_old(0)
{

}

/*!
	@brief copy constructor
*/
LAMathVolFuncBase::LAMathVolFuncBase(const LAMathVolFuncBase& v) 
: LAFunctionBase(v), m_i(v.m_i), m_j(v.m_j), mpVolatility(NULL), mSDEAttrName(v.mSDEAttrName)
, mIsMultiVariables(v.mIsMultiVariables), mpTimes(v.mpTimes), mVolData(v.mVolData), mPos_old(v.mPos_old)
{
	if (v.mpVolatility != NULL)
		mpVolatility = dynamic_cast<LAFunctionBase*>(v.mpVolatility->clone());
}

/*!
	@brief destructor
*/
LAMathVolFuncBase::~LAMathVolFuncBase() 
{
	delete mpVolatility;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncBase::clone() const
{
    try 
	{
		return new LAMathVolFuncBase(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAMathVolFuncBase::isTypeOf(function_t id) const
{
	return (id==FN_VOLFUNCBASE ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathVolFuncBase::getType() const
{
	return FN_VOLFUNCBASE;
}

/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1], ....  
    @return drift value
*/
double
LAMathVolFuncBase::operator()(const DoubleArray& x) const
{
	if (mIsMultiVariables) return mpVolatility->operator ()(x);	
	else if (mpTimes == 0) return mpVolatility->operator ()(x[0]);	
	else
	{
		unsigned int pos;
		if (x[0] == 0.0) pos = 0;
		else if (x[0] == (*mpTimes)[mPos_old]) pos = mPos_old;
		else if (mPos_old + 1 < mpTimes->size() && x[0] == (*mpTimes)[mPos_old + 1]) pos = mPos_old + 1;
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0], 0, mpTimes->size() - 1, pos))
		{
			pos = 0;
			mPos_old = 0;
			return mpVolatility->operator ()(x[0]);	
		}
		mPos_old = pos; 
	
		return mVolData[pos];
	}
}


/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix )
*/
LAString
LAMathVolFuncBase::convertToString(void) const
{
	LAString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += LADataInt(m_i).convertToString();
	ret += ":";
	ret += LADataInt(m_j).convertToString();
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion sde attr name : suffix i : suffix j :  or sde attr name : suffix i or sde attr name 
*/
void
LAMathVolFuncBase::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() == 1)
	{
		m_i = 0;
		m_j = 0;
		mSDEAttrName = tmp.get()[0];
	}
	else if (tmp.getSize() == 2)
	{
		m_i = tmp.get()[1].getIntValue();
		m_j = 0;
		mSDEAttrName = tmp.get()[0];	
	}
	else if (tmp.getSize() == 3)
	{
		m_i = tmp.get()[1].getIntValue();
		m_j = tmp.get()[2].getIntValue();
		mSDEAttrName = tmp.get()[0];	
	}
	else
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

}


/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAMathVolFuncBase::setUp(LAMathPathEntity& path)
{
	LAString name = VOL;
	name += "_";
	name += mSDEAttrName;
	LADataHolder* dh = &path.getData(name, ISNOTNULL);
	LADataReference* ref = &dynamic_cast<LADataReference&>(dh->get());
	LAMathVolatility* vol = &dynamic_cast<LAMathVolatility&>(ref->get().get());
	delete mpVolatility;
	mpVolatility = vol->getVolatilityFunc(m_i, m_j);
	mpTimes = 0;
	mVolData.clear();
	mPos_old = 0;
	if (!mIsMultiVariables)
	{
		dh = &path.getData(mSDEAttrName);
		LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
		mpTimes = &pattrsde->getSDE().getBM()->getTimeGrid();
		unsigned int size = mpTimes->size();
		mVolData.resize(size);
		mIntegratedVolData.resize(size);
		for (unsigned int i = 0; i < size; i++)
			mVolData[i] = 	mpVolatility->operator ()((*mpTimes)[i]);	
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		for (unsigned int i = 1; i < size; i++)
			mIntegratedVolData[i] = mIntegratedVolData[i - 1] + integral((*mpTimes)[i - 1], (*mpTimes)[i], &gl);
	}

	// forward fx setting 	
	//check FN_VOLFUNCFXDD
	if (mpVolatility->isTypeOf(FN_VOLFUNCFXDD) || mpVolatility->isTypeOf(FN_VOLFUNCSZDD))
	{	
		// get target currnecy and fx
		LAMathVolFuncFX *pFxVol  = dynamic_cast<LAMathVolFuncFX *>(mpVolatility);
		LAString fx = pFxVol->getCurrency();
		fx.toUpper();
		LAStringVector curs = fx.toToken('/');

		// get pos
		const LADataMultiReference &initialValues = path.getInitialValues();
		const LAStringVector &sdeNames = path.getSDEAttrNames().get();
		int pos_dIR = -1, pos_fIR = -1, pos_FX = -1;
		for (unsigned int i = 0; i < sdeNames.size(); ++i)
		{
			const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdeNames[i], ISNOTNULL).get());
			LAString cur = sde.getCurrency();
			cur.toUpper();
			if (curs[0] == cur && sde.getSDEPathType() == IR)
			{
				pos_dIR = i;
			}
			else if (curs[1] == cur && sde.getSDEPathType() == IR)
			{
				pos_fIR = i;
			}
			else if (fx == cur && sde.getSDEPathType() == FX)
			{
				pos_FX = i;
			}
		}

		if (pos_dIR < 0 || pos_fIR < 0 || pos_FX < 0)
		{
			LAString msg = "IR_SDE or FX_SDE Data is not registered in PathEntity, FX = " + fx;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// get yield and fx object and calc forward fx
		const LAMathYieldCurve &dYield = dynamic_cast<const LAMathYieldCurve &>(initialValues.get(pos_dIR).get());
		const LAMathYieldCurve &fYield = dynamic_cast<const LAMathYieldCurve &>(initialValues.get(pos_fIR).get());
		const LAMathFXEntity &fxEntity = dynamic_cast<const LAMathFXEntity &>(initialValues.get(pos_FX).get());

		const double spotRate = fxEntity.getRate(curs[1], curs[0], path.getAsOfDate().get());
		const DoubleArray& timeGrid = pFxVol->getTimeGrid();
		DoubleVector fwdFX(timeGrid.size());
		for (unsigned int i = 0; i < timeGrid.size(); ++i)
		{
			fwdFX[i] = spotRate * fYield.getBasisDF(timeGrid[i]) / dYield.getBasisDF(timeGrid[i]);
		}
		//set forward fx
		if (mpVolatility->isTypeOf(FN_VOLFUNCFXDD))
		{
			dynamic_cast<LAMathVolFuncFXDD *>(pFxVol)->setFwdFX(fwdFX);
		}
		else if (mpVolatility->isTypeOf(FN_VOLFUNCSZDD))
		{
			dynamic_cast<LAMathVolFuncSZDD *>(pFxVol)->setFwdFX(fwdFX);
		}
	}
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LAMathVolFuncBase::integral(const std::vector<std::pair<double,double> >& x) const
{
	if (mIsMultiVariables) 
		return mpVolatility->integral(x);
	else if (mpTimes == 0)
	{
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}

	unsigned int pos_e;
	if (x[0].second == (*mpTimes)[mPos_old]) pos_e = mPos_old;
	else if (x[0].second == (*mpTimes)[mPos_old + 1]) pos_e = mPos_old + 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
	{
		pos_e = 0;
		mPos_old = 0;
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	mPos_old = pos_e; 
	if (x[0].first == 0.0)
		return LAMath::sqrt(mIntegratedVolData[pos_e]);	
	
    
	unsigned int pos_s;
	if (x[0].first == (*mpTimes)[mPos_old - 1]) pos_s = mPos_old - 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].first, 0, mpTimes->size() - 1, pos_s))
	{
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	
	return LAMath::sqrt(mIntegratedVolData[pos_e] - mIntegratedVolData[pos_s]);
}

/*!
    @brief Return integral result
	@param[in] t1 left end of integral region 
	@param[in] t2 right end of integral region 
	@param[in] pIntegral integral method 
	
	@return integral result
*/
double
LAMathVolFuncBase::integral(double t1, double t2, LA1DIntegral* pIntegral) const
{
	if (mpVolatility->isTypeOf(FN_CONSTANT))
	{
		double vol = mpVolatility->getParam()[0];
		return vol * vol * (t2 - t1);
	}
	else if (mpVolatility->isTypeOf(FN_1DDATASET) 
		&& dynamic_cast<const LA1DDataSet*>(mpVolatility)->getInterpolationType() == FN_STEPINTERPOLATION)
	{
		const DoubleArray& grid = dynamic_cast<const LA1DDataSet*>(mpVolatility)->getGrids();
		unsigned int pos1, pos2;
		LAAlgorithm::locate<DoubleArray, double>(grid, t1, grid.size(), pos1);
		LAAlgorithm::locate<DoubleArray, double>(grid, t2, grid.size(), pos2);

		double sum = 0.0;
		double xx1, xx2;
		double vol;
		if (pos1 == pos2)
		{
    		vol = (*mpVolatility)(0.5 * (t2 + t1));
			return	(t2 - t1) * vol * vol;
		}
		sum = 0.0;
        xx1 = t1;
		xx2 = grid[pos1];
		vol = (*mpVolatility)(0.5 * (xx2 + xx1));
		sum += (xx2 - xx1) * vol * vol;
		for (unsigned int i = pos1; i < pos2 - 1; i++)
		{
			xx1 = grid[i];
			xx2 = grid[i + 1];
			vol = (*mpVolatility)(0.5 * (xx2 + xx1));
			sum += (xx2 - xx1) * vol * vol;
		}
		xx1 = grid[pos2-1];
		xx2 = t2;
		vol = (*mpVolatility)(0.5 * (xx2 + xx1));
		sum += (xx2 - xx1) * vol * vol;
		return sum;
	}
	else
		return ((*mpVolatility) * (*mpVolatility)).integral(t1, t2, pIntegral);

}
