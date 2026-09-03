/*! @file
    @brief Source code for class to represent MC Path.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(AQLDataDate)<BR>
			3.IR_MODEL_DATA_DAYCOUNT(AQLPriceDataDayCount)<BR>
			4.IR_MODEL_DATA_SDETIMEGRID(AQLDataDoubles)<BR>
			5.IR_MODEL_DATA_SDEINTEGRALTIMEGRID(AQLDataDoubles)<BR>
			6.IR_MODEL_DATA_RANDGENERATOR(AQLPriceDataRand)<BR>
			7.IR_MODEL_DATA_STARTPATHNUM(AQLDataInt)<BR>
			8.IR_MODEL_DATA_SDEDATANAMES(AQLDataStrings)<BR>
			9.IR_MODEL_DATA_INITIALVALUES(AQLDataMultiReference)<BR>
			10.IR_MODEL_DATA_CORRELATIONMATRIX(AQLDataDoubleMatrix)<BR>
			11.IR_MODEL_DATA_SDEINTEGRALDIVNUM(AQLDataInt)<BR>
			12.IR_MODEL_DATA_ISANTITHETIC(AQLDataBool)<BR>
			13.IR_MODEL_DATA_CACHESIZE(AQLDataInt)<BR>
			14.IR_MODEL_DATA_CORRELATIONMATRIXREF(AQLDataReference)<BR>

*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathPathEntity.h"

#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"

#include "AQLMathDefine.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataRand.h"
#include "AQLMathDriftFuncBase.h"
#include "AQLMathVolFuncBase.h"
#include "AQLMathCorrelation.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathFXEntity.h"
#include "AQLMathFXUtility.h"

#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsBase.h"
#include "AQLModelDynamicsScalar.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLMatrix.h"
#include "AQLCholeskyDecompSC.h"
#include "AQLAlgorithm.h"
#include "AQLRatesBM_BB.h"
#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "AQLRatesCurveLogLinearInterpolation.h"
#endif
#include "AQLModelDynamicsLMMCurve.h"
#include "AQLMathVolatility.h"
#include "AQLRatesSpotSDEQuantAdjustment.h"
#include "AQLPriceQuantAdjustmentFuncBase.h"
#include "AQLPriceSZDDIntegralMelstein.h"
#include "AQLRatesHWIntegral.h"
#include "AQLRatesHWIntegral3F.h"

#include <cmath>
#include <algorithm>
#include <iterator>

#define COR "COR"
#define VOL "VOL"
#define STD "STD"

using namespace std;

/*!
    @brief default constructor

	@param[in] dataInstance pointer of AQLDataInstance object

*/
AQLMathPathEntity::AQLMathPathEntity(AQLDataInstance* dataInstance) : 
AQLObject(),
/*mIsAntithetic(false), *//*mCacheSize(0), */mCachePos(-1), mAntiCachePos(-1),
mPos(0), mIsOdd(true),	mpBM(0), mPathVersion(0)
{
	setDataInstance(dataInstance);

	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(IR_MODEL_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(IR_MODEL_DATA_SDETIMEGRID, DATA_DOUBLES);
	dm.setData(IR_MODEL_DATA_SDEINTEGRALTIMEGRID, DATA_DOUBLES);
	dm.setData(IR_MODEL_DATA_RANDGENERATOR, DATA_RAND);
	dm.setData(IR_MODEL_DATA_STARTPATHNUM, DATA_INT);
	dm.setData(IR_MODEL_DATA_SDEDATANAMES, DATA_STRINGS);
	dm.setData(IR_MODEL_DATA_SIMULATIONSDEDATANAMES, DATA_STRINGS);
	dm.setData(IR_MODEL_DATA_INITIALVALUES, DATA_MULTIREFERENCE);
	dm.setData(IR_MODEL_DATA_CORRELATIONMATRIX, DATA_DOUBLE_MATRIX);
	dm.setData(IR_MODEL_DATA_CORRELATIONMATRIXREF, DATA_REFERENCE);
	dm.setData(IR_MODEL_DATA_SDEINTEGRALDIVNUM, DATA_INT);
	dm.setData(IR_MODEL_DATA_ISANTITHETIC, DATA_BOOL);
	dm.setData(IR_MODEL_DATA_CACHESIZE, DATA_INT);
	dm.setData(IR_MODEL_DATA_ISBROWNIANBRIDGE, DATA_BOOL);
	dm.setData(IR_MODEL_DATA_IRCURVEPRONAMES, DATA_STRINGS);
	
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &add(CALIBRATION_DATA_ASOFDATE);
	mpDC   = &add(IR_MODEL_DATA_DAYCOUNT);
	getDayCount().setDayCount(ACT_365_ISDA);
	mpSDETimeGrid = &add(IR_MODEL_DATA_SDETIMEGRID);
	mpSDEIntegralTimeGrid = &add(IR_MODEL_DATA_SDEINTEGRALTIMEGRID);
	mpRand = &add(IR_MODEL_DATA_RANDGENERATOR);
	mpStartPathNum = &add(IR_MODEL_DATA_STARTPATHNUM);
	mpSDEAttrNames = &add(IR_MODEL_DATA_SDEDATANAMES);
	mpSimSDEAttrNames = &add(IR_MODEL_DATA_SIMULATIONSDEDATANAMES);
	mpInitialValues = &add(IR_MODEL_DATA_INITIALVALUES);
	mpCor = &add(IR_MODEL_DATA_CORRELATIONMATRIXREF);
	mpSDEIntegralDivNum = &add(IR_MODEL_DATA_SDEINTEGRALDIVNUM);
	mpIsAntithetic = &add(IR_MODEL_DATA_ISANTITHETIC);
	dynamic_cast<AQLDataBool&>(mpIsAntithetic->get()).set(false);
	mpCacheSize = &add(IR_MODEL_DATA_CACHESIZE);
	dynamic_cast<AQLDataInt&>(mpCacheSize->get()).set(0);
	mpIsBrownianBridge = &add(IR_MODEL_DATA_ISBROWNIANBRIDGE);
	dynamic_cast<AQLDataBool&>(mpIsBrownianBridge->get()).set(false);
	mpIRCurveProNames = &add(IR_MODEL_DATA_IRCURVEPRONAMES);
}
/*!
    @brief copy constructor

	@param[in] path AQLMathPathEntity object

	@note path cash data is not copied. this object become initial condition
*/
AQLMathPathEntity::AQLMathPathEntity(
	const AQLMathPathEntity& path) : 
	AQLObject(path),
/*mIsAntithetic(path.mIsAntithetic), *//*mCacheSize(path.mCacheSize),*/ 
mCachePos(path.mCachePos), mAntiCachePos(path.mAntiCachePos),
mPos(path.mPos), mpPath(path.mpPath), mIsOdd(path.mIsOdd), mpBM(0), 
mPathVersion(path.mPathVersion), mSDEIntegralTimeGrid(path.mSDEIntegralTimeGrid)
{
	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &getData(CALIBRATION_DATA_ASOFDATE);
	mpDC   = &getData(IR_MODEL_DATA_DAYCOUNT);
	mpSDETimeGrid = &getData(IR_MODEL_DATA_SDETIMEGRID);
	mpSDEIntegralTimeGrid = &getData(IR_MODEL_DATA_SDEINTEGRALTIMEGRID);
	mpRand = &getData(IR_MODEL_DATA_RANDGENERATOR);
	mpStartPathNum = &getData(IR_MODEL_DATA_STARTPATHNUM);
	mpSDEAttrNames = &getData(IR_MODEL_DATA_SDEDATANAMES);
	mpSimSDEAttrNames = &getData(IR_MODEL_DATA_SIMULATIONSDEDATANAMES);
	mpInitialValues = &getData(IR_MODEL_DATA_INITIALVALUES);
	mpCor = &getData(IR_MODEL_DATA_CORRELATIONMATRIXREF);
	mpSDEIntegralDivNum = &getData(IR_MODEL_DATA_SDEINTEGRALDIVNUM);
	mpIsAntithetic = &getData(IR_MODEL_DATA_ISANTITHETIC);
	mpCacheSize = &getData(IR_MODEL_DATA_CACHESIZE);
	mpIsBrownianBridge = &getData(IR_MODEL_DATA_ISBROWNIANBRIDGE);
	mpIRCurveProNames = &getData(IR_MODEL_DATA_IRCURVEPRONAMES);

	const AQLStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	//const AQLStringVector& sde_attrnames = getSDEAttrNames().get();
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	mSDEs.resize(simsde_attrnames.size());
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		mSDEs[i] = &dynamic_cast<AQLMathAttrSDE&>(getData(simsde_attrnames[i]).get()).getSDE();
		AQLRatesBM *pBM =  mSDEs[i]->getBM();
		if (pBM)
		{
			set<AQLRatesBM*> bms = pBM->getReferenceBM();
		mpBMs.insert(bms.begin(), bms.end());
		}
	}
	if (mSDEs.size() == 1)
		mpBM = mSDEs[0]->getBM();
	else
	{
		set<AQLRatesBM*>::const_iterator it;
		for (it = mpBMs.begin(); it != mpBMs.end(); it++)
			if ((*it)->isGeneratorType())
			{
				mpBM = *it;
				break;
			}
	}
#endif

	mCache.resize(path.mCache.size());
	for (unsigned int i = 0; i < mCache.size(); i++)
	{
		mCache[i].resize(path.mCache[i].size());
		for (unsigned int j = 0; j < mCache[i].size(); j++)
		{
			mCache[i][j].resize(path.mCache[i][j].size());
			for (unsigned int k = 0; k < mCache[i][j].size(); k++)
				mCache[i][j][k] = path.mCache[i][j][k]->clone();
		}
	}
	mAntiCache.resize(path.mAntiCache.size());
	for (unsigned int i = 0; i < mAntiCache.size(); i++)
	{
		mAntiCache[i].resize(path.mAntiCache[i].size());
		for (unsigned int j = 0; j < mAntiCache[i].size(); j++)
		{
			mAntiCache[i][j].resize(path.mAntiCache[i][j].size());
			for (unsigned int k = 0; k < mAntiCache[i][j].size(); k++)
				mAntiCache[i][j][k] = path.mAntiCache[i][j][k]->clone();
		}
	}


}
/*!
    @brief destructor
*/
AQLMathPathEntity::~AQLMathPathEntity()
{
	set<AQLRatesBM*>::const_iterator it;
	for (it = mpBMs.begin(); it != mpBMs.end(); it++)
		delete (*it);	
	clearCache();
}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
AQLMathPathEntity::getType(void) const
{
	return ENTITY_PATH;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
AQLMathPathEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_PATH ? true : AQLObject::isTypeOf(id));
}
/*!
    @brief get basedate
	@return basedate
*/
const AQLDataDate&  
AQLMathPathEntity::getAsOfDate(void) const
{
	return dynamic_cast<const AQLDataDate&>(mpAsOfDate->get());
}
/*!
    @brief Get basedate.The setting of basedate is also possible.
	@return basedate
*/
AQLDataDate&  
AQLMathPathEntity::getAsOfDate(void)
{
	return dynamic_cast<AQLDataDate&>(mpAsOfDate->get());
}
/*!
    @brief get this Path Object-name.
	@return name
*/
const AQLDataString&	
AQLMathPathEntity::getName() const	
{
	return dynamic_cast<const AQLDataString&>(mpName->get());
}
/*!
    @brief get this Path Object-name.The setting of name is also possible.
	@return name
*/
AQLDataString&	
AQLMathPathEntity::getName()
{
	return dynamic_cast<AQLDataString&>(mpName->get());
}
/*!
	@brief get DayCount
	@return DayCount
*/
const AQLPriceDataDayCount&	
AQLMathPathEntity::getDayCount(void) const
{
	return dynamic_cast<const AQLPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get DayCount.The setting of DayCount is also possible.
	@return DayCount
*/
AQLPriceDataDayCount&
AQLMathPathEntity::getDayCount(void)
{
	return dynamic_cast<AQLPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get sde time grid
	@return sde time grid
*/
const AQLDataDoubles&
AQLMathPathEntity::getSDETimeGrid() const
{
	return dynamic_cast<const AQLDataDoubles&>(mpSDETimeGrid->get());
}
/*!
	@brief get sde time grid. The setting of sde time grid is also possible. 
	@return sde time grid
*/
AQLDataDoubles&
AQLMathPathEntity::getSDETimeGrid()
{
	return dynamic_cast<AQLDataDoubles&>(mpSDETimeGrid->get());
}
/*!
	@brief get sde integral time grid
	@return sde integral time grid
*/
const AQLDataDoubles&
AQLMathPathEntity::getSDEIntegralTimeGrid() const
{
	return dynamic_cast<const AQLDataDoubles&>(mpSDEIntegralTimeGrid->get());
}
/*!
	@brief get sde integral time grid. The setting of sde integral time grid is also possible. 
	@return sde integral time grid
*/
AQLDataDoubles&
AQLMathPathEntity::getSDEIntegralTimeGrid()
{
	return dynamic_cast<AQLDataDoubles&>(mpSDEIntegralTimeGrid->get());
}
/*!
	@brief get divided number of integral time grid 
	@return divided number
*/
const AQLDataInt&
AQLMathPathEntity::getSDEIntegralDivNum() const
{
	return dynamic_cast<const AQLDataInt&>(mpSDEIntegralDivNum->get());
}
/*!
	@brief get divided number of integral time grid . The setting of divided number is also possible. 
	@return divided number
*/
AQLDataInt&
AQLMathPathEntity::getSDEIntegralDivNum()
{
	return dynamic_cast<AQLDataInt&>(mpSDEIntegralDivNum->get());
}
/*!
	@brief get rand generator
	@return rand generator
*/
const AQLPriceDataRand&
AQLMathPathEntity::getRand() const
{
	return dynamic_cast<const AQLPriceDataRand&>(mpRand->get());
}
/*!
	@brief get rand generator. The setting of integral time grid is also possible. 
	@return rand generator
*/
AQLPriceDataRand&
AQLMathPathEntity::getRand()
{
	return dynamic_cast<AQLPriceDataRand&>(mpRand->get());
}
/*!
	@brief get data names of SDEs
	@return data names of SDEs
*/
const AQLDataStrings&
AQLMathPathEntity::getSDEAttrNames() const
{
	return dynamic_cast<const AQLDataStrings&>(mpSDEAttrNames->get());
}
/*!
	@brief get data names of SDEs. The setting of data names of SDEs is also possible. 
	@return data names of SDEs
*/
AQLDataStrings&
AQLMathPathEntity::getSDEAttrNames()
{
	return dynamic_cast<AQLDataStrings&>(mpSDEAttrNames->get());
}

/*!
	@brief get data names of SDEs
	@return data names of SimulationSDEs
*/
const AQLDataStrings&
AQLMathPathEntity::getSimulationSDEAttrNames() const
{
	return dynamic_cast<const AQLDataStrings&>(mpSimSDEAttrNames->get());
}
/*!
	@brief get data names of SDEs. The setting of data names of SDEs is also possible. 
	@return data names of SimulationSDEs
*/
AQLDataStrings&
AQLMathPathEntity::getSimulationSDEAttrNames()
{
	return dynamic_cast<AQLDataStrings&>(mpSimSDEAttrNames->get());
}


	// get initial values of SDEs
/*!
	@brief get initial values of SDEs
	@return initial values of SDEs
*/
const AQLDataMultiReference&	
AQLMathPathEntity::getInitialValues() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpInitialValues->get());
}
/*!
	@brief get initial values of SDEs. The setting of initial values of SDEs is also possible. 
	@return initial values of SDEs
*/
AQLDataMultiReference&
AQLMathPathEntity::getInitialValues()
{
	return dynamic_cast<AQLDataMultiReference&>(mpInitialValues->get());
}
/*!
	@brief get start path number of MC simulation
	@return start path number

	@note first path number is 0
*/
const AQLDataInt&
AQLMathPathEntity::getStartPathNum() const
{
	return dynamic_cast<const AQLDataInt&>(mpStartPathNum->get());
}
/*!
	@brief get start path number of MC simulation. The setting of start path number is also possible. 
	@return start path number

	@note first path number is 0
	
*/
AQLDataInt&
AQLMathPathEntity::getStartPathNum()
{
	return dynamic_cast<AQLDataInt&>(mpStartPathNum->get());
}
/*!
	@brief get data holder which has correlation matrix. If object has been not created yet, make it.
	@return correlation matrix holder
*/
AQLDataHolder*
AQLMathPathEntity::getCorrelationHolder() const
{
	if (mpCor->get().isNull())
	{
		AQLObject* pPathCor = new AQLObject;
		pPathCor->add(IR_MODEL_DATA_CORRELATIONMATRIX, new AQLDataDoubleMatrix);
		const AQLString pathCorName = getName().get() + "_" + IR_MODEL_DATA_CORRELATIONMATRIX;
		getDataInstance()->getObjectPool().set(pathCorName, pPathCor);
		dynamic_cast<AQLDataReference&>(mpCor->get()).convertFromString(pathCorName);
	}
	return mpCor;
}

