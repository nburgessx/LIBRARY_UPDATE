
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <map>

#include "AQLLinearRatesVolatilityManager.h"
#include "AQLLinearRatesVolatility.h"
#include "AQLDataBasics.h"
#include "AQLObject.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLLinearRatesModel.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLMathVolFuncFXVannaVolga.h"


std::map<AQLString, AQLLinearRatesVolatility*> AQLLinearRatesVolatilityManager::mVolatilityMap;
// vanna-volga
std::map<AQLString, AQLLinearRatesModel*> AQLLinearRatesVolatilityManager::mModelMap;
AQLLinearRatesVolatilityManager *AQLLinearRatesVolatilityManager::mpInstance = 0;
//================ AQLLinearRatesVolatilityManager ===================================
// constructor
/*!

*/
AQLLinearRatesVolatilityManager::AQLLinearRatesVolatilityManager(void)
{
}

// destructor
/*!

*/
AQLLinearRatesVolatilityManager::~AQLLinearRatesVolatilityManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLLinearRatesVolatilityManager *
*/
AQLLinearRatesVolatilityManager *
AQLLinearRatesVolatilityManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLLinearRatesVolatilityManager();
	}
	return mpInstance;
}

// 
/*!
    @brief create AQLLinearRatesVolatility

	@param[in] model
	@return  AQLLinearRatesVolatility *
*/
AQLLinearRatesVolatility *
AQLLinearRatesVolatilityManager::createPlainVanillaVolatiltyGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const
{
	AQLString keyname(producttype);
	keyname += "_" + modelname;

	bool isvolinput = false;
	AQLDataHolder* dh = &(object.getData(PRICING_DATA_VOLATILITYDIRECTINPUT,NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		keyname += "_VOLINPUT";
		isvolinput = true;
	}

	bool isvolinputs = false;
	dh = &(object.getData(PRICING_DATA_VOLATILITYDIRECTINPUTS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		keyname += "_VOLINPUTS";
		isvolinputs = true;
	}

	bool ispvvoluse = false;
	dh = &(object.getData(PRICING_DATA_PVVOLMATRIX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		keyname += "_PVVolMatrix";
		ispvvoluse = true;
	}

	std::map<AQLString, AQLLinearRatesVolatility*>::iterator it = mVolatilityMap.find(keyname);
	if (it == mVolatilityMap.end())
	{
		AQLLinearRatesVolatility* pvol = 0;
		if (isvolinput)
		{
			pvol = new AQLPricePlainVolatilityFromDirectInput();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		if (isvolinputs)
		{
			pvol = new AQLPricePlainVolatilityFromDirectInputOfCashlets();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		if (ispvvoluse)
		{
			pvol = new AQLPricePlainVolatilityPVVolMatrixUse();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		// vanna-volga
		bool isfxproduct = (productname.findString("fn_fx") != -1);
		if (isfxproduct)
		{
			if (BSVALUEMODEL == modelname)
			{
				if(producttype == FN_FXKNOCKOUTREBATEVALUE || 
				   producttype == FN_FXSINGLEBARRIEROPTIONVALUE ||
				   producttype == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE)
				{
					pvol = new AQLPriceFXStrangleSolverATMVolatility();
					mVolatilityMap.insert(std::make_pair(keyname, pvol));
				}
				else
				{
					pvol = new AQLPriceFXStrangleSolverVolatility();  
					mVolatilityMap.insert(std::make_pair(keyname, pvol));
				}
			}
			else if (VVVALUEMODEL == modelname)
			{
				pvol = new AQLPriceFXVannaVolgaVolatility();
				mVolatilityMap.insert(std::make_pair(keyname, pvol));
			}
			else
			{
				throw AQLCoreInvalidData("Model Name Error",__FILE__,__LINE__);
			}
		}
		else if (producttype == FN_IR_CAPFLOOROPTIONVALUE)
		{
			//pvol = new AQLPricePlainVolatilityFromDirectInputOfCashlets();
			pvol = new AQLPriceIRSABRVolatility();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
		}
		else if (producttype == FN_IR_SWAPTIONVALUE)
		{
			pvol = new AQLPriceIRSABRVolatility();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
		}
		else if (producttype == FN_IR_SWAPTIONVALUEFROMCASHFLOW)
		{
			pvol = new AQLPriceIRSABRVolatility();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
		}
		else 
		{
			return 0;
		}

		return pvol;
	}
	else
	{
		return it->second;
	}
	
	return 0;
}


// vanna-volga
// 
/*!
    @brief create AQLLinearRatesModel

	@param[in] model
	@return  AQLLinearRatesModel *
*/
AQLLinearRatesModel *
AQLLinearRatesVolatilityManager::createPlainVanillaModelGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const
{
	AQLString keyname(modelname);

	std::map<AQLString, AQLLinearRatesModel*>::iterator it = mModelMap.find(keyname);
	if (it == mModelMap.end())
	{
		AQLLinearRatesModel* pmodel = 0;

		if (BSVALUEMODEL == modelname)
		{
			pmodel = new AQLPriceBSValueModel();  
			mModelMap.insert(std::make_pair(keyname, pmodel));
		}
		else if (VVVALUEMODEL == modelname)
		{
			pmodel = new AQLPriceVVValueModel();
			mModelMap.insert(std::make_pair(keyname, pmodel));
		}
		else
		{
			return 0;
		}

		return pmodel;
	}
	else
	{
		return it->second;
	}
	
	return 0;
}

void 
AQLLinearRatesVolatilityManager::finalize(void)
{
	try
	{
		std::map<AQLString, AQLLinearRatesVolatility*>::iterator it = mVolatilityMap.begin();
		while (it != mVolatilityMap.end())
		{
			delete it->second;
			++it;
		}
		mVolatilityMap.clear();

		// vanna-volga
		std::map<AQLString, AQLLinearRatesModel*>::iterator itmodel = mModelMap.begin();
		while (itmodel != mModelMap.end())
		{
			delete itmodel->second;
			++itmodel;
		}
		mModelMap.clear();

		if (mpInstance)
		{
			delete AQLLinearRatesVolatilityManager::mpInstance;
			mpInstance = 0;
		}
	}
	catch(AQLCoreError& e)
	{
        AQLCoreError ex("Error at AQLLinearRatesVolatilityManager::finalize", __FILE__, __LINE__);
        ex += e;
		throw ex;
	}
	catch (...)
	{
        throw AQLCoreSystemError(__FILE__, __LINE__);
	}
}

