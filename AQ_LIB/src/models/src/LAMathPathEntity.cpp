/*! @file
    @brief Source code for class to represent MC Path.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(LADataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(LADataDate)<BR>
			3.IR_MODEL_DATA_DAYCOUNT(LAPriceDataDayCount)<BR>
			4.IR_MODEL_DATA_SDETIMEGRID(LADataDoubles)<BR>
			5.IR_MODEL_DATA_SDEINTEGRALTIMEGRID(LADataDoubles)<BR>
			6.IR_MODEL_DATA_RANDGENERATOR(LAPriceDataRand)<BR>
			7.IR_MODEL_DATA_STARTPATHNUM(LADataInt)<BR>
			8.IR_MODEL_DATA_SDEDATANAMES(LADataStrings)<BR>
			9.IR_MODEL_DATA_INITIALVALUES(LADataMultiReference)<BR>
			10.IR_MODEL_DATA_CORRELATIONMATRIX(LADataDoubleMatrix)<BR>
			11.IR_MODEL_DATA_SDEINTEGRALDIVNUM(LADataInt)<BR>
			12.IR_MODEL_DATA_ISANTITHETIC(LADataBool)<BR>
			13.IR_MODEL_DATA_CACHESIZE(LADataInt)<BR>
			14.IR_MODEL_DATA_CORRELATIONMATRIXREF(LADataReference)<BR>

*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathPathEntity.h"

#include "LADataInstance.h"
#include "LABasic.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"

#include "LAMathDefine.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataRand.h"
#include "LAMathDriftFuncBase.h"
#include "LAMathVolFuncBase.h"
#include "LAMathCorrelation.h"
#include "LAMathYieldCurve.h"
#include "LAMathFXEntity.h"
#include "LAMathFXUtility.h"

#include "LARatesSDEBase.h"
#include "LAModelDynamicsBase.h"
#include "LAModelDynamicsScalar.h"
#include "LAModelDynamicsCurve.h"
#include "LAMatrix.h"
#include "LACholeskyDecompSC.h"
#include "LAAlgorithm.h"
#include "LARatesBM_BB.h"
#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LARatesCurveLogLinearInterpolation.h"
#endif
#include "LAModelDynamicsLMMCurve.h"
#include "LAMathVolatility.h"
#include "LARatesSpotSDEQuantAdjustment.h"
#include "LAPriceQuantAdjustmentFuncBase.h"
#include "LAPriceSZDDIntegralMelstein.h"
#include "LARatesHWIntegral.h"
#include "LARatesHWIntegral3F.h"

#include <cmath>
#include <algorithm>
#include <iterator>

#define COR "COR"
#define VOL "VOL"
#define STD "STD"

using namespace std;

/*!
    @brief default constructor

	@param[in] dataInstance pointer of LADataInstance object

*/
LAMathPathEntity::LAMathPathEntity(LADataInstance* dataInstance) : 
LAObject(),
/*mIsAntithetic(false), *//*mCacheSize(0), */mCachePos(-1), mAntiCachePos(-1),
mPos(0), mIsOdd(true),	mpBM(0), mPathVersion(0)
{
	setDataInstance(dataInstance);

	LAPriceDataManager& dm = dataInstance->getDataMaster();
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
	dynamic_cast<LADataBool&>(mpIsAntithetic->get()).set(false);
	mpCacheSize = &add(IR_MODEL_DATA_CACHESIZE);
	dynamic_cast<LADataInt&>(mpCacheSize->get()).set(0);
	mpIsBrownianBridge = &add(IR_MODEL_DATA_ISBROWNIANBRIDGE);
	dynamic_cast<LADataBool&>(mpIsBrownianBridge->get()).set(false);
	mpIRCurveProNames = &add(IR_MODEL_DATA_IRCURVEPRONAMES);
}
/*!
    @brief copy constructor

	@param[in] path LAMathPathEntity object

	@note path cash data is not copied. this object become initial condition
*/
LAMathPathEntity::LAMathPathEntity(
	const LAMathPathEntity& path) : 
	LAObject(path),
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

	const LAStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	//const LAStringVector& sde_attrnames = getSDEAttrNames().get();
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	mSDEs.resize(simsde_attrnames.size());
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		mSDEs[i] = &dynamic_cast<LAMathAttrSDE&>(getData(simsde_attrnames[i]).get()).getSDE();
		LARatesBM *pBM =  mSDEs[i]->getBM();
		if (pBM)
		{
			set<LARatesBM*> bms = pBM->getReferenceBM();
		mpBMs.insert(bms.begin(), bms.end());
		}
	}
	if (mSDEs.size() == 1)
		mpBM = mSDEs[0]->getBM();
	else
	{
		set<LARatesBM*>::const_iterator it;
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
LAMathPathEntity::~LAMathPathEntity()
{
	set<LARatesBM*>::const_iterator it;
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
LAMathPathEntity::getType(void) const
{
	return ENTITY_PATH;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathPathEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_PATH ? true : LAObject::isTypeOf(id));
}
/*!
    @brief get basedate
	@return basedate
*/
const LADataDate&  
LAMathPathEntity::getAsOfDate(void) const
{
	return dynamic_cast<const LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief Get basedate.The setting of basedate is also possible.
	@return basedate
*/
LADataDate&  
LAMathPathEntity::getAsOfDate(void)
{
	return dynamic_cast<LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief get this Path Object-name.
	@return name
*/
const LADataString&	
LAMathPathEntity::getName() const	
{
	return dynamic_cast<const LADataString&>(mpName->get());
}
/*!
    @brief get this Path Object-name.The setting of name is also possible.
	@return name
*/
LADataString&	
LAMathPathEntity::getName()
{
	return dynamic_cast<LADataString&>(mpName->get());
}
/*!
	@brief get DayCount
	@return DayCount
*/
const LAPriceDataDayCount&	
LAMathPathEntity::getDayCount(void) const
{
	return dynamic_cast<const LAPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get DayCount.The setting of DayCount is also possible.
	@return DayCount
*/
LAPriceDataDayCount&
LAMathPathEntity::getDayCount(void)
{
	return dynamic_cast<LAPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get sde time grid
	@return sde time grid
*/
const LADataDoubles&
LAMathPathEntity::getSDETimeGrid() const
{
	return dynamic_cast<const LADataDoubles&>(mpSDETimeGrid->get());
}
/*!
	@brief get sde time grid. The setting of sde time grid is also possible. 
	@return sde time grid
*/
LADataDoubles&
LAMathPathEntity::getSDETimeGrid()
{
	return dynamic_cast<LADataDoubles&>(mpSDETimeGrid->get());
}
/*!
	@brief get sde integral time grid
	@return sde integral time grid
*/
const LADataDoubles&
LAMathPathEntity::getSDEIntegralTimeGrid() const
{
	return dynamic_cast<const LADataDoubles&>(mpSDEIntegralTimeGrid->get());
}
/*!
	@brief get sde integral time grid. The setting of sde integral time grid is also possible. 
	@return sde integral time grid
*/
LADataDoubles&
LAMathPathEntity::getSDEIntegralTimeGrid()
{
	return dynamic_cast<LADataDoubles&>(mpSDEIntegralTimeGrid->get());
}
/*!
	@brief get divided number of integral time grid 
	@return divided number
*/
const LADataInt&
LAMathPathEntity::getSDEIntegralDivNum() const
{
	return dynamic_cast<const LADataInt&>(mpSDEIntegralDivNum->get());
}
/*!
	@brief get divided number of integral time grid . The setting of divided number is also possible. 
	@return divided number
*/
LADataInt&
LAMathPathEntity::getSDEIntegralDivNum()
{
	return dynamic_cast<LADataInt&>(mpSDEIntegralDivNum->get());
}
/*!
	@brief get rand generator
	@return rand generator
*/
const LAPriceDataRand&
LAMathPathEntity::getRand() const
{
	return dynamic_cast<const LAPriceDataRand&>(mpRand->get());
}
/*!
	@brief get rand generator. The setting of integral time grid is also possible. 
	@return rand generator
*/
LAPriceDataRand&
LAMathPathEntity::getRand()
{
	return dynamic_cast<LAPriceDataRand&>(mpRand->get());
}
/*!
	@brief get data names of SDEs
	@return data names of SDEs
*/
const LADataStrings&
LAMathPathEntity::getSDEAttrNames() const
{
	return dynamic_cast<const LADataStrings&>(mpSDEAttrNames->get());
}
/*!
	@brief get data names of SDEs. The setting of data names of SDEs is also possible. 
	@return data names of SDEs
*/
LADataStrings&
LAMathPathEntity::getSDEAttrNames()
{
	return dynamic_cast<LADataStrings&>(mpSDEAttrNames->get());
}

/*!
	@brief get data names of SDEs
	@return data names of SimulationSDEs
*/
const LADataStrings&
LAMathPathEntity::getSimulationSDEAttrNames() const
{
	return dynamic_cast<const LADataStrings&>(mpSimSDEAttrNames->get());
}
/*!
	@brief get data names of SDEs. The setting of data names of SDEs is also possible. 
	@return data names of SimulationSDEs
*/
LADataStrings&
LAMathPathEntity::getSimulationSDEAttrNames()
{
	return dynamic_cast<LADataStrings&>(mpSimSDEAttrNames->get());
}


	// get initial values of SDEs
/*!
	@brief get initial values of SDEs
	@return initial values of SDEs
*/
const LADataMultiReference&	
LAMathPathEntity::getInitialValues() const
{
	return dynamic_cast<const LADataMultiReference&>(mpInitialValues->get());
}
/*!
	@brief get initial values of SDEs. The setting of initial values of SDEs is also possible. 
	@return initial values of SDEs
*/
LADataMultiReference&
LAMathPathEntity::getInitialValues()
{
	return dynamic_cast<LADataMultiReference&>(mpInitialValues->get());
}
/*!
	@brief get start path number of MC simulation
	@return start path number

	@note first path number is 0
*/
const LADataInt&
LAMathPathEntity::getStartPathNum() const
{
	return dynamic_cast<const LADataInt&>(mpStartPathNum->get());
}
/*!
	@brief get start path number of MC simulation. The setting of start path number is also possible. 
	@return start path number

	@note first path number is 0
	
*/
LADataInt&
LAMathPathEntity::getStartPathNum()
{
	return dynamic_cast<LADataInt&>(mpStartPathNum->get());
}
/*!
	@brief get data holder which has correlation matrix. If object has been not created yet, make it.
	@return correlation matrix holder
*/
LADataHolder*
LAMathPathEntity::getCorrelationHolder() const
{
	if (mpCor->get().isNull())
	{
		LAObject* pPathCor = new LAObject;
		pPathCor->add(IR_MODEL_DATA_CORRELATIONMATRIX, new LADataDoubleMatrix);
		const LAString pathCorName = getName().get() + "_" + IR_MODEL_DATA_CORRELATIONMATRIX;
		getDataInstance()->getObjectPool().set(pathCorName, pPathCor);
		dynamic_cast<LADataReference&>(mpCor->get()).convertFromString(pathCorName);
	}
	return mpCor;
}

/*!
	@brief get object name which has correlation matrix.
	@return correlation object name
*/
LAString
LAMathPathEntity::getCorrelationMatrixEntityName() const
{
	return dynamic_cast<const LADataReference&>(getCorrelationHolder()->get()).get().getName();
}

/*!
	@brief get correlation matrix between SDEs
	@return correlation matrix
*/
const LADataDoubleMatrix&
LAMathPathEntity::getCorrelationMatrix() const
{
	const LAObjectHolder& correlationEntity = dynamic_cast<const LADataReference&>(getCorrelationHolder()->get()).get();
	return dynamic_cast<const LADataDoubleMatrix&>(correlationEntity.getData(IR_MODEL_DATA_CORRELATIONMATRIX).get());
}
/*!
	@brief get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	@return correlation matrix
*/
LADataDoubleMatrix&
LAMathPathEntity::getCorrelationMatrix()
{
	LAObjectHolder& correlationEntity = dynamic_cast<LADataReference&>(getCorrelationHolder()->get()).get();
	return dynamic_cast<LADataDoubleMatrix&>(correlationEntity.getData(IR_MODEL_DATA_CORRELATIONMATRIX).get());
}

/*!
    @brief Make copy(clone) of this FX Object object.
    @return pointer of this FX Object object.
*/
LAObject* 
LAMathPathEntity::clone() const
{
    try 
	{
    	return new LAMathPathEntity(*this);
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
 
#ifndef VISUAL_STUDIO_2010_ANALYTICS

/*!
    @brief get path 
	@param[in] pos sde position
	@param[in] t time to get 
*/
const LARatesPathElementBase&
LAMathPathEntity::getPath(unsigned int pos, double t) const
{
	unsigned int i;
	const DoubleArray& timegrid = dynamic_cast<const LADataDoubles&>(getSDETimeGrid()).get();
	LAAlgorithm::locate<DoubleArray, double>(timegrid, t, timegrid.size(), i);
	if (i == timegrid.size())
	{
		//error
        throw LACoreInvalidData("input t is after last timegrid", __FILE__, __LINE__);
	}
	else if (timegrid[i] == t)
		return *(*mpPath[pos])[i];
	else if (i == 0)
	{
		//error
        throw LACoreInvalidData("input t is before first time of timegrid", __FILE__, __LINE__);
	}
	else
	{
		LARatesPEInterpolationBase* pinter = mSDEs[pos]->getInterpolationMethod();
		return pinter->value(t, timegrid[i - 1], timegrid[i], *(*mpPath[pos])[i - 1], *(*mpPath[pos])[i]);
	}
}

/*!
    @brief get Cache path 
	@param[in] cache mc number
	@param[in] pos sde position
*/
const ONEPATH&  
LAMathPathEntity::getCache(unsigned int mnum, unsigned int pos) const
{
	if(mCache.size() >= mnum+1 && pos < mCache[mnum].size() && mCache[mnum][pos].size()>=1)
			return mCache[mnum][pos];
	else
	{
		throw LACoreInvalidData("PathError",__FILE__,__LINE__);
	}

}


/*!
    @brief set next path
*/
void
LAMathPathEntity::setNextPath()
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
				LARatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const LARatesPathElementCurve*>((*mpPath[i])[j]));
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
				LARatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const LARatesPathElementCurve*>((*mpPath[i])[j]));
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
				LARatesNumeraireBase *mpNume = mSDEs[i]->getNumeraire();
				if (mpNume && (*mpPath[i])[0]->isTypeOf(PE_CURVE))
				{
					const DoubleArray& timegrid = mSDEs[i]->getTimeGrid();
					const int timeSize = timegrid.size();
					for (int j = 0; j < timeSize; ++j)
					{
						mpNume->setCurve(timegrid[j], dynamic_cast<const LARatesPathElementCurve*>((*mpPath[i])[j]));
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
LAMathPathEntity::setUpforMC()
{
	
	if (mPathVersion != getModel())//first call or calculation condition is changed, so set up from first
	{	
		const DoubleArray& timegrid_output = dynamic_cast<const LADataDoubles&>(getSDETimeGrid()).get();
		
		//mSDEIntegralTimeGrid;
		if (!getSDEIntegralTimeGrid().isNull())
		{
			const DoubleArray& timegrid_integral = dynamic_cast<const LADataDoubles&>(getSDEIntegralTimeGrid()).get();
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
			int divnum = dynamic_cast<const LADataInt&>(getSDEIntegralDivNum()).get();
			if (divnum <= 0)
			{
				//error
				throw LACoreInvalidData("IntegralDivNum must be more than 1", __FILE__, __LINE__);
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
		const DoubleArray& timegrid_output = dynamic_cast<const LADataDoubles&>(getSDETimeGrid()).get();
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
	const LARandBase& rand = dynamic_cast<const LARandBase&>(getRand().getMethod());//rand generator
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
LAMathPathEntity::isAntithetic(void)
{
	return dynamic_cast<const LADataBool&>(mpIsAntithetic->get()).get();
}

/*!
	@brief set antithetic or not
	@param[in] flag true:antithetic,flase:not antithetic
*/
void
LAMathPathEntity::setAntithetic(bool flag) 
{	
	bool tmp_b = (mPathVersion == getModel());
	dynamic_cast<LADataBool&>(mpIsAntithetic->get()).set(flag);
	update(TYPE_ANTITHETICFLAG_CHANGE);
    if (tmp_b) mPathVersion = getModel();
}			

/*!
	@brief get cache size
	@return cache size
*/
int
LAMathPathEntity::getCacheSize() const
{
	return dynamic_cast<const LADataInt&>(mpCacheSize->get()).get();
}


/*!
	@brief set cache size
	@param[in] size cache size
*/
void
LAMathPathEntity::setCacheSize(unsigned int size)
{
	bool flag = (mPathVersion == getModel());
	dynamic_cast<LADataInt&>(mpCacheSize->get()).set(size);
	update(TYPE_CACHESIZE_CHANGE);
    if (flag) mPathVersion = getModel();
}
	
/*!
	@brief check brownian bridge or not
	@return true:brownian bridge,false:not brownian bridge
*/
bool
LAMathPathEntity::isBrownianBridge(void)
{
	return dynamic_cast<const LADataBool&>(mpIsBrownianBridge->get()).get();
}

/*!
	@brief set brownian bridge or not
	@param[in] flag true:brownian bridge,flase:not brownian bridge
*/
void
LAMathPathEntity::setBrownianBridge(bool flag) 
{	
	bool tmp_b = (mPathVersion == getModel());
	dynamic_cast<LADataBool&>(mpIsBrownianBridge->get()).set(flag);
	update(TYPE_BROWNIANBRIDGE_CHANGE);
    if (tmp_b) mPathVersion = getModel();
}			


// get ir curvepros names
const LADataStrings&	
LAMathPathEntity::getIRCurveProNames() const
{
	return dynamic_cast<const LADataStrings&>(mpIRCurveProNames->get());
}
// get ir curvepros names
LADataStrings&
LAMathPathEntity::getIRCurveProNames()
{
	return dynamic_cast<LADataStrings&>(mpIRCurveProNames->get());
}


/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathPathEntity::remove(
	const LAString& dataName)
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
	LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathPathEntity::reset(void)
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
	@brief copy LAMathPathEntity
	@param[in] e copy source
	@return reference to this object
*/
LAObject&
LAMathPathEntity::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAObject::copy(e);
	if (!e.isTypeOf(ENTITY_PATH))
	{
		LAString err = "Assignement error for LAMathPathEntity : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
	set<LARatesBM*>::const_iterator it;
	for (it = mpBMs.begin(); it != mpBMs.end(); it++)
		delete (*it);
	mpBMs.clear();

	const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(e);

//	mIsAntithetic = path.mIsAntithetic; 
//	mCacheSize = path.mCacheSize; 
	mCachePos = path.mCachePos; 
	mAntiCachePos = path.mAntiCachePos; 
	mPos = path.mPos; 
	mpPath = path.mpPath; 
	mIsOdd = path.mIsOdd; 
	mPathVersion = path.mPathVersion; 
	mSDEIntegralTimeGrid = path.mSDEIntegralTimeGrid;
	
	const LAStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	mSDEs.resize(simsde_attrnames.size());
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		mSDEs[i] = &dynamic_cast<LAMathAttrSDE&>(getData(simsde_attrnames[i]).get()).getSDE();
		set<LARatesBM*> bms = mSDEs[i]->getBM()->getReferenceBM();
		mpBMs.insert(bms.begin(), bms.end());
	}
	if (mSDEs.size() == 1)
		mpBM = mSDEs[0]->getBM();
	else
	{
		set<LARatesBM*>::const_iterator it;
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
LADataHolder&
LAMathPathEntity::add(const LAString& name)
{
	// search Data of name
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}

/*!
	@brief celar cash
*/
void
LAMathPathEntity::clearCache(void)
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
LAMathPathEntity::setInitialValue()
{
	const LAStringVector& sde_attrnames = getSDEAttrNames().get();
	const LADataMultiReference& initialvalues = getInitialValues();
	if (sde_attrnames.size() != initialvalues.getSize())
	{
		//error
		throw LACoreInvalidData("number of sde and number of initial values are not same", __FILE__, __LINE__);
	}

	const LAStringVector& simsde_attrnames = getSimulationSDEAttrNames().get();
	
	for (unsigned int i = 0; i < sde_attrnames.size(); i++)
	{
		if (simsde_attrnames.size() !=0 && 
			simsde_attrnames.end() == std::find(simsde_attrnames.begin(), simsde_attrnames.end(), sde_attrnames[i]))
			continue;
		
		LAMathAttrSDE& attrsde = dynamic_cast<LAMathAttrSDE&>(getData(sde_attrnames[i]).get());
		LARatesSDEBase& sde = attrsde.getSDE();
		SDEPATH_TYPE type = attrsde.getSDEPathType();
		LAObjectHolder& objHolder = initialvalues.get(i);
		if (type == IR)
		{
			if (!objHolder.isTypeOf(ENTITY_IRYIELDCURVE))
			{
				throw LACoreInvalidData("Initial Value Object is not YieldCurve", __FILE__, __LINE__);
			}

			LAMathYieldCurve& yield = dynamic_cast<LAMathYieldCurve&>(objHolder.get());
			yield.setCurveType(STD);
			sde.setInitialValue(yield.getCurve(0, getDayCount().getDayCount()));
			
			LARatesNumeraireBase *nume = sde.getNumeraire();
			const LAObjectHolder &yieldData =  yield.getYieldData().get();
			const LADataHolder &ahDF2 = yieldData.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
			if (ahDF2.isDefined() && !ahDF2.isNull())
			{
				// set basis spread to numeraire
				const DoubleArray &dfs2 = dynamic_cast<const LADataDoubles &>(ahDF2.get()).get();
				const DoubleArray &dfs = dynamic_cast<const LADataDoubles &>(yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();
				const DoubleArray &terms = dynamic_cast<const LADataDoubles &>(yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();

				unsigned int size = terms.size();
				if (size != dfs.size() || size != dfs2.size())
				{
					throw LACoreInvalidData("Term and DF and DF2 should be the same size.",__FILE__,__LINE__);
				}
				DoubleArray spreads(size, 0.0);
				for (unsigned int i = 1; i < size; ++i)
				{
					if (dfs2[i] <= 0.0 || dfs[i] <= 0.0)
					{
						throw LACoreInvalidData("DF or DF2 is below zero value.",__FILE__,__LINE__);
					}
					spreads[i] = -LAMath::log(dfs2[i] / dfs[i]) / terms[i];
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
				throw LACoreInvalidData("Initial Value Object is not FX", __FILE__, __LINE__);
			}		
			LAMathFXEntity& fx = dynamic_cast<LAMathFXEntity&>(objHolder.get());	
			const LAString& currency = attrsde.getCurrency();
		    const LAStringVector& curs = LAMathFXUtility::getCurrencyPair(currency);
			SCALAR rate = static_cast<SCALAR>(fx.getRate(curs[1], curs[0], 0));
			LARatesPathElementScalar element(rate);
			sde.setInitialValue(element);
		}
		else if (type == sVOL)
		{
			if (!objHolder.isTypeOf(ENTITY_IR_VOLATILITY))
			{
				throw LACoreInvalidData("Initial Value Object is not Volatility", __FILE__, __LINE__);
			}		
			LAMathVolatility& vol = dynamic_cast<LAMathVolatility&>(objHolder.get());	
			SCALAR rate = static_cast<SCALAR>(vol.getInitialValue());
			LARatesPathElementScalar element(rate);
			sde.setInitialValue(element);
		}
		else
		{
			//error
			throw LACoreInvalidData("Not support this SDEPATH_TPYE", __FILE__, __LINE__);
		}

	}
}


/*!
	@brief set up sde
*/
void
LAMathPathEntity::setUpSDE(void)
{
	const LAStringVector& sde_attrnames = getSDEAttrNames().get();
	if (sde_attrnames.size() == 0)
	{
		//error
		LACoreInvalidData err("SDEAttrNames size is zero!", __FILE__, __LINE__);
	}
	/////////////
	//setUp SDE//
	/////////////
	//set initial value to sdes
	setInitialValue();
	const DoubleArray& timegrid_output = dynamic_cast<const LADataDoubles&>(getSDETimeGrid()).get();
	mSDEs.clear();
	//mSDEs.resize(sde_attrnames.size());

	LAStringVector simsde_attrnames = getSimulationSDEAttrNames().get();
	if (simsde_attrnames.size() == 0)
		simsde_attrnames = sde_attrnames;

	mSDEs.resize(simsde_attrnames.size());
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		LAMathAttrSDE& sde = dynamic_cast<LAMathAttrSDE&>(getData(simsde_attrnames[i]).get());
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
		LAString name = COR;
		name += "_" + simsde_attrnames[i];
		LADataHolder* dh = &getData(name, NOCHECK);
		if (!dh->isDefined() || dh->isNull())//
		{
			loadings[i].resize(1);
			loadings[i][0].resize(1);
			loadings[i][0][0].resize(timegrid_loading.size(), 1);
		}
		else
		{
			LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
			LAMathCorrelation& cor = dynamic_cast<LAMathCorrelation&>(ref.get().get());
			vector<DoubleMatrix> whole_loading = cor.getFactorLoading(timegrid_loading);
			const DoubleArray whole_timegrid = cor.getTGrid().get();

			DoubleArray::const_iterator it = std::upper_bound(whole_timegrid.begin(), whole_timegrid.end(), integralTermination);
			unsigned int use_size = (unsigned int)std::distance(whole_timegrid.begin(), it);
			std::copy(whole_loading.begin(), whole_loading.begin() + use_size, std::back_inserter(loadings[i]));
		}
		factor_size += loadings[i][0].size();
		if (loadings[i][0].size() > factor_max) factor_max = loadings[i][0].size();

	}	
	LACoreAutoPtr<LARandBase> rand_auto(dynamic_cast<LARandBase*>(getRand().getMethod().clone()));//rand generator
	UintArray dim = rand_auto->getDim();
	dim[0] = timegrid_loading.size() * factor_size;
	rand_auto->setDim(dim);

	set<LARatesBM*>::const_iterator it;
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
				mpBM = new LARatesBM_BB(mSDEIntegralTimeGrid, *rand_auto.get(), loadings[0]);
			}
			else
			{
				mpBM = new LARatesBM(mSDEIntegralTimeGrid, *rand_auto.get(), loadings[0]);
			}
		}
		catch (bad_alloc & e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
				mpBM = new LARatesBM_BB(mSDEIntegralTimeGrid, *rand_auto.get(), factor_size);
			}
			else
			{
				mpBM = new LARatesBM(mSDEIntegralTimeGrid, *rand_auto.get(), factor_size);
			}
		}
		catch (bad_alloc & e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
		}
		mpBMs.insert(mpBM);
			
		vector<LARatesBM*> bms_mid(factor_max);
			
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
	
			vector<pair<LARatesBM*, unsigned int> >  tmp_pair(flmat.size());
			for (unsigned int j = 0; j < tmp_pair.size(); j++)
				tmp_pair[j] = pair<LARatesBM*, unsigned int>(mpBM, pos++);			
			try
			{
				if (isBB)
				{
					bms_mid[i] = new LARatesBM_BB(tmp_pair, fl, false); 
				}
				else
				{
					bms_mid[i] = new LARatesBM(tmp_pair, fl, false); 
				}
			}
			catch (bad_alloc & e)
			{
				throw LACoreSystemError(e.what(), __FILE__, __LINE__);
			}
			mpBMs.insert(bms_mid[i]);
	            
		}
		for (unsigned int i = 0; i < mSDEs.size(); i++)
		{
			vector<pair<LARatesBM*, unsigned int> >  tmp_pair(loadings[i][0].size());
			for (unsigned int j = 0; j < tmp_pair.size(); j++)
				tmp_pair[j] = pair<LARatesBM*, unsigned int>(bms_mid[j], i);					
			try
			{
				if (isBB)
				{
					mSDEs[i]->setBM(new LARatesBM_BB(tmp_pair, loadings[i], false));
				}
				else
				{
					mSDEs[i]->setBM(new LARatesBM(tmp_pair, loadings[i], false));
				}
			}
			catch (bad_alloc & e)
			{
				throw LACoreSystemError(e.what(), __FILE__, __LINE__);
			}			
		}
	}
	//volatility function setup

	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		const vector<vector<LAFunctionBase*> >& vol = mSDEs[i]->getVolatility();
		for (unsigned int j = 0; j < vol.size(); j++)
			for (unsigned int k = 0; k < vol[j].size(); k++)
				if (vol[j][k]->isTypeOf(FN_VOLFUNCBASE))
					dynamic_cast<LAMathVolFuncBase*>(vol[j][k])->setUp(*this);
	}
	//drift function setup
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		const vector<LAFunctionBase*>& drift = mSDEs[i]->getDrift();
		for (unsigned int j = 0; j < drift.size(); j++)
			if (drift[j]->isTypeOf(FN_DRIFTFUNCBASE))
				dynamic_cast<LAMathDriftFuncBase*>(drift[j])->setUp(*this);
	}

	//quanto adjust set up
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		if (mSDEs[i]->isTypeOf(FN_SPOTSDEQUANTADJUSTMENT))
		{
			dynamic_cast<LARatesSpotSDEQuantAdjustment*>(mSDEs[i])->getQuantAdjuster().setUp(*this);
		}
	}

	// integralfunc
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		LARatesSDEIntegralBase *pIntegralBase = mSDEs[i]->getIntegralFunction();
		if (pIntegralBase->isTypeOf(FN_SZDDINTEGRAL2))
		{
			dynamic_cast<LAPriceSZDDIntegralMelstein*>(pIntegralBase)->setUp(*this);
		}
		if (pIntegralBase->isTypeOf(FN_HWINTEGRAL) || pIntegralBase->isTypeOf(FN_HWINTEGRAL3F))
		{
			dynamic_cast<LARatesHWIntegral*>(pIntegralBase)->setUp(*this);
		}
	}
	
	//call setup method
	for (unsigned int i = 0; i < mSDEs.size(); i++)
		mSDEs[i]->setUp();

	// set volatility for loglinearinterpolation class
	for (unsigned int i = 0; i < mSDEs.size(); i++)
	{
		LARatesPEInterpolationBase *inter = mSDEs[i]->getInterpolationMethod();
		if (inter && inter->isTypeOf(FN_CURVELOGLINEARINTERPOLATION))
		{
			const LARatesPathElementBase *path = mSDEs[i]->getPathElement(0);
			if (path && path->isTypeOf(PE_LMMCURVE))
			{
				LAString name = VOL;
				name += "_" + simsde_attrnames[i];
				LADataHolder* dh = &getData(name, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					LADataReference &ref = dynamic_cast<LADataReference &>(dh->get());
					LAMathVolatility &vol = dynamic_cast<LAMathVolatility&>(ref.get().get());
					DoubleMatrix volMat;
					DoubleArray grid_t = *(dynamic_cast<const LARatesPathElementLMMCurve *>(path)->getTenor());
					if (grid_t.empty())
					{
						throw LACoreInvalidData("LARatesPathElementLMMCurve tenor size is empty", __FILE__, __LINE__);
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
							LAFunctionBase *volFunc = vol.getVolatilityFunc(j, 0);
							volMat[i][j] = volFunc->operator ()(grid_t[i]);
							delete volFunc;
						}
					}
					dynamic_cast<LARatesCurveLogLinearInterpolation *>(inter)->setVolForInterpolation(volMat);
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
LAMathPathEntity::calcFactorLoading(const DoubleMatrix& cor)
{
	LAMatrix mat_cor(cor);
	for (unsigned int i = 0; i < cor.size(); i++)
	{
		for (unsigned int j = 0; j < cor.size(); j++)
		{
			mat_cor.setValue(i, j, cor.at(i).at(j));
		}
	}
	const LAMatrix& mat_fl = LACholeskyDecompSC::choleskyDecompositionSC(mat_cor);
	DoubleMatrix ret(cor.size());
	for (unsigned int i = 0; i < cor.size(); i++)
	{
		ret[i].resize(cor.size());
		for (unsigned int j = 0; j < cor.size(); j++)
			ret[i][j] = mat_fl[i][j];
	}
	return ret;
}
