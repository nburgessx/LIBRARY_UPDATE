/*! @file
    @brief Source code of HW sde integral class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesHWIntegral.cpp
//
//  SYNOPSIS    :       AQLRatesHWIntegral
//  DESCRIPTION :       Source code of Euler-Maruyama sde integral class
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesHWIntegral.h"
#include "AQLMathVolFuncHW.h"
#include "AQLPriceDriftHW.h"
#include "AQLMathVolFuncBase.h"
#include "AQLBasic.h"

using namespace std;
//================ AQLRatesHWIntegral ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
AQLRatesHWIntegral::AQLRatesHWIntegral(SDEINTEGRAL_TYPE type)
: 
AQLRatesSDEIntegralBase(type),
mpHWtool(0),
mE_cache( new map<double, double>() ),
mCumulatedVal(0.0), 
is_cloned(false)
{
}
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
AQLRatesHWIntegral::AQLRatesHWIntegral(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName)
: 
AQLRatesSDEIntegralBase(type, sdeAttrName),
mpHWtool(0),
mE_cache( new map<double, double>() ),
mCumulatedVal(0.0), 
is_cloned(false)
{
}
/*!
	@brief copy constructor
*/
AQLRatesHWIntegral::AQLRatesHWIntegral(const AQLRatesHWIntegral& v) 
:
AQLRatesSDEIntegralBase(v),
mCumulatedVal(v.mCumulatedVal),
mVar(v.mVar),
mpHWtool(v.mpHWtool),
mE_cache( new map<double, double>() ),
is_cloned(true)
{
}

/*!
	@brief destructor
*/
AQLRatesHWIntegral::~AQLRatesHWIntegral() 
{
	delete mE_cache; 
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesHWIntegral::clone() const
{
    try 
	{
		return new AQLRatesHWIntegral(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesHWIntegral::isTypeOf(function_t id) const
{
	return (id==FN_HWINTEGRAL? true : AQLRatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesHWIntegral::getType() const
{
	return FN_HWINTEGRAL;
}

//void
//AQLRatesHWIntegral::setUpInitialVal(const AQLRatesPathElementBase& curve0, const SCALARARRAY& rate0)
//{ 
//    mpCurve0 = dynamic_cast<const AQLRatesPathElementHW1FCurve*>(&curve0);
//	mCumulatedVal = rate0.front();
//}

void
AQLRatesHWIntegral::setUpInitialVal(const SCALARARRAY& rate0)
{ 
	mCumulatedVal = rate0.front();
}

void
AQLRatesHWIntegral::setUp(const AQLMathPathEntity& path)
{
	const AQLDataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	const AQLMathAttrSDE* pattrsde = &dynamic_cast<const AQLMathAttrSDE&>(dh->get());
	const std::vector<std::vector<AQLFunctionBase* > > mVolatility = pattrsde->getSDE().getVolatility();

	const AQLMathVolFuncHW* pvolHW=0;
	//in case of calculating vega
	if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCHW))
		pvolHW = dynamic_cast<AQLMathVolFuncHW*>(mVolatility[0][0]);
	else//otherwise
	{
		const AQLFunctionBase* pbase = dynamic_cast<AQLMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
		pvolHW = dynamic_cast<const AQLMathVolFuncHW*>(pbase);
	}
	mpHWtool = pvolHW->getHWFuncTool();

	delete mE_cache;
	mE_cache = new map<double, double>();
}

/*!
    @brief excecute integral
    @param[in] ts starttime
    @param[in] te endtime
    @param[in] drift drift
    @param[in] vol volatility
    @param[in] bm brownian motion
    @param[in, out] x_in_out input and output 
    @param[in] varnum number of input(output)
*/
void
AQLRatesHWIntegral::integral(double ts, double te, 
							vector<AQLFunctionBase*>::const_iterator drift,										
							vector<vector<AQLFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
    if ( te < ts )
    {
        throw AQLCoreInvalidData("ts < te", __FILE__, __LINE__);
    }

    mVar.resize(varnum + 2);
    mVar[0] = ts;
    mVar[1] = te;
    mVar[2] = *x_in_out;
    mCumulatedVal += (*drift++)->operator()(mVar) + (*vol)[0]->operator()(mVar) * (*bm++);
	(*x_in_out) = mCumulatedVal / E(te);

}

/*!
    @brief E function
	@param[in] T
*/
double AQLRatesHWIntegral::E( double T ) const
{
    if((*mE_cache)[T] == 0.0)
	{
		(*mE_cache)[T] = mpHWtool->ExpIntegralMR(T);
	}
	return (*mE_cache)[T];
}


