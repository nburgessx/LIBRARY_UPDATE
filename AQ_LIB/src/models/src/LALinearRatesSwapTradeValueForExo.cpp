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
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADate.h"
#include "LADataValuation.h"
#include "LADataProcedure.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObjectHolder.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAMathValuableEntity.h"
#include "LAMathIndexEntity.h"
#include "LAPriceDataFunction.h"
#include "LAMathFXEntity.h"
#include "LAAlgorithm.h"
#include "LAPricePayOff.h"
#include "LAPricePayOffTool.h"
#include "LAPriceAccruedInterest.h"
#include "LAMathPlainVanillaEntity.h"
#include "LABasic.h"
#include "LAMathYieldCurve.h"
#include "LAPriceDataInterpolation.h"

#include "LAPriceCouponTool.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAMathYieldCurvePro.h"
#include "LAMatrix.h"
#include "LALinearFunc.h"
#include "LALinearInterpolation.h"
#include "LAPriceDataSlidingRule.h"
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
LACoreFunctionBase*
LALinearRatesSwapTradeValueForExo::clone() const
{
    try 
	{
  		return new LALinearRatesSwapTradeValueForExo(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LALinearRatesSwapTradeValueForExo::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	LADataInstance *dataInstance = object.getDataInstance();
	LAObjectPool &objPool = dataInstance->getObjectPool();
	LADataReference &refP = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	LAMathPathEntity &path = dynamic_cast<LAMathPathEntity &>(refP.get().get());
	const LAString &pathName = dynamic_cast<const LADataString &>(path.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString pathName_v = pathName + "_" + LAString(VNL);

	// create vanilla path object
	LAMathPlainVanillaEntity* pPath_v = NULL;
	LAObjectHolder objHolder = objPool.getObject(pathName_v, ENCHKTYPE_NOCHECK);
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

	LAString curveNames = "";
	LAMathFXEntity *pFX = NULL;
	LAString ccyVolName = "";
	LAString fxName = "";
	LAString fxVolName = "";
	const LADataMultiReference &initialRef = path.getInitialValues();
	const LADataStrings& sdeNames = path.getSDEAttrNames();
	LAStringVector ccys, fxs;
	for (unsigned int i = 0; i < initialRef.getSize(); ++i)
	{
		const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdeNames[i]).get());
		const LAString &ccy = sde.getCurrency();
		
		if (ccy.findString('/') >= 0)
		{
			fxs.push_back(ccy);
			pFX = &dynamic_cast<LAMathFXEntity &>(initialRef.get(i).get());
			fxName = dynamic_cast<const LADataString &>(initialRef.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			fxVolName += LAString(VOL) + "_" + sdeNames[i] + ":";
		}
		else
		{
			ccys.push_back(ccy);
			pPath_v->getIRCurveTypes().push_back(STD);
			const LAString &name = dynamic_cast<const LADataString &>(initialRef.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			curveNames += name + ":";
			ccyVolName += LAString(VOL) + "_" + sdeNames[i] + ":";
		}
	}
	LAStringVector ccys_simu;
	const LADataStrings& sdeNames_simu = path.getSimulationSDEAttrNames();
	for (unsigned int i = 0; i < sdeNames_simu.getSize(); ++i)
	{
		const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdeNames_simu[i]).get());
		const LAString &ccy = sde.getCurrency();
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
	LACoreReferencePool &rp = dataInstance->getReferencePool();
	LAObjectHolder &ehPah = rp.getReference(pathName);
	if (!ehPah.isDefined())
	{
		throw LACoreInvalidData("LAMathPathEntity reference pool is not set.", __FILE__, __LINE__);
	}
	pPath_v->update();
	ehPah.setEntity(pPath_v, false);
	// setup fx object reference
	if (pFX)
	{
		LADataHolder &dh = object.getData(PRICING_DATA_FXRATE, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			const LAString &fxName_orig = dynamic_cast<LADataReference &>(dh.get()).get().getName();
			LAObjectHolder &ehFX = rp.getReference(fxName_orig);
			if (!ehFX.isDefined())
			{
				throw LACoreInvalidData("LAMathFXEntity type USEMODEL reference pool is not set.", __FILE__, __LINE__);
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
	object.add(PRICING_DATA_PVCURRENCY, new LADataString(dataProvider.basecur));

	object.remove(PRICING_DATA_ISCALCVANILLA);
	object.add(PRICING_DATA_ISCALCVANILLA, new LADataBool(true));

	path.update();
	ehPah.setEntity(&path, false);
	if (pFX)
	{
		LADataHolder &dh = object.getData(PRICING_DATA_FXRATE, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			LADataReference &refFX = dynamic_cast<LADataReference &>(dh.get());
			const LAString &fxName_orig = refFX.get().getName();
			LAMathFXEntity &fx = dynamic_cast<LAMathFXEntity &>(refFX.get().get());
			LAObjectHolder &ehFX = rp.getReference(fxName_orig);
			fx.update();
			ehFX.setEntity(&fx, false);
		}
	}

	return dirtyprice1;
}