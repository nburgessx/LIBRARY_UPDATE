/*! @file
    @brief FX sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelFX.cpp
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


#include "AQLCalibrateModelFX.h"
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLRatesSpotSDE.h"
#include "AQLRatesLJSpotSDE.h"
#include "AQLRatesScalarLinearInterpolation.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include <time.h>

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelFX::AQLCalibrateModelFX(void)
:AQLCalibrateModel()
{
}

// destructor
/*!

*/
AQLCalibrateModelFX::~AQLCalibrateModelFX(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelFX::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(fx);
	// check LJ
	if (isLJ(fx))
	{
		return new AQLRatesLJSpotSDE(type);
	}
	else
	{
		return new AQLRatesSpotSDE(type);
	}
}

/*!
	@brief set numerarie

	note: fx sde need not set numeraire

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelFX::setNumeraire(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFX::setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	(void)fx;
	sde.setOutputTemplate(new AQLRatesPathElementScalar());
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] fx
	@param[out] sde
*/
void
AQLCalibrateModelFX::setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	(void)fx;
	sde.setInterpolationMethod(new AQLRatesScalarLinearInterpolation());
}

// 
/*!
    @brief generate sde market data

	FX is only set volatility

	@param[in] fx ex.JPY/USD
	@param[in] dataInstance
*/
void
AQLCalibrateModelFX::loadModelDataAndCalibrate(const AQLString &fx, AQLDataInstance &dataInstance, const bool isCurve, const bool isModel, const AQLString & curveID, const AQLString & marketName) const
{
	if (isModel)
	{
	// generate only volatility
cout << "[ Currency = " << fx << " ]" << endl;
cout << "AQLCalibrateModelFX generate volatility called.." << endl;
clock_t cstart = clock();
		loadVolatilityDataAndCalibrate(fx, dataInstance);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "AQLCalibrateModelFX generate volatility end.." << endl;
cout << "-> time = " << time << endl;
	}
}


// 
/*!
    @brief get SDE data name

	@param[in] fx 
*/
AQLString
AQLCalibrateModelFX::getSDEAttrName(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);
}



