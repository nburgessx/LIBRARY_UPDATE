/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
//   2007,AlgoQuantHub.
// 
//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceCouponForDigital2.h"
using namespace std;

/*!
    @brief constructor
*/
AQLPriceCouponForDigital2::AQLPriceCouponForDigital2()
: AQLPriceCouponBase(),mIsCallSpread(false),mCallSpreadVal(0.0)
{

}
/*!
    @brief destructor

*/
AQLPriceCouponForDigital2::~AQLPriceCouponForDigital2()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceCouponForDigital2::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORFORDIGITAL2 ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
AQLPriceCouponForDigital2::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceCouponForDigital2::clone() const
{
    try 
	{
        return new AQLPriceCouponForDigital2(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
    @brief Return this class type
    @return function type5
*/
function_t			
AQLPriceCouponForDigital2::getType() const
{
	return FN_CPNSLTOPERATORFORDIGITAL2;
}

/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
AQLPriceCouponForDigital2::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	if(x.size() != mParam.size()+2)
		throw AQLCoreInvalidData("cpninfos size should be trigger strike size +2",__FILE__,__LINE__);

	//check input if mParam[0] < mParam[1] < ... or not
	if(mParam.size() > 1)
	{
		for(unsigned int i =1; i<mParam.size();i++)
		{
			if(mParam[i-1] > mParam[i])
				throw AQLCoreInvalidData("trigger strike should be in order",__FILE__,__LINE__);
		}
	}
	pos = 0;
	double basecpn = x[0];
	if (!mIsCallSpread)
	{
	for(unsigned int i =0; i<mParam.size();i++)
	{
		if( basecpn < mParam[i])
		{
			pos = i+1;
			return x[i+1];
		}
	}
	pos = x.size()-1;
	return x[x.size()-1];
	}
	else 
	{
		double interval = 0.5 * mCallSpreadVal;
		for (unsigned int i =0; i<mParam.size();i++)
		{
			if (basecpn < mParam[i] - interval)
			{
				pos = i+1;
				return x[i+1];
			}
			else if (mParam[i] - interval <= basecpn && basecpn <= mParam[i] + interval)
			{
				pos=i+1;
				double val = x[i+1] + (x[i+2]-x[i+1]) / mCallSpreadVal * (basecpn - mParam[i] + interval);
				return val;
			}
		}
		pos = x.size()-1;
		return x[x.size()-1];
	}
}

void
AQLPriceCouponForDigital2::setUp(const AQLDate& basedate, const AQLObject& trade,
								unsigned int legNo, const AQLObject& cashlet)
{
	(void)basedate;(void)trade;(void)legNo;(void)cashlet;

	//check the mCallSpreadVal;

	if(mIsCallSpread && mCallSpreadVal == 0.0)
		throw AQLCoreInvalidData("CallSpread must not be 0 when IsCallSpread is true",__FILE__,__LINE__);

}


