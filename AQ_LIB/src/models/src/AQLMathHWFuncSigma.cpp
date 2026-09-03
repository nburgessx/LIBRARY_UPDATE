#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif
#include "AQLMathHWFuncSigma.h"

using namespace std;
//
//------------------------------ AQLMathHWFuncSigma ------------------------------
//

/*!
	@brief constructor

	@param[in] s      function value

*/
AQLMathHWFuncSigma:: AQLMathHWFuncSigma( double sigma )
: AQLFunctionBase(), mSigma(sigma)
{
}

/*!
	@brief destructor
*/
AQLMathHWFuncSigma::~AQLMathHWFuncSigma(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathHWFuncSigma::AQLMathHWFuncSigma(const AQLMathHWFuncSigma &rhs) 
: AQLFunctionBase(rhs), mSigma(rhs.mSigma)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncSigma::clone() const
{
    try 
	{
		return new AQLMathHWFuncSigma(*this);
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
AQLMathHWFuncSigma::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCSIGMA ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathHWFuncSigma::getType() const
{
	return FN_HWFUNCSIGMA;
}

//
//------------------------------ AQLMathHWFuncSigmaTMDPT ------------------------------
//

/*!
	@brief constructor

	@param[in] T_Grid function value
	@param[in] s      function value

*/
AQLMathHWFuncSigmaTMDPT::AQLMathHWFuncSigmaTMDPT(const DoubleArray& T_grid_, const DoubleArray& s_grid_, AQLInterpolationBase& interpolation)
:
AQLMathHWFuncSigma(s_grid_[0]),
T_grid(T_grid_),
s_grid(s_grid_),
mpInter(&interpolation),
is_cloned(false)
{
	size_t n = T_grid.size();
	if ( n != s_grid.size() )
	{
		// error
		throw AQLCoreInvalidData(" T_grid.size() != s.size()", __FILE__, __LINE__);
	}
	mpInter->set(T_grid,s_grid);
}

/*!
	@brief destructor
*/
AQLMathHWFuncSigmaTMDPT::~AQLMathHWFuncSigmaTMDPT(void)
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
AQLMathHWFuncSigmaTMDPT::AQLMathHWFuncSigmaTMDPT(const AQLMathHWFuncSigmaTMDPT &rhs) 
:
AQLMathHWFuncSigma(rhs),
T_grid(rhs.T_grid),
s_grid(rhs.s_grid),
is_cloned(true)
{
	mpInter = rhs.mpInter != 0 ? dynamic_cast<AQLInterpolationBase* >(rhs.mpInter->clone()) : 0;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncSigmaTMDPT::clone() const
{
    try 
	{
		return new AQLMathHWFuncSigmaTMDPT(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

//
//AQLMathHWFuncSigmaTMDPT& AQLMathHWFuncSigmaTMDPT::operator =( const AQLMathHWFuncSigmaTMDPT& rhs )
//{
//    if(this == &rhs) return *this; 
//    
//    (*this).AQLMathHWFuncSigma::operator =(rhs);
//
//     T_grid = rhs.T_grid;
//     s_grid = rhs.s_grid;
//	 mpInter = rhs.mpInter != 0 ? dynamic_cast<AQLInterpolationBase* >(rhs.mpInter->clone()) : 0;
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
AQLMathHWFuncSigmaTMDPT::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCSIGMATMDPT ? true : AQLMathHWFuncSigma::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathHWFuncSigmaTMDPT::getType() const
{
	return FN_HWFUNCSIGMATMDPT;
}

/*!
    @brief set sigmavector
    @return void
*/

void AQLMathHWFuncSigmaTMDPT::set_s(const DoubleArray& sgrid)
{
	if(sgrid.size()!= T_grid.size())
		throw AQLCoreInvalidData("SigmaSize should be the same as Tgrid",__FILE__,__LINE__);

	s_grid = sgrid;
	mpInter->set(T_grid,s_grid);
	return;
}
