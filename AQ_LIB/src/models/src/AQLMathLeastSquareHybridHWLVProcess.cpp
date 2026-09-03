// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

/*! @file
    @brief Source code of drift function of HW sde



*/

//+++++ INCLUDE +++++
#include "AQLMathLeastSquareHybridHWLVProcessQlib.h"
#include "AQLMathLeastSquareHybridHWLVProcess.h"
#include <algorithm>


using namespace std;

/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_
*/
AQLMathLeastSquareHybridHWLVProcess::AQLMathLeastSquareHybridHWLVProcess(const DoubleArray& fx0,
                                                                   vector<AQLMathHybridHWLVProcessHelper*>& processHelper,
                                                                   AQLMathTarget2fit2& t2f,
                                                                   mode mode_)
{
}


/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWLVProcess::AQLMathLeastSquareHybridHWLVProcess( const AQLMathLeastSquareHybridHWLVProcess& rhs )
{
}

/*!
    @brief destructor
*/
AQLMathLeastSquareHybridHWLVProcess::~AQLMathLeastSquareHybridHWLVProcess()
{
 
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWLVProcess& AQLMathLeastSquareHybridHWLVProcess::operator =(const AQLMathLeastSquareHybridHWLVProcess& rhs)
{
    if ( this != &rhs )
    {
		//mCostFunction = (rhs.mCostFunction != NULL ? dynamic_cast<AQLMathLeastSquareHybridHWLVProcessQlib*>(rhs.mCostFunction)->clone() : NULL);
    }
    return *this;

}


/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_
*/
AQLMathLeastSquareHybridHWDDProcess::AQLMathLeastSquareHybridHWDDProcess(const DoubleArray& fx0,
                                                                   vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                                                   AQLMathTarget2fit2& t2f,
                                                                   mode mode_)
:
AQLMathLeastSquareHybridHWLVProcess()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess::AQLMathLeastSquareHybridHWDDProcess( const AQLMathLeastSquareHybridHWDDProcess& rhs )
:
AQLMathLeastSquareHybridHWLVProcess()
{
//	mCostFunction = (rhs.mCostFunction != NULL ? new AQLMathLeastSquareHybridHWDDProcessQlib(*(dynamic_cast<AQLMathLeastSquareHybridHWDDProcessQlib*>(rhs.mCostFunction))) : NULL);
}

/*!
    @brief destructor
*/
AQLMathLeastSquareHybridHWDDProcess::~AQLMathLeastSquareHybridHWDDProcess()
{
 
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess& AQLMathLeastSquareHybridHWDDProcess::operator =(const AQLMathLeastSquareHybridHWDDProcess& rhs)
{
    if ( this != &rhs )
    {
//		mCostFunction = (rhs.mCostFunction != NULL ? new AQLMathLeastSquareHybridHWDDProcessQlib(*(dynamic_cast<AQLMathLeastSquareHybridHWDDProcessQlib*>(rhs.mCostFunction))) : NULL);
    }
    return *this;
}

/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] vol2fit
	@param[in] skew2fit
	@param[in] weightVolSkew
	@param[in] mode_
*/
AQLMathLeastSquareHybridHWDDProcess1::AQLMathLeastSquareHybridHWDDProcess1(const DoubleArray& fx0,
                                                                     vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                                                     AQLMathTarget2fit2& vol2fit,
                                                                     AQLMathTarget2fit2& skew2fit,
                                                                     DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                                                     mode mode_ )
:
AQLMathLeastSquareHybridHWDDProcess(fx0, processHelper, skew2fit, mode_)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareHybridHWDDProcess1Qlib>(fx0, processHelper, vol2fit, skew2fit, weightVolSkew, static_cast<AQLMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}


/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess1::AQLMathLeastSquareHybridHWDDProcess1( const AQLMathLeastSquareHybridHWDDProcess1& rhs )
:
AQLMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess1Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess1Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess1Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
AQLMathLeastSquareHybridHWDDProcess1::~AQLMathLeastSquareHybridHWDDProcess1()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess1& AQLMathLeastSquareHybridHWDDProcess1::operator =(const AQLMathLeastSquareHybridHWDDProcess1& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess1Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess1Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess1Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
    }
    return *this;
}


/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] vol2fit
	@param[in] ts
	@param[in] mode_
*/
AQLMathLeastSquareHybridHWDDProcess2::AQLMathLeastSquareHybridHWDDProcess2(const DoubleArray& fx0,
                                                                     vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                                                     AQLMathTarget2fit2& vol2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
AQLMathLeastSquareHybridHWDDProcess(fx0, processHelper, vol2fit, mode_)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareHybridHWDDProcess2Qlib>(fx0, processHelper, vol2fit, ts, static_cast<AQLMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess2::AQLMathLeastSquareHybridHWDDProcess2( const AQLMathLeastSquareHybridHWDDProcess2& rhs )
:
AQLMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess2Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess2Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess2Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
AQLMathLeastSquareHybridHWDDProcess2::~AQLMathLeastSquareHybridHWDDProcess2()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess2& AQLMathLeastSquareHybridHWDDProcess2::operator =(const AQLMathLeastSquareHybridHWDDProcess2& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess2Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess2Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess2Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
    }
    return *this;
}

/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] skew2fit
	@param[in] ts
	@param[in] mode_
*/
AQLMathLeastSquareHybridHWDDProcess3::AQLMathLeastSquareHybridHWDDProcess3(const DoubleArray& fx0,
                                                                     vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                                                     AQLMathTarget2fit2& skew2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
AQLMathLeastSquareHybridHWDDProcess(fx0, processHelper, skew2fit, mode_)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareHybridHWDDProcess3Qlib>(fx0, processHelper, skew2fit, ts, static_cast<AQLMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess3::AQLMathLeastSquareHybridHWDDProcess3( const AQLMathLeastSquareHybridHWDDProcess3& rhs )
:
AQLMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess3Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess3Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess3Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
AQLMathLeastSquareHybridHWDDProcess3::~AQLMathLeastSquareHybridHWDDProcess3()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
AQLMathLeastSquareHybridHWDDProcess3& AQLMathLeastSquareHybridHWDDProcess3::operator =(const AQLMathLeastSquareHybridHWDDProcess3& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<AQLMathLeastSquareHybridHWDDProcess3Qlib>(*(dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess3Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<AQLMathLeastSquareHybridHWDDProcess3Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
    }
    return *this;
}
