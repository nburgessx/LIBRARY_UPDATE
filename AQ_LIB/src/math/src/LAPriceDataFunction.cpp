/*! @file
    @brief Source code of class representing a mathmatical function

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDataFunction.cpp
//
//  DESCRIPTION :       Source code of class representing a mathmatical function
//                      
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceDataFunction.h"
#include "LAFunctionManager.h"
#include "LAFunctionBase.h"
#include "LAIntegralBase.h"
#include "LADataInstance.h"

using namespace std;

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
LAPriceDataFunction::LAPriceDataFunction(void) : 
    LAPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mpIntegral(NULL), mFnHolder(), mName("")
{
}
/*!
    @brief copy constructor
*/
LAPriceDataFunction::LAPriceDataFunction(const LAPriceDataFunction& attr) :
    LAPriceDataType(attr),
    mpMethods(NULL), mFnHolder(attr.mFnHolder), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    if (attr.mName != "")
    {
        if (mFnHolder.isDefined())
        {
            mpMethods = dynamic_cast<LAFunctionBase*>(attr.mFnHolder.get().clone());
            mFnHolder.set(mpMethods,true);
            
//          mpMethods = &(dynamic_cast<const class LAFunctionBase&>
//              (mFnHolder.get()));
        }
        mName = attr.mName;
		mParamStr = attr.mParamStr;
    } 
}
/*!
    @brief constructor

    @param[in] h function's holder to be set
    @param[in] name name of function
*/
LAPriceDataFunction::LAPriceDataFunction(const LACoreFunctionHolder& h, 
                                         const LAString& name) :
    LAPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mFnHolder(h), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    if (mFnHolder.isDefined() && name.isDefined() && name != "")
    {
        setNull(false);
        mpMethods = dynamic_cast<LAFunctionBase*>(h.get().clone());
        mFnHolder.set(mpMethods,true);

//      mpMethods = &(dynamic_cast<const class LAFunctionBase&>
//                                                  (mFnHolder.get()));
        mName = name;
		mParamStr = mpMethods->convertToString();
    } 
}
/*!
    @brief constructor
    @param[in] b pointer to function object
    @param[in] name name of function object
*/
LAPriceDataFunction::LAPriceDataFunction(LAFunctionBase* b, 
                                         const LAString& name) :
    LAPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mFnHolder(), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    setFunction(b, name);
}
/*!
    @brief destructor
*/
LAPriceDataFunction::~LAPriceDataFunction(void)
{
//  delete mpDataProvider;
}