/*!
	@brief get object name which has correlation matrix.
	@return correlation object name
*/
AQLString
AQLMathPathEntity::getCorrelationMatrixEntityName() const
{
	return dynamic_cast<const AQLDataReference&>(getCorrelationHolder()->get()).get().getName();
}

/*!
	@brief get correlation matrix between SDEs
	@return correlation matrix
*/
const AQLDataDoubleMatrix&
AQLMathPathEntity::getCorrelationMatrix() const
{
	const AQLObjectHolder& correlationEntity = dynamic_cast<const AQLDataReference&>(getCorrelationHolder()->get()).get();
	return dynamic_cast<const AQLDataDoubleMatrix&>(correlationEntity.getData(IR_MODEL_DATA_CORRELATIONMATRIX).get());
}
/*!
	@brief get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	@return correlation matrix
*/
AQLDataDoubleMatrix&
AQLMathPathEntity::getCorrelationMatrix()
{
	AQLObjectHolder& correlationEntity = dynamic_cast<AQLDataReference&>(getCorrelationHolder()->get()).get();
	return dynamic_cast<AQLDataDoubleMatrix&>(correlationEntity.getData(IR_MODEL_DATA_CORRELATIONMATRIX).get());
}

/*!
    @brief Make copy(clone) of this FX Object object.
    @return pointer of this FX Object object.
*/
AQLObject* 
AQLMathPathEntity::clone() const
{
    try 
	{
    	return new AQLMathPathEntity(*this);
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
 
#ifndef VISUAL_STUDIO_2010_ANALYTICS

/*!
    @brief get path 
	@param[in] pos sde position
	@param[in] t time to get 
*/
const AQLRatesPathElementBase&
AQLMathPathEntity::getPath(unsigned int pos, double t) const
{
	unsigned int i;
	const DoubleArray& timegrid = dynamic_cast<const AQLDataDoubles&>(getSDETimeGrid()).get();
	AQLAlgorithm::locate<DoubleArray, double>(timegrid, t, timegrid.size(), i);
	if (i == timegrid.size())
	{
		//error
        throw AQLCoreInvalidData("input t is after last timegrid", __FILE__, __LINE__);
	}
	else if (timegrid[i] == t)
		return *(*mpPath[pos])[i];
	else if (i == 0)
	{
		//error
        throw AQLCoreInvalidData("input t is before first time of timegrid", __FILE__, __LINE__);
	}
	else
	{
		AQLRatesPEInterpolationBase* pinter = mSDEs[pos]->getInterpolationMethod();
		return pinter->value(t, timegrid[i - 1], timegrid[i], *(*mpPath[pos])[i - 1], *(*mpPath[pos])[i]);
	}
}

/*!
    @brief get Cache path 
	@param[in] cache mc number
	@param[in] pos sde position
*/
const ONEPATH&  
AQLMathPathEntity::getCache(unsigned int mnum, unsigned int pos) const
{
	if(mCache.size() >= mnum+1 && pos < mCache[mnum].size() && mCache[mnum][pos].size()>=1)
			return mCache[mnum][pos];
	else
	{
		throw AQLCoreInvalidData("PathError",__FILE__,__LINE__);
	}

}


/*!
    @brief set next path
*/
void
AQLMathPathEntity::setNextPath()
{
	if (mPos != 0) mIsOdd = !mIsOdd;
	
	int cache_size = getCacheSize();
	if (!isAntithetic())// not antithetic
	{
		if (mCachePos < mPos) 
		{
			mpBM->calcBM();//generate new brownian motion
			for (unsigned int i = 0; i < mSDEs.size(); i++)
				mpPath[i] = &mSDEs[i]->getPath();		
			
			if (cache_size > mPos)
			{
				for (unsigned int i = 0; i < mSDEs.size(); i++)
					for (unsigned int j = 0; j < mpPath[i]->size(); j++)
						mCache[mPos][i][j] = (*mpPath[i])[j]->clone(); 
				mCachePos = mPos;
			}
		}
		else
		{
			const int sdeSize = mSDEs.size();
			for (int i = 0; i < sdeSize; ++i)
			{
				mpPath[i] = &mCache[mPos][i];
				AQLRatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const AQLRatesPathElementCurve*>((*mpPath[i])[j]));
					}
				}
			}
			//for (unsigned int i = 0; i < mSDEs.size(); i++)
			//	mpPath[i] = &mCache[mPos][i];
		}

		
	}
	else if (mIsOdd) //antithetic and odd
	{
		if (mCachePos < mPos / 2) 
		{
			mpBM->calcBM();//generate new brownian motion
			for (unsigned int i = 0; i < mSDEs.size(); i++)
				mpPath[i] = &mSDEs[i]->getPath();		
			
			if (cache_size > mPos / 2)
			{
				for (unsigned int i = 0; i < mSDEs.size(); i++)
					for (unsigned int j = 0; j < mpPath[i]->size(); j++)
						mCache[mPos / 2][i][j] = (*mpPath[i])[j]->clone(); 
				mCachePos = mPos / 2;
			}
		}
		else
		{
			const int sdeSize = mSDEs.size();
			for (int i = 0; i < sdeSize; ++i)
			{
				mpPath[i] = &mCache[mPos / 2][i];
				AQLRatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const AQLRatesPathElementCurve*>((*mpPath[i])[j]));
					}
				}
			}
			//for (unsigned int i = 0; i < mSDEs.size(); i++)
			//	mpPath[i] = &mCache[mPos / 2][i];
			
			if (mAntiCachePos < mPos / 2) mpBM->calcBM();//generate new brownian motion

		}
	}
	else	// antithetic and even
	{
		if (mAntiCachePos < mPos / 2) 
		{	
			mpBM->calcBM();
			for (unsigned int i = 0; i < mSDEs.size(); i++)
				mpPath[i] = &mSDEs[i]->getPath();

			if (cache_size > mPos / 2)
			{
				for (unsigned int i = 0; i < mSDEs.size(); i++)
					for (unsigned int j = 0; j < mpPath[i]->size(); j++)
						mAntiCache[mPos / 2][i][j] = (*mpPath[i])[j]->clone(); 
				mAntiCachePos = mPos / 2;
			}

		}
		else
		{
			const int sdeSize = mSDEs.size();
			for (int i = 0; i < sdeSize; ++i)
			{
				mpPath[i] = &mAntiCache[mPos / 2][i];
				AQLRatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const AQLRatesPathElementCurve*>((*mpPath[i])[j]));
					}
				}
			}		
			//for (unsigned int i = 0; i < mSDEs.size(); i++)
			//	mpPath[i] = &mAntiCache[mPos / 2][i];
		
			if (mCachePos < mPos / 2) mpBM->calcBM();
		}
	}	

	mPos++;
}


