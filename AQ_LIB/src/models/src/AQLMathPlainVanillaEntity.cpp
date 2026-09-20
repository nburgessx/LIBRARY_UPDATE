/*! @file
    @brief Source code for class to represent Market Parameter Data for Plain Vanilla Calculation.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(AQLDataDate)<BR>
			10.IR_MODEL_DATA_CORRELATIONMATRIX(AQLDataDoubleMatrix)<BR>
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathPathEntity.h"
#include "AQLMathVolatility.h"

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
#include "AQLMathVolFuncBase.h"
#include "AQLMathCorrelation.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathFXEntity.h"
#include "AQLMathFXUtility.h"

#include "AQLMatrix.h"
#include "AQLCholeskyDecompSC.h"
#include "AQLAlgorithm.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"


#include <cmath>
#include <algorithm>

#define COR "COR"

using namespace std;

/*!
    @brief default constructor

	@param[in] dataInstance pointer of AQLDataInstance object

*/
AQLMathPlainVanillaEntity::AQLMathPlainVanillaEntity(AQLDataInstance* dataInstance) :
AQLObject()
{
	setDataInstance(dataInstance);

	AQLPriceDataManager& dm = dataInstance->getDataMaster();
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

	@param[in] path AQLMathPlainVanillaEntity object

	@note 
*/
AQLMathPlainVanillaEntity::AQLMathPlainVanillaEntity(
	const AQLMathPlainVanillaEntity& evanilla) :
AQLObject(evanilla)
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
AQLMathPlainVanillaEntity::~AQLMathPlainVanillaEntity()
{
	clearLiborRateMap();
}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
AQLMathPlainVanillaEntity::getType(void) const
{
	return ENTITY_PLAINVANILLA;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
AQLMathPlainVanillaEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_PLAINVANILLA ? true : AQLObject::isTypeOf(id));
}
/*!
    @brief get basedate
	@return basedate
*/
const AQLDataDate&  
AQLMathPlainVanillaEntity::getAsOfDate(void) const
{
	return dynamic_cast<const AQLDataDate&>(mpAsOfDate->get());
}
/*!
    @brief Get basedate.The setting of basedate is also possible.
	@return basedate
*/
AQLDataDate&  
AQLMathPlainVanillaEntity::getAsOfDate(void)
{
	return dynamic_cast<AQLDataDate&>(mpAsOfDate->get());
}
/*!
    @brief get this Plain vanilla Object-name.
	@return name
*/
const AQLDataString&	
AQLMathPlainVanillaEntity::getName() const	
{
	return dynamic_cast<const AQLDataString&>(mpName->get());
}

/*!
    @brief get this Plain vanilla Object-name.
	@return name
*/
AQLDataString&	
AQLMathPlainVanillaEntity::getName() 	
{
	return dynamic_cast<AQLDataString&>(mpName->get());
}

/*!
	@brief get DayCount
	@return DayCount
*/
const AQLPriceDataDayCount&	
AQLMathPlainVanillaEntity::getDayCount(void) const
{
	return dynamic_cast<const AQLPriceDataDayCount&>(mpDC->get());
}
/*!
	@brief get DayCount.The setting of DayCount is also possible.
	@return DayCount
*/
AQLPriceDataDayCount&
AQLMathPlainVanillaEntity::getDayCount(void)
{
	return dynamic_cast<AQLPriceDataDayCount&>(mpDC->get());
}
/*!



/*!
    @brief get this Plain vanilla IR CCYS.
	@return CCYS
*/
const AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRCurrencys() const
{
	return dynamic_cast<const AQLDataStrings &>(mpIRCurs->get());
}

/*!
    @brief get this Plain vanilla IR CCYS.
	@return CCYS
*/
AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRCurrencys()
{
	return dynamic_cast<AQLDataStrings &>(mpIRCurs->get());
}

/*!
    @brief get this Plain vanilla IR Simulation CCYS.
	@return CCYS
*/
const AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRSimCurrencys() const
{
	return dynamic_cast<const AQLDataStrings &>(mpIRSimCurs->get());
}

