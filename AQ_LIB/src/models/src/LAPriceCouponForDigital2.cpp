/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
//   2007,Mizuho International London.
// 
//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceCouponForDigital2.h"
using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponForDigital2::LAPriceCouponForDigital2()
: LAPriceCouponBase(),mIsCallSpread(false),mCallSpreadVal(0.0)
{

}
/*!
    @brief destructor

*/
LAPriceCouponForDigital2::~LAPriceCouponForDigital2()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponForDigital2::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORFORDIGITAL2 ? true : LAFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
LAPriceCouponForDigital2::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCouponForDigital2::clone() const
{
    try 
	{
        return new LAPriceCouponForDigital2(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
    @brief Return this class type
    @return function type5
*/
function_t			
LAPriceCouponForDigital2::getType() const
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
LAPriceCouponForDigital2::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	if(x.size() != mParam.size()+2)
		throw LACoreInvalidData("cpninfos size should be trigger strike size +2",__FILE__,__LINE__);

	//check input if mParam[0] < mParam[1] < ... or not
	if(mParam.size() > 1)
	{
		for(unsigned int i =1; i<mParam.size();i++)
		{
			if(mParam[i-1] > mParam[i])
				throw LACoreInvalidData("trigger strike should be in order",__FILE__,__LINE__);
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
LAPriceCouponForDigital2::setUp(const LADate& basedate, const LAObject& trade,
								unsigned int legNo, const LAObject& cashlet)
{
	(void)basedate;(void)trade;(void)legNo;(void)cashlet;

	//check the mCallSpreadVal;

	if(mIsCallSpread && mCallSpreadVal == 0.0)
		throw LACoreInvalidData("CallSpread must not be 0 when IsCallSpread is true",__FILE__,__LINE__);

}