/*!
    @brief set up for MC calculation
*/
void
AQLMathPathEntity::setUpforMC()
{
	
	if (mPathVersion != getModel())//first call or calculation condition is changed, so set up from first
	{	
		const DoubleArray& timegrid_output = dynamic_cast<const AQLDataDoubles&>(getSDETimeGrid()).get();
		
		//mSDEIntegralTimeGrid;
		if (!getSDEIntegralTimeGrid().isNull())
		{
			const DoubleArray& timegrid_integral = dynamic_cast<const AQLDataDoubles&>(getSDEIntegralTimeGrid()).get();
			mSDEIntegralTimeGrid.clear();			
			unsigned int j = 0;
			for (unsigned int i = 0; i < timegrid_integral.size(); i++)
			{
				for (; j < timegrid_output.size(); j++)
				{
					if (timegrid_output[j] < timegrid_integral[i])
						mSDEIntegralTimeGrid.push_back(timegrid_output[j]);
					else if (timegrid_output[j] == timegrid_integral[i])
					{
						++j;
						break;
					}
					else
						break;
				}
				mSDEIntegralTimeGrid.push_back(timegrid_integral[i]);
			}		
		}
		else if (!getSDEIntegralDivNum().isNull() || getSDEIntegralDivNum().get() > 1)
		{
			int divnum = dynamic_cast<const AQLDataInt&>(getSDEIntegralDivNum()).get();
			if (divnum <= 0)
			{
				//error
				throw AQLCoreInvalidData("IntegralDivNum must be more than 1", __FILE__, __LINE__);
			}
			mSDEIntegralTimeGrid.resize(1 + (timegrid_output.size() - 1)* divnum);
			mSDEIntegralTimeGrid[0] = timegrid_output[0];
			for (unsigned int i = 0; i < timegrid_output.size() - 1; i++)
			{
				double delta = (timegrid_output[i + 1] - timegrid_output[i]) / double(divnum);
				for (int j = 1; j <= divnum; j++)
					mSDEIntegralTimeGrid[i * divnum + j] = timegrid_output[i] + j * delta; 
			}
		}
		else
			mSDEIntegralTimeGrid = timegrid_output;
		
		//setUp SDE
		setUpSDE();
		//setUp mpPath
		mpPath.resize(mSDEs.size());
		//setUp Cache		
		clearCache();
		unsigned int cache_size = getCacheSize();
		mCache.resize(cache_size);
		for (unsigned int i = 0; i < cache_size; i++)
		{
			mCache[i].resize(mSDEs.size());
			for (unsigned int j = 0; j < mSDEs.size(); j++)			
				mCache[i][j].resize(timegrid_output.size());	
		}		
		if (isAntithetic())
		{
			mAntiCache.resize(cache_size);
			for (unsigned int i = 0; i < cache_size; i++)
			{
				mAntiCache[i].resize(mSDEs.size());
				for (unsigned int j = 0; j < mSDEs.size(); j++)			
					mAntiCache[i][j].resize(timegrid_output.size());	
			}
		}
		mCachePos = -1;
		mAntiCachePos = -1;

	}
	else //set up cash only
	{
		const DoubleArray& timegrid_output = dynamic_cast<const AQLDataDoubles&>(getSDETimeGrid()).get();
		//setUp Cache
		unsigned int oldsize = mCache.size();
		unsigned int cache_size = getCacheSize();
		if (cache_size > oldsize)
		{
			mCache.resize(cache_size);
			for (unsigned int i = oldsize; i < cache_size; i++)
			{
				mCache[i].resize(mSDEs.size());
				for (unsigned int j = 0; j < mSDEs.size(); j++)
					mCache[i][j].resize(timegrid_output.size());
			}
		}
		else if (cache_size < oldsize)
		{
			for (unsigned int i = cache_size; i < oldsize; i++)
			{
				for (unsigned int j = 0; j < mSDEs.size(); j++)
					for (unsigned int k = 0; j < mCache[i][j].size(); k++)
						delete mCache[i][j][k];
			}
			mCache.resize(cache_size);
			mCachePos = mCachePos < (int)cache_size - 1 ? mCachePos : (int)cache_size - 1; 		
		}		
		
		oldsize = mAntiCache.size();
		if (isAntithetic() && cache_size > mAntiCache.size())
		{
			mAntiCache.resize(cache_size);
			for (unsigned int i = oldsize; i < cache_size; i++)
			{
				mAntiCache[i].resize(mSDEs.size());
				for (unsigned int j = 0; j < mSDEs.size(); j++)
					mAntiCache[i][j].resize(timegrid_output.size());
			}	
		}
		else if (cache_size < oldsize)
		{
			for (unsigned int i = cache_size; i < oldsize; i++)
			{
				for (unsigned int j = 0; j < mSDEs.size(); j++)
					for (unsigned int k = 0; j < mAntiCache[i][j].size(); k++)
						delete mAntiCache[i][j][k];
			}
			mAntiCache.resize(cache_size);
			mAntiCachePos = mAntiCachePos < (int)cache_size - 1 ? mAntiCachePos : (int)cache_size - 1; 
		}		
	}
	
	//initialize rand seed
	const AQLRandBase& rand = dynamic_cast<const AQLRandBase&>(getRand().getMethod());//rand generator
	mpBM->setSeed(rand.getSeed());	

	// set path start position
	mPos = 0;
	
	// set antithetic or not
	for (unsigned int i = 0; i < mSDEs.size(); i++)
		mSDEs[i]->setAntithetic(isAntithetic());
	
	// path start number
	int start = getStartPathNum();
	
	// BM idling
	for (int i = 0; i < start ;i++)
		mpBM->calcBM(true);
	if (isAntithetic())
	{
		int loopsize = mCachePos > mAntiCachePos ? (mAntiCachePos + 1) * 2 : (mCachePos + 1) * 2;
		for (int i = 0; i < loopsize ;i++)
			mpBM->calcBM(true);
	}
	else
		for (int i = 0; i <= mCachePos ;i++)
			mpBM->calcBM(true);

	// odd flag
	mIsOdd = (start % 2 == 0);


	
	mPathVersion = getModel();
}

