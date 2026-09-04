/*! @file
    @brief Source code of base class of volatility function class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceVolLMMDisplacedDiffusion.h"
#include "AQLMathVolatility.h"
#include "AQLMathPathEntity.h"

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathYieldCurve.h"

using namespace std;

//================ AQLPriceVolLMMDisplacedDiffusion ===================================
/*!
	@brief default constructor
	@param[in] isMultiVariables volatility function(before displaced diffusion)  depends L or not(only t depend)
*/
AQLPriceVolLMMDisplacedDiffusion::AQLPriceVolLMMDisplacedDiffusion(bool isMultiVariables)
: AQLMathVolFuncBase(isMultiVariables), mBeta(1)
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
AQLPriceVolLMMDisplacedDiffusion::AQLPriceVolLMMDisplacedDiffusion(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double beta, bool isMultiVariables)
: AQLMathVolFuncBase(sdeAttrName, i, 0, isMultiVariables), mBeta(beta), mTenor(tenor), mDeltaTenor(delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw AQLCoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
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
//AQLPriceVolLMMDisplacedDiffusion::AQLPriceVolLMMDisplacedDiffusion(const AQLPriceVolLMMDisplacedDiffusion& v) 
//: AQLMathVolFuncBase(v)
//{
//
//}

/*!
	@brief destructor
*/
AQLPriceVolLMMDisplacedDiffusion::~AQLPriceVolLMMDisplacedDiffusion() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceVolLMMDisplacedDiffusion::clone() const
{
    try 
	{
		return new AQLPriceVolLMMDisplacedDiffusion(*this);
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
AQLPriceVolLMMDisplacedDiffusion::isTypeOf(function_t id) const
{
	return (id==FN_VOLDISPLACEDDIFFUSION ? true : AQLMathVolFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceVolLMMDisplacedDiffusion::getType() const
{
	return FN_VOLDISPLACEDDIFFUSION;
}

/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} reset time <= t     
    @return drift value
*/
double
AQLPriceVolLMMDisplacedDiffusion::operator()(const DoubleArray& x) const
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
AQLString
AQLPriceVolLMMDisplacedDiffusion::convertToString(void) const
{
	AQLString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += AQLDataInt(m_i).convertToString();
	ret += ":";
	ret += AQLDataDoubles(mTenor).convertToString();
	ret += ":";
	ret += AQLDataDoubles(mDeltaTenor).convertToString();
	ret += ":";
	ret += AQLDataDouble(mBeta).convertToString();
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion (sde attr name : suffix : tenor : deltatenor : beta)
*/
void
AQLPriceVolLMMDisplacedDiffusion::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 5 || tmp.getSize() % 2 == 1)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	
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
AQLPriceVolLMMDisplacedDiffusion::setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw AQLCoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
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
AQLPriceVolLMMDisplacedDiffusion::setUp(AQLMathPathEntity& path)
{
	AQLMathVolFuncBase::setUp(path);
    
	const AQLStringVector& names = path.getSDEAttrNames().get();
	unsigned int i;
	for (i = 0; i < names.size(); i++)
	{
		if (mSDEAttrName == names[i]) break;
	}
	if (i == names.size())
	{
		//error
		AQLString msg = "SDEAttrName: " + mSDEAttrName;
		msg += " does not exsist";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLMathYieldCurve& yield = dynamic_cast<const AQLMathYieldCurve&>(path.getInitialValues().get(i).get());
	const AQLRatesPathElementCurve& curve = yield.getCurve(0, path.getDayCount().getDayCount());
	mL0 = (curve.getP(mTenor.at(m_i)) / curve.getP(mTenor.at(m_i + 1)) - 1) / mDeltaTenor.at(m_i);
	
}
