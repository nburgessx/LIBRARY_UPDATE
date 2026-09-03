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

#include "LAPriceCouponForDigital.h"
using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponForDigital::LAPriceCouponForDigital()
: LAPriceCouponBase()
{

}
/*!
    @brief destructor

*/
LAPriceCouponForDigital::~LAPriceCouponForDigital()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponForDigital::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORFORDIGITAL ? true : LAFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
LAPriceCouponForDigital::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCouponForDigital::clone() const
{
    try 
	{
        return new LAPriceCouponForDigital(*this);
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
LAPriceCouponForDigital::getType() const
{
	return FN_CPNSLTOPERATORFORDIGITAL;
}

/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
LAPriceCouponForDigital::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	if(x.size() != mParam.size())
		throw LACoreInvalidData("cpn size should be the digital trigger size",__FILE__,__LINE__);

	if(x.size() < 2)
		throw LACoreInvalidData("cpn and digital size should be more than 2",__FILE__,__LINE__);

	//check input if  x[1] < x[2] < ... < x[n] or not
	if(x.size() > 2)
	{
		for(unsigned int i =2; i<x.size();i++)
		{
			if(x[i-1] > x[i])
				throw LACoreInvalidData("digital trigge should be in order",__FILE__,__LINE__);
		}
	}
	
	pos = 0;
	double basecpn = x[0];
	for(unsigned int i =1; i<x.size();i++)
	{
		if( basecpn < x[i])
		{
			return mParam[i-1];
		}

	}

	return mParam[x.size()-1];


}