#endif

/*!
	@brief check antithetic or not
	@return true:antithetic,false:not antithetic
*/
bool
AQLMathPathEntity::isAntithetic(void)
{
	return dynamic_cast<const AQLDataBool&>(mpIsAntithetic->get()).get();
}

/*!
	@brief set antithetic or not
	@param[in] flag true:antithetic,flase:not antithetic
*/
void
AQLMathPathEntity::setAntithetic(bool flag) 
{	
	bool tmp_b = (mPathVersion == getModel());
	dynamic_cast<AQLDataBool&>(mpIsAntithetic->get()).set(flag);
	update(TYPE_ANTITHETICFLAG_CHANGE);
    if (tmp_b) mPathVersion = getModel();
}			

/*!
	@brief get cache size
	@return cache size
*/
int
AQLMathPathEntity::getCacheSize() const
{
	return dynamic_cast<const AQLDataInt&>(mpCacheSize->get()).get();
}


/*!
	@brief set cache size
	@param[in] size cache size
*/
void
AQLMathPathEntity::setCacheSize(unsigned int size)
{
	bool flag = (mPathVersion == getModel());
	dynamic_cast<AQLDataInt&>(mpCacheSize->get()).set(size);
	update(TYPE_CACHESIZE_CHANGE);
    if (flag) mPathVersion = getModel();
}
	
