/*! @file
    @brief Source code of drift function of quant adjustment



*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftQuantAdjustment.h"
#include "AQLPriceFXVolatility.h"
#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathAttrSDE.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathVolFuncBase.h"
#include "AQLRatesSpotSDE.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLModelDynamicsScalar.h"
#include "AQLAlgorithm.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLCombinationFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLGaussLegendre.h"
#include "AQLCombinationFunc.h"
#include "AQLBasic.h"


using namespace std;

//================ AQLPriceDriftQuantAdjustment ===================================
/*!
	@brief default constructor
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftQuantAdjustment::AQLPriceDriftQuantAdjustment(AQLFunctionBase* pDriftIR)
: mpVolatility(0), mpSDEFX(0), mpFxVolatility(0), m_i(0), mpDriftIR(pDriftIR), mPos_old(0), mInitialFxVol(0.0)
, mpVar(0), mpVarPos(0), mpVar2(0), mpVarPos2(0)
{
	if (pDriftIR == 0)
	{
		//error
		throw AQLCoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}

}
/*!
	@brief default constructor
	@param[in] sdeAttrNameIR data name of ir model
	@param[in] sdeAttrNameFX data name of fx model
	@param[in] i suffix
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftQuantAdjustment::AQLPriceDriftQuantAdjustment(const AQLString& sdeAttrNameIR, const AQLString& sdeAttrNameFX, unsigned int i, AQLFunctionBase* pDriftIR)
: mpVolatility(0), mpSDEFX(0), mpFxVolatility(0), m_i(i), mSDEAttrNameIR(sdeAttrNameIR), mSDEAttrNameFX(sdeAttrNameFX), mpDriftIR(pDriftIR), mPos_old(0), mInitialFxVol(0.0)
, mpVar(0), mpVarPos(0), mpVar2(0), mpVarPos2(0)
{
	if (pDriftIR == 0)
	{
		//error
		throw AQLCoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLPriceDriftQuantAdjustment::AQLPriceDriftQuantAdjustment(const AQLPriceDriftQuantAdjustment& v) 
: AQLMathDriftFuncBase(v), mpVolatility(v.mpVolatility), 
mCorrelation(v.mCorrelation), mpSDEFX(v.mpSDEFX), mpFxVolatility(v.mpFxVolatility),	
m_i(v.m_i), mSDEAttrNameIR(v.mSDEAttrNameIR), mSDEAttrNameFX(v.mSDEAttrNameFX), 
mpDriftIR(0), mPos_old(v.mPos_old), mInitialFxVol(v.mInitialFxVol)
, mpVar(v.mpVar), mpVarPos(v.mpVarPos), mpVar2(v.mpVar2), mpVarPos2(v.mpVarPos2)
{
	if (v.mpDriftIR != 0)
		mpDriftIR = dynamic_cast<AQLFunctionBase*>(v.mpDriftIR->clone());
}

/*!
	@brief destructor
*/
AQLPriceDriftQuantAdjustment::~AQLPriceDriftQuantAdjustment() 
{
	if (mpDriftIR != 0) delete mpDriftIR;
	if (m_i == 0)
	{
		delete mpVar;
		delete mpVarPos;
		delete mpVar2;
		delete mpVarPos2;

	}
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftQuantAdjustment::clone() const	
{
    try 
	{
		return new AQLPriceDriftQuantAdjustment(*this);
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
AQLPriceDriftQuantAdjustment::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTQUANTADJ ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftQuantAdjustment::getType() const
{
	return FN_DRIFTQUANTADJ;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} rest time <= t  
    @return drift value

*/
double
AQLPriceDriftQuantAdjustment::operator()(const DoubleArray& x) const
{
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int pos;
	if (x[0] == 0.0) pos = 0;
	else if (x[0] == timegrid[mPos_old]) pos = mPos_old;
	else if (x[0] == timegrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, x[0], 0, timegrid.size() - 1, pos))
	{
		//error
		AQLString msg = "Time =" + AQLDataDouble(x[0]).convertToString();
		msg += " is not in sde integral time grid";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	double fxvol;
	if (mpVarPos == 0 || *mpVarPos != pos)
	{
		SCALAR fx = mpSDEFX->getPathElement(pos)->get()[0];
		DoubleArray var(2);
		var[0] = x[0];
		var[1] = static_cast<double>(fx);
		fxvol = mpSDEFX->getVolatility()[0][0]->operator ()(var);
		if (mpSDEFX->getSDEType() == dX) fxvol /= var[1];
		if (mpVarPos != 0)
		{
			*mpVarPos = pos;
			*mpVar = fxvol;
		}
	}
	else
		fxvol = *mpVar;

    double ret = -mpVolatility->operator ()(x)
			  * fxvol
			  * mCorrelation[pos];
	
	ret += (*mpDriftIR)(x);

	mPos_old = pos;
	
	return ret;
}


/*!
    @brief return string representaion
    @return string representaion (ir sde attr name : fx sde attr name : suffix : mpDriftIR.convertToString())
*/
AQLString
AQLPriceDriftQuantAdjustment::convertToString(void) const
{
	AQLString ret;
	ret = mSDEAttrNameIR;
	ret += ":";
	ret += mSDEAttrNameFX;
	ret += ":";
	ret += AQLDataInt(m_i).convertToString();
	if (mpDriftIR != 0)
	{
		ret += ":";
		ret += mpDriftIR->convertToString();
	}
	return ret;

}

/*!
    @brief transform from string representaion
    @param[in] string representaion (ir sde attr name : fx sde attr name : suffix)
*/
void
AQLPriceDriftQuantAdjustment::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 3)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrNameIR = tmp.get()[0];
	mSDEAttrNameFX = tmp.get()[1];
	m_i = tmp.get()[2].getIntValue();
	
	if (tmp.getSize() == 3 || mpDriftIR == 0) return;
	AQLString str2 = tmp.get()[3];
	for (unsigned int i = 4; i < tmp.getSize(); i++)
	{
		str2 += ":";
		str2 += tmp.get()[i];
	}
	mpDriftIR->convertFromString(str2);
}
/*!
	@brief set fx sde
	@param[in] psde fx sde
	@note this class is not pointer owner of input sde
*/
void
AQLPriceDriftQuantAdjustment::setFXSDE(AQLRatesSpotSDE* psde)
{
	mpSDEFX = psde;
	if (mpSDEFX->getVolatility()[0][0]->isTypeOf(FN_VOLFUNCBASE))
		mpFxVolatility = dynamic_cast<AQLMathVolFuncBase*>(mpSDEFX->getVolatility()[0][0])->getVolatility();
	else 
		mpFxVolatility = mpSDEFX->getVolatility()[0][0];

}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftQuantAdjustment::setUp(AQLMathPathEntity& path)
{
	AQLDataHolder* dh = &path.getData(mSDEAttrNameIR, ISNOTNULL);
	AQLMathAttrSDE* pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpVolatility = pattrsde->getSDE().getVolatility()[m_i][0];
	AQLRatesBM* bm1 = pattrsde->getSDE().getBM();

	if (m_i == 0)
	{
		delete  mpVar;
		delete  mpVarPos;
		delete  mpVar2;
		delete  mpVarPos2;
		mpVar = new double;
		mpVarPos = new unsigned int(1000);
		mpVar2 = new double;
		mpVarPos2 = new unsigned int(1000);
	}
	else
	{
		AQLFunctionBase* pfunc = pattrsde->getSDE().getDrift()[0];
		if (!pfunc->isTypeOf(FN_DRIFTQUANTADJ))
		{
			//error
			throw AQLCoreInvalidData("dirft function is not AQLPriceDriftQuantAdjustment", __FILE__, __LINE__);
		}
		AQLPriceDriftQuantAdjustment* pfunc2 = dynamic_cast<AQLPriceDriftQuantAdjustment*>(pfunc);
		mpVar = pfunc2->mpVar;
		mpVarPos = pfunc2->mpVarPos;
		mpVar2 = pfunc2->mpVar2;
		mpVarPos2 = pfunc2->mpVarPos2;
	}	
		
	dh = &path.getData(mSDEAttrNameFX, ISNOTNULL);
	pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpSDEFX = dynamic_cast<AQLRatesSpotSDE*>(&pattrsde->getSDE());
	AQLRatesBM* bm2 = pattrsde->getSDE().getBM();

	mCorrelation = bm1->calcCorrelation(*bm2, 0, m_i);

	if (mpDriftIR->isTypeOf(FN_DRIFTFUNCBASE))
		dynamic_cast<AQLMathDriftFuncBase*>(mpDriftIR)->setUp(path);

	if (mpSDEFX->getVolatility()[0][0]->isTypeOf(FN_VOLFUNCBASE))
		mpFxVolatility = dynamic_cast<AQLMathVolFuncBase*>(mpSDEFX->getVolatility()[0][0])->getVolatility();
	else 
		mpFxVolatility = mpSDEFX->getVolatility()[0][0];
	
	mPos_old = 0;
	
	if (!mpFxVolatility->isTypeOf(FN_FXVOLATILITY)) return;
	
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int size = timegrid.size();

	mIntegratedData.resize(size);
	AQLGaussLegendre gl(20);
	for (unsigned int i = 1; i < size; i++)
		mIntegratedData[i] = mIntegratedData[i - 1] + integral(i - 1, &gl);
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQLPriceDriftQuantAdjustment::integral(const std::vector<std::pair<double,double> >& x) const
{
	const AQLPriceFXVolatility* pFxVol = dynamic_cast<const AQLPriceFXVolatility*>(mpFxVolatility);
	const AQLFunctionBase* p_fx_and_t_part = pFxVol->get_fx_and_t_part();

	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int size = timegrid.size();
	unsigned int pos_s, pos_e;

	if (x[0].second == timegrid[1]) pos_e = 1;
	else if (x[0].second == timegrid[mPos_old + 1]) pos_e = mPos_old + 1;
	else if (x[0].second == timegrid[mPos_old]) pos_e = mPos_old;
	else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, x[0].second, 0, size - 1, pos_e))
	{
		//error
		AQLString msg = "Time =" + AQLDataDouble(x[0].second).convertToString();
		msg += " is not in sde integral time grid";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	mPos_old = pos_e;

	if (x[0].first == 0.0) pos_s = 0;
	else if (x[0].first == timegrid[mPos_old - 1]) pos_s = mPos_old - 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, x[0].first, 0, size - 1, pos_s))
	{
		//error
		AQLString msg = "Time =" + AQLDataDouble(x[0].first).convertToString();
		msg += " is not in sde integral time grid";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
	}
	
	double fx_and_t_part;
	if (mpVarPos2 == 0 || *mpVarPos2 != pos_e)
	{
		if (pos_e == 1 || pos_s != 0)
		{
			double fx1 = static_cast<double>(mpSDEFX->getPathElement(pos_s)->get()[0]);
			DoubleArray var1(2);
			var1[0] = x[0].first;
			var1[1] = fx1;
			fx_and_t_part = 0.5 * (*p_fx_and_t_part)(var1);
			if (pos_s == 0)
				mInitialFxVol = fx_and_t_part;
		}
		else
			fx_and_t_part = mInitialFxVol;

		double fx2 = static_cast<double>(mpSDEFX->getPathElement(pos_e)->get()[0]);
		DoubleArray var2(2);
		var2[0] = x[0].second;
		var2[1] = fx2;
		fx_and_t_part += 0.5 * (*p_fx_and_t_part)(var2);
		if (mpVarPos2 != 0)
		{
			*mpVarPos2 = pos_e;
			*mpVar2 = fx_and_t_part; 
		}
	}
	else
		fx_and_t_part = *mpVar2;


