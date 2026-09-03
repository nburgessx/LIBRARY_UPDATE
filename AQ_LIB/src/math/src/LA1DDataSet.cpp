/*! @file
    @brief Source code for class to express functions with data points and interplaton

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LA1DDataSet.cpp
//
//  SYNOPSIS    :       LA1DDataSet
//  DESCRIPTION :       Class to express functions with data points and interpolation
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LA1DDataSet.h"
#include "LAIntegralBase.h"
#include "LAInterpolationBase.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAAlgorithm.h"

using namespace std;
//================ LA1DDataSet ===================================
/*!
	@brief Default constructor
	@param[in] checkflag check flag
*/
LA1DDataSet::LA1DDataSet(bool checkflag) 
: LAFunctionBase(checkflag), mpInter(NULL),
    mGridDifferentiableFlag1(false), 
    mGridDifferentiableFlag2(false)
{
}
/*!
	@brief copy constructor
*/
LA1DDataSet::LA1DDataSet(const LA1DDataSet& v) 
: LAFunctionBase(v), mpInter(NULL)
{
	mGrids = v.mGrids;
	mGridDifferentiableFlag1 = v.mGridDifferentiableFlag1;
	mGridDifferentiableFlag2 = v.mGridDifferentiableFlag2;
	if (v.mpInter) mpInter = dynamic_cast<LAInterpolationBase*>(v.mpInter->clone());
}

