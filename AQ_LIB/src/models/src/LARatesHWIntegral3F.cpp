/*! @file
    @brief Source code of HW sde integral class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesHWIntegral3F.cpp
//
//  SYNOPSIS    :       LARatesHWIntegral3F
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


#include "LARatesHWIntegral3F.h"
#include "LAPriceDriftHWQuantAdjustment3F.h"
#include "LAPriceDriftHW.h"
#include "AQLBasic.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"

using namespace std;
//================ LARatesHWIntegral3F ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LARatesHWIntegral3F::LARatesHWIntegral3F(SDEINTEGRAL_TYPE type)
: 
LARatesHWIntegral(type)
{
}
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LARatesHWIntegral3F::LARatesHWIntegral3F(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName)
: 
LARatesHWIntegral(type, sdeAttrName)
{
}
/*!
	@brief copy constructor
*/
LARatesHWIntegral3F::LARatesHWIntegral3F(const LARatesHWIntegral3F& v) 
:
LARatesHWIntegral(v)
{
}

/*!
	@brief destructor
*/
LARatesHWIntegral3F::~LARatesHWIntegral3F() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesHWIntegral3F::clone() const
{
    try 
	{
		return new LARatesHWIntegral3F(*this);
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
LARatesHWIntegral3F::isTypeOf(function_t id) const
{
	return (id==FN_HWINTEGRAL3F? true : LARatesHWIntegral::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesHWIntegral3F::getType() const
{
	return FN_HWINTEGRAL3F;
}

void
//LARatesHWIntegral3F::setUpInitialVal(const LARatesPathElementBase& curve0, const SCALARARRAY& rate0)
LARatesHWIntegral3F::setUpInitialVal(const SCALARARRAY& rate0)
{ 
    //mpCurve0 = dynamic_cast<const LARatesPathElementHW1FCurve*>(&curve0);
	mCumulatedVal = 0.0;
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
LARatesHWIntegral3F::integral(double ts, double te, 
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
	// calc x, x = r - alhpa
	const double s = (*vol)[0]->operator()(mVar);
	const double bm_ = (*bm++);
	mCumulatedVal += s * bm_;
    //mCumulatedVal += (*vol)[0]->operator()(mVar) * (*bm++);
	const LAPriceDriftHW *pIRDrift = 0;
	if ((*drift)->isTypeOf(FN_DRIFTHW))
	{
		pIRDrift = dynamic_cast<const LAPriceDriftHW *>(*drift);
	}
	else if ((*drift)->isTypeOf(FN_DRIFTHWQUANTADJ3F))
	{
		pIRDrift = dynamic_cast<LAPriceDriftHWQuantAdjustment *>(*drift)->getDrift();
	}
	else
	{
		throw AQLCoreInvalidData("Drift calss must be LAPriceDriftHW or LAPriceDriftHWQuantAdjustment", __FILE__, __LINE__);
	}
	const double alpha = pIRDrift->getAlpha(te);
	(*x_in_out) = mCumulatedVal / E(te) + alpha;
	if ((*drift)->isTypeOf(FN_DRIFTHWQUANTADJ3F))
	{
		const LAPriceDriftHWQuantAdjustment3F *drift3f = dynamic_cast<const LAPriceDriftHWQuantAdjustment3F *>(*drift);
		drift3f->getQuantAdjuster().calcQuantAdjust(ts, te, (*x_in_out));
		//drift3f->calcQuantAdjust(ts, te, (*x_in_out));
		//(*x_in_out) -= quantadjust;
		//// set x
		//mCumulatedVal =  ((*x_in_out) - alpha) * mpCurve0->E(te);
	}

}

