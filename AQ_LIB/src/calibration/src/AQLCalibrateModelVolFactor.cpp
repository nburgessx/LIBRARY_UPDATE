// $Id: AQLCalibrateModelVolFactor.cpp,v 1.4 2016/10/11 10:51:51 fukuitak Exp $
/*! @file
    @brief Volatility factor sde generator class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateModelVolFactor.h"
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
AQLCalibrateModelVolFactor::AQLCalibrateModelVolFactor(void)
:AQLCalibrateModel()
{
}

// destructor
/*!

*/
AQLCalibrateModelVolFactor::~AQLCalibrateModelVolFactor(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] vola
	@param[in] dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelVolFactor::createSDEInstance(const AQLString &vola, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(vola);
	// check LJ
	if (isLJ(vola))
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

	note: vola sde need not set numeraire

	@param[in] vola
	@param[out] sde

*/
void
AQLCalibrateModelVolFactor::setNumeraire(const AQLString &vola, AQLRatesSDEBase &sde) const
{
	(void)vola;
	(void)sde;
}

// 
/*!
    @brief set path

	@param[in] vola
	@param[out] sde
*/
void
AQLCalibrateModelVolFactor::setOutputTemplate(const AQLString &vola, AQLRatesSDEBase &sde) const
{
	(void)vola;
	sde.setOutputTemplate(new AQLRatesPathElementScalar());
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] vola
	@param[out] sde
*/
void
AQLCalibrateModelVolFactor::setInterpolationMethod(const AQLString &vola, AQLRatesSDEBase &sde) const
{
	(void)vola;
	sde.setInterpolationMethod(new AQLRatesScalarLinearInterpolation());
}

// 
/*!
    @brief generate sde market data

	FX is only set volatility

	@param[in] vola ex.JPY/USD
	@param[in] dataInstance
*/
void
AQLCalibrateModelVolFactor::loadModelDataAndCalibrate(const AQLString &vola, AQLDataInstance &dataInstance, const bool isCurve, const bool isModel, const AQLString & curveID, const AQLString & marketName ) const
{
	if (isModel)
	{
		AQLString spot = getSpotIndex(vola);
	// generate only volatility
cout << "[ Currency = " << spot << " ]" << endl;
cout << "AQLCalibrateModelVolFactor generate volatility called.." << endl;
clock_t cstart = clock();
		loadVolatilityDataAndCalibrate(vola, dataInstance);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "AQLCalibrateModelVolFactor generate volatility end.." << endl;
cout << "-> time = " << time << endl;
	}
}


// 
/*!
    @brief get SDE data name

	@param[in] vola 
*/
AQLString
AQLCalibrateModelVolFactor::getSDEAttrName(const AQLString &vola) const
{
	AQLString spotKey = getSpotIndex(vola).toLower();
	return mpStaticData->getStaticData(spotKey + ".volatility" + STATIC_DATA_FX_KEY_SDE_NAME);
}


///*!
//    @brief return is cancel for funding 
//
//	@param[in] ccy
//	@return bool 
//*/
//bool
//AQLCalibrateModelVolFactor::isCancelForFunding(const AQLString &vola) const
//{
//	AQLString ccy(getSpotIndex(vola));
//	return AQLCalibrateModel::isCancelForFunding(ccy);
//}
//
// 
/*!
    @brief get spot index name function

	@param[in] vola
*/
AQLString
AQLCalibrateModelVolFactor::getSpotIndex(const AQLString &vola) const
{
	AQLString spot(vola);
	int pos = vola.findString(POSTFIX_VOL);
	if (pos > 0)
	{
		spot.remove(pos, AQLString(POSTFIX_VOL).size());
	}
	return spot;
}