/*	if (pos_e == 1 || pos_s != 0)
	{
		double fx1 = static_cast<double>(mpSDEFX->getPathElement(pos_s)->get()[0]);
		DoubleArray var1(2);
		var1[0] = x[0].first;
		var1[1] = fx1;
		fx_and_t_part = 0.5 * (*p_fx_and_t_part)(var1);
		if (pos_s == 0)
			mInitialFxVol = fx_and_t_part;
	}
	else
		fx_and_t_part = mInitialFxVol;

	double fx2 = static_cast<double>(mpSDEFX->getPathElement(pos_e)->get()[0]);
	DoubleArray var2(2);
	var2[0] = x[0].second;
	var2[1] = fx2;
	fx_and_t_part += 0.5 * (*p_fx_and_t_part)(var2);*/
	
	if (mIntegratedData.size() == 0)
	{
		mIntegratedData.resize(size);
		AQLGaussLegendre gl(20);
		for (unsigned int i = 1; i < size; i++)
			mIntegratedData[i] = mIntegratedData[i - 1] + integral(i - 1, &gl);
	}
	if (pos_s == 0)
		return mpDriftIR->integral(x) - fx_and_t_part * mIntegratedData[pos_e];	
	else
		return mpDriftIR->integral(x) - fx_and_t_part * (mIntegratedData[pos_e] - mIntegratedData[pos_s]);

}

