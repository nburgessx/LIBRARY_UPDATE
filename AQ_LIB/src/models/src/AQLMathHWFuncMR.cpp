#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLMathHWFuncMR.h"
using namespace std;

//
//------------------------------ AQLMathHWFuncMR ------------------------------
//

AQLMathHWFuncMR::AQLMathHWFuncMR()
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLMathHWFuncMR::~AQLMathHWFuncMR(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathHWFuncMR::AQLMathHWFuncMR(const AQLMathHWFuncMR &rhs) 
: AQLFunctionBase(rhs), mMR(rhs.mMR)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncMR::clone() const
{
    try 
	{
		return new AQLMathHWFuncMR(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
AQLMathHWFuncMR::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCMR ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathHWFuncMR::getType() const
{
	return FN_HWFUNCMR;
}

//
//------------------------------ AQLMathHWFuncMRTMDPT ------------------------------
//

AQLMathHWFuncMRTMDPT::AQLMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                            const DoubleArray&    a_grid_,
							AQLInterpolationBase&        interpolation_)
:
AQLMathHWFuncMR(a_grid_[0]),
a_grid(a_grid_),
T_grid( T_grid_ ),
mpInter( &interpolation_ ),
is_cloned( false ),
mGL(HWGAUSSLEGENDRENUM)
{
    mpInter->set(T_grid,a_grid);
}

AQLMathHWFuncMRTMDPT::AQLMathHWFuncMRTMDPT(const AQLMathHWFuncMRTMDPT& rhs)
:
AQLMathHWFuncMR( rhs ),
a_grid( rhs.a_grid ),
T_grid( rhs.T_grid ),
mpInter( rhs.mpInter != 0 ? dynamic_cast<AQLInterpolationBase* >(rhs.mpInter->clone()) : 0 ),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM)
{
    mpInter->set(T_grid,a_grid);
}

//
AQLMathHWFuncMRTMDPT::~AQLMathHWFuncMRTMDPT()
{
    if ( is_cloned )
    {
        delete mpInter;
    }
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncMRTMDPT::clone() const
{
    try 
	{
		return new AQLMathHWFuncMRTMDPT(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
AQLMathHWFuncMRTMDPT::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCMRTMDPT ? true : AQLMathHWFuncMR::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathHWFuncMRTMDPT::getType() const
{
	return FN_HWFUNCMRTMDPT;
}

//AQLMathHWFuncMRTMDPT& AQLMathHWFuncMRTMDPT::operator =( const AQLMathHWFuncMRTMDPT& rhs )
//{
//    if(this == &rhs) return *this; 
//
//    (*this).AQLMathHWFuncMR::operator =(rhs);
//
//    T_grid = rhs.T_grid;
//    a_grid = rhs.a_grid;
//    mpInter = rhs.mpInter != 0 ? dynamic_cast<AQLInterpolationBase* >(rhs.mpInter->clone()) : 0;
//    is_cloned = true;
//    mGL = rhs.mGL;
//    return *this;        
//}

//
double AQLMathHWFuncMRTMDPT::get_a( double t ) const
{
	return mpInter->value(t);
}

//
double AQLMathHWFuncMRTMDPT::integrate( double t ) const
{
	return mGL.integrate(*this,0.0,t);
//	  
}

//
double AQLMathHWFuncMRTMDPT::integrate( double ts, double te ) const
{
	return mGL.integrate(*this,ts,te);
}
