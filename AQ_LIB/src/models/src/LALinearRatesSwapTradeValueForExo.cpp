/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include <set>
#include "LALinearRatesSwapTradeValueForExo.h"
#include "LAPricePortfolioValue.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDate.h"
#include "AQLDataValuation.h"
#include "AQLDataProcedure.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectHolder.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLMathValuableEntity.h"
#include "LAMathIndexEntity.h"
#include "AQLPriceDataFunction.h"
#include "LAMathFXEntity.h"
#include "AQLAlgorithm.h"
#include "LAPricePayOff.h"
#include "LAPricePayOffTool.h"
#include "LAPriceAccruedInterest.h"
#include "LAMathPlainVanillaEntity.h"
#include "AQLBasic.h"
#include "LAMathYieldCurve.h"
#include "AQLPriceDataInterpolation.h"

#include "LAPriceCouponTool.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAMathYieldCurvePro.h"
#include "AQLMatrix.h"
#include "AQLLinearFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "LAMathPathEntity.h"

#define VOL "VOL"
#define VNL "VNL"
using namespace std;


/*!
    @brief constructor
	@param[in] pacc pointer to accured interest calculation class
*/
LALinearRatesSwapTradeValueForExo::LALinearRatesSwapTradeValueForExo(LAPriceAccruedInterest* pacc) :
LALinearRatesSwapTradeValue(pacc)
{
}
/*!
    @brief copy constructor
	@param[in] v copy source 
*/
LALinearRatesSwapTradeValueForExo::LALinearRatesSwapTradeValueForExo(const LALinearRatesSwapTradeValueForExo& v)
: LALinearRatesSwapTradeValue(v)
{
}
/*!
    @brief destructor

*/
LALinearRatesSwapTradeValueForExo::~LALinearRatesSwapTradeValueForExo()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LALinearRatesSwapTradeValueForExo::isTypeOf(function_t id) const
{
	return (id == FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO ? true : LAPriceTradeValue::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
AQLCoreFunctionBase*
LALinearRatesSwapTradeValueForExo::clone() const
{
    try 
	{
  		return new LALinearRatesSwapTradeValueForExo(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
LALinearRatesSwapTradeValueForExo::getType() const
{
	return FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO;
}

/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LALinearRatesSwapTradeValueForExo::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	AQLDataInstance *dataInstance = object.getDataInstance();
	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLDataReference &refP = dynamic_cast<AQLDataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	LAMathPathEntity &path = dynamic_cast<LAMathPathEntity &>(refP.get().get());
	const AQLString &pathName = dynamic_cast<const AQLDataString &>(path.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const AQLString pathName_v = pathName + "_" + AQLString(VNL);

	// create vanilla path object
	LAMathPlainVanillaEntity* pPath_v = NULL;
	AQLObjectHolder objHolder = objPool.getObject(pathName_v, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pPath_v = new LAMathPlainVanillaEntity(dataInstance);
		objPool.set(pathName_v, pPath_v);
	
	}
	else
	{
		pPath_v = &dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(pathName_v).get());
		pPath_v->reset();
	}

	AQLString curveNames = "";
	LAMathFXEntity *pFX = NULL;
	AQLString ccyVolName = "";
	AQLString fxName = "";
	AQLString fxVolName = "";
	const AQLDataMultiReference &initialRef = path.getInitialValues();
	const AQLDataStrings& sdeNames = path.getSDEAttrNames();
	AQLStringVector ccys, fxs;
	for (unsigned int i = 0; i < initialRef.getSize(); ++i)
	{
		const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdeNames[i]).get());
		const AQLString &ccy = sde.getCurrency();
		
		if (ccy.findString('/') >= 0)
		{
			fxs.push_back(ccy);
			pFX = &dynamic_cast<LAMathFXEntity &>(initialRef.get(i).get());
			fxName = dynamic_cast<const AQLDataString &>(initialRef.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			fxVolName += AQLString(VOL) + "_" + sdeNames[i] + ":";
		}
		else
		{
			ccys.push_back(ccy);
			pPath_v->getIRCurveTypes().push_back(STD);
			const AQLString &name = dynamic_cast<const AQLDataString &>(initialRef.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			curveNames += name + ":";
			ccyVolName += AQLString(VOL) + "_" + sdeNames[i] + ":";
		}
	}
	AQLStringVector ccys_simu;
	const AQLDataStrings& sdeNames_simu = path.getSimulationSDEAttrNames();
	for (unsigned int i = 0; i < sdeNames_simu.getSize(); ++i)
	{
		const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdeNames_simu[i]).get());
		const AQLString &ccy = sde.getCurrency();
		if (ccy.findString('/') >= 0)
		{
			continue;
		}
		else
		{
			ccys_simu.push_back(ccy);
		}
	}
	// setup vanilla path object
	pPath_v->getAsOfDate() = path.getAsOfDate();
	pPath_v->getDayCount() = path.getDayCount();
	pPath_v->getIRCurrencys().set(ccys);
	pPath_v->getIRSimCurrencys().set(ccys_simu);
	ccyVolName = ccyVolName.subString(0, ccyVolName.size() - 2);
	pPath_v->getIRVolatilitys().convertFromString(ccyVolName);
	if (!fxs.empty())
	{
		pPath_v->getFXEntity().convertFromString(fxName);
		pPath_v->getFXCurrencys().set(fxs);
		fxVolName = fxVolName.subString(0, fxVolName.size() - 2);
		pPath_v->getFXVolatilitys().convertFromString(fxVolName);
	}
	curveNames = curveNames.subString(0, curveNames.size() - 2);
	pPath_v->getIRCurves().convertFromString(curveNames);
	pPath_v->setUpIRCurveTypes();
	pPath_v->getIRCurveProNames().set(path.getIRCurveProNames().get());
	// setup path object reference
	AQLCoreReferencePool &rp = dataInstance->getReferencePool();
	AQLObjectHolder &ehPah = rp.getReference(pathName);
	if (!ehPah.isDefined())
	{
		throw AQLCoreInvalidData("LAMathPathEntity reference pool is not set.", __FILE__, __LINE__);
	}
	pPath_v->update();
	ehPah.setEntity(pPath_v, false);
	// setup fx object reference
	if (pFX)
	{
		AQLDataHolder &dh = object.getData(PRICING_DATA_FXRATE, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			const AQLString &fxName_orig = dynamic_cast<AQLDataReference &>(dh.get()).get().getName();
			AQLObjectHolder &ehFX = rp.getReference(fxName_orig);
			if (!ehFX.isDefined())
			{
				throw AQLCoreInvalidData("LAMathFXEntity type USEMODEL reference pool is not set.", __FILE__, __LINE__);
			}
			pFX->update();
			ehFX.setEntity(pFX, false);

		}
	}
	// value
	double dirtyprice1 = LALinearRatesSwapTradeValue::value(basedate, object, att);

	// setup object for exo format
	object.remove(PRICING_DATA_PVCURRENCY);
	const LALinearRatesSwapTradeValueDataProvider &dataProvider = dynamic_cast<const LALinearRatesSwapTradeValueDataProvider&>(att.getDataProvider());
	object.add(PRICING_DATA_PVCURRENCY, new AQLDataString(dataProvider.basecur));

	object.remove(PRICING_DATA_ISCALCVANILLA);
	object.add(PRICING_DATA_ISCALCVANILLA, new AQLDataBool(true));

	path.update();
	ehPah.setEntity(&path, false);
	if (pFX)
	{
		AQLDataHolder &dh = object.getData(PRICING_DATA_FXRATE, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			AQLDataReference &refFX = dynamic_cast<AQLDataReference &>(dh.get());
			const AQLString &fxName_orig = refFX.get().getName();
			LAMathFXEntity &fx = dynamic_cast<LAMathFXEntity &>(refFX.get().get());
			AQLObjectHolder &ehFX = rp.getReference(fxName_orig);
			fx.update();
			ehFX.setEntity(&fx, false);
		}
	}

	return dirtyprice1;
}