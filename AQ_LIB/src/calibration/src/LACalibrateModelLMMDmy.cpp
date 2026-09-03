/*! @file
    @brief LMM Dmy sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelLMMDmy.cpp
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

#include "LACalibrateModelLMMDmy.h"
#include "AQLDataInstance.h"
#include "LARatesDmySDE.h"
#include "LARatesCurveLogLinearInterpolationDmy.h"

using namespace std;

// constructor
/*!
	@param[in] baseCurrency

*/
LACalibrateModelLMMDmy::LACalibrateModelLMMDmy(const AQLString &baseCurrency)
: LACalibrateModelLMM(baseCurrency)
{
}

// destructor
/*!

*/
LACalibrateModelLMMDmy::~LACalibrateModelLMMDmy(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in]  currency
	@param[in]  dataInstance
*/
LARatesSDEBase *
LACalibrateModelLMMDmy::createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	currency;
	dataInstance;
	return new LARatesDmySDE();
}


// 
/*!
    @brief curve create log linear interpolation

	@return LARatesCurveLogLinearInterpolation *
*/
LARatesCurveLogLinearInterpolation *
LACalibrateModelLMMDmy::createCurveLogLinearInterpolation() const
{
	return new LARatesCurveLogLinearInterpolationDmy();
}
