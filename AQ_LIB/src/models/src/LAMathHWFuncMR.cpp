#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAMathHWFuncMR.h"
using namespace std;

//
//------------------------------ LAMathHWFuncMR ------------------------------
//

LAMathHWFuncMR::LAMathHWFuncMR()
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LAMathHWFuncMR::~LAMathHWFuncMR(void)
{
}


/*!
	@brief copy constructor
*/
LAMathHWFuncMR::LAMathHWFuncMR(const LAMathHWFuncMR &rhs) 
: LAFunctionBase(rhs), mMR(rhs.mMR)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathHWFuncMR::clone() const
{
    try 
	{
		return new LAMathHWFuncMR(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathHWFuncMR::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCMR ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathHWFuncMR::getType() const
{
	return FN_HWFUNCMR;
}

//
//------------------------------ LAMathHWFuncMRTMDPT ------------------------------
//

LAMathHWFuncMRTMDPT::LAMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                            const DoubleArray&    a_grid_,
							LAInterpolationBase&        interpolation_)
:
LAMathHWFuncMR(a_grid_[0]),
a_grid(a_grid_),
T_grid( T_grid_ ),
mpInter( &interpolation_ ),
is_cloned( false ),
mGL(HWGAUSSLEGENDRENUM)
{
    mpInter->set(T_grid,a_grid);
}

LAMathHWFuncMRTMDPT::LAMathHWFuncMRTMDPT(const LAMathHWFuncMRTMDPT& rhs)
:
LAMathHWFuncMR( rhs ),
a_grid( rhs.a_grid ),
T_grid( rhs.T_grid ),
mpInter( rhs.mpInter != 0 ? dynamic_cast<LAInterpolationBase* >(rhs.mpInter->clone()) : 0 ),
is_cloned( true ),
mGL(HWGAUSSLEGENDRENUM)
{
    mpInter->set(T_grid,a_grid);
}

//
LAMathHWFuncMRTMDPT::~LAMathHWFuncMRTMDPT()
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
LACoreFunctionBase*	
LAMathHWFuncMRTMDPT::clone() const
{
    try 
	{
		return new LAMathHWFuncMRTMDPT(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathHWFuncMRTMDPT::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCMRTMDPT ? true : LAMathHWFuncMR::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathHWFuncMRTMDPT::getType() const
{
	return FN_HWFUNCMRTMDPT;
}

//LAMathHWFuncMRTMDPT& LAMathHWFuncMRTMDPT::operator =( const LAMathHWFuncMRTMDPT& rhs )
//{
//    if(this == &rhs) return *this; 
//
//    (*this).LAMathHWFuncMR::operator =(rhs);
//
//    T_grid = rhs.T_grid;
//    a_grid = rhs.a_grid;
//    mpInter = rhs.mpInter != 0 ? dynamic_cast<LAInterpolationBase* >(rhs.mpInter->clone()) : 0;
//    is_cloned = true;
//    mGL = rhs.mGL;
//    return *this;        
//}

//
double LAMathHWFuncMRTMDPT::get_a( double t ) const
{
	return mpInter->value(t);
}

//
double LAMathHWFuncMRTMDPT::integrate( double t ) const
{
	return mGL.integrate(*this,0.0,t);
//	  
}

//
double LAMathHWFuncMRTMDPT::integrate( double ts, double te ) const
{
	return mGL.integrate(*this,ts,te);
}
