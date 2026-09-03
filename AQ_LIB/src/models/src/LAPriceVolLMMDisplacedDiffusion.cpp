/*! @file
    @brief Source code of base class of volatility function class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceVolLMMDisplacedDiffusion.cpp
//
//  SYNOPSIS    :       LAPriceVolLMMDisplacedDiffusion
//  DESCRIPTION :       Source code of base class of volatility function class
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


#include "LAPriceVolLMMDisplacedDiffusion.h"
#include "LAMathVolatility.h"
#include "LAMathPathEntity.h"

#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObjectHolder.h"
#include "LAMathYieldCurve.h"

using namespace std;

//================ LAPriceVolLMMDisplacedDiffusion ===================================
/*!
	@brief default constructor
	@param[in] isMultiVariables volatility function(before displaced diffusion)  depends L or not(only t depend)
*/
LAPriceVolLMMDisplacedDiffusion::LAPriceVolLMMDisplacedDiffusion(bool isMultiVariables)
: LAMathVolFuncBase(isMultiVariables), mBeta(1)
{

}
/*!
	@brief constructor
	@param[in] sdeAttrName attirubte name of sde
	@param[in] i suffics
	@param[in] tenor tenor 
	@param[in] delta_tenor span of each libor
	@param[in] beta beta
	@param[in] isMultiVariables volatility function(before displaced diffusion)  depends L or not(only t depend)
*/
LAPriceVolLMMDisplacedDiffusion::LAPriceVolLMMDisplacedDiffusion(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double beta, bool isMultiVariables)
: LAMathVolFuncBase(sdeAttrName, i, 0, isMultiVariables), mBeta(beta), mTenor(tenor), mDeltaTenor(delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw LACoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}
	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}
}

/*!
	@brief copy constructor
*/
//LAPriceVolLMMDisplacedDiffusion::LAPriceVolLMMDisplacedDiffusion(const LAPriceVolLMMDisplacedDiffusion& v) 
//: LAMathVolFuncBase(v)
//{
//
//}

/*!
	@brief destructor
*/
LAPriceVolLMMDisplacedDiffusion::~LAPriceVolLMMDisplacedDiffusion() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceVolLMMDisplacedDiffusion::clone() const
{
    try 
	{
		return new LAPriceVolLMMDisplacedDiffusion(*this);
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
LAPriceVolLMMDisplacedDiffusion::isTypeOf(function_t id) const
{
	return (id==FN_VOLDISPLACEDDIFFUSION ? true : LAMathVolFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceVolLMMDisplacedDiffusion::getType() const
{
	return FN_VOLDISPLACEDDIFFUSION;
}

/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} reset time <= t     
    @return drift value
*/
double
LAPriceVolLMMDisplacedDiffusion::operator()(const DoubleArray& x) const
{
	int i = x.size() + m_i - mDeltaTenor.size();
	if (i <= 0 ) return 0.0;
	if (mIsMultiVariables)
		return mpVolatility->operator ()(x) * (mBeta * x[i] + (1.0 - mBeta) * mL0) / mBeta / x[i];	
	else
		return mpVolatility->operator ()(x[0]) * (mBeta * x[i] + (1.0 - mBeta) * mL0) / mBeta / x[i];	

	/*	unsigned int i = 0;
	for (; i < m_i; i++)
	{
		if (x[0] <= mTenor[i]) break;
	}*/
	
	//return mpVolatility->operator ()(x[0]) * (mBeta * x[m_i - i + 1] - (1 - mBeta) * mL0) / x[m_i - i + 1];	
}


/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix : tenor : deltatenor : beta)
*/
LAString
LAPriceVolLMMDisplacedDiffusion::convertToString(void) const
{
	LAString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += LADataInt(m_i).convertToString();
	ret += ":";
	ret += LADataDoubles(mTenor).convertToString();
	ret += ":";
	ret += LADataDoubles(mDeltaTenor).convertToString();
	ret += ":";
	ret += LADataDouble(mBeta).convertToString();
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion (sde attr name : suffix : tenor : deltatenor : beta)
*/
void
LAPriceVolLMMDisplacedDiffusion::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 5 || tmp.getSize() % 2 == 1)
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	
	}
	
	if (tmp.getSize() % 2 == 0)
	{
		mSDEAttrName = tmp.get()[0];
		m_i = tmp.get()[1].getIntValue();
		m_j = 0;

		unsigned int size = tmp.getSize() - 3;
		mTenor.resize(size / 2 + 1);
		mDeltaTenor.resize(size / 2);
		for (unsigned int i = 0; i < mTenor.size(); i++)
			mTenor[i] = tmp.get()[2 + i].getDoubleValue();
		size = 2 + mTenor.size();
		for (unsigned int i = 0; i < mDeltaTenor.size(); i++)
			mDeltaTenor[i] = tmp.get()[size + i].getDoubleValue();
		mBeta = tmp.get().back().getDoubleValue();
		
	}
/*	else
	{
		m_i = tmp.get()[0].getIntValue();
		m_j = 0;
		mSDEAttrName = tmp.get()[2];
		mBeta = tmp.get()[3].getDoubleValue();
		unsigned int size = tmp.getSize() - 4;
		mTenor.resize(size / 2 + 1);
		mDeltaTenor.resize(size / 2);
		for (unsigned int i = 0; i < mTenor.size(); i++)
			mTenor[i] = tmp.get()[4 + i].getDoubleValue();
		size = 4 + mTenor.size();
		for (unsigned int i = 0; i < mDeltaTenor.size(); i++)
			mDeltaTenor[i] = tmp.get()[size + i].getDoubleValue();
	}*/
	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}	

}
/*!
	@brief set tenor
	@param[in] tenor tenor 
	@param[in] delta_tenor span of each libor

	@note day count of tenor must be same as one of path object 
	@note day count of delta_tenor must be same as one of libor
*/
void
LAPriceVolLMMDisplacedDiffusion::setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw LACoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}
	mTenor = tenor;
	mDeltaTenor = delta_tenor;
	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceVolLMMDisplacedDiffusion::setUp(LAMathPathEntity& path)
{
	LAMathVolFuncBase::setUp(path);
    
	const LAStringVector& names = path.getSDEAttrNames().get();
	unsigned int i;
	for (i = 0; i < names.size(); i++)
	{
		if (mSDEAttrName == names[i]) break;
	}
	if (i == names.size())
	{
		//error
		LAString msg = "SDEAttrName: " + mSDEAttrName;
		msg += " does not exsist";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const LAMathYieldCurve& yield = dynamic_cast<const LAMathYieldCurve&>(path.getInitialValues().get(i).get());
	const LARatesPathElementCurve& curve = yield.getCurve(0, path.getDayCount().getDayCount());
	mL0 = (curve.getP(mTenor.at(m_i)) / curve.getP(mTenor.at(m_i + 1)) - 1) / mDeltaTenor.at(m_i);
	
}
