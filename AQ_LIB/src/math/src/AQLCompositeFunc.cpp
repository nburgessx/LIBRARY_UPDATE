/*! @file
    @brief Source code of class to represent composite function of two functions like f(x,y,z) = f1(x,y) * f2(x,z), (f1+f2,f1-f2,f1/f2 are also available)

	This class derives from AQLFunctionBase
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCompositeFunc.h"

using namespace std;
//================ AQLCompositeMethod ===================================
/*!
	@brief default constructor
	@param[in] var variable names of composite function
	@param[in] func1 function1
	@param[in] var1 variable names of func1
	@param[in] func2 function2
	@param[in] var2 variable names of func2
	@param[in] type operation type
	@param[in] checkflag check flag

	@note Example of f(x, y, z) = f1(x,y) - f2(z,x)
		input variables are 
		var[0] = "x", var[1] = "y", var[2] = "z"
		func1 = f1,	var1[0] = "x", var1[1] = "y"
		func2 = f1,	var2[0] = "z", var1[1] = "x"
		type = SUBTRACTION
*/
AQLCompositeMethod::AQLCompositeMethod(const AQLStringVector& var,
					const AQLFunctionBase& func1, const AQLStringVector& var1,
					const AQLFunctionBase& func2, const AQLStringVector& var2,
					OPERATION_TYPE type,	
					bool checkflag) 
: AQLFunctionBase(checkflag), mType(type), mpF1(NULL), mpF2(NULL)
{
	mpF1 = dynamic_cast<AQLFunctionBase*>(func1.clone());
	mpF2 = dynamic_cast<AQLFunctionBase*>(func2.clone());
	mVarPos1.resize(var1.size());
	mVarPos2.resize(var2.size());
	mVar1.resize(var1.size());
	mVar2.resize(var2.size());

	unsigned int count1 = 0, count2 = 0;
	for (unsigned int i = 0; i < var.size(); i++)
	{
		if (count1 < var1.size())
			for (unsigned int j = 0; j < var1.size(); j++)
			{
				if (var1[j] != var[i]) continue;
				mVarPos1[j] = i;
				count1++;
				break;
			}

		if (count2 < var2.size())
			for (unsigned int j = 0; j < var2.size(); j++)
			{
				if (var2[j] != var[i]) continue;
				mVarPos2[j] = i;
				count2++;
				break;
			}
	}

	if (count1 != var1.size() || count2 != var2.size())
	{
		//error
		throw AQLCoreInvalidData("Input is something wrong", __FILE__, __LINE__);
	}	

}
/*!
	@brief copy constructor
*/
AQLCompositeMethod::AQLCompositeMethod(const AQLCompositeMethod& v) 
: AQLFunctionBase(v), 
mType(v.mType), mpF1(NULL), mpF2(NULL), mVarPos1(v.mVarPos1), mVarPos2(v.mVarPos2)
, mVar1(v.mVar1), mVar2(v.mVar2)
{
	mpF1 = dynamic_cast<AQLFunctionBase*>(v.mpF1->clone());
	mpF2 = dynamic_cast<AQLFunctionBase*>(v.mpF2->clone());
}

