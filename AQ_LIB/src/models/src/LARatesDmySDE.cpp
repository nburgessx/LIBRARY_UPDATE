/*! @file
    @brief Source code of class for no simulation SDE

	This class derives from LARatesSDEBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesDmySDE.h
//
//  SYNOPSIS    :       LARatesDmySDE
//  DESCRIPTION :       Class declaration for no simulation SDE
//						This class derives from LARatesSDEBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARatesDmySDE.h"
#include "LARatesCurveLogLinearInterpolationDmy.h"

using namespace std;

//================ LARatesDmySDE ===================================
/*!
	@brief constructor
*/

LARatesDmySDE::LARatesDmySDE()
: LARatesSDEBase(dX)
{
}

/*!
	@brief destructor
*/
LARatesDmySDE::~LARatesDmySDE(void)
{

}


/*!
	@brief copy constructor
*/
LARatesDmySDE::LARatesDmySDE(const LARatesDmySDE &rhs) 
: LARatesSDEBase(rhs)
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesDmySDE::isTypeOf(function_t id) const
{
	return (id==FN_DMYSDE ? true : LARatesSDEBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesDmySDE::getType() const
{
	return FN_DMYSDE;
}


/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesDmySDE::clone() const
{
    try 
	{
		return new LARatesDmySDE(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief set up this class 

*/
void
LARatesDmySDE::setUp()
{
	LARatesSDEBase::setUp();

	const DoubleArray &initialVal = mPath[0]->get();
	const int pathSize = mPath.size();

	if (mpInter->isTypeOf(FN_CURVELOGLINEARINTERPOLATIONDMY))
	{
		dynamic_cast<LARatesCurveLogLinearInterpolationDmy *>(mpInter)->clearVal();
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
			mpNumeraire->setCurve(mTimeGrid[i], dynamic_cast<const LARatesPathElementCurve*>(mPath[i]));
		}
	}	
}

/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
const LARatesPathElementBase*
LARatesDmySDE::getPathElement(unsigned int pos)
{
	const DoubleArray& grid = mpBM->getTimeGrid();
	if (pos > grid.size() - 1)
	{
		throw LACoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	unsigned int pos_e;
	LAAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), pos_e);

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
LARatesDmySDE::calcPath(unsigned int pos)
{
	pos;
}