/*!
    @brief get this Plain vanilla IR Simulation CCYS.
	@return CCYS
*/
AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRSimCurrencys()
{
	return dynamic_cast<AQLDataStrings &>(mpIRSimCurs->get());
}

/*!
    @brief get this Plain vanilla IR CurveTypes.
	@return CCYS
*/
const AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRCurveTypes() const
{
	return dynamic_cast<const AQLDataStrings &>(mpIRCurveTypes->get());
}

/*!
    @brief get this Plain vanilla IR CurveTypes.
	@return CCYS
*/
AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRCurveTypes()
{
	return dynamic_cast<AQLDataStrings &>(mpIRCurveTypes->get());
}

/*!
    @brief get this Plain vanilla FX CCYS.
	@return CCYS
*/
const AQLDataStrings&	
AQLMathPlainVanillaEntity::getFXCurrencys() const
{
	return dynamic_cast<const AQLDataStrings &>(mpFXCurs->get());
}

/*!
    @brief get this Plain vanilla FX CCYS.
	@return CCYS
*/
AQLDataStrings&	
AQLMathPlainVanillaEntity::getFXCurrencys()
{
	return dynamic_cast<AQLDataStrings &>(mpFXCurs->get());
}


// get ir curves
const AQLDataMultiReference&	
AQLMathPlainVanillaEntity::getIRCurves() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpIRCurves->get());
}
// get ir curves
AQLDataMultiReference&
AQLMathPlainVanillaEntity::getIRCurves()
{
	return dynamic_cast<AQLDataMultiReference&>(mpIRCurves->get());
}

// get ir curvepro names
const AQLDataStrings&	
AQLMathPlainVanillaEntity::getIRCurveProNames() const
{
	return dynamic_cast<const AQLDataStrings&>(mpIRCurveProNames->get());
}
// get ir curvepro names
AQLDataStrings&
AQLMathPlainVanillaEntity::getIRCurveProNames()
{
	return dynamic_cast<AQLDataStrings&>(mpIRCurveProNames->get());
}



// get ir volatilitys
const AQLDataMultiReference&	
AQLMathPlainVanillaEntity::getIRVolatilitys() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpIRVols->get());
}
// get ir volatilitys
AQLDataMultiReference&
AQLMathPlainVanillaEntity::getIRVolatilitys()
{
	return dynamic_cast<AQLDataMultiReference&>(mpIRVols->get());
}

// get fx volatilitys
const AQLDataMultiReference&	
AQLMathPlainVanillaEntity::getFXVolatilitys() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpFXVols->get());
}
// get fx volatilitys
AQLDataMultiReference&
AQLMathPlainVanillaEntity::getFXVolatilitys()
{
	return dynamic_cast<AQLDataMultiReference&>(mpFXVols->get());
}

// get fx object
const AQLDataReference&
AQLMathPlainVanillaEntity::getFXEntity() const
{
	return dynamic_cast<const AQLDataReference&>(mpFX->get());
}
// get fx object
AQLDataReference&
AQLMathPlainVanillaEntity::getFXEntity()
{
	return dynamic_cast<AQLDataReference&>(mpFX->get());
}

