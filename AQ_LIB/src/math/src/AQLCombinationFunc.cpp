/*! @file
    @brief Source code of class to represent function sums and products 

	This class derives from AQLFunctionBase

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCombinationFunc.h"

using namespace std;
//================ AQLCombinationMethod ===================================
/*!
	@brief default constructor
	@param[in] checkflag check flag
*/
AQLCombinationMethod::AQLCombinationMethod(bool checkflag) 
: AQLFunctionBase(checkflag), mConst(0)
{
}
/*!
	@brief copy constructor
*/
AQLCombinationMethod::AQLCombinationMethod(const AQLCombinationMethod& v) 
: AQLFunctionBase(v)
{
	mConst = v.mConst;
	for (unsigned int i=0; i < v.mFuncs.size(); i++)
	{
		double a = v.mFuncs[i].first;
		vector<AQLFunctionBase*> vfunc;
		for (unsigned int j=0; j < v.mFuncs[i].second.size(); j++)
		{
			vfunc.push_back(dynamic_cast<AQLFunctionBase*>(v.mFuncs[i].second[j]->clone()));
		}
		mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(a, vfunc));
	}
}

/*!
	@brief destructor
*/
AQLCombinationMethod::~AQLCombinationMethod() 
{
	for (unsigned int i=0;i<mFuncs.size();i++)
	{
		for (unsigned int j=0; j<mFuncs[i].second.size(); j++)
		{			
			delete mFuncs[i].second[j];
		}
	}		
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLCombinationMethod::clone() const
{
    try 
	{
		return new AQLCombinationMethod(*this);
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
AQLCombinationMethod::isTypeOf(function_t id) const
{
	return (id == FN_COMBINATIONFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLCombinationMethod::getType() const
{
	return FN_COMBINATIONFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCombinationMethod::operator()(const DoubleArray& x) const
{
	if (mCheckFlag && !isInDomain(x))
	{
		throw AQLCoreNumericalError("Not in Domain" , __FILE__, __LINE__);
	}
	
	double ret=0.0;
	for (unsigned int i=0; i < mFuncs.size(); i++)
	{
		double a = mFuncs[i].first;
		for (unsigned int j=0; j < mFuncs[i].second.size(); j++)
		{
			a *= mFuncs[i].second[j]->operator()(x);
		}
		ret += a;
	}
	ret += mConst;
	return ret;
}

/*!
    @brief Return function value
	@param[in] x point, use mX except x
    @return function value
*/
double
AQLCombinationMethod::operator()(double x) const
{
	double ret=0.0;
	for (unsigned int i=0; i < mFuncs.size(); i++)
	{
		double a = mFuncs[i].first;
		for (unsigned int j=0; j < mFuncs[i].second.size(); j++)
		{
			a *= mFuncs[i].second[j]->operator()(x);
		}
		ret += a;
	}
	ret += mConst;
	return ret;
}

/*!
    @brief Operator definition of "+"
	@param[in] method added function
*/
AQLCombinationMethod
AQLCombinationMethod::operator+(const AQLFunctionBase& method) const
{
	AQLCombinationMethod combifunc(*this);
	if (method.isCheck()) combifunc.mCheckFlag = true;

	vector<AQLFunctionBase*> vfunc;
	vfunc.push_back(dynamic_cast<AQLFunctionBase*>(method.clone()));
	combifunc.mFuncs.push_back(pair<double, vector<AQLFunctionBase*> >(1, vfunc));

	set<double> points = method.getDiscontPoints();
	points.insert(mDiscontPoints.begin(), mDiscontPoints.end());
	combifunc.setDiscontPoints(points);
	
	return combifunc;
}

/*!
    @brief Operator definition of "+"
	@param[in] a added constant
*/
AQLCombinationMethod
AQLCombinationMethod::operator+(double a) const
{
	AQLCombinationMethod combifunc(*this);
	combifunc.mConst += a;
	return combifunc;
}

/*!
    @brief Operator definition of "*"
	@param[in] method multiplied function
*/
AQLCombinationMethod
AQLCombinationMethod::operator*(const AQLFunctionBase& method) const
{
	AQLCombinationMethod combifunc(*this);
	if (method.isCheck()) combifunc.mCheckFlag = true;

	for (unsigned int i=0; i < mFuncs.size(); i++)
	{
		combifunc.mFuncs[i].second.push_back(dynamic_cast<AQLFunctionBase*>(method.clone()));
	}

	set<double> points = method.getDiscontPoints();
	points.insert(mDiscontPoints.begin(), mDiscontPoints.end());
	combifunc.setDiscontPoints(points);

	return combifunc;
}

/*!
    @brief Operator definition of "*"
	@param[in] a multiplied constant
*/
AQLCombinationMethod
AQLCombinationMethod::operator*(double a) const
{
	AQLCombinationMethod combifunc(*this);
	for (unsigned int i=0; i<mFuncs.size(); i++)
	{
		combifunc.mFuncs[i].first *= a;
	}
	combifunc.mConst *= a;
	return combifunc;
}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
AQLCombinationMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
										CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);

	if (mCheckFlag && !isDifferentiable(x, pos))
	{
		throw AQLCoreNumericalError("Not Derivable" , __FILE__, __LINE__);
	}	

	try
	{
		double ret = 0.0;
		for (unsigned int i=0; i < mFuncs.size(); i++)
		{
			
			DoubleArray f(mFuncs[i].second.size());
			for (unsigned int j=0; j < mFuncs[i].second.size(); j++)
				f[j] = mFuncs[i].second[j]->operator()(x);			
			
			double ret2 = mFuncs[i].first;
			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
			{			
				ret2 *= mFuncs[i].second[j]->partialDerivative(x,pos,calctype);	
				for (unsigned int k = 0; k < mFuncs[i].second.size(); k++)
				{			
					if (j != k) ret2 *= f[k];
				}
			}
			ret += ret2;
		}
		return ret;
	}
	catch(AQLCoreError& e)
	{
		AQLCoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return second partial deribative value
*/
double
AQLCombinationMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
										CALC_TYPE calctype, double delta) const
{

	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x,posi,posj,calctype,delta);

	if (mCheckFlag && !isDifferentiable(x, posi, posj))
	{
		throw AQLCoreNumericalError("Not Derivable" , __FILE__, __LINE__);
	}	

	try
	{
		double ret = 0.0;
		for (unsigned int i = 0; i < mFuncs.size(); i++)
		{
			DoubleArray f(mFuncs[i].second.size());
			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
				f[j] = mFuncs[i].second[j]->operator()(x);
			
			double ret2 = mFuncs[i].first;
			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
			{			
				ret2 *= mFuncs[i].second[j]->partialDerivative2(x, posi, posj, calctype);	
				for (unsigned int k = 0; k < mFuncs[i].second.size(); k++)
				{			
					if (j != k) ret2 *= f[k];
				}
			}
			ret += ret2;

			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
			{			
				ret2 = mFuncs[i].first;
				ret2 *= mFuncs[i].second[j]->partialDerivative(x, posi, calctype);	
				for (unsigned int k = 0; k < mFuncs[i].second.size(); k++)
				{			
					if (j != k) ret2 *= mFuncs[j].second[j]->partialDerivative(x, posj, calctype);	
					for (unsigned int l=0;l<mFuncs[i].second.size();l++)
					{	
						if (j != l && k != l) ret *= f[l];
					}
				}	
				ret += ret2;
			}
		}
		return ret;
	}
	catch(AQLCoreError& e)
	{
		AQLCoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}


}

/*!
    @brief Return point is in legal domain or not
	@param[in] x point
    @return point is in legal domain or not
*/
bool
AQLCombinationMethod::isInDomain(const DoubleArray& x)const
{
	try
	{
		for (unsigned int i = 0; i < mFuncs.size(); i++)
		{
			for (unsigned int j = 0;j < mFuncs[i].second.size(); j++)
			{			
				if (mFuncs[i].second[j]->isCheck() && !mFuncs[i].second[j]->isInDomain(x))return false;
			}
		}
		return true;
	}
	catch(AQLCoreError& e)
	{
		AQLCoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] pos variable location to implement derivative
    @return derivable or not
*/
bool
AQLCombinationMethod::isDifferentiable(const DoubleArray& x, unsigned int pos)const
{
	try
	{
		for (unsigned int i = 0; i < mFuncs.size(); i++)
		{
			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
			{			
				if (mFuncs[i].second[j]->isCheck() && !mFuncs[i].second[j]->isDifferentiable(x, pos)) return false;
			}
		}
		return true;
	}
	catch(AQLCoreError& e)
	{
		AQLCoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
    @return derivable or not
*/	
bool
AQLCombinationMethod::isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const
{
	try
	{
		for (unsigned int i = 0; i < mFuncs.size(); i++)
		{
			for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
			{			
				if (mFuncs[i].second[j]->isCheck() && !mFuncs[i].second[j]->isDifferentiable(x, posi, posj)) return false;
			}
		}
		return true;
	}
	catch(AQLCoreError& e)
	{
		AQLCoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}
}

/*!
    @brief Set check flag
	@param[in] checkflag check flag to be set
*/	
void
AQLCombinationMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag = checkflag;
	for (unsigned int i = 0; i < mFuncs.size(); i++)
	{
		for (unsigned int j = 0; j < mFuncs[i].second.size(); j++)
		{			
			mFuncs[i].second[j]->setCheckFlag(checkflag);
		}
	}
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
AQLCombinationMethod& 
AQLCombinationMethod::operator = (const AQLCombinationMethod& v)
{
	if (&v == this) return *this;
	AQLFunctionBase::operator =(v);
	for (unsigned int i = 0; i < mFuncs.size(); i++)
	{
		for (unsigned int j=0; j<mFuncs[i].second.size(); j++)
		{			
			delete mFuncs[i].second[j];
		}
	}
	mFuncs.clear();
	
	mConst = v.mConst;
	for (unsigned int i=0; i < v.mFuncs.size(); i++)
	{
		double a = v.mFuncs[i].first;
		vector<AQLFunctionBase*> vfunc;
		for (unsigned int j=0; j < v.mFuncs[i].second.size(); j++)
		{
			vfunc.push_back(dynamic_cast<AQLFunctionBase*>(v.mFuncs[i].second[j]->clone()));
		}
		mFuncs.push_back(pair<double,vector<AQLFunctionBase*> >(a, vfunc));
	}
	return *this;
}