/*!
	@brief destructor
*/
AQLCompositeMethod::~AQLCompositeMethod() 
{
	delete mpF1;
	delete mpF2;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLCompositeMethod::clone() const
{
    try 
	{
		return new AQLCompositeMethod(*this);
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
AQLCompositeMethod::isTypeOf(function_t id) const
{
	return (id == FN_COMPOSITEFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLCompositeMethod::getType() const
{
	return FN_COMPOSITEFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompositeMethod::operator()(const DoubleArray& x) const
{
	if (mCheckFlag && !isInDomain(x))
	{
		throw AQLCoreNumericalError("Not in Domain" , __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < mVar1.size(); i++)
		mVar1[i] = x[mVarPos1[i]];
	for (unsigned int i = 0; i < mVar2.size(); i++)
		mVar2[i] = x[mVarPos2[i]];
	
	switch (mType)
	{
	case ADDITION:
		return (*mpF1)(mVar1) + (*mpF2)(mVar2);		
	case SUBTRACTION:
		return (*mpF1)(mVar1) - (*mpF2)(mVar2);		
	case MULTIPLICATION:
		return (*mpF1)(mVar1) * (*mpF2)(mVar2);	
	default:
		return (*mpF1)(mVar1) / (*mpF2)(mVar2);	
	}

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
AQLCompositeMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);

	if (mCheckFlag && !isDifferentiable(x, pos))
	{
		throw AQLCoreNumericalError("Not Derivable" , __FILE__, __LINE__);
	}	

	try
	{
		int pos1 = -1, pos2 = -1;
		for (unsigned int i = 0; i < mVarPos1.size(); i++)
			if (mVarPos1[i] == pos)
			{
				pos1 = i;
				break;
			}
		for (unsigned int i = 0; i < mVarPos2.size(); i++)
			if (mVarPos2[i] == pos)
			{
				pos2 = i;
				break;
			}
		
		for (unsigned int i = 0; i < mVar1.size(); i++)
			mVar1[i] = x[mVarPos1[i]];
		for (unsigned int i = 0; i < mVar2.size(); i++)
			mVar2[i] = x[mVarPos2[i]];


		double d1 = 0.0, d2 = 0.0; 
		if (pos1 != -1) d1 = mpF1->partialDerivative(mVar1, pos1, calctype);
		if (pos2 != -1) d2 = mpF2->partialDerivative(mVar2, pos2, calctype);

		switch (mType)
		{
		case ADDITION:
			return d1 + d2; 
		case SUBTRACTION:
			return d1 - d2; 
		case MULTIPLICATION:
			return (*mpF1)(mVar1) * d2 +  (*mpF2)(mVar2) * d1;
		default:
			double ret =  (*mpF2)(mVar2);
			return d1 / ret
				- (*mpF1)(mVar1) * d2 / ret / ret;	
		}
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
AQLCompositeMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
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
		int posi1 = -1, posi2 = -1, posj1 = -1, posj2 = -1;
		for (unsigned int i = 0; i < mVarPos1.size(); i++)
		{
			if (mVarPos1[i] == posi)
				posi1 = i;
			if (mVarPos1[i] == posj)
				posj1 = i;
		}
		for (unsigned int i = 0; i < mVarPos2.size(); i++)
		{
			if (mVarPos2[i] == posi)
				posi2 = i;
			if (mVarPos2[i] == posj)
				posj2 = i;
		}
		
		for (unsigned int i = 0; i < mVar1.size(); i++)
			mVar1[i] = x[mVarPos1[i]];
		for (unsigned int i = 0; i < mVar2.size(); i++)
			mVar2[i] = x[mVarPos2[i]];


		double f1 = 0.0, f2 = 0.0, di1 = 0.0, di2 = 0.0, dj1 = 0.0, dj2 = 0.0, ddij1 = 0.0, ddij2 = 0.0; 
		if (mType == MULTIPLICATION || mType == DIVISION)
		{
			f1 =  (*mpF1)(mVar1);
			f2 =  (*mpF2)(mVar2);
			if (posi1 != -1) di1 = mpF1->partialDerivative(mVar1, posi1, calctype);
			if (posj1 != -1) dj1 = mpF1->partialDerivative(mVar1, posj1, calctype);
			if (posi2 != -1) di2 = mpF2->partialDerivative(mVar2, posi2, calctype);
			if (posj2 != -1) dj2 = mpF2->partialDerivative(mVar2, posj2, calctype);
		}
		if (posi1 != -1 && posj1 != -1) ddij1 = mpF1->partialDerivative2(mVar1, posi1, posj1, calctype);
		if (posi2 != -1 && posj2 != -1) ddij2 = mpF2->partialDerivative2(mVar2, posi2, posj2, calctype);

		switch (mType)
		{
		case ADDITION:
			return ddij1 + ddij2;
		case SUBTRACTION:
			return ddij1 - ddij2; 
		case MULTIPLICATION:
			return ddij1 * f2 + di1 * dj2 + dj1 * di2 + f1 * ddij2;
		default:
			return ddij1 / f2 - (di1 * dj2 + dj1 * di2 + f1 * ddij2) / f2 / f2 + 2 * f1 * di2 * dj2 / f2 / f2 / f2;	
		}
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
AQLCompositeMethod::isInDomain(const DoubleArray& x)const
{
	try
	{
		if (mpF1->isCheck())
		{
			for (unsigned int i = 0; i < mVar1.size(); i++)
				mVar1[i] = x[mVarPos1[i]];
			if (!mpF1->isInDomain(mVar1)) return false;
		}
		
		if (mpF2->isCheck())
		{
			for (unsigned int i = 0; i < mVar2.size(); i++)
				mVar2[i] = x[mVarPos2[i]];
			if (!mpF2->isInDomain(mVar2)) return false;
		}
		else if (mType == DIVISION)
		{
			for (unsigned int i = 0; i < mVar2.size(); i++)
				mVar2[i] = x[mVarPos2[i]];		
		}
		
		if (mType == DIVISION && (*mpF2)(mVar2) == 0) return false;

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
AQLCompositeMethod::isDifferentiable(const DoubleArray& x, unsigned int pos)const
{
	try
	{
		if (mpF1->isCheck())
		{
	
			for (unsigned int i = 0; i < mVarPos1.size(); i++)
				if (mVarPos1[i] == pos)
				{
					for (unsigned int j = 0; j < mVar1.size(); j++)
						mVar1[j] = x[mVarPos1[j]];
					if (!mpF1->isDifferentiable(mVar1, i)) return false;
					break;
				}
		}
		
		if (mpF2->isCheck())
		{
			for (unsigned int i = 0; i < mVarPos2.size(); i++)
				if (mVarPos2[i] == pos)
				{
					for (unsigned int j = 0; i < mVar2.size(); j++)
						mVar2[j] = x[mVarPos2[j]];
					if (!mpF2->isDifferentiable(mVar2, i)) return false;
					break;
				}		
		}
		else if (mType == DIVISION)
		{
			for (unsigned int i = 0; i < mVar2.size(); i++)
				mVar2[i] = x[mVarPos2[i]];		
		}
		
		if (mType == DIVISION && (*mpF2)(mVar2) == 0) return false;

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
AQLCompositeMethod::isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const
{
	try
	{
		if (mpF1->isCheck())
		{
			int posi1 = -1, posj1 = -1;
			for (unsigned int i = 0; i < mVarPos1.size(); i++)
			{
				if (mVarPos1[i] == posi)
					posi1 = i;
				else if (mVarPos1[i] == posj)
					posj1 = i;
			}

			if (posi1 != -1 || posi1 != -1)
            {
				for (unsigned int i = 0; i < mVar1.size(); i++)
					mVar1[i] = x[mVarPos1[i]];
            }
			if (posi1 != -1 && posi1 != -1)
            {
				if (!mpF1->isDifferentiable(mVar1, posi1, posj1)) return false;
            }
			else if (posi1 != -1)
            {
				if (!mpF1->isDifferentiable(mVar1, posi1)) return false;
            }
			else if (posj1 != -1)
            {
				if (!mpF1->isDifferentiable(mVar1, posj1)) return false;
            }
				
		}
		if (mpF2->isCheck())
		{
			int posi2 = -1, posj2 = -1;
			for (unsigned int i = 0; i < mVarPos2.size(); i++)
			{
				if (mVarPos2[i] == posi)
					posi2 = i;
				else if (mVarPos2[i] == posj)
					posj2 = i;
			}
	
			if (posi2 != -1 || posi2 != -1)
				for (unsigned int i = 0; i < mVar2.size(); i++)
					mVar2[i] = x[mVarPos2[i]];
			if (posi2 != -1 && posi2 != -1)
				if (!mpF2->isDifferentiable(mVar2, posi2, posj2)) return false;			
			else if (posi2 != -1)
				if (!mpF2->isDifferentiable(mVar2, posi2)) return false;
			else if (posj2 != -1)
				if (!mpF2->isDifferentiable(mVar2, posj2)) return false;
				
		}
		else if (mType == DIVISION)
		{
			for (unsigned int i = 0; i < mVar2.size(); i++)
				mVar2[i] = x[mVarPos2[i]];		
		}
		
		if (mType == DIVISION && (*mpF2)(mVar2) == 0) return false;

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
AQLCompositeMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag = checkflag;
	mpF1->setCheckFlag(checkflag);
	mpF2->setCheckFlag(checkflag);
}