/*!
	@brief check brownian bridge or not
	@return true:brownian bridge,false:not brownian bridge
*/
bool
AQLMathPathEntity::isBrownianBridge(void)
{
	return dynamic_cast<const AQLDataBool&>(mpIsBrownianBridge->get()).get();
}

/*!
	@brief set brownian bridge or not
	@param[in] flag true:brownian bridge,flase:not brownian bridge
*/
void
AQLMathPathEntity::setBrownianBridge(bool flag) 
{	
	bool tmp_b = (mPathVersion == getModel());
	dynamic_cast<AQLDataBool&>(mpIsBrownianBridge->get()).set(flag);
	update(TYPE_BROWNIANBRIDGE_CHANGE);
    if (tmp_b) mPathVersion = getModel();
}			


// get ir curvepros names
const AQLDataStrings&	
AQLMathPathEntity::getIRCurveProNames() const
{
	return dynamic_cast<const AQLDataStrings&>(mpIRCurveProNames->get());
}
// get ir curvepros names
AQLDataStrings&
AQLMathPathEntity::getIRCurveProNames()
{
	return dynamic_cast<AQLDataStrings&>(mpIRCurveProNames->get());
}


/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
AQLMathPathEntity::remove(
	const AQLString& dataName)
{
	if(dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_ASOFDATE 
		|| dataName == IR_MODEL_DATA_DAYCOUNT
		|| dataName == IR_MODEL_DATA_SDETIMEGRID
		|| dataName == IR_MODEL_DATA_SDEINTEGRALTIMEGRID
		|| dataName == IR_MODEL_DATA_RANDGENERATOR
		|| dataName == IR_MODEL_DATA_STARTPATHNUM
		|| dataName == IR_MODEL_DATA_SDEDATANAMES
		|| dataName == IR_MODEL_DATA_SIMULATIONSDEDATANAMES
		|| dataName == IR_MODEL_DATA_INITIALVALUES
		|| dataName == IR_MODEL_DATA_CORRELATIONMATRIXREF
		|| dataName == IR_MODEL_DATA_SDEINTEGRALDIVNUM
		|| dataName == IR_MODEL_DATA_ISANTITHETIC
		|| dataName == IR_MODEL_DATA_CACHESIZE
		|| dataName == IR_MODEL_DATA_ISBROWNIANBRIDGE
		|| dataName == IR_MODEL_DATA_IRCURVEPRONAMES)
	{
		return; 
	}
	AQLObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
AQLMathPathEntity::reset(void)
{
	clear();
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &add(CALIBRATION_DATA_ASOFDATE);
	mpDC		 = &add(IR_MODEL_DATA_DAYCOUNT);
	mpSDETimeGrid = &add(IR_MODEL_DATA_SDETIMEGRID);
	mpSDEIntegralTimeGrid = &add(IR_MODEL_DATA_SDEINTEGRALTIMEGRID);
	mpRand = &add(IR_MODEL_DATA_RANDGENERATOR);
	mpStartPathNum = &add(IR_MODEL_DATA_STARTPATHNUM);
	mpSDEAttrNames = &add(IR_MODEL_DATA_SDEDATANAMES);
	mpSimSDEAttrNames = &add(IR_MODEL_DATA_SIMULATIONSDEDATANAMES);
	mpInitialValues = &add(IR_MODEL_DATA_INITIALVALUES);
	mpCor = &add(IR_MODEL_DATA_CORRELATIONMATRIXREF);
	mpSDEIntegralDivNum = &add(IR_MODEL_DATA_SDEINTEGRALDIVNUM);
	mpIsAntithetic = &add(IR_MODEL_DATA_ISANTITHETIC);
	mpCacheSize = &add(IR_MODEL_DATA_CACHESIZE);
	mpIsBrownianBridge = &add(IR_MODEL_DATA_ISBROWNIANBRIDGE);
	mpIRCurveProNames = &add(IR_MODEL_DATA_IRCURVEPRONAMES);

	clearCache();

	for (unsigned int i = 0; i < mSDEs.size(); i++)
		mSDEs[i]->setBM(0);
	mSDEs.clear();
	mpPath.clear();
	//mIsAntithetic = false; 
	mPos = 0;
	mIsOdd = true;
	mpBM = 0;
	mPathVersion = 0;
	mSDEIntegralTimeGrid.clear();
}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy AQLMathPathEntity
	@param[in] e copy source
	@return reference to this object
*/
AQLObject&
AQLMathPathEntity::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	AQLObject::copy(e);
	if (!e.isTypeOf(ENTITY_PATH))
	{
		AQLString err = "Assignement error for AQLMathPathEntity : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &getData(CALIBRATION_DATA_ASOFDATE);
	mpDC	 = &getData(IR_MODEL_DATA_DAYCOUNT);
	mpSDETimeGrid = &getData(IR_MODEL_DATA_SDETIMEGRID);
	mpSDEIntegralTimeGrid = &getData(IR_MODEL_DATA_SDEINTEGRALTIMEGRID);
	mpRand = &getData(IR_MODEL_DATA_RANDGENERATOR);
	mpStartPathNum = &getData(IR_MODEL_DATA_STARTPATHNUM);
	mpSDEAttrNames = &getData(IR_MODEL_DATA_SDEDATANAMES);
	mpSimSDEAttrNames = &getData(IR_MODEL_DATA_SIMULATIONSDEDATANAMES);
	mpInitialValues = &getData(IR_MODEL_DATA_INITIALVALUES);
	mpCor = &getData(IR_MODEL_DATA_CORRELATIONMATRIXREF);
	mpSDEIntegralDivNum = &getData(IR_MODEL_DATA_SDEINTEGRALDIVNUM);
	mpIsAntithetic = &getData(IR_MODEL_DATA_ISANTITHETIC);
	mpCacheSize = &getData(IR_MODEL_DATA_CACHESIZE);
	mpIsBrownianBridge = &getData(IR_MODEL_DATA_ISBROWNIANBRIDGE);
	mpIRCurveProNames = &getData(IR_MODEL_DATA_IRCURVEPRONAMES);
	
	clearCache();
	for (unsigned int i = 0; i < mSDEs.size(); i++)
		mSDEs[i]->setBM(0);
	set<AQLRatesBM*>::const_iterator it;
	for (it = mpBMs.begin(); it != mpBMs.end(); it++)
		delete (*it);
	mpBMs.clear();

	const AQLMathPathEntity& path = dynamic_cast<const AQLMathPathEntity&>(e);

//	mIsAntithetic = path.mIsAntithetic; 
//	mCacheSize = path.mCacheSize; 
	mCachePos = path.mCachePos; 
	mAntiCachePos = path.mAntiCachePos; 
	mPos = path.mPos; 
	mpPath = path.mpPath; 
	mIsOdd = path.mIsOdd; 
	mPathVersion = path.mPathVersion; 
	mSDEIntegralTimeGrid = path.mSDEIntegralTimeGrid;
	
	const AQLStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	mSDEs.resize(simsde_attrnames.size());
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		mSDEs[i] = &dynamic_cast<AQLMathAttrSDE&>(getData(simsde_attrnames[i]).get()).getSDE();
		set<AQLRatesBM*> bms = mSDEs[i]->getBM()->getReferenceBM();
		mpBMs.insert(bms.begin(), bms.end());
	}
	if (mSDEs.size() == 1)
		mpBM = mSDEs[0]->getBM();
	else
	{
		set<AQLRatesBM*>::const_iterator it;
		for (it = mpBMs.begin(); it != mpBMs.end(); it++)
			if ((*it)->isGeneratorType())
			{
				mpBM = *it;
				break;
			}
	}
#endif
	
	mCache.resize(path.mCache.size());
	for (unsigned int i = 0; i < mCache.size(); i++)
	{
		mCache[i].resize(path.mCache[i].size());
		for (unsigned int j = 0; j < mCache[i].size(); j++)
		{
			mCache[i][j].resize(path.mCache[i][j].size());
			for (unsigned int k = 0; k < mCache[i][j].size(); k++)
				mCache[i][j][k] = path.mCache[i][j][k]->clone();
		}
	}
	mAntiCache.resize(path.mAntiCache.size());
	for (unsigned int i = 0; i < mAntiCache.size(); i++)
	{
		mAntiCache[i].resize(path.mAntiCache[i].size());
		for (unsigned int j = 0; j < mAntiCache[i].size(); j++)
		{
			mAntiCache[i][j].resize(path.mAntiCache[i][j].size());
			for (unsigned int k = 0; k < mAntiCache[i][j].size(); k++)
				mAntiCache[i][j][k] = path.mAntiCache[i][j][k]->clone();
		}
	}

	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
AQLDataHolder&
AQLMathPathEntity::add(const AQLString& name)
{
	// search Data of name
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}

/*!
	@brief celar cash
*/
void
AQLMathPathEntity::clearCache(void)
{
	for (unsigned int i = 0; i < mCache.size(); i++)
	{
		for (unsigned int j = 0; j < mCache[i].size(); j++)
		{
			for (unsigned int k = 0; k < mCache[i][j].size(); k++)
				delete mCache[i][j][k];
			mCache[i][j].clear();	
		}
		mCache[i].clear();
	}
	mCache.clear();
	for (unsigned int i = 0; i < mAntiCache.size(); i++)
	{
		for (unsigned int j = 0; j < mAntiCache[i].size(); j++)
		{
			for (unsigned int k = 0; k < mAntiCache[i][j].size(); k++)
				delete mAntiCache[i][j][k];
			mAntiCache[i][j].clear();
		}
		mAntiCache[i].clear();
	}
	mAntiCache.clear();

	mCachePos = -1;
	mAntiCachePos = -1;
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS
/*!
	@brief set intial values to SDEs
*/
void
AQLMathPathEntity::setInitialValue()
{
	const AQLStringVector& sde_attrnames = getSDEAttrNames().get();
	const AQLDataMultiReference& initialvalues = getInitialValues();
	if (sde_attrnames.size() != initialvalues.getSize())
	{
		//error
		throw AQLCoreInvalidData("number of sde and number of initial values are not same", __FILE__, __LINE__);
	}

	const AQLStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	
	for (unsigned int i = 0; i < sde_attrnames.size(); i++)
	{
		if (simsde_attrnames.size() !=0 && 
			simsde_attrnames.end() == std::find(simsde_attrnames.begin(), simsde_attrnames.end(), sde_attrnames[i]))
			continue;
		
		AQLMathAttrSDE& attrsde = dynamic_cast<AQLMathAttrSDE&>(getData(sde_attrnames[i]).get());
		AQLRatesSDEBase& sde = attrsde.getSDE();
		SDEPATH_TYPE type = attrsde.getSDEPathType();
		AQLObjectHolder& objHolder = initialvalues.get(i);
		if (type == IR)
		{
			if (!objHolder.isTypeOf(ENTITY_IRYIELDCURVE))
			{
				throw AQLCoreInvalidData("Initial Value Object is not YieldCurve", __FILE__, __LINE__);
			}

			AQLMathYieldCurve& yield = dynamic_cast<AQLMathYieldCurve&>(objHolder.get());
			yield.setCurveType(STD);
			sde.setInitialValue(yield.getCurve(0, getDayCount().getDayCount()));
			
			AQLRatesNumeraireBase *nume = sde.getNumeraire();
			const AQLObjectHolder &yieldData =  yield.getYieldData().get();
			const AQLDataHolder &ahDF2 = yieldData.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
			if (ahDF2.isDefined() && !ahDF2.isNull())
			{
				// set basis spread to numeraire
				const DoubleArray &dfs2 = dynamic_cast<const AQLDataDoubles &>(ahDF2.get()).get();
				const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles &>(yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();
				const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>(yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();

				unsigned int size = terms.size();
				if (size != dfs.size() || size != dfs2.size())
				{
					throw AQLCoreInvalidData("Term and DF and DF2 should be the same size.",__FILE__,__LINE__);
				}
				DoubleArray spreads(size, 0.0);
				for (unsigned int i = 1; i < size; ++i)
				{
					if (dfs2[i] <= 0.0 || dfs[i] <= 0.0)
					{
						throw AQLCoreInvalidData("DF or DF2 is below zero value.",__FILE__,__LINE__);
					}
					spreads[i] = -AQLMath::log(dfs2[i] / dfs[i]) / terms[i];
				}
				nume->reset();
				nume->setBasisSpread(SIMUBASIS, terms, spreads);
			}
			else
			{
				// reset numeraire
				nume->reset();
			}
		}
		else if (type == FX)
		{
			if (!objHolder.isTypeOf(ENTITY_FX))
			{
				throw AQLCoreInvalidData("Initial Value Object is not FX", __FILE__, __LINE__);
			}		
			AQLMathFXEntity& fx = dynamic_cast<AQLMathFXEntity&>(objHolder.get());	
			const AQLString& currency = attrsde.getCurrency();
		    const AQLStringVector& curs = AQLMathFXUtility::getCurrencyPair(currency);
			SCALAR rate = static_cast<SCALAR>(fx.getRate(curs[1], curs[0], 0));
			AQLRatesPathElementScalar element(rate);
			sde.setInitialValue(element);
		}
		else if (type == sVOL)
		{
			if (!objHolder.isTypeOf(ENTITY_IR_VOLATILITY))
			{
				throw AQLCoreInvalidData("Initial Value Object is not Volatility", __FILE__, __LINE__);
			}		
			AQLMathVolatility& vol = dynamic_cast<AQLMathVolatility&>(objHolder.get());	
			SCALAR rate = static_cast<SCALAR>(vol.getInitialValue());
			AQLRatesPathElementScalar element(rate);
			sde.setInitialValue(element);
		}
		else
		{
			//error
			throw AQLCoreInvalidData("Not support this SDEPATH_TPYE", __FILE__, __LINE__);
		}

	}
}


/*!
	@brief set up sde
*/
void
AQLMathPathEntity::setUpSDE(void)
{
	const AQLStringVector& sde_attrnames = getSDEAttrNames().get();
	if (sde_attrnames.size() == 0)
	{
		//error
		AQLCoreInvalidData err("SDEAttrNames size is zero!", __FILE__, __LINE__);
	}
	/////////////
	//setUp SDE//
	/////////////
	//set initial value to sdes
	setInitialValue();
	const DoubleArray& timegrid_output = dynamic_cast<const AQLDataDoubles&>(getSDETimeGrid()).get();
	mSDEs.clear();
	//mSDEs.resize(sde_attrnames.size());

	AQLStringVector simsde_attrnames = getSimulationSDEAttrNames().get();
	if (simsde_attrnames.size() == 0)
		simsde_attrnames = sde_attrnames;

	mSDEs.resize(simsde_attrnames.size());
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		AQLMathAttrSDE& sde = dynamic_cast<AQLMathAttrSDE&>(getData(simsde_attrnames[i]).get());
		mSDEs[i] = &sde.getSDE();
		//set output timegrid
		mSDEs[i]->setTimeGrid(timegrid_output);			
	}
	
	////////////
	//setUp BM//
	////////////
	vector<vector<DoubleMatrix> > loadings(simsde_attrnames.size());
	DoubleArray timegrid_loading =  mSDEIntegralTimeGrid;
	double integralTermination = *std::max_element(mSDEIntegralTimeGrid.begin(), mSDEIntegralTimeGrid.end());
	timegrid_loading.pop_back();
	unsigned int factor_size = 0;//for total factor size
	unsigned int factor_max = 0;//max factor number of each sdes
	//get factor loading for each sde
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		AQLString name = COR;
		name += "_" + simsde_attrnames[i];
		AQLDataHolder* dh = &getData(name, NOCHECK);
		if (!dh->isDefined() || dh->isNull())//
		{
			loadings[i].resize(1);
			loadings[i][0].resize(1);
			loadings[i][0][0].resize(timegrid_loading.size(), 1);
		}
		else
		{
			AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
			AQLMathCorrelation& cor = dynamic_cast<AQLMathCorrelation&>(ref.get().get());
			vector<DoubleMatrix> whole_loading = cor.getFactorLoading(timegrid_loading);
			const DoubleArray whole_timegrid = cor.getTGrid().get();

			DoubleArray::const_iterator it = std::upper_bound(whole_timegrid.begin(), whole_timegrid.end(), integralTermination);
			unsigned int use_size = (unsigned int)std::distance(whole_timegrid.begin(), it);
			std::copy(whole_loading.begin(), whole_loading.begin() + use_size, std::back_inserter(loadings[i]));
		}
		factor_size += loadings[i][0].size();
		if (loadings[i][0].size() > factor_max) factor_max = loadings[i][0].size();

	}	
	AQLCoreAutoPtr<AQLRandBase> rand_auto(dynamic_cast<AQLRandBase*>(getRand().getMethod().clone()));//rand generator
	UintArray dim = rand_auto->getDim();
	dim[0] = timegrid_loading.size() * factor_size;
	rand_auto->setDim(dim);

	set<AQLRatesBM*>::const_iterator it;
	for (it = mpBMs.begin(); it != mpBMs.end(); it++)
		delete (*it);	

	mpBMs.clear();

	const bool isBB = isBrownianBridge();
	if (mSDEs.size() == 1)//case of one sde
	{
		try
		{
			if (isBB)
			{
				mpBM = new AQLRatesBM_BB(mSDEIntegralTimeGrid, *rand_auto.get(), loadings[0]);
			}
			else
			{
				mpBM = new AQLRatesBM(mSDEIntegralTimeGrid, *rand_auto.get(), loadings[0]);
			}
		}
		catch (bad_alloc & e)
		{
			throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
		}
		//set brownian motion class
		mSDEs[0]->setBM(mpBM);
	}
	else//case of more than two sdes
	{
		vector<UintArray> tmp_mat(factor_max);
		for (unsigned int i = 0; i < mSDEs.size(); i++)
			for (unsigned int j = 0; j < loadings[i][0].size(); j++)
				tmp_mat[j].push_back(i);			
		const DoubleMatrix& cor = getCorrelationMatrix().get();
		try
		{
			if (isBB)
			{
				mpBM = new AQLRatesBM_BB(mSDEIntegralTimeGrid, *rand_auto.get(), factor_size);
			}
			else
			{
				mpBM = new AQLRatesBM(mSDEIntegralTimeGrid, *rand_auto.get(), factor_size);
			}
		}
		catch (bad_alloc & e)
		{
			throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
		}
		mpBMs.insert(mpBM);
			
		vector<AQLRatesBM*> bms_mid(factor_max);
			
		unsigned int pos = 0;
		for (unsigned int i = 0; i < factor_max; i++)
		{
			DoubleMatrix cor_tmp(tmp_mat[i].size());
			for (unsigned int j = 0; j < cor_tmp.size(); j++)
			{
				cor_tmp[j].resize(cor_tmp.size(), 1);
				for (unsigned int k = 0; k < j; k++)
				{
					cor_tmp[j][k] = cor[tmp_mat[i][j]][tmp_mat[i][k]];
					cor_tmp[k][j] = cor_tmp[j][k];
				}
			}
			const DoubleMatrix& flmat = calcFactorLoading(cor_tmp);
			vector<DoubleMatrix> fl(flmat.size());
			for (unsigned int j = 0; j < fl.size(); j++)
			{
				fl[j].resize(flmat[j].size());
				for (unsigned int k = 0; k < fl[j].size(); k++)
				{
					fl[j][k].resize(timegrid_loading.size(), flmat[j][k]);
				}
			}
	
			vector<pair<AQLRatesBM*, unsigned int> >  tmp_pair(flmat.size());
			for (unsigned int j = 0; j < tmp_pair.size(); j++)
				tmp_pair[j] = pair<AQLRatesBM*, unsigned int>(mpBM, pos++);			
			try
			{
				if (isBB)
				{
					bms_mid[i] = new AQLRatesBM_BB(tmp_pair, fl, false); 
				}
				else
				{
					bms_mid[i] = new AQLRatesBM(tmp_pair, fl, false); 
				}
			}
			catch (bad_alloc & e)
			{
				throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
			}
			mpBMs.insert(bms_mid[i]);
	            
		}
		for (unsigned int i = 0; i < mSDEs.size(); i++)
		{
			vector<pair<AQLRatesBM*, unsigned int> >  tmp_pair(loadings[i][0].size());
			for (unsigned int j = 0; j < tmp_pair.size(); j++)
				tmp_pair[j] = pair<AQLRatesBM*, unsigned int>(bms_mid[j], i);					
			try
			{
				if (isBB)
				{
					mSDEs[i]->setBM(new AQLRatesBM_BB(tmp_pair, loadings[i], false));
				}
				else
				{
					mSDEs[i]->setBM(new AQLRatesBM(tmp_pair, loadings[i], false));
				}
			}
			catch (bad_alloc & e)
			{
				throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
			}			
		}
	}
	//volatility function setup

	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		const vector<vector<AQLFunctionBase*> >& vol = mSDEs[i]->getVolatility();
		for (unsigned int j = 0; j < vol.size(); j++)
			for (unsigned int k = 0; k < vol[j].size(); k++)
				if (vol[j][k]->isTypeOf(FN_VOLFUNCBASE))
					dynamic_cast<AQLMathVolFuncBase*>(vol[j][k])->setUp(*this);
	}
	//drift function setup
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		const vector<AQLFunctionBase*>& drift = mSDEs[i]->getDrift();
		for (unsigned int j = 0; j < drift.size(); j++)
			if (drift[j]->isTypeOf(FN_DRIFTFUNCBASE))
				dynamic_cast<AQLMathDriftFuncBase*>(drift[j])->setUp(*this);
	}

	//quanto adjust set up
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		if (mSDEs[i]->isTypeOf(FN_SPOTSDEQUANTADJUSTMENT))
		{
			dynamic_cast<AQLRatesSpotSDEQuantAdjustment*>(mSDEs[i])->getQuantAdjuster().setUp(*this);
		}
	}

	// integralfunc
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		AQLRatesSDEIntegralBase *pIntegralBase = mSDEs[i]->getIntegralFunction();
		if (pIntegralBase->isTypeOf(FN_SZDDINTEGRAL2))
		{
			dynamic_cast<AQLPriceSZDDIntegralMelstein*>(pIntegralBase)->setUp(*this);
		}
		if (pIntegralBase->isTypeOf(FN_HWINTEGRAL) || pIntegralBase->isTypeOf(FN_HWINTEGRAL3F))
		{
			dynamic_cast<AQLRatesHWIntegral*>(pIntegralBase)->setUp(*this);
		}
	}
	
	//call setup method
	for (unsigned int i = 0; i < mSDEs.size(); i++)
		mSDEs[i]->setUp();

	// set volatility for loglinearinterpolation class
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		AQLRatesPEInterpolationBase *inter = mSDEs[i]->getInterpolationMethod();
		if (inter && inter->isTypeOf(FN_CURVELOGLINEARINTERPOLATION))
		{
			const AQLRatesPathElementBase *path = mSDEs[i]->getPathElement(0);
			if (path && path->isTypeOf(PE_LMMCURVE))
			{
				AQLString name = VOL;
				name += "_" + simsde_attrnames[i];
				AQLDataHolder* dh = &getData(name, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					AQLDataReference &ref = dynamic_cast<AQLDataReference &>(dh->get());
					AQLMathVolatility &vol = dynamic_cast<AQLMathVolatility&>(ref.get().get());
					DoubleMatrix volMat;
					DoubleArray grid_t = *(dynamic_cast<const AQLRatesPathElementLMMCurve *>(path)->getTenor());
					if (grid_t.empty())
					{
						throw AQLCoreInvalidData("AQLRatesPathElementLMMCurve tenor size is empty", __FILE__, __LINE__);
					}
					unsigned int marketSize = 0;
					if (grid_t[0] == 0.0)
					{
						marketSize = grid_t.size() - 2;
					}
					else
					{
						marketSize = grid_t.size() - 1;
						grid_t.insert(grid_t.begin(), 0.0);
					}
					grid_t.pop_back();
					const unsigned int tSize = grid_t.size();
					volMat.resize(tSize);
					// set volatility val
					for (unsigned int i = 0; i < tSize; ++i)
					{
						volMat[i].resize(marketSize, 0.0);
						for (unsigned int j = 0; j < marketSize; ++j)
						{
							AQLFunctionBase *volFunc = vol.getVolatilityFunc(j, 0);
							volMat[i][j] = volFunc->operator ()(grid_t[i]);
							delete volFunc;
						}
					}
					dynamic_cast<AQLRatesCurveLogLinearInterpolation *>(inter)->setVolForInterpolation(volMat);
				}
			}
		}
	}
}

#endif


/*!
	@brief calculate factor loading from correlation
	@param[in] cor correlation matrix

	@return factor loading
*/
DoubleMatrix
AQLMathPathEntity::calcFactorLoading(const DoubleMatrix& cor)
{
	AQLMatrix mat_cor(cor);
	for (unsigned int i = 0; i < cor.size(); i++)
	{
		for (unsigned int j = 0; j < cor.size(); j++)
		{
			mat_cor.setValue(i, j, cor.at(i).at(j));
		}
	}
	const AQLMatrix& mat_fl = AQLCholeskyDecompSC::choleskyDecompositionSC(mat_cor);
	DoubleMatrix ret(cor.size());
	for (unsigned int i = 0; i < cor.size(); i++)
	{
		ret[i].resize(cor.size());
		for (unsigned int j = 0; j < cor.size(); j++)
			ret[i][j] = mat_fl[i][j];
	}
	return ret;
}
