/*! @file
    @brief Source code for class to represent Market Parameter Data for Plain Vanilla Calculation.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(LADataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(LADataDate)<BR>
			10.IR_MODEL_DATA_CORRELATIONMATRIX(LADataDoubleMatrix)<BR>
			

*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathPlainVanillaEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathVolatility.h"

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
#include "LAMathVolFuncBase.h"
#include "LAMathCorrelation.h"
#include "LAMathYieldCurve.h"
#include "LAMathFXEntity.h"
#include "LAMathFXUtility.h"

#include "LAMatrix.h"
#include "LACholeskyDecompSC.h"
#include "LAAlgorithm.h"
#include "LAMathYieldCurvePro.h"
#include "LALinearInterpolation.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"


#include <cmath>
#include <algorithm>

#define COR "COR"

using namespace std;

/*!
    @brief default constructor

	@param[in] dataInstance pointer of LADataInstance object

*/
LAMathPlainVanillaEntity::LAMathPlainVanillaEntity(LADataInstance* dataInstance) :
LAObject()
{
	setDataInstance(dataInstance);

	LAPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(IR_MODEL_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(IR_MODEL_DATA_CORRELATIONMATRIX, DATA_DOUBLE_MATRIX);


	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &add(CALIBRATION_DATA_ASOFDATE);
	mpDC   = &add(IR_MODEL_DATA_DAYCOUNT);
	mpCor = &add(IR_MODEL_DATA_CORRELATIONMATRIX);
	
	dm.setData(IR_MODEL_DATA_IRCURRENCYS, DATA_STRINGS);
	mpIRCurs = &add(IR_MODEL_DATA_IRCURRENCYS);

	dm.setData(IR_MODEL_DATA_IRSIMCURRENCYS, DATA_STRINGS);
	mpIRSimCurs = &add(IR_MODEL_DATA_IRSIMCURRENCYS);

	dm.setData(IR_MODEL_DATA_IRCURVETYPES, DATA_STRINGS);
	mpIRCurveTypes = &add(IR_MODEL_DATA_IRCURVETYPES);

	dm.setData(IR_MODEL_DATA_FXCURRENCYS, DATA_STRINGS);
	mpFXCurs = &add(IR_MODEL_DATA_FXCURRENCYS);

	dm.setData(IR_MODEL_DATA_IRCURVES, DATA_MULTIREFERENCE);
	mpIRCurves = &add(IR_MODEL_DATA_IRCURVES);

	dm.setData(IR_MODEL_DATA_IRCURVEPRONAMES, DATA_STRINGS);
	mpIRCurveProNames = &add(IR_MODEL_DATA_IRCURVEPRONAMES);

	dm.setData(IR_MODEL_DATA_IRVOLATILITYS, DATA_MULTIREFERENCE);
	mpIRVols = &add(IR_MODEL_DATA_IRVOLATILITYS);

	dm.setData(IR_MODEL_DATA_FXVOLATILITYS, DATA_MULTIREFERENCE);
	mpFXVols = &add(IR_MODEL_DATA_FXVOLATILITYS);

	dm.setData(IR_MODEL_DATA_FXRATE, DATA_REFERENCE);
	mpFX = &add(IR_MODEL_DATA_FXRATE);
}
/*!
    @brief copy constructor

	@param[in] path LAMathPlainVanillaEntity object

	@note 
*/
LAMathPlainVanillaEntity::LAMathPlainVanillaEntity(
	const LAMathPlainVanillaEntity& evanilla) :
LAObject(evanilla)
{
	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &getData(CALIBRATION_DATA_ASOFDATE);
	mpDC   = &getData(IR_MODEL_DATA_DAYCOUNT);
	mpCor = &getData(IR_MODEL_DATA_CORRELATIONMATRIX);
	mpIRCurs = &getData(IR_MODEL_DATA_IRCURRENCYS);
	mpIRSimCurs = &getData(IR_MODEL_DATA_IRSIMCURRENCYS);
	mpFXCurs = &getData(IR_MODEL_DATA_FXCURRENCYS);
	mpIRCurves = &getData(IR_MODEL_DATA_IRCURVES);
	mpIRCurveProNames =  &getData(IR_MODEL_DATA_IRCURVEPRONAMES);
	mpIRCurveTypes = &getData(IR_MODEL_DATA_IRCURVETYPES);
	mpIRVols = &getData(IR_MODEL_DATA_IRVOLATILITYS);
	mpFXVols = &getData(IR_MODEL_DATA_FXVOLATILITYS);
	mpFX = &getData(IR_MODEL_DATA_FXRATE);
}
/*!
    @brief destructor
*/
LAMathPlainVanillaEntity::~LAMathPlainVanillaEntity()
{
	clearLiborRateMap();
}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
LAMathPlainVanillaEntity::getType(void) const
{
	return ENTITY_PLAINVANILLA;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathPlainVanillaEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_PLAINVANILLA ? true : LAObject::isTypeOf(id));
}
/*!
    @brief get basedate
	@return basedate
*/
const LADataDate&  
LAMathPlainVanillaEntity::getAsOfDate(void) const
{
	return dynamic_cast<const LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief Get basedate.The setting of basedate is also possible.
	@return basedate
*/
LADataDate&  
LAMathPlainVanillaEntity::getAsOfDate(void)
{
	return dynamic_cast<LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief get this Plain vanilla Object-name.
	@return name
*/
const LADataString&	
LAMathPlainVanillaEntity::getName() const	
{
	return dynamic_cast<const LADataString&>(mpName->get());
}

/*!
    @brief get this Plain vanilla Object-name.
	@return name
*/
LADataString&	
LAMathPlainVanillaEntity::getName() 	
{
	return dynamic_cast<LADataString&>(mpName->get());
}

/*!
	@brief get DayCount
	@return DayCount
*/
const LAPriceDataDayCount&	
LAMathPlainVanillaEntity::getDayCount(void) const
{
	return dynamic_cast<const LAPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get DayCount.The setting of DayCount is also possible.
	@return DayCount
*/
LAPriceDataDayCount&
LAMathPlainVanillaEntity::getDayCount(void)
{
	return dynamic_cast<LAPriceDataDayCount&>(mpDC->get());
}
/*!



/*!
    @brief get this Plain vanilla IR CCYS.
	@return CCYS
*/
const LADataStrings&	
LAMathPlainVanillaEntity::getIRCurrencys() const
{
	return dynamic_cast<const LADataStrings &>(mpIRCurs->get());
}

/*!
    @brief get this Plain vanilla IR CCYS.
	@return CCYS
*/
LADataStrings&	
LAMathPlainVanillaEntity::getIRCurrencys()
{
	return dynamic_cast<LADataStrings &>(mpIRCurs->get());
}

/*!
    @brief get this Plain vanilla IR Simulation CCYS.
	@return CCYS
*/
const LADataStrings&	
LAMathPlainVanillaEntity::getIRSimCurrencys() const
{
	return dynamic_cast<const LADataStrings &>(mpIRSimCurs->get());
}

/*!
    @brief get this Plain vanilla IR Simulation CCYS.
	@return CCYS
*/
LADataStrings&	
LAMathPlainVanillaEntity::getIRSimCurrencys()
{
	return dynamic_cast<LADataStrings &>(mpIRSimCurs->get());
}

/*!
    @brief get this Plain vanilla IR CurveTypes.
	@return CCYS
*/
const LADataStrings&	
LAMathPlainVanillaEntity::getIRCurveTypes() const
{
	return dynamic_cast<const LADataStrings &>(mpIRCurveTypes->get());
}

/*!
    @brief get this Plain vanilla IR CurveTypes.
	@return CCYS
*/
LADataStrings&	
LAMathPlainVanillaEntity::getIRCurveTypes()
{
	return dynamic_cast<LADataStrings &>(mpIRCurveTypes->get());
}

/*!
    @brief get this Plain vanilla FX CCYS.
	@return CCYS
*/
const LADataStrings&	
LAMathPlainVanillaEntity::getFXCurrencys() const
{
	return dynamic_cast<const LADataStrings &>(mpFXCurs->get());
}

/*!
    @brief get this Plain vanilla FX CCYS.
	@return CCYS
*/
LADataStrings&	
LAMathPlainVanillaEntity::getFXCurrencys()
{
	return dynamic_cast<LADataStrings &>(mpFXCurs->get());
}


// get ir curves
const LADataMultiReference&	
LAMathPlainVanillaEntity::getIRCurves() const
{
	return dynamic_cast<const LADataMultiReference&>(mpIRCurves->get());
}
// get ir curves
LADataMultiReference&
LAMathPlainVanillaEntity::getIRCurves()
{
	return dynamic_cast<LADataMultiReference&>(mpIRCurves->get());
}

// get ir curvepro names
const LADataStrings&	
LAMathPlainVanillaEntity::getIRCurveProNames() const
{
	return dynamic_cast<const LADataStrings&>(mpIRCurveProNames->get());
}
// get ir curvepro names
LADataStrings&
LAMathPlainVanillaEntity::getIRCurveProNames()
{
	return dynamic_cast<LADataStrings&>(mpIRCurveProNames->get());
}



// get ir volatilitys
const LADataMultiReference&	
LAMathPlainVanillaEntity::getIRVolatilitys() const
{
	return dynamic_cast<const LADataMultiReference&>(mpIRVols->get());
}
// get ir volatilitys
LADataMultiReference&
LAMathPlainVanillaEntity::getIRVolatilitys()
{
	return dynamic_cast<LADataMultiReference&>(mpIRVols->get());
}

// get fx volatilitys
const LADataMultiReference&	
LAMathPlainVanillaEntity::getFXVolatilitys() const
{
	return dynamic_cast<const LADataMultiReference&>(mpFXVols->get());
}
// get fx volatilitys
LADataMultiReference&
LAMathPlainVanillaEntity::getFXVolatilitys()
{
	return dynamic_cast<LADataMultiReference&>(mpFXVols->get());
}

// get fx object
const LADataReference&
LAMathPlainVanillaEntity::getFXEntity() const
{
	return dynamic_cast<const LADataReference&>(mpFX->get());
}
// get fx object
LADataReference&
LAMathPlainVanillaEntity::getFXEntity()
{
	return dynamic_cast<LADataReference&>(mpFX->get());
}

/*!
    @brief setup curve types

*/
void 
LAMathPlainVanillaEntity::setUpIRCurveTypes()
{
	const unsigned int size = getIRCurveTypes().getSize();
	if (size != getIRCurves().getSize())
	{
		throw LACoreInvalidData("Curve size and CurveType size is not same", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < size; ++i)
	{
		LAMathYieldCurve &curve = dynamic_cast<LAMathYieldCurve &>(getIRCurves().get(i).get());
		curve.setCurveType(getIRCurveTypes().get()[i]);
	}
}

// get IR Curve
const LAMathYieldCurve& 
LAMathPlainVanillaEntity::getIRCurve(const LAString &key) const
{
	LAString tmpKey = key;
	const LAStringVector& ircurs = getIRCurrencys().get();
	LAStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw LACoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	const LAMathYieldCurve &curve = dynamic_cast<LAMathYieldCurve &>(getIRCurves().get(pos).get());
	return curve;
}

// get IR Curve
LAMathYieldCurve&
LAMathPlainVanillaEntity::getIRCurve(const LAString &key)
{
	LAString tmpKey = key;
	const LAStringVector& ircurs = getIRCurrencys().get();
	LAStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw LACoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	LAMathYieldCurve &curve = dynamic_cast<LAMathYieldCurve &>(getIRCurves().get(pos).get());
	return curve;
}

// get CurvePro Names
const LAString&
LAMathPlainVanillaEntity::getIRCurveProName(const LAString &key) const
{
	LAString tmpKey = key;
	const LAStringVector& ircurs = getIRCurrencys().get();
	LAStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
	{
		LAString msg = "The ccy is not registered. ccy = " + key;
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	const LAStringVector &curveProNames = getIRCurveProNames().get();
	if (curveProNames.size() <= pos)
	{
		LAString msg = "YieldCurvePro Name is not registered. ccy = " + key;
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	return curveProNames[pos];
}

// get IR CurvePro
const LAMathYieldCurvePro& 
LAMathPlainVanillaEntity::getIRCurvePro(const LAString &key) const
{
	const LAString &name = getIRCurveProName(key);
	LAObjectPool &objPool = getDataInstance()->getObjectPool();
	return  dynamic_cast<const LAMathYieldCurvePro &>(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());
}

// get IR CurvePro
LAMathYieldCurvePro&
LAMathPlainVanillaEntity::getIRCurvePro(const LAString &key)
{
	const LAString &name = getIRCurveProName(key);
	LAObjectPool &objPool = getDataInstance()->getObjectPool();
	return  dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS

// get FX Vol
LAMathVolatility& 
LAMathPlainVanillaEntity::getFXVol(const LAString &key)
{
	LAString tmpKey = key;
	const LAStringVector& fxcurs = getFXCurrencys().get();
	
	LAStringVector::const_iterator it = std::find(fxcurs.begin(), fxcurs.end(), tmpKey.toUpper());
	if (it == fxcurs.end())
		throw LACoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - fxcurs.begin());
	return dynamic_cast<LAMathVolatility &>(getFXVolatilitys().get(pos).get());
}



// get FXVol method
const LAFunctionBase*
LAMathPlainVanillaEntity::getFXVolFunc(const LAString &key)
{
	LAMathVolatility& fxvol = getFXVol(key);
	const LAFunctionBase* ret = fxvol.getVolatilityFunc();
	return ret;
}

// get IR Vol
LAMathVolatility& 
LAMathPlainVanillaEntity::getIRVol(const LAString &key)
{
	LAString tmpKey = key;
	const LAStringVector& ircurs = getIRCurrencys().get();
	LAStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw LACoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	return dynamic_cast<LAMathVolatility &>(getIRVolatilitys().get(pos).get());
}

// get IRVol method
const LAFunctionBase*
LAMathPlainVanillaEntity::getIRVolFunc(const LAString &key)
{
	LAMathVolatility& irvol = getIRVol(key);
	const LAFunctionBase* ret = irvol.getVolatilityFunc();
	return ret;
}

#endif

/*!
	@brief get correlation matrix between SDEs
	@return correlation matrix
*/
const LADataDoubleMatrix&
LAMathPlainVanillaEntity::getCorrelationMatrix() const
{
	return dynamic_cast<const LADataDoubleMatrix&>(mpCor->get());
}
/*!
	@brief get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	@return correlation matrix
*/
LADataDoubleMatrix&
LAMathPlainVanillaEntity::getCorrelationMatrix()
{
	return dynamic_cast<LADataDoubleMatrix&>(mpCor->get());
}

/*!
    @brief Make copy(clone) of this PlainVanilla Object object.
    @return pointer of this PlainVanilla Object object.
*/
LAObject* 
LAMathPlainVanillaEntity::clone() const
{
    try 
	{
    	return new LAMathPlainVanillaEntity(*this);
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathPlainVanillaEntity::remove(
	const LAString& dataName)
{
	if(dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_ASOFDATE 
		|| dataName == IR_MODEL_DATA_DAYCOUNT
		|| dataName == IR_MODEL_DATA_CORRELATIONMATRIX
		|| dataName == IR_MODEL_DATA_IRCURRENCYS
		|| dataName == IR_MODEL_DATA_IRSIMCURRENCYS
		|| dataName == IR_MODEL_DATA_FXCURRENCYS
		|| dataName == IR_MODEL_DATA_IRCURVES
		|| dataName == IR_MODEL_DATA_IRCURVETYPES
		|| dataName == IR_MODEL_DATA_IRVOLATILITYS
		|| dataName == IR_MODEL_DATA_FXVOLATILITYS
		|| dataName == IR_MODEL_DATA_FXRATE
		|| dataName == IR_MODEL_DATA_IRCURVEPRONAMES
		)
	{
		return; 
	}
	LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathPlainVanillaEntity::reset(void)
{
	clear();
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &add(CALIBRATION_DATA_ASOFDATE);
	mpDC		 = &add(IR_MODEL_DATA_DAYCOUNT);
	mpCor = &add(IR_MODEL_DATA_CORRELATIONMATRIX);
	mpIRCurs = &add(IR_MODEL_DATA_IRCURRENCYS);
	mpIRSimCurs = &add(IR_MODEL_DATA_IRSIMCURRENCYS);
	mpFXCurs = &add(IR_MODEL_DATA_FXCURRENCYS);
	mpIRCurves = &add(IR_MODEL_DATA_IRCURVES);
	mpIRCurveProNames = &add(IR_MODEL_DATA_IRCURVEPRONAMES);

	mpIRVols = &add(IR_MODEL_DATA_IRVOLATILITYS);
	mpFXVols = &add(IR_MODEL_DATA_FXVOLATILITYS);
	mpFX	 = &add(IR_MODEL_DATA_FXRATE);
	mpIRCurveTypes = &add(IR_MODEL_DATA_IRCURVETYPES);
	
	clearLiborRateMap();
}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy LAMathPlainVanillaEntity
	@param[in] e copy source
	@return reference to this object
*/
LAObject&
LAMathPlainVanillaEntity::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAObject::copy(e);
	if (!e.isTypeOf(ENTITY_PLAINVANILLA))
	{
		LAString err = "Assignement error for LAMathPlainVanillaEntity : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate   = &getData(CALIBRATION_DATA_ASOFDATE);
	mpDC	 = &getData(IR_MODEL_DATA_DAYCOUNT);
	mpCor = &getData(IR_MODEL_DATA_CORRELATIONMATRIX);
	mpIRCurs = &getData(IR_MODEL_DATA_IRCURRENCYS);
	mpIRSimCurs = &getData(IR_MODEL_DATA_IRSIMCURRENCYS);
	mpFXCurs = &getData(IR_MODEL_DATA_FXCURRENCYS);
	mpIRCurves = &getData(IR_MODEL_DATA_IRCURVES);
	mpIRCurveProNames = &getData(IR_MODEL_DATA_IRCURVEPRONAMES);

	mpIRVols = &getData(IR_MODEL_DATA_IRVOLATILITYS);
	mpFXVols = &getData(IR_MODEL_DATA_FXVOLATILITYS);
	mpFX	= &getData(IR_MODEL_DATA_FXRATE);
	mpIRCurveTypes = &getData(IR_MODEL_DATA_IRCURVETYPES);

	
	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
LADataHolder&
LAMathPlainVanillaEntity::add(const LAString& name)
{
	// search Data of name
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}

// 
/*!
    @brief setUpLiborRateMap

	set digitalspread from property file
	
	@param[in] object pool
	
*/
void 
LAMathPlainVanillaEntity::setUpLiborRateMap(void) const
{
	if (!mLiborRateMap.empty())
		return;
	
	LAObjectPool& objPool = getDataInstance()->getObjectPool();
	LADate asOf = getAsOfDate().get();
	LAPriceDataDayCount dc(ACT_365_ISDA);

	const LAStringVector &ircurs = getIRCurrencys().get();
	for (unsigned int i = 0; i < ircurs.size(); i++)
	{
		LAString ccy = ircurs[i];
		ccy.toLower();
		
		const LAMathYieldCurvePro& bYieldPro = getIRCurvePro(ccy);
		const LADataMultiReference& refMarketDatas = bYieldPro.getMarketData();
		
		const unsigned int dataSize = refMarketDatas.getSize();
		if (dataSize == 0)
		{
			return;
		}
		DoubleVector termvec,ratevec;
		for (unsigned int j = 0; j < dataSize; ++j)
		{
			const LAObject& data = refMarketDatas.get(j).get();
			LAString type = dynamic_cast<const LADataString&> ((data.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			type.toUpper();
			if (type !=  "ZERORATE") //Libor only
				continue;
			
			double rate  = dynamic_cast<const LADataDouble &>(data.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
			const LAString& termStr = dynamic_cast<const LADataString &>(data.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();			
			const LADate& spotdate = dynamic_cast<const LADataDate&> ((data.getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
			const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((data.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const LAString& freq = dynamic_cast<const LADataString &>(data.getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
			const LADataBool& eom  = dynamic_cast<const LADataBool&> ((data.getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL)).get());
			LAString roll_conv("");
			if (freq == "LUNAR") roll_conv = "LUNAR";
			else if (eom) roll_conv = "EOM";
			else roll_conv = "NORMAL";

			LADate enddate = LAMathDateCalculations::getDate(spotdate, termStr, sld, &cal, true, &roll_conv);
		
			const double term = dc.getTerm(spotdate, enddate, false);
			
			termvec.push_back(term);
			ratevec.push_back(rate);
		}
		if (termvec.size() < 1)
		{
			bool isFwdFX = false;
			const LADataHolder& dh = bYieldPro.getYieldData().get().getData(IR_CALIBRATION_DATA_ISFWDFX);
			if (dh.isDefined() && !dh.isNull()) 
				isFwdFX = dynamic_cast<const LADataBool&>(dh.get()).get();
			if (!isFwdFX)
				throw LACoreInvalidData("Libor Rate Map Error",__FILE__,__LINE__);
		}
		
		LALinearInterpolation* pInter = new LALinearInterpolation();
		pInter->set(termvec, ratevec);

		mLiborRateMap.insert(std::make_pair(ircurs[i], pInter));
		mLiborGridTermMap.insert(std::make_pair(ircurs[i], termvec));
	}
}


// 
/*!
    @brief clearLiborMap
*/
void 
LAMathPlainVanillaEntity::clearLiborRateMap(void) const
{
	std::map<LAString, LAInterpolationBase *>::iterator it = mLiborRateMap.begin();
	while (it != mLiborRateMap.end())
	{
		delete it->second;
		++it;
	}
	mLiborRateMap.clear();
	mLiborGridTermMap.clear();
}