/*!
    @brief setup curve types

*/
void 
AQLMathPlainVanillaEntity::setUpIRCurveTypes()
{
	const unsigned int size = getIRCurveTypes().getSize();
	if (size != getIRCurves().getSize())
	{
		throw AQLCoreInvalidData("Curve size and CurveType size is not same", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < size; ++i)
	{
		AQLMathYieldCurve &curve = dynamic_cast<AQLMathYieldCurve &>(getIRCurves().get(i).get());
		curve.setCurveType(getIRCurveTypes().get()[i]);
	}
}

// get IR Curve
const AQLMathYieldCurve& 
AQLMathPlainVanillaEntity::getIRCurve(const AQLString &key) const
{
	AQLString tmpKey = key;
	const AQLStringVector& ircurs = getIRCurrencys().get();
	AQLStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw AQLCoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	const AQLMathYieldCurve &curve = dynamic_cast<AQLMathYieldCurve &>(getIRCurves().get(pos).get());
	return curve;
}

// get IR Curve
AQLMathYieldCurve&
AQLMathPlainVanillaEntity::getIRCurve(const AQLString &key)
{
	AQLString tmpKey = key;
	const AQLStringVector& ircurs = getIRCurrencys().get();
	AQLStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw AQLCoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	AQLMathYieldCurve &curve = dynamic_cast<AQLMathYieldCurve &>(getIRCurves().get(pos).get());
	return curve;
}

// get CurvePro Names
const AQLString&
AQLMathPlainVanillaEntity::getIRCurveProName(const AQLString &key) const
{
	AQLString tmpKey = key;
	const AQLStringVector& ircurs = getIRCurrencys().get();
	AQLStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
	{
		AQLString msg = "The ccy is not registered. ccy = " + key;
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	const AQLStringVector &curveProNames = getIRCurveProNames().get();
	if (curveProNames.size() <= pos)
	{
		AQLString msg = "YieldCurvePro Name is not registered. ccy = " + key;
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	return curveProNames[pos];
}

// get IR CurvePro
const AQLMathYieldCurvePro& 
AQLMathPlainVanillaEntity::getIRCurvePro(const AQLString &key) const
{
	const AQLString &name = getIRCurveProName(key);
	AQLObjectPool &objPool = getDataInstance()->getObjectPool();
	return  dynamic_cast<const AQLMathYieldCurvePro &>(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());
}

// get IR CurvePro
AQLMathYieldCurvePro&
AQLMathPlainVanillaEntity::getIRCurvePro(const AQLString &key)
{
	const AQLString &name = getIRCurveProName(key);
	AQLObjectPool &objPool = getDataInstance()->getObjectPool();
	return  dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS

// get FX Vol
AQLMathVolatility& 
AQLMathPlainVanillaEntity::getFXVol(const AQLString &key)
{
	AQLString tmpKey = key;
	const AQLStringVector& fxcurs = getFXCurrencys().get();
	
	AQLStringVector::const_iterator it = std::find(fxcurs.begin(), fxcurs.end(), tmpKey.toUpper());
	if (it == fxcurs.end())
		throw AQLCoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - fxcurs.begin());
	return dynamic_cast<AQLMathVolatility &>(getFXVolatilitys().get(pos).get());
}



// get FXVol method
const AQLFunctionBase*
AQLMathPlainVanillaEntity::getFXVolFunc(const AQLString &key)
{
	AQLMathVolatility& fxvol = getFXVol(key);
	const AQLFunctionBase* ret = fxvol.getVolatilityFunc();
	return ret;
}

// get IR Vol
AQLMathVolatility& 
AQLMathPlainVanillaEntity::getIRVol(const AQLString &key)
{
	AQLString tmpKey = key;
	const AQLStringVector& ircurs = getIRCurrencys().get();
	AQLStringVector::const_iterator it = std::find(ircurs.begin(), ircurs.end(), tmpKey.toUpper());
	if (it == ircurs.end())
		throw AQLCoreInvalidData("Error",__FILE__,__LINE__);

	unsigned int pos = static_cast<unsigned int>(it - ircurs.begin());
	return dynamic_cast<AQLMathVolatility &>(getIRVolatilitys().get(pos).get());
}

// get IRVol method
const AQLFunctionBase*
AQLMathPlainVanillaEntity::getIRVolFunc(const AQLString &key)
{
	AQLMathVolatility& irvol = getIRVol(key);
	const AQLFunctionBase* ret = irvol.getVolatilityFunc();
	return ret;
}

#endif

/*!
	@brief get correlation matrix between SDEs
	@return correlation matrix
*/
const AQLDataDoubleMatrix&
AQLMathPlainVanillaEntity::getCorrelationMatrix() const
{
	return dynamic_cast<const AQLDataDoubleMatrix&>(mpCor->get());
}
/*!
	@brief get correlation matrix between SDEs. The setting of correlation matrix is also possible. 
	@return correlation matrix
*/
AQLDataDoubleMatrix&
AQLMathPlainVanillaEntity::getCorrelationMatrix()
{
	return dynamic_cast<AQLDataDoubleMatrix&>(mpCor->get());
}

/*!
    @brief Make copy(clone) of this PlainVanilla Object object.
    @return pointer of this PlainVanilla Object object.
*/
AQLObject* 
AQLMathPlainVanillaEntity::clone() const
{
    try 
	{
    	return new AQLMathPlainVanillaEntity(*this);
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
AQLMathPlainVanillaEntity::remove(
	const AQLString& dataName)
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
	AQLObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
AQLMathPlainVanillaEntity::reset(void)
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
	@brief copy AQLMathPlainVanillaEntity
	@param[in] e copy source
	@return reference to this object
*/
AQLObject&
AQLMathPlainVanillaEntity::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	AQLObject::copy(e);
	if (!e.isTypeOf(ENTITY_PLAINVANILLA))
	{
		AQLString err = "Assignement error for AQLMathPlainVanillaEntity : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
AQLDataHolder&
AQLMathPlainVanillaEntity::add(const AQLString& name)
{
	// search Data of name
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}

// 
/*!
    @brief setUpLiborRateMap

	set digitalspread from property file
	
	@param[in] object pool
	
*/
void 
AQLMathPlainVanillaEntity::setUpLiborRateMap(void) const
{
	if (!mLiborRateMap.empty())
		return;
	
	AQLObjectPool& objPool = getDataInstance()->getObjectPool();
	AQLDate asOf = getAsOfDate().get();
	AQLPriceDataDayCount dc(ACT_365_ISDA);

	const AQLStringVector &ircurs = getIRCurrencys().get();
	for (unsigned int i = 0; i < ircurs.size(); i++)
	{
		AQLString ccy = ircurs[i];
		ccy.toLower();
		
		const AQLMathYieldCurvePro& bYieldPro = getIRCurvePro(ccy);
		const AQLDataMultiReference& refMarketDatas = bYieldPro.getMarketData();
		
		const unsigned int dataSize = refMarketDatas.getSize();
		if (dataSize == 0)
		{
			return;
		}
		DoubleVector termvec,ratevec;
		for (unsigned int j = 0; j < dataSize; ++j)
		{
			const AQLObject& data = refMarketDatas.get(j).get();
			AQLString type = dynamic_cast<const AQLDataString&> ((data.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			type.toUpper();
			if (type !=  "ZERORATE") //Libor only
				continue;
			
			double rate  = dynamic_cast<const AQLDataDouble &>(data.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
			const AQLString& termStr = dynamic_cast<const AQLDataString &>(data.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();			
			const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data.getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
			const AQLPriceDataCalendar& cal  = dynamic_cast<const AQLPriceDataCalendar&> ((data.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& sld  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const AQLString& freq = dynamic_cast<const AQLDataString &>(data.getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
			const AQLDataBool& eom  = dynamic_cast<const AQLDataBool&> ((data.getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL)).get());
			AQLString roll_conv("");
			if (freq == "LUNAR") roll_conv = "LUNAR";
			else if (eom) roll_conv = "EOM";
			else roll_conv = "NORMAL";

			AQLDate enddate = AQLDateCalculations::getDate(spotdate, termStr, sld, &cal, true, &roll_conv);
		
			const double term = dc.getTerm(spotdate, enddate, false);
			
			termvec.push_back(term);
			ratevec.push_back(rate);
		}
		if (termvec.size() < 1)
		{
			bool isFwdFX = false;
			const AQLDataHolder& dh = bYieldPro.getYieldData().get().getData(IR_CALIBRATION_DATA_ISFWDFX);
			if (dh.isDefined() && !dh.isNull()) 
				isFwdFX = dynamic_cast<const AQLDataBool&>(dh.get()).get();
			if (!isFwdFX)
				throw AQLCoreInvalidData("Libor Rate Map Error",__FILE__,__LINE__);
		}
		
		AQLLinearInterpolation* pInter = new AQLLinearInterpolation();
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
AQLMathPlainVanillaEntity::clearLiborRateMap(void) const
{
	std::map<AQLString, AQLInterpolationBase *>::iterator it = mLiborRateMap.begin();
	while (it != mLiborRateMap.end())
	{
		delete it->second;
		++it;
	}
	mLiborRateMap.clear();
	mLiborGridTermMap.clear();
}

