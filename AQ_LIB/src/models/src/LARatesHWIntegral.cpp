/*! @file
    @brief Source code of HW sde integral class



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesHWIntegral.cpp
//
//  SYNOPSIS    :       LARatesHWIntegral
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


#include "LARatesHWIntegral.h"
#include "LAMathVolFuncHW.h"
#include "LAPriceDriftHW.h"
#include "LAMathVolFuncBase.h"
#include "LABasic.h"

using namespace std;
//================ LARatesHWIntegral ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LARatesHWIntegral::LARatesHWIntegral(SDEINTEGRAL_TYPE type)
: 
LARatesSDEIntegralBase(type),
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
LARatesHWIntegral::LARatesHWIntegral(SDEINTEGRAL_TYPE type, const LAString& sdeAttrName)
: 
LARatesSDEIntegralBase(type, sdeAttrName),
mpHWtool(0),
mE_cache( new map<double, double>() ),
mCumulatedVal(0.0), 
is_cloned(false)
{
}
/*!
	@brief copy constructor
*/
LARatesHWIntegral::LARatesHWIntegral(const LARatesHWIntegral& v) 
:
LARatesSDEIntegralBase(v),
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
LARatesHWIntegral::~LARatesHWIntegral() 
{
	delete mE_cache; 
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesHWIntegral::clone() const
{
    try 
	{
		return new LARatesHWIntegral(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesHWIntegral::isTypeOf(function_t id) const
{
	return (id==FN_HWINTEGRAL? true : LARatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesHWIntegral::getType() const
{
	return FN_HWINTEGRAL;
}

//void
//LARatesHWIntegral::setUpInitialVal(const LARatesPathElementBase& curve0, const SCALARARRAY& rate0)
//{ 
//    mpCurve0 = dynamic_cast<const LARatesPathElementHW1FCurve*>(&curve0);
//	mCumulatedVal = rate0.front();
//}

void
LARatesHWIntegral::setUpInitialVal(const SCALARARRAY& rate0)
{ 
	mCumulatedVal = rate0.front();
}

void
LARatesHWIntegral::setUp(const LAMathPathEntity& path)
{
	const LADataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	const LAMathAttrSDE* pattrsde = &dynamic_cast<const LAMathAttrSDE&>(dh->get());
	const std::vector<std::vector<LAFunctionBase* > > mVolatility = pattrsde->getSDE().getVolatility();

	const LAMathVolFuncHW* pvolHW=0;
	//in case of calculating vega
	if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCHW))
		pvolHW = dynamic_cast<LAMathVolFuncHW*>(mVolatility[0][0]);
	else//otherwise
	{
		const LAFunctionBase* pbase = dynamic_cast<LAMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
		pvolHW = dynamic_cast<const LAMathVolFuncHW*>(pbase);
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
LARatesHWIntegral::integral(double ts, double te, 
							vector<LAFunctionBase*>::const_iterator drift,										
							vector<vector<LAFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
    if ( te < ts )
    {
        throw LACoreInvalidData("ts < te", __FILE__, __LINE__);
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
double LARatesHWIntegral::E( double T ) const
{
    if((*mE_cache)[T] == 0.0)
	{
		(*mE_cache)[T] = mpHWtool->ExpIntegralMR(T);
	}
	return (*mE_cache)[T];
}