double
AQLPriceDriftQuantAdjustment::integral(unsigned int pos_s, AQL1DIntegral* pIntegral) const
{
	const AQLPriceFXVolatility* pFxVol = dynamic_cast<const AQLPriceFXVolatility*>(mpFxVolatility);
	const AQLFunctionBase* p_t_part = pFxVol->get_t_part();
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	

	if (p_t_part->isTypeOf(FN_CONSTANT))
	{
		if (mpVolatility->isTypeOf(FN_CONSTANT)) 
			return (*p_t_part)(timegrid[0])
					* (*mpVolatility)(timegrid[0])
					* mCorrelation[pos_s]
					* (timegrid[pos_s + 1] - timegrid[pos_s]);

		else if (mpVolatility->isTypeOf(FN_1DDATASET) 
			&& (dynamic_cast<const AQL1DDataSet*>(mpVolatility)->getInterpolationType() == FN_STEPINTERPOLATION
			|| dynamic_cast<const AQL1DDataSet*>(mpVolatility)->getInterpolationType() == FN_LINEARINTERPOLATION))
			return (*p_t_part)(timegrid[0])
					* mpVolatility->integral(timegrid[pos_s], timegrid[pos_s + 1])
					* mCorrelation[pos_s]
					* (timegrid[pos_s + 1] - timegrid[pos_s]);
		else 
			return pIntegral->integrate((*p_t_part) * (*mpVolatility),timegrid[pos_s], timegrid[pos_s + 1])
				    * mCorrelation[pos_s]
					* (timegrid[pos_s + 1] - timegrid[pos_s]);
	
	}
	else if (p_t_part->isTypeOf(FN_1DDATASET))
	{
		if (mpVolatility->isTypeOf(FN_CONSTANT) 
			&& (dynamic_cast<const AQL1DDataSet*>(p_t_part)->getInterpolationType() == FN_STEPINTERPOLATION
			|| dynamic_cast<const AQL1DDataSet*>(p_t_part)->getInterpolationType() == FN_LINEARINTERPOLATION))
			return (*mpVolatility)(timegrid[0])
					* p_t_part->integral(timegrid[pos_s], timegrid[pos_s + 1])
					* mCorrelation[pos_s]
					* (timegrid[pos_s + 1] - timegrid[pos_s]);
		else if (mpVolatility->isTypeOf(FN_1DDATASET) 
			&& dynamic_cast<const AQL1DDataSet*>(mpVolatility)->getInterpolationType() == FN_STEPINTERPOLATION
			&& dynamic_cast<const AQL1DDataSet*>(p_t_part)->getInterpolationType() == FN_STEPINTERPOLATION)
		{
			set<double> grid_set;
			const DoubleArray& grid = dynamic_cast<const AQL1DDataSet*>(mpVolatility)->getGrids();
			const DoubleArray& grid2 = dynamic_cast<const AQL1DDataSet*>(p_t_part)->getGrids();
			unsigned int grid_size = grid.size();
			for (unsigned int j = 0; j < grid_size; j++)
			{
				if (grid[j] > timegrid[pos_s] && grid[j] < timegrid[pos_s + 1]) grid_set.insert(grid[j]);
				else if (grid[j] >= timegrid[pos_s + 1]) break;
			}
			grid_size = grid2.size();
			for (unsigned int j = 0; j < grid_size; j++)
			{
				if (grid2[j] > timegrid[pos_s] && grid2[j] < timegrid[pos_s + 1]) grid_set.insert(grid2[j]);
				else if (grid2[j] >= timegrid[pos_s + 1]) break;
			}
					
			set<double>::const_iterator it;
			double start = timegrid[pos_s];
			double ret = 0.0;
			double ret2;
			for (it = grid_set.begin(); it != grid_set.end(); it++)
			{
				ret2 = *it - start;
				ret2 *= (*p_t_part)(0.5 * (start + *it));
				ret2 *= (*mpVolatility)(0.5 * (start + *it));
				
				start = *it;
				ret += ret2;
			}
			ret2 = (timegrid[pos_s + 1] - start);
			ret2 *= (*p_t_part)(0.5 * (start + timegrid[pos_s + 1]));
			ret2 *= (*mpVolatility)(0.5 * (start + timegrid[pos_s + 1]));
			
			return (ret + ret2) * mCorrelation[pos_s];
		}
		else
			return pIntegral->integrate((*p_t_part) * (*mpVolatility),timegrid[pos_s], timegrid[pos_s + 1])
			    * mCorrelation[pos_s]
				* (timegrid[pos_s + 1] - timegrid[pos_s]);
	}	
	else
		return pIntegral->integrate((*p_t_part) * (*mpVolatility),timegrid[pos_s], timegrid[pos_s + 1])
			    * mCorrelation[pos_s]
				* (timegrid[pos_s + 1] - timegrid[pos_s]);


}


