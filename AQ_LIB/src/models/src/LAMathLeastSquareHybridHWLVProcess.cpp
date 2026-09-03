// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

/*! @file
    @brief Source code of drift function of HW sde



*/

//+++++ INCLUDE +++++
#include "LAMathLeastSquareHybridHWLVProcessQlib.h"
#include "LAMathLeastSquareHybridHWLVProcess.h"
#include <algorithm>


using namespace std;

/*!
    @brief constructor
	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_
*/
LAMathLeastSquareHybridHWLVProcess::LAMathLeastSquareHybridHWLVProcess(const DoubleArray& fx0,
                                                                   vector<LAMathHybridHWLVProcessHelper*>& processHelper,
                                                                   LAMathTarget2fit2& t2f,
                                                                   mode mode_)
{
}


/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWLVProcess::LAMathLeastSquareHybridHWLVProcess( const LAMathLeastSquareHybridHWLVProcess& rhs )
{
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWLVProcess::~LAMathLeastSquareHybridHWLVProcess()
{
 
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWLVProcess& LAMathLeastSquareHybridHWLVProcess::operator =(const LAMathLeastSquareHybridHWLVProcess& rhs)
{
    if ( this != &rhs )
    {
		//mCostFunction = (rhs.mCostFunction != NULL ? dynamic_cast<LAMathLeastSquareHybridHWLVProcessQlib*>(rhs.mCostFunction)->clone() : NULL);
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
LAMathLeastSquareHybridHWDDProcess::LAMathLeastSquareHybridHWDDProcess(const DoubleArray& fx0,
                                                                   vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                   LAMathTarget2fit2& t2f,
                                                                   mode mode_)
:
LAMathLeastSquareHybridHWLVProcess()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess::LAMathLeastSquareHybridHWDDProcess( const LAMathLeastSquareHybridHWDDProcess& rhs )
:
LAMathLeastSquareHybridHWLVProcess()
{
//	mCostFunction = (rhs.mCostFunction != NULL ? new LAMathLeastSquareHybridHWDDProcessQlib(*(dynamic_cast<LAMathLeastSquareHybridHWDDProcessQlib*>(rhs.mCostFunction))) : NULL);
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess::~LAMathLeastSquareHybridHWDDProcess()
{
 
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess& LAMathLeastSquareHybridHWDDProcess::operator =(const LAMathLeastSquareHybridHWDDProcess& rhs)
{
    if ( this != &rhs )
    {
//		mCostFunction = (rhs.mCostFunction != NULL ? new LAMathLeastSquareHybridHWDDProcessQlib(*(dynamic_cast<LAMathLeastSquareHybridHWDDProcessQlib*>(rhs.mCostFunction))) : NULL);
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
LAMathLeastSquareHybridHWDDProcess1::LAMathLeastSquareHybridHWDDProcess1(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& vol2fit,
                                                                     LAMathTarget2fit2& skew2fit,
                                                                     DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                                                     mode mode_ )
:
LAMathLeastSquareHybridHWDDProcess(fx0, processHelper, skew2fit, mode_)
{
	mCostFunction = std::make_shared<LAMathLeastSquareHybridHWDDProcess1Qlib>(fx0, processHelper, vol2fit, skew2fit, weightVolSkew, static_cast<LAMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}


/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess1::LAMathLeastSquareHybridHWDDProcess1( const LAMathLeastSquareHybridHWDDProcess1& rhs )
:
LAMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess1Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess1Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess1Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess1::~LAMathLeastSquareHybridHWDDProcess1()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess1& LAMathLeastSquareHybridHWDDProcess1::operator =(const LAMathLeastSquareHybridHWDDProcess1& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess1Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess1Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess1Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
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
LAMathLeastSquareHybridHWDDProcess2::LAMathLeastSquareHybridHWDDProcess2(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& vol2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
LAMathLeastSquareHybridHWDDProcess(fx0, processHelper, vol2fit, mode_)
{
	mCostFunction = std::make_shared<LAMathLeastSquareHybridHWDDProcess2Qlib>(fx0, processHelper, vol2fit, ts, static_cast<LAMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess2::LAMathLeastSquareHybridHWDDProcess2( const LAMathLeastSquareHybridHWDDProcess2& rhs )
:
LAMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess2Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess2Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess2Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess2::~LAMathLeastSquareHybridHWDDProcess2()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess2& LAMathLeastSquareHybridHWDDProcess2::operator =(const LAMathLeastSquareHybridHWDDProcess2& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess2Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess2Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess2Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
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
LAMathLeastSquareHybridHWDDProcess3::LAMathLeastSquareHybridHWDDProcess3(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& skew2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
LAMathLeastSquareHybridHWDDProcess(fx0, processHelper, skew2fit, mode_)
{
	mCostFunction = std::make_shared<LAMathLeastSquareHybridHWDDProcess3Qlib>(fx0, processHelper, skew2fit, ts, static_cast<LAMathLeastSquareHybridHWLVProcessQlib::mode>(mode_));
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess3::LAMathLeastSquareHybridHWDDProcess3( const LAMathLeastSquareHybridHWDDProcess3& rhs )
:
LAMathLeastSquareHybridHWDDProcess(rhs)
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess3Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess3Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess3Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess3::~LAMathLeastSquareHybridHWDDProcess3()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess3& LAMathLeastSquareHybridHWDDProcess3::operator =(const LAMathLeastSquareHybridHWDDProcess3& rhs)
{
	if ( this != &rhs )
    {
		mCostFunction = (rhs.mCostFunction != nullptr ? std::make_shared<LAMathLeastSquareHybridHWDDProcess3Qlib>(*(dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess3Qlib>(rhs.mCostFunction))) : dynamic_pointer_cast<LAMathLeastSquareHybridHWDDProcess3Qlib>(std::shared_ptr<QuantLib::CostFunction>()));
    }
    return *this;
}
