/*! @file
    @brief Source code of class for no simulation SDE

	This class derives from AQLRatesSDEBase

*/
//  2007, AlgoQuantHub.


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesDmySDE.h"
#include "AQLRatesCurveLogLinearInterpolationDmy.h"

using namespace std;

//================ AQLRatesDmySDE ===================================
/*!
	@brief constructor
*/

AQLRatesDmySDE::AQLRatesDmySDE()
: AQLRatesSDEBase(dX)
{
}

/*!
	@brief destructor
*/
AQLRatesDmySDE::~AQLRatesDmySDE(void)
{

}


/*!
	@brief copy constructor
*/
AQLRatesDmySDE::AQLRatesDmySDE(const AQLRatesDmySDE &rhs) 
: AQLRatesSDEBase(rhs)
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesDmySDE::isTypeOf(function_t id) const
{
	return (id==FN_DMYSDE ? true : AQLRatesSDEBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesDmySDE::getType() const
{
	return FN_DMYSDE;
}


/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesDmySDE::clone() const
{
    try 
	{
		return new AQLRatesDmySDE(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief set up this class 

*/
void
AQLRatesDmySDE::setUp()
{
	AQLRatesSDEBase::setUp();

	const DoubleArray &initialVal = mPath[0]->get();
	const int pathSize = mPath.size();

	if (mpInter->isTypeOf(FN_CURVELOGLINEARINTERPOLATIONDMY))
	{
		dynamic_cast<AQLRatesCurveLogLinearInterpolationDmy *>(mpInter)->clearVal();
	}

	for (int i = 0; i < pathSize; i++)
	{
		if (i != 0)
		{
			// set initial value
			mPath[i]->set(initialVal);
		}
		// numeraire initial curve set
		if (mpTemplate->isTypeOf(PE_CURVE) && mpNumeraire)
		{
			mpNumeraire->setInterpolationMethod(mpInter);
			mpNumeraire->setCurve(mTimeGrid[i], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[i]));
		}
	}	
}

/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
const AQLRatesPathElementBase*
AQLRatesDmySDE::getPathElement(unsigned int pos)
{
	const DoubleArray& grid = mpBM->getTimeGrid();
	if (pos > grid.size() - 1)
	{
		throw AQLCoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	unsigned int pos_e;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), pos_e);

	if (grid[pos] == mTimeGrid[pos_e])
	{
		return mPath[pos_e];
	}
	else
	{
		return &mpInter->value(grid[pos], mTimeGrid[pos_e - 1], mTimeGrid[pos_e], *mPath[pos_e - 1], *mPath[pos_e]);
	}
}

/*!
    @brief no calc path
*/
void
AQLRatesDmySDE::calcPath(unsigned int pos)
{
	pos;
}
