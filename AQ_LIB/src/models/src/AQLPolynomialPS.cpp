/*! @file
    @brief Source code of power series plynomial class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPolynomialPS.h"

using namespace std;


//================ AQLPolynomialPS ===================================
/*!
	@brief default constructor
	@param[in] order order of polynomial
	@param[in] varnum number of variables
*/
AQLPolynomialPS::AQLPolynomialPS(unsigned int order, unsigned int varnum)
: AQLPolynomialBase(order, varnum)
{

}
/*!
	@brief copy constructor
*/
/*AQLPolynomialPS::AQLPolynomialPS(const AQLPolynomialPS& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLPolynomialPS::~AQLPolynomialPS() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPolynomialPS::clone() const
{
    try 
	{
		return new AQLPolynomialPS(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPolynomialPS::isTypeOf(function_t id) const
{
	return (id == FN_POLYNOMIALPS ? true : AQLPolynomialBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPolynomialPS::getType() const
{
	return FN_POLYNOMIALPS;
}
/*!
	@brief calculate basis function
	@param[in] x variables
	@param[out] basis_func values of basis functions
*/
void
AQLPolynomialPS::calcBasisFunc(const DoubleArray& x, DoubleArray& basis_func) const
{
	if (x.size() != mNumVar)
	{
		//error
		throw AQLCoreInvalidData("size of x is not same as mNumVar", __FILE__, __LINE__);
	}


    DoubleMatrix tmpPoly(mOrderBF + 1);
	std::vector<UintArray> tmpNum(mOrderBF + 1);
	for (unsigned int i = 0; i < tmpPoly.size(); i++)
	{
		tmpPoly[i].resize(combin(i + mNumVar - 1, i));	
		tmpNum[i].resize(tmpPoly[i].size());
	}

	// 0-order
    tmpPoly[0][0] = 1.0;
    // 1-order
    if (1 <= mOrderBF)
	{	
		tmpPoly[1] = x;
		for (unsigned int i = 0; i < tmpNum[1].size(); i++)
			tmpNum[1][i] = i;
	}
    // more than 2-order
    for (unsigned int i = 2; i <= mOrderBF; i++)
    {
        unsigned int l = 0;
		for (unsigned int j = 0; j < tmpNum[i - 1].size(); j++)
        {
            for (unsigned int k = tmpNum[i - 1][j]; k < mNumVar; k++)
            {
                tmpNum[i][l] = k;
				tmpPoly[i][l++] = tmpPoly[i - 1][j] * x[k];            
            }
        }
    }

	unsigned int size = combin(mNumVar + mOrderBF, mOrderBF);
	if (basis_func.size() != size) basis_func.resize(size);
		
    //construction of basis
	unsigned int k = 0;  
	for (unsigned int i = 0; i < mOrderBF + 1; i++)
    {
        for (unsigned int j = 0; j < tmpPoly[i].size(); j++)
        {
            basis_func[k++] = tmpPoly[i][j];
        }
    }

}
	


