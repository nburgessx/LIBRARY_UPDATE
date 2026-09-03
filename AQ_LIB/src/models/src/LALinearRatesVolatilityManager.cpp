
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <map>

#include "LALinearRatesVolatilityManager.h"
#include "LALinearRatesVolatility.h"
#include "LADataBasics.h"
#include "LAObject.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesModel.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LAMathVolFuncFXVannaVolga.h"

#ifdef __HAS_MIC__

#endif

#ifdef __HAS_MIC__
common_lib::StaticMutex LALinearRatesVolatilityManager::mMutex;
#endif
std::map<LAString, LALinearRatesVolatility*> LALinearRatesVolatilityManager::mVolatilityMap;
//hishida vannavolga
std::map<LAString, LALinearRatesModel*> LALinearRatesVolatilityManager::mModelMap;
LALinearRatesVolatilityManager *LALinearRatesVolatilityManager::mpInstance = 0;
//================ LALinearRatesVolatilityManager ===================================
// constructor
/*!

*/
LALinearRatesVolatilityManager::LALinearRatesVolatilityManager(void)
{
}

// destructor
/*!

*/
LALinearRatesVolatilityManager::~LALinearRatesVolatilityManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LALinearRatesVolatilityManager *
*/
LALinearRatesVolatilityManager *
LALinearRatesVolatilityManager::getInstance()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (!mpInstance)
	{
		mpInstance = new LALinearRatesVolatilityManager();
	}
	return mpInstance;
}

// 
/*!
    @brief create LALinearRatesVolatility

	@param[in] model
	@return  LALinearRatesVolatility *
*/
LALinearRatesVolatility *
LALinearRatesVolatilityManager::createPlainVanillaVolatiltyGenerator(LADataProvider* dataProvider, LAObject& object, function_t producttype, LAString productname, LAString modelname) const
{
	LAString keyname(producttype);
	keyname += "_" + modelname;

	bool isvolinput = false;
	LADataHolder* dh = &(object.getData(PRICING_DATA_VOLATILITYDIRECTINPUT,NOCHECK));
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

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	std::map<LAString, LALinearRatesVolatility*>::iterator it = mVolatilityMap.find(keyname);
	if (it == mVolatilityMap.end())
	{
		LALinearRatesVolatility* pvol = 0;
		if (isvolinput)
		{
			pvol = new LAPricePlainVolatilityFromDirectInput();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		if (isvolinputs)
		{
			pvol = new LAPricePlainVolatilityFromDirectInputOfCashlets();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		if (ispvvoluse)
		{
			pvol = new LAPricePlainVolatilityPVVolMatrixUse();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
			return pvol;
		}

		//hishida vannavolga
		bool isfxproduct = (productname.findString("fn_fx") != -1);
		if (isfxproduct)
		{
			if (BSVALUEMODEL == modelname)
			{
				if(producttype == FN_FXKNOCKOUTREBATEVALUE || 
				   producttype == FN_FXSINGLEBARRIEROPTIONVALUE ||
				   producttype == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE)
				{
					pvol = new LAPriceFXStrangleSolverATMVolatility();
					mVolatilityMap.insert(std::make_pair(keyname, pvol));
				}
				else
				{
					pvol = new LAPriceFXStrangleSolverVolatility();  
					mVolatilityMap.insert(std::make_pair(keyname, pvol));
				}
			}
			else if (VVVALUEMODEL == modelname)
			{
				pvol = new LAPriceFXVannaVolgaVolatility();
				mVolatilityMap.insert(std::make_pair(keyname, pvol));
			}
			else
			{
				throw LACoreInvalidData("Model Name Error",__FILE__,__LINE__);
			}
		}
		else if (producttype == FN_IR_CAPFLOOROPTIONVALUE)
		{
			//pvol = new LAPricePlainVolatilityFromDirectInputOfCashlets();
			pvol = new LAPriceIRSABRVolatility();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
		}
		else if (producttype == FN_IR_SWAPTIONVALUE)
		{
			pvol = new LAPriceIRSABRVolatility();
			mVolatilityMap.insert(std::make_pair(keyname, pvol));
		}
		else if (producttype == FN_IR_SWAPTIONVALUEFROMCASHFLOW)
		{
			pvol = new LAPriceIRSABRVolatility();
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


//hishida vanna volga
// 
/*!
    @brief create LALinearRatesModel

	@param[in] model
	@return  LALinearRatesModel *
*/
LALinearRatesModel *
LALinearRatesVolatilityManager::createPlainVanillaModelGenerator(LADataProvider* dataProvider, LAObject& object, function_t producttype, LAString productname, LAString modelname) const
{
	LAString keyname(modelname);

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	std::map<LAString, LALinearRatesModel*>::iterator it = mModelMap.find(keyname);
	if (it == mModelMap.end())
	{
		LALinearRatesModel* pmodel = 0;

		if (BSVALUEMODEL == modelname)
		{
			pmodel = new LAPriceBSValueModel();  
			mModelMap.insert(std::make_pair(keyname, pmodel));
		}
		else if (VVVALUEMODEL == modelname)
		{
			pmodel = new LAPriceVVValueModel();
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
LALinearRatesVolatilityManager::finalize(void)
{
	try
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		std::map<LAString, LALinearRatesVolatility*>::iterator it = mVolatilityMap.begin();
		while (it != mVolatilityMap.end())
		{
			delete it->second;
			++it;
		}
		mVolatilityMap.clear();

		//hishida vanna volga
		std::map<LAString, LALinearRatesModel*>::iterator itmodel = mModelMap.begin();
		while (itmodel != mModelMap.end())
		{
			delete itmodel->second;
			++itmodel;
		}
		mModelMap.clear();

		if (mpInstance)
		{
			delete LALinearRatesVolatilityManager::mpInstance;
			mpInstance = 0;
		}
	}
	catch(LACoreError& e)
	{
        LACoreError ex("Error at LALinearRatesVolatilityManager::finalize", __FILE__, __LINE__);
        ex += e;
		throw ex;
	}
	catch (...)
	{
        throw LACoreSystemError(__FILE__, __LINE__);
	}
}

