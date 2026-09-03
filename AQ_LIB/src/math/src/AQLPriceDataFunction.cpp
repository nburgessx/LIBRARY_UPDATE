/*! @file
    @brief Source code of class representing a mathmatical function

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPriceDataFunction.cpp
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


#include "AQLPriceDataFunction.h"
#include "AQLFunctionManager.h"
#include "AQLFunctionBase.h"
#include "AQLIntegralBase.h"
#include "AQLDataInstance.h"

using namespace std;

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
AQLPriceDataFunction::AQLPriceDataFunction(void) : 
    AQLPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mpIntegral(NULL), mFnHolder(), mName("")
{
}
/*!
    @brief copy constructor
*/
AQLPriceDataFunction::AQLPriceDataFunction(const AQLPriceDataFunction& attr) :
    AQLPriceDataType(attr),
    mpMethods(NULL), mFnHolder(attr.mFnHolder), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    if (attr.mName != "")
    {
        if (mFnHolder.isDefined())
        {
            mpMethods = dynamic_cast<AQLFunctionBase*>(attr.mFnHolder.get().clone());
            mFnHolder.set(mpMethods,true);
            
//          mpMethods = &(dynamic_cast<const class AQLFunctionBase&>
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
AQLPriceDataFunction::AQLPriceDataFunction(const AQLCoreFunctionHolder& h, 
                                         const AQLString& name) :
    AQLPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mFnHolder(h), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    if (mFnHolder.isDefined() && name.isDefined() && name != "")
    {
        setNull(false);
        mpMethods = dynamic_cast<AQLFunctionBase*>(h.get().clone());
        mFnHolder.set(mpMethods,true);

//      mpMethods = &(dynamic_cast<const class AQLFunctionBase&>
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
AQLPriceDataFunction::AQLPriceDataFunction(AQLFunctionBase* b, 
                                         const AQLString& name) :
    AQLPriceDataType(DATA_FUNCTION),
    mpMethods(NULL), mFnHolder(), mName(""), mParamStr("")/*, mpDataProvider(NULL)*/
{
    setFunction(b, name);
}
/*!
    @brief destructor
*/
AQLPriceDataFunction::~AQLPriceDataFunction(void)
{
//  delete mpDataProvider;
}

//// QUERY ////
/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
AQLPriceDataType*    
AQLPriceDataFunction::clone() const
{
    try {
        AQLPriceDataFunction*    pAttr = new AQLPriceDataFunction(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of function which this object holds
    @return name of function
*/
AQLString      
AQLPriceDataFunction::convertToString(void) const
{
	if (isNull()) return NULL_STR;
	const AQLString& param = mpMethods->convertToString();
	if (param == "" || param == NULL_STR) return mName;
	return mName + "(" + param + ")";
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool
AQLPriceDataFunction::isTypeOf(function_t id) const
{
    return (! isNull() && mFnHolder.isDefined()) ? 
                        mpMethods->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
AQLPriceDataFunction::getType(void) const
{
    if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
    return FN_FUNCTION;
}
/*!
    @brief return the function object which this class holds
    @return function object
*/
const AQLFunctionBase&
AQLPriceDataFunction::getFunction(void) const 
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}

/*!
    @brief return the function object which this class holds
    @return function object
*/
AQLFunctionBase&
AQLPriceDataFunction::getFunction(void)
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	update();
    return *mpMethods;
}

/*!
    @brief set function object with string repsentation "str"
    @param[in] str string representation of function
*/
void          
AQLPriceDataFunction::convertFromString(const AQLString& str)
{
    AQLString data;
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
AQLPriceDataFunction::setFunction(const AQLString& name)
{
    int pos = name.findString('(');
    if(pos == (int)name.size() - 1 || pos == 0)
    {
        // exception
        throw AQLCoreInvalidData(name.getCString(), __FILE__, __LINE__);      
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
			throw AQLCoreInvalidData(name.getCString(), __FILE__, __LINE__);      
                               
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
AQLPriceDataFunction::setFunction(AQLFunctionBase* b, 
                                         const AQLString& name)
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
AQLPriceDataFunction::operator()(const DoubleArray& x) const
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
AQLPriceDataFunction::operator()(double x) const
{
    if (isNull() == true) return 0;
    return (*mpMethods)(x);
}

/*!
    @brief get function paramters
    @return array with paramters
*/
const DoubleArray&
AQLPriceDataFunction::getParam()const
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	return mpMethods->getParam();
}
/*DoubleArray&
AQLPriceDataFunction::getParam()
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
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
AQLPriceDataFunction::setParam(const DoubleArray& param)
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
AQLPriceDataType& 
AQLPriceDataFunction::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_FUNCTION) 
    {
        AQLString err = "Assignement error for AQLPriceDataFunction : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataFunction& in = dynamic_cast<const AQLPriceDataFunction&>(a);
    setNull(a.isNull());
    mpMethods = dynamic_cast<AQLFunctionBase*>(in.mpMethods->clone());
    mFnHolder.set(mpMethods,true);
    mName = in.mName;

    return *this;
}
/*!
    @brief compare this object with another
	if argument is not AQLPriceDataFunction, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal
*/
int          
AQLPriceDataFunction::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_FUNCTION) 
    {
        AQLString err = "Compare error for AQLPriceDataFunction : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataFunction& in = dynamic_cast<const AQLPriceDataFunction&>(a);
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
AQLPriceDataFunction::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
	if(mpMethods == NULL)
	{
		setFunction();
	}
}
/*!
    @brief inner method to set up AttrFunction with name "mName"
*/
void 
AQLPriceDataFunction::setFunction(void)
{
	update();

    AQLObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        AQLDataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            AQLFunctionManager& em = dataInstance->getFunctionMaster();
            const AQLCoreFunctionHolder& h = em.getFunction(mName);
            if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
            {
                mpMethods = NULL;
                AQLString msg(mName);
                mName = "";
                msg += " is not found in Function Master as AQLFunctionBase";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            
            mpMethods = dynamic_cast<AQLFunctionBase*>(h.get().clone());
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
