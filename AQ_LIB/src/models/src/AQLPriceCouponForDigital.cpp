/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
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

#include "AQLPriceCouponForDigital.h"
using namespace std;

/*!
    @brief constructor
*/
AQLPriceCouponForDigital::AQLPriceCouponForDigital()
: AQLPriceCouponBase()
{

}
/*!
    @brief destructor

*/
AQLPriceCouponForDigital::~AQLPriceCouponForDigital()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceCouponForDigital::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORFORDIGITAL ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
AQLPriceCouponForDigital::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceCouponForDigital::clone() const
{
    try 
	{
        return new AQLPriceCouponForDigital(*this);
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
AQLPriceCouponForDigital::getType() const
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
AQLPriceCouponForDigital::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	if(x.size() != mParam.size())
		throw AQLCoreInvalidData("cpn size should be the digital trigger size",__FILE__,__LINE__);

	if(x.size() < 2)
		throw AQLCoreInvalidData("cpn and digital size should be more than 2",__FILE__,__LINE__);

	//check input if  x[1] < x[2] < ... < x[n] or not
	if(x.size() > 2)
	{
		for(unsigned int i =2; i<x.size();i++)
		{
			if(x[i-1] > x[i])
				throw AQLCoreInvalidData("digital trigge should be in order",__FILE__,__LINE__);
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


