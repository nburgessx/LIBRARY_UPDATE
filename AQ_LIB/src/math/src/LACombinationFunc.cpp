/*! @file
    @brief Source code of class to represent function sums and products 

	This class derives from LAFunctionBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACombinationMethod.cpp
//
//  SYNOPSIS    :       LACombinationMethod
//  DESCRIPTION :       Source code of class to represent function sums and products
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


#include "LACombinationFunc.h"

using namespace std;
//================ LACombinationMethod ===================================
/*!
	@brief default constructor
	@param[in] checkflag check flag
*/
LACombinationMethod::LACombinationMethod(bool checkflag) 
: LAFunctionBase(checkflag), mConst(0)
{
}
/*!
	@brief copy constructor
*/
LACombinationMethod::LACombinationMethod(const LACombinationMethod& v) 
: LAFunctionBase(v)
{
	mConst = v.mConst;
	for (unsigned int i=0; i < v.mFuncs.size(); i++)
	{
		double a = v.mFuncs[i].first;
		vector<LAFunctionBase*> vfunc;
		for (unsigned int j=0; j < v.mFuncs[i].second.size(); j++)
		{
			vfunc.push_back(dynamic_cast<LAFunctionBase*>(v.mFuncs[i].second[j]->clone()));
		}
		mFuncs.push_back(pair<double,vector<LAFunctionBase*> >(a, vfunc));
	}
}

/*!
	@brief destructor
*/
LACombinationMethod::~LACombinationMethod() 
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
LACoreFunctionBase*	
LACombinationMethod::clone() const
{
    try 
	{
		return new LACombinationMethod(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LACombinationMethod::isTypeOf(function_t id) const
{
	return (id == FN_COMBINATIONFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LACombinationMethod::getType() const
{
	return FN_COMBINATIONFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LACombinationMethod::operator()(const DoubleArray& x) const
{
	if (mCheckFlag && !isInDomain(x))
	{
		throw LACoreNumericalError("Not in Domain" , __FILE__, __LINE__);
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
LACombinationMethod::operator()(double x) const
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
LACombinationMethod
LACombinationMethod::operator+(const LAFunctionBase& method) const
{
	LACombinationMethod combifunc(*this);
	if (method.isCheck()) combifunc.mCheckFlag = true;

	vector<LAFunctionBase*> vfunc;
	vfunc.push_back(dynamic_cast<LAFunctionBase*>(method.clone()));
	combifunc.mFuncs.push_back(pair<double, vector<LAFunctionBase*> >(1, vfunc));

	set<double> points = method.getDiscontPoints();
	points.insert(mDiscontPoints.begin(), mDiscontPoints.end());
	combifunc.setDiscontPoints(points);
	
	return combifunc;
}

/*!
    @brief Operator definition of "+"
	@param[in] a added constant
*/
LACombinationMethod
LACombinationMethod::operator+(double a) const
{
	LACombinationMethod combifunc(*this);
	combifunc.mConst += a;
	return combifunc;
}

/*!
    @brief Operator definition of "*"
	@param[in] method multiplied function
*/
LACombinationMethod
LACombinationMethod::operator*(const LAFunctionBase& method) const
{
	LACombinationMethod combifunc(*this);
	if (method.isCheck()) combifunc.mCheckFlag = true;

	for (unsigned int i=0; i < mFuncs.size(); i++)
	{
		combifunc.mFuncs[i].second.push_back(dynamic_cast<LAFunctionBase*>(method.clone()));
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
LACombinationMethod
LACombinationMethod::operator*(double a) const
{
	LACombinationMethod combifunc(*this);
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
LACombinationMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
										CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);

	if (mCheckFlag && !isDifferentiable(x, pos))
	{
		throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
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
	catch(LACoreError& e)
	{
		LACoreNumericalError err("", __FILE__, __LINE__);
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
LACombinationMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
										CALC_TYPE calctype, double delta) const
{

	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x,posi,posj,calctype,delta);

	if (mCheckFlag && !isDifferentiable(x, posi, posj))
	{
		throw LACoreNumericalError("Not Derivable" , __FILE__, __LINE__);
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
	catch(LACoreError& e)
	{
		LACoreNumericalError err("", __FILE__, __LINE__);
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
LACombinationMethod::isInDomain(const DoubleArray& x)const
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
	catch(LACoreError& e)
	{
		LACoreNumericalError err("", __FILE__, __LINE__);
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
LACombinationMethod::isDifferentiable(const DoubleArray& x, unsigned int pos)const
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
	catch(LACoreError& e)
	{
		LACoreNumericalError err("", __FILE__, __LINE__);
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
LACombinationMethod::isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const
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
	catch(LACoreError& e)
	{
		LACoreNumericalError err("", __FILE__, __LINE__);
		err += e;
		throw err;
	}
}

/*!
    @brief Set check flag
	@param[in] checkflag check flag to be set
*/	
void
LACombinationMethod::setCheckFlag(bool checkflag)
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
LACombinationMethod& 
LACombinationMethod::operator = (const LACombinationMethod& v)
{
	if (&v == this) return *this;
	LAFunctionBase::operator =(v);
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
		vector<LAFunctionBase*> vfunc;
		for (unsigned int j=0; j < v.mFuncs[i].second.size(); j++)
		{
			vfunc.push_back(dynamic_cast<LAFunctionBase*>(v.mFuncs[i].second[j]->clone()));
		}
		mFuncs.push_back(pair<double,vector<LAFunctionBase*> >(a, vfunc));
	}
	return *this;
}

