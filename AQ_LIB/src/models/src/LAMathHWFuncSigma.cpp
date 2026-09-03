#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
#include "LAMathHWFuncSigma.h"

using namespace std;
//
//------------------------------ LAMathHWFuncSigma ------------------------------
//

/*!
	@brief constructor

	@param[in] s      function value

*/
LAMathHWFuncSigma:: LAMathHWFuncSigma( double sigma )
: LAFunctionBase(), mSigma(sigma)
{
}

/*!
	@brief destructor
*/
LAMathHWFuncSigma::~LAMathHWFuncSigma(void)
{
}


/*!
	@brief copy constructor
*/
LAMathHWFuncSigma::LAMathHWFuncSigma(const LAMathHWFuncSigma &rhs) 
: LAFunctionBase(rhs), mSigma(rhs.mSigma)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathHWFuncSigma::clone() const
{
    try 
	{
		return new LAMathHWFuncSigma(*this);
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
LAMathHWFuncSigma::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCSIGMA ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathHWFuncSigma::getType() const
{
	return FN_HWFUNCSIGMA;
}

//
//------------------------------ LAMathHWFuncSigmaTMDPT ------------------------------
//

/*!
	@brief constructor

	@param[in] T_Grid function value
	@param[in] s      function value

*/
LAMathHWFuncSigmaTMDPT::LAMathHWFuncSigmaTMDPT(const DoubleArray& T_grid_, const DoubleArray& s_grid_, LAInterpolationBase& interpolation)
:
LAMathHWFuncSigma(s_grid_[0]),
T_grid(T_grid_),
s_grid(s_grid_),
mpInter(&interpolation),
is_cloned(false)
{
	size_t n = T_grid.size();
	if ( n != s_grid.size() )
	{
		// error
		throw LACoreInvalidData(" T_grid.size() != s.size()", __FILE__, __LINE__);
	}
	mpInter->set(T_grid,s_grid);
}

/*!
	@brief destructor
*/
LAMathHWFuncSigmaTMDPT::~LAMathHWFuncSigmaTMDPT(void)
{
    //if(is_cloned)
    //{
        delete mpInter;
        mpInter = 0;
    //}
	
}


/*!
	@brief copy constructor
*/
LAMathHWFuncSigmaTMDPT::LAMathHWFuncSigmaTMDPT(const LAMathHWFuncSigmaTMDPT &rhs) 
:
LAMathHWFuncSigma(rhs),
T_grid(rhs.T_grid),
s_grid(rhs.s_grid),
is_cloned(true)
{
	mpInter = rhs.mpInter != 0 ? dynamic_cast<LAInterpolationBase* >(rhs.mpInter->clone()) : 0;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathHWFuncSigmaTMDPT::clone() const
{
    try 
	{
		return new LAMathHWFuncSigmaTMDPT(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

//
//LAMathHWFuncSigmaTMDPT& LAMathHWFuncSigmaTMDPT::operator =( const LAMathHWFuncSigmaTMDPT& rhs )
//{
//    if(this == &rhs) return *this; 
//    
//    (*this).LAMathHWFuncSigma::operator =(rhs);
//
//     T_grid = rhs.T_grid;
//     s_grid = rhs.s_grid;
//	 mpInter = rhs.mpInter != 0 ? dynamic_cast<LAInterpolationBase* >(rhs.mpInter->clone()) : 0;
//     is_cloned = true;
//     
//     return *this;        
//}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathHWFuncSigmaTMDPT::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCSIGMATMDPT ? true : LAMathHWFuncSigma::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathHWFuncSigmaTMDPT::getType() const
{
	return FN_HWFUNCSIGMATMDPT;
}

/*!
    @brief set sigmavector
    @return void
*/

void LAMathHWFuncSigmaTMDPT::set_s(const DoubleArray& sgrid)
{
	if(sgrid.size()!= T_grid.size())
		throw LACoreInvalidData("SigmaSize should be the same as Tgrid",__FILE__,__LINE__);

	s_grid = sgrid;
	mpInter->set(T_grid,s_grid);
	return;
}
