// $Id: LACalibrateModelVolFactor.cpp,v 1.4 2016/10/11 10:51:51 fukuitak Exp $
/*! @file
    @brief Volatility factor sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelVolFactor.cpp
//
//  DESCRIPTION :        Volatility factor SDE generator 
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


#include "LACalibrateModelVolFactor.h"
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
LACalibrateModelVolFactor::LACalibrateModelVolFactor(void)
:LACalibrateModel()
{
}

// destructor
/*!

*/
LACalibrateModelVolFactor::~LACalibrateModelVolFactor(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] vola
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelVolFactor::createSDEInstance(const LAString &vola, LADataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(vola);
	// check LJ
	if (isLJ(vola))
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

	note: vola sde need not set numeraire

	@param[in] vola
	@param[out] sde

*/
void
LACalibrateModelVolFactor::setNumeraire(const LAString &vola, LARatesSDEBase &sde) const
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
LACalibrateModelVolFactor::setOutputTemplate(const LAString &vola, LARatesSDEBase &sde) const
{
	(void)vola;
	sde.setOutputTemplate(new LARatesPathElementScalar());
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] vola
	@param[out] sde
*/
void
LACalibrateModelVolFactor::setInterpolationMethod(const LAString &vola, LARatesSDEBase &sde) const
{
	(void)vola;
	sde.setInterpolationMethod(new LARatesScalarLinearInterpolation());
}

// 
/*!
    @brief generate sde market data

	FX is only set volatility

	@param[in] vola ex.JPY/USD
	@param[in] dataInstance
*/
void
LACalibrateModelVolFactor::loadModelDataAndCalibrate(const LAString &vola, LADataInstance &dataInstance, const bool isCurve, const bool isModel, const LAString & curveID, const LAString & marketName ) const
{
	if (isModel)
	{
		LAString spot = getSpotIndex(vola);
	// generate only volatility
cout << "[ Currency = " << spot << " ]" << endl;
cout << "LACalibrateModelVolFactor generate volatility called.." << endl;
clock_t cstart = clock();
		loadVolatilityDataAndCalibrate(vola, dataInstance);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LACalibrateModelVolFactor generate volatility end.." << endl;
cout << "-> time = " << time << endl;
	}
}


// 
/*!
    @brief get SDE data name

	@param[in] vola 
*/
LAString
LACalibrateModelVolFactor::getSDEAttrName(const LAString &vola) const
{
	LAString spotKey = getSpotIndex(vola).toLower();
	return mpStaticData->getStaticData(spotKey + ".volatility" + STATIC_DATA_FX_KEY_SDE_NAME);
}


///*!
//    @brief return is cancel for funding 
//
//	@param[in] ccy
//	@return bool 
//*/
//bool
//LACalibrateModelVolFactor::isCancelForFunding(const LAString &vola) const
//{
//	LAString ccy(getSpotIndex(vola));
//	return LACalibrateModel::isCancelForFunding(ccy);
//}
//
// 
/*!
    @brief get spot index name function

	@param[in] vola
*/
LAString
LACalibrateModelVolFactor::getSpotIndex(const LAString &vola) const
{
	LAString spot(vola);
	int pos = vola.findString(POSTFIX_VOL);
	if (pos > 0)
	{
		spot.remove(pos, LAString(POSTFIX_VOL).size());
	}
	return spot;
}




