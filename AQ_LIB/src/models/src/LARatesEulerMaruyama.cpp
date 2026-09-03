/*! @file
    @brief Source code of Euler-Maruyama sde integral class



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesEulerMaruyama.cpp
//
//  SYNOPSIS    :       LARatesEulerMaruyama
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


#include "LARatesEulerMaruyama.h"
#include "LABasic.h"
#include "LAPriceDriftFX.h"

using namespace std;
//================ LARatesEulerMaruyama ===================================
/*!
	@brief default constructor

	@param[in] type sde integral type

*/
LARatesEulerMaruyama::LARatesEulerMaruyama(SDEINTEGRAL_TYPE type)
: LARatesSDEIntegralBase(type)
{

}
/*!
	@brief copy constructor
*/
LARatesEulerMaruyama::LARatesEulerMaruyama(const LARatesEulerMaruyama& v) 
: LARatesSDEIntegralBase(v)
{

}

/*!
	@brief destructor
*/
LARatesEulerMaruyama::~LARatesEulerMaruyama() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesEulerMaruyama::clone() const
{
    try 
	{
		return new LARatesEulerMaruyama(*this);
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
LARatesEulerMaruyama::isTypeOf(function_t id) const
{
	return (id==FN_EULERMARUYMA ? true : LARatesSDEIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesEulerMaruyama::getType() const
{
	return FN_EULERMARUYMA;
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
LARatesEulerMaruyama::integral(double ts, double te, 
							vector<LAFunctionBase*>::const_iterator drift,										
							vector<vector<LAFunctionBase*> >::const_iterator vol,
							DoubleArray::const_iterator	bm,
							SCALARARRAY::iterator	x_in_out,
							unsigned int varnum
							) const
{
	if ((mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT) 
		&& vol->size() > 1)
	{
		//error
		throw LACoreInvalidData("Multi volatility is not support when SDE IntegralType is LOG", __FILE__, __LINE__);
	}

	SCALARARRAY::const_iterator it = x_in_out;
	mVar.resize(varnum + 1);
	mVar[0] = ts;

	for (unsigned int i = 0 ; i < varnum; i++) mVar[i + 1] = (*it++);

	if ((*drift)->isTypeOf(FN_DRIFTFX))
			mVar.push_back(te);

	for (unsigned int i = 0 ; i < varnum; i++)
	{
		double del = 0.0;
		if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
			del += (*drift++)->operator()(mVar) * (te - ts);	
		else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
			del += (*drift++)->operator()(mVar) * mVar[i + 1] * (te - ts);	
		else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL)
		{
			double v = (*vol)[0]->operator()(mVar) / mVar[i + 1];
			del += ((*drift++)->operator()(mVar) / mVar[i + 1] - 0.5 * v * v) * (te - ts);		
		}
		else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
		{
			double v = (*vol)[0]->operator()(mVar) / LAMath::exp(mVar[i + 1]);
			del += ((*drift++)->operator()(mVar) / LAMath::exp(mVar[i + 1]) - 0.5 * v * v) * (te - ts);		
		}
		else if (mSdeType == DIVIDEdXbyX && (mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT))
		{
			double v = (*vol)[0]->operator()(mVar);
			del += ((*drift++)->operator()(mVar) - 0.5 * v * v) * (te - ts);
		}
		///////////////
		if ((*vol).size() == 1)
		{
			if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
				del += (*vol)[0]->operator()(mVar) * (*bm++);	
			else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
				del += (*vol)[0]->operator()(mVar) * mVar[i + 1] * (*bm++);		
			else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL)
				del += (*vol)[0]->operator()(mVar) / mVar[i + 1] * (*bm++);
			else if (mSdeType == dX && mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
				del += (*vol)[0]->operator()(mVar) / LAMath::exp(mVar[i + 1]) * (*bm++);
			else if (mSdeType == DIVIDEdXbyX && (mIntegralType == LOG_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT))
				del += (*vol)[0]->operator()(mVar) * (*bm++);	
		}
		else 
		{
			DoubleArray::const_iterator	it = bm;
			for (unsigned int j = 0 ; j < (*vol).size(); j++)
			{
				if (mSdeType == dX && mIntegralType == NORMAL_INTEGRAL) 
					del += (*vol)[j]->operator()(mVar) * (*it++);	
				else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL_INTEGRAL)
					del += (*vol)[j]->operator()(mVar) * mVar[i + 1] * (*it++);		
				/*else if (mSdeType == dX && mIntegralType == LOG)
					del += (*vol)[j]->operator()(mVar) / mVar[i + 1] * (*it++);	
				else if (mSdeType == DIVIDEdXbyX && mIntegralType == LOG)
					del += (*vol)[j]->operator()(mVar) * (*it++);*/
			}
		}
		if (mIntegralType == NORMAL_INTEGRAL || mIntegralType == LOG_INTEGRAL_LOG_OUTPUT)
		{
#ifdef __SCALAR_FLOAT__
			(*x_in_out) += static_cast<SCALAR>(del);
#else	
			(*x_in_out) += del;
#endif
		}
		else
		{
#ifdef __SCALAR_FLOAT__
			(*x_in_out) *= static_cast<SCALAR>(LAMath::exp(del));
#else		
			(*x_in_out) *= LAMath::exp(del);
#endif
		}
		vol++;
		x_in_out++;
	}

/*	for (unsigned int i = 0 ; i < varnum; i++)
	{
		if (mSdeType == dX && mIntegralType == NORMAL) 
			(*x_in_out) += (*drift++)->operator()(mVar) * (te - ts);	
		else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL)
			(*x_in_out) += (*drift++)->operator()(mVar) * mVar[i + 1] * (te - ts);	
		else if (mSdeType == dX && mIntegralType == LOG)
		{
			double v = (*vol)[i]->operator()(mVar) / mVar[i + 1];
			(*x_in_out) += ((*drift++)->operator()(mVar) / mVar[i + 1] - 0.5 * v * v) * (te - ts);		
		}
		else if (mSdeType == DIVIDEdXbyX && mIntegralType == LOG)
		{
			double v = (*vol)[i]->operator()(mVar);
			(*x_in_out) += ((*drift++)->operator()(mVar) - 0.5 * v * v) * (te - ts);
		}

		for (unsigned int j = 0 ; j < varnum; j++)
		{
			if (mSdeType == dX && mIntegralType == NORMAL) 
				(*x_in_out) += (*vol)[j]->operator()(mVar) * (*bm++);	
			else if (mSdeType == DIVIDEdXbyX && mIntegralType == NORMAL)
				(*x_in_out) += (*vol)[j]->operator()(mVar) * mVar[i + 1] * (*bm++);		
			else if (mSdeType == dX && mIntegralType == LOG)
				(*x_in_out) += (*vol)[j]->operator()(mVar) / mVar[i + 1] * (*bm++);	
			else if (mSdeType == DIVIDEdXbyX && mIntegralType == LOG)
				(*x_in_out) += (*vol)[j]->operator()(mVar) * (*bm++);	
			

			vol++;
		}
		x_in_out++;
	}
*/
}