/*!
	@brief Destructor
*/
LA1DDataSet::~LA1DDataSet() 
{
	if (mpInter) delete mpInter;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LA1DDataSet::clone() const
{
    try 
	{
		return new LA1DDataSet(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID ID to check
    @return True or False
*/
bool
LA1DDataSet::isTypeOf(function_t id) const
{
	return (id == FN_1DDATASET ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LA1DDataSet::getType() const
{
	return FN_1DDATASET;
}

/*!
    @brief Return interpolation type
    @return interpolation type
*/
function_t
LA1DDataSet::getInterpolationType() const
{
	if (mpInter == NULL) return NULL_INTERPOLATION;
	return mpInter->getType();
}


/*!
    @brief Return interpolated value
	@param[in] x point
    @return interpolated value
*/
double
LA1DDataSet::operator()(const DoubleArray& x) const
{
	if ((mCheckFlag && x.size() != 1) || x.size() == 0)
	{
		throw LACoreInvalidData("x size is wrong", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}

/*!
    @brief Return interpolated value
	@param[in] x point
    @return interpolated value
*/
double
LA1DDataSet::operator()(double x) const
{
	if (mGrids.size() == 0)
	{
		throw LACoreInvalidData("No data is set", __FILE__, __LINE__);
	}
	if (mpInter == NULL)
	{
		unsigned int pos;
		if (LAAlgorithm::find<DoubleArray, double>(mGrids, x, 0, mGrids.size() - 1, pos))
			return mParam[pos];
		else
			return 0;
	}
	if (mUpdateFlag)
	{
		mpInter->set(mGrids, mParam);
		mUpdateFlag = false;
	}
	return mpInter->value(x);
}

/*!
    @brief Set up interpolation function
	@param[in] v reference to interpolation function
*/
void
LA1DDataSet::setInterpolation(const LAInterpolationBase& v)
{
	mUpdateFlag = true;
	mpInter = dynamic_cast<LAInterpolationBase*>(v.clone());
	switch(mpInter->getType())
	{
	case FN_STEPINTERPOLATION:
		setGridDerivableFlag(false, false);
		break;
	case FN_LINEARINTERPOLATION:
		setGridDerivableFlag(false, false);
		break;
	case FN_SPLINEINTERPOLATION:
		setGridDerivableFlag(true, true);
		break;
	}

	if (mpInter == NULL ||
		(mpInter->isTypeOf(FN_STEPINTERPOLATION) || mpInter->isTypeOf(FN_LINEARINTERPOLATION)))
	{
		mDiscontPoints.clear();
		for (unsigned int i = 0; i < mGrids.size(); i++)
			mDiscontPoints.insert(mGrids[i]);
	}
}

/*!
    @brief Set points & values
	@param[in] x point
	@param[in] y function value
*/
void
LA1DDataSet::set(const DoubleArray& x, const DoubleArray& y)

{	
	if (x.size() != y.size())
	{
		throw LACoreInvalidData("x size and y size must be one", __FILE__, __LINE__);
	}
	mUpdateFlag = true;
	mGrids = x;
	mParam = y;

	if (mpInter == NULL ||
		(mpInter->isTypeOf(FN_STEPINTERPOLATION) || mpInter->isTypeOf(FN_LINEARINTERPOLATION)))
	{
		mDiscontPoints.clear();
		for (unsigned int i = 0; i < mGrids.size(); i++)
			mDiscontPoints.insert(mGrids[i]);
	}
};

/*!
    @brief Return integral result
	@param[in] x integral region
	@param[in] pIntegral pointer to integral method
    @return integral result
*/
double
LA1DDataSet::integral(const std::vector<std::pair<double,double> >& x,
					  const LAIntegralBase* pIntegral)const
{
	if (x.size() != 1)
	{
		throw LACoreInvalidData("vecotr size must be one", __FILE__, __LINE__);
	}
	return integral(x[0].first, x[0].second, pIntegral);
}

/*!
    @brief Return integral result
	@param[in] x1 lower side of integral region
	@param[in] x2 upper side of integral region
	@param[in] pIntegral pointer to integral method
    @return integral result
*/
double
LA1DDataSet::integral(double x1, double x2, const LAIntegralBase* pIntegral) const
{
	if (mpInter == NULL) return 0;//No intepolation methods
    if (pIntegral == NULL) return integral(x1, x2);
	if (x1 > x2) return integral(x2, x1, pIntegral);
	
	if (mUpdateFlag)
	{
		mpInter->set(mGrids, mParam);
		mUpdateFlag = false;
	}

	unsigned int pos1, pos2;
	LAAlgorithm::locate<DoubleArray,double>(mGrids, x1, mGrids.size(), pos1);
	LAAlgorithm::locate<DoubleArray,double>(mGrids, x2, mGrids.size(), pos2);

	double sum = 0.0;
	//Case pIntegral != NULL
	//We numerically integrate.
	vector<pair<double,double> > x(1);
	if (pos1 == pos2)
	{
        x[0].first = x1;
		x[0].second = x2;
		return pIntegral->integrate(*this, x);
	}

	sum = 0.0;
    x[0].first = x1;
	x[0].second = mGrids[pos1];
	sum += pIntegral->integrate(*this,x);
	for (unsigned int i = pos1; i < pos2 - 1; i++)
	{
		x[0].first = mGrids[i];
		x[0].second = mGrids[i+1];
		sum += pIntegral->integrate(*this, x);
	}
	x[0].first = mGrids[pos2-1];
	x[0].second = x2;
	sum += pIntegral->integrate(*this, x);
	return sum;
	
}
/*!
    @brief Return integral result
	@param[in] x1 lower side of integral region
	@param[in] x2 upper side of integral region
    @return integral result
*/
double
LA1DDataSet::integral(double x1, double x2) const
{
	if (mpInter == NULL) return 0;//No interpolation methods
	if (x1 > x2) return integral(x2, x1);

	if (mUpdateFlag)
	{
		mpInter->set(mGrids, mParam);
		mUpdateFlag = false;
	}

	unsigned int pos1, pos2;
	LAAlgorithm::locate<DoubleArray, double>(mGrids, x1, mGrids.size(), pos1);
	LAAlgorithm::locate<DoubleArray, double>(mGrids, x2, mGrids.size(), pos2);

	double sum = 0.0;
	double xx1, xx2;
	switch(mpInter->getType())
	{
	case FN_STEPINTERPOLATION:
	case FN_LINEARINTERPOLATION:
		if (pos1 == pos2)
		{
    		return	(x2 - x1) * mpInter->value(0.5 * (x2 + x1));
		}
		sum = 0.0;
        xx1 = x1;
		xx2 = mGrids[pos1];
		sum += (xx2 - xx1) * mpInter->value(0.5 * (xx2 + xx1));
		for (unsigned int i = pos1; i < pos2 - 1; i++)
		{
			xx1 = mGrids[i];
			xx2 = mGrids[i + 1];
			sum += (xx2 - xx1) * mpInter->value(0.5 * (xx2 + xx1));
		}
		xx1 = mGrids[pos2-1];
		xx2 = x2;
		sum += (xx2 - xx1) * mpInter->value(0.5 * (xx2 + xx1));
		return sum;		
	default:
		throw LACoreNumericalError("This function does not support this method", __FILE__, __LINE__);
	}

}

/*!
    @brief Return partial derivative value
	@param[in] x point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
LA1DDataSet::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == DEFAULT)
	{
		if (mpInter != NULL && 
			(mpInter->getType() == FN_STEPINTERPOLATION || mpInter->getType() == FN_LINEARINTERPOLATION))
			return partialDerivative(x, pos, ANALYTICAL);
		else
			return partialDerivative(x, pos, NUMERICAL, difftype, delta);	
	}
	
	if (pos != 0 || x.size() != 1)
	{
		throw LACoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);

	
	unsigned int upos;
	bool isOn = LAAlgorithm::find<DoubleArray, double>(mGrids, x[0], 0, mGrids.size() - 1, upos);
	if (mpInter == NULL)
	{
		if (!isOn)
			return 0.0;
		else
			throw LACoreNumericalError("Non Derivable." , __FILE__, __LINE__);
	}
	

	switch(mpInter->getType())
	{
	case FN_STEPINTERPOLATION:
		if (!isOn)
			return 0.0;
		else
			throw LACoreNumericalError("Non Derivable." , __FILE__, __LINE__);
	case FN_LINEARINTERPOLATION:
		if (!isOn)
		{
			LAAlgorithm::locate<DoubleArray, double>(mGrids, x[0], mGrids.size(), upos);
			if (upos == 0 || upos == mGrids.size()) return 0.0;
			else return (mParam[upos] - mParam[upos - 1]) / (mGrids[upos] - mGrids[upos - 1]);
		}
		else
			throw LACoreNumericalError("Non Derivable." , __FILE__, __LINE__);	
		;		
	default:
		throw LACoreNumericalError("This function does not support this method", __FILE__, __LINE__);
	};

}
/*!
    @brief Return second partial derivative value
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
LA1DDataSet::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == DEFAULT)
	{
		if (mpInter != NULL && 
			(mpInter->getType() == FN_STEPINTERPOLATION || mpInter->getType() == FN_LINEARINTERPOLATION))
			return partialDerivative2(x, posi, posj, ANALYTICAL);
		else
			return partialDerivative2(x, posi, posj, NUMERICAL, delta);	
	}

	if (posi != 0 || posj != 0 || x.size() != 1)
	{
		throw LACoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	
	unsigned int upos;
	bool isOn = LAAlgorithm::find<DoubleArray,double>(mGrids, x[0], 0, mGrids.size() - 1, upos);
	if (mpInter == NULL)
	{
		if (!isOn)
			return 0.0;
		else
			throw LACoreNumericalError("Non Derivable." , __FILE__, __LINE__);
	}
	
	switch(mpInter->getType())
	{
	case FN_STEPINTERPOLATION:
	case FN_LINEARINTERPOLATION:
		if (!isOn)
			return 0.0;
		else
			throw LACoreNumericalError("Non Derivable." , __FILE__, __LINE__);
	default:
		throw LACoreNumericalError("This function does not support this method", __FILE__, __LINE__);
	};
}
/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] pos variable location to implement derivative
    @return derivable or not
*/
bool
LA1DDataSet::isDifferentiable(const DoubleArray& x, unsigned int pos)const
{
	(void)pos;

	unsigned int upos;
	if (!LAAlgorithm::find<DoubleArray,double>(mGrids, x[0], 0, mGrids.size() - 1, upos))
		return true;
	else 
		return mGridDifferentiableFlag1;

}
								
/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
    @return derivable or not
*/	
bool
LA1DDataSet::isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj)const
{
	(void)posi;
	(void)posj;
	unsigned int upos;
	if (!LAAlgorithm::find<DoubleArray,double>(mGrids, x[0], 0, mGrids.size() - 1, upos))
		return true;
	else 
		return mGridDifferentiableFlag2;
}