//// QUERY ////
/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
LAPriceDataType*    
LAPriceDataFunction::clone() const
{
    try {
        LAPriceDataFunction*    pAttr = new LAPriceDataFunction(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of function which this object holds
    @return name of function
*/
LAString      
LAPriceDataFunction::convertToString(void) const
{
	if (isNull()) return NULL_STR;
	const LAString& param = mpMethods->convertToString();
	if (param == "" || param == NULL_STR) return mName;
	return mName + "(" + param + ")";
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool
LAPriceDataFunction::isTypeOf(function_t id) const
{
    return (! isNull() && mFnHolder.isDefined()) ? 
                        mpMethods->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
LAPriceDataFunction::getType(void) const
{
    if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
    return FN_FUNCTION;
}
/*!
    @brief return the function object which this class holds
    @return function object
*/
const LAFunctionBase&
LAPriceDataFunction::getFunction(void) const 
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}

/*!
    @brief return the function object which this class holds
    @return function object
*/
LAFunctionBase&
LAPriceDataFunction::getFunction(void)
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	update();
    return *mpMethods;
}

/*!
    @brief set function object with string repsentation "str"
    @param[in] str string representation of function
*/
void          
LAPriceDataFunction::convertFromString(const LAString& str)
{
    LAString data;
    bool ret = strToData(str, data);
    if (ret || data == "")
    {
        update();
        setNull();
        mName = "";
    }
    else
    {  
		setFunction(data);
    }
}
/*!
    @brief set function object specified by "name"
    @param[in] name function name to be specified
*/
void
LAPriceDataFunction::setFunction(const LAString& name)
{
    int pos = name.findString('(');
    if(pos == (int)name.size() - 1 || pos == 0)
    {
        // exception
        throw LACoreInvalidData(name.getCString(), __FILE__, __LINE__);      
    }

    // setting of content
    if(pos == -1) //no exist'('
    {
		mName = name;
		mParamStr = "";
		if (name != "")
		{
			setNull(false);
		}
		setFunction();

    }
    else
    {
		if (name.subString(name.size() - 1, name.size() - 1) != ")")
			throw LACoreInvalidData(name.getCString(), __FILE__, __LINE__);      
                               
        mName = name.subString(0, pos - 1);
		mParamStr = name.subString(pos + 1, name.size() - 2);
		setNull(false);
		setFunction();		   
	}

}
/*!
    @brief set function object specified by pointer as "name". This class controls pointer.
    @param[in] b specified function
    @param[in] name function name
*/
void 
LAPriceDataFunction::setFunction(LAFunctionBase* b, 
                                         const LAString& name)
{
    if (b != NULL && name.isDefined() && name != "")
    {
        update();
		mFnHolder.set(b, true);
        setNull(false);
        mpMethods = b;
        mName = name;
		mParamStr = b->convertToString();
    }
}
////////////////////////////////////////////////
// VALUATION METHODS

/*!
    @brief get function value
    @param[in] x point
    @return function value
*/
double       
LAPriceDataFunction::operator()(const DoubleArray& x) const
{
    if (isNull() == true) return 0;
    return (*mpMethods)(x);
}

/*!
    @brief get function value to be one-dimensionalized
    @param[in] x point
    @return function value
*/
double       
LAPriceDataFunction::operator()(double x) const
{
    if (isNull() == true) return 0;
    return (*mpMethods)(x);
}

/*!
    @brief get function paramters
    @return array with paramters
*/
const DoubleArray&
LAPriceDataFunction::getParam()const
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	return mpMethods->getParam();
}
/*DoubleArray&
LAPriceDataFunction::getParam()
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	update();
	return mpMethods->getParam();
}*/

//
/*!
    @brief set function paramters
    @param[in] param function parameters to be set
*/
void
LAPriceDataFunction::setParam(const DoubleArray& param)
{
    if (isNull() == true) return;
    mpMethods->setParam(param);
}
///////////////////////////////////////////
/*!
    @brief initialize this class with another
    @param[in] a object to be referenced for initialization
    @return initialized object(this object)
*/
LAPriceDataType& 
LAPriceDataFunction::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_FUNCTION) 
    {
        LAString err = "Assignement error for LAPriceDataFunction : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataFunction& in = dynamic_cast<const LAPriceDataFunction&>(a);
    setNull(a.isNull());
    mpMethods = dynamic_cast<LAFunctionBase*>(in.mpMethods->clone());
    mFnHolder.set(mpMethods,true);
    mName = in.mName;

    return *this;
}
/*!
    @brief compare this object with another
	if argument is not LAPriceDataFunction, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal
*/
int          
LAPriceDataFunction::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_FUNCTION) 
    {
        LAString err = "Compare error for LAPriceDataFunction : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataFunction& in = dynamic_cast<const LAPriceDataFunction&>(a);
    if (isNull() && in.isNull()) return 0;
    if (isNull()) return -1;
    if (in.isNull()) return 1;

    return getType() - in.getType();
}
/*!
    @brief set pointer to data holder of this object
    @param[in] holder pointer to holder to be set
*/
void                
LAPriceDataFunction::setHolder(LADataHolder* holder)
{
    LAPriceDataType::setHolder(holder);
	if(mpMethods == NULL)
	{
		setFunction();
	}
}
/*!
    @brief inner method to set up AttrFunction with name "mName"
*/
void 
LAPriceDataFunction::setFunction(void)
{
	update();

    LAObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        LADataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            LAFunctionManager& em = dataInstance->getFunctionMaster();
            const LACoreFunctionHolder& h = em.getFunction(mName);
            if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
            {
                mpMethods = NULL;
                LAString msg(mName);
                mName = "";
                msg += " is not found in Function Master as LAFunctionBase";
                throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            
            mpMethods = dynamic_cast<LAFunctionBase*>(h.get().clone());
            mFnHolder.set(mpMethods, true);
			if (mParamStr != "") mpMethods->convertFromString(mParamStr);			
			setNull(false);
        }
    }
	else
	{
		if(mpMethods!=NULL)
		{
			mFnHolder.set(NULL, false);
			mpMethods=NULL;
			setNull(true);
		}
	}
}
