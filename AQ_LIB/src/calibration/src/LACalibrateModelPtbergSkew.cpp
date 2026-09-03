/*! @file
    @brief Ptberg SDE generator class. The SDE includes skew
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtbergSkew.cpp
//
//  DESCRIPTION :       Ptberg SDE generator class. The SDE includes skew 
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


#include "LACalibrateModelPtbergSkew.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelPtbergSkew::LACalibrateModelPtbergSkew()
: LACalibrateModelPtberg()
{
}

// destructor
/*!

*/
LACalibrateModelPtbergSkew::~LACalibrateModelPtbergSkew(void)
{
}


/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelPtbergSkew::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	// TO DO
	LACalibrateModelPtberg::setDrift(fx, sde);
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelPtbergSkew::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	// TO DO
	LACalibrateModelPtberg::setIntegralFunction(fx, sde);
}


// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelPtbergSkew::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	// TO DO
	LACalibrateModelPtberg::setUpVolFunc(fx, vol, dataInstance);
}