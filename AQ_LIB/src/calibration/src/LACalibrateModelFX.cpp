/*! @file
    @brief FX sde generator class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFX.cpp
//
//  DESCRIPTION :        FX SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LACalibrateModelFX.h"
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LARatesSpotSDE.h"
#include "LARatesLJSpotSDE.h"
#include "LARatesScalarLinearInterpolation.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include <time.h>

using namespace std;

// constructor
/*!

*/
LACalibrateModelFX::LACalibrateModelFX(void)
:LACalibrateModel()
{
}

// destructor
/*!

*/
LACalibrateModelFX::~LACalibrateModelFX(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelFX::createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(fx);
	// check LJ
	if (isLJ(fx))
	{
		return new LARatesLJSpotSDE(type);
	}
	else
	{
		return new LARatesSpotSDE(type);
	}
}

/*!
	@brief set numerarie

	note: fx sde need not set numeraire

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelFX::setNumeraire(const LAString &fx, LARatesSDEBase &sde) const
{
	(void)fx;
	(void)sde;
}

// 
/*!
    @brief set path

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFX::setOutputTemplate(const LAString &fx, LARatesSDEBase &sde) const
{
	(void)fx;
	sde.setOutputTemplate(new LARatesPathElementScalar());
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFX::setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const
{
	(void)fx;
	sde.setInterpolationMethod(new LARatesScalarLinearInterpolation());
}

// 
/*!
    @brief generate sde market data

	FX is only set volatility

	@param[in] fx ex.JPY/USD
	@param[in] dataInstance
*/
void
LACalibrateModelFX::loadModelDataAndCalibrate(const LAString &fx, LADataInstance &dataInstance, const bool isCurve, const bool isModel, const LAString & curveID, const LAString & marketName) const
{
	if (isModel)
	{
	// generate only volatility
cout << "[ Currency = " << fx << " ]" << endl;
cout << "LACalibrateModelFX generate volatility called.." << endl;
clock_t cstart = clock();
		loadVolatilityDataAndCalibrate(fx, dataInstance);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LACalibrateModelFX generate volatility end.." << endl;
cout << "-> time = " << time << endl;
	}
}


// 
/*!
    @brief get SDE data name

	@param[in] fx 
*/
LAString
LACalibrateModelFX::getSDEAttrName(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);
}



