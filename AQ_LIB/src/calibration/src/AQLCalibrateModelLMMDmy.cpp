/*! @file
    @brief LMM Dmy sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelLMMDmy.cpp
//
//  DESCRIPTION :        LMM Dmy SDE generator 
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
//#define __DATAOUT__

#include "AQLCalibrateModelLMMDmy.h"
#include "AQLDataInstance.h"
#include "AQLRatesDmySDE.h"
#include "AQLRatesCurveLogLinearInterpolationDmy.h"

using namespace std;

// constructor
/*!
	@param[in] baseCurrency

*/
AQLCalibrateModelLMMDmy::AQLCalibrateModelLMMDmy(const AQLString &baseCurrency)
: AQLCalibrateModelLMM(baseCurrency)
{
}

// destructor
/*!

*/
AQLCalibrateModelLMMDmy::~AQLCalibrateModelLMMDmy(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in]  currency
	@param[in]  dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelLMMDmy::createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	currency;
	dataInstance;
	return new AQLRatesDmySDE();
}


// 
/*!
    @brief curve create log linear interpolation

	@return AQLRatesCurveLogLinearInterpolation *
*/
AQLRatesCurveLogLinearInterpolation *
AQLCalibrateModelLMMDmy::createCurveLogLinearInterpolation() const
{
	return new AQLRatesCurveLogLinearInterpolationDmy();
}
