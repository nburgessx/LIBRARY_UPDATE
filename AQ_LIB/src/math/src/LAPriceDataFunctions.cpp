/*! @file
    @brief Source of data class to hold a function array

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDataFunctions.cpp
//
//  DESCRIPTION :       Source code of data class to hold a function array
//                      
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceDataFunctions.h"
#include "LAFunctionManager.h"
#include "LADataInstance.h"
#include "LADataVector.h"

using namespace std;

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
LAPriceDataFunctions::LAPriceDataFunctions(void) : 
    LAPriceDataType(DATA_FUNCTIONS)
{
}
/*!
    @brief copy constructor
*/
LAPriceDataFunctions::LAPriceDataFunctions(const LAPriceDataFunctions& attr) :
    LAPriceDataType(attr),mFnHolders(attr.mFnHolders)
{
    unsigned int size = attr.getSize();
	if(size != 0)
	{
		mMethods.resize(size);
		mNames = attr.mNames;
		mParamStrs = attr.mParamStrs;
		for(unsigned int i=0;i<size;i++)
		{
			if(attr.mNames[i] != "")
			{
				if (mFnHolders[i].isDefined())
				{
					mMethods[i] = dynamic_cast<LAFunctionBase*>(attr.mFnHolders[i].get().clone());					
					mFnHolders[i].set(mMethods[i],true);
				}
			}
					
		}
	}
}
/*!
    @brief constructor

    @param[in] h vector of function holder
    @param[in] names names of function array

*/
LAPriceDataFunctions::LAPriceDataFunctions(const FunctionHolderVector& h, 
                                    const LAStringVector& names) :
    LAPriceDataType(DATA_FUNCTIONS),mFnHolders(h)
{
    if(h.size()!=names.size())
	{
        throw LACoreInvalidData("Function size and names size is not same", __FILE__, __LINE__);
	}
	
	unsigned int size = names.size();
	if(size!=0)
	{
		mMethods.resize(size);
		mNames.resize(size);
		setNull(false);	
		for(unsigned int i=0;i<size;i++)
		{
			if (mFnHolders[i].isDefined() && names[i].isDefined() && names[i] != "")
			{
				mMethods[i] = dynamic_cast<LAFunctionBase*>(h[i].get().clone());
				mFnHolders[i].set(mMethods[i],true);
			}
			else
			{
				setNull(true);
			}
			mNames[i] = names[i];
			mParamStrs[i] = mMethods[i]->convertToString();

		}
	}
}
/*!
    @brief constructor
    @param[in] b array of pointer to function objects
    @param[in] names name array of function name
*/
LAPriceDataFunctions::LAPriceDataFunctions(FunctionVector& b,
                                         const LAStringVector& names) :
    LAPriceDataType(DATA_FUNCTIONS)
{
    setFunctions(b, names);
}
/*!
    @brief destructor
*/
LAPriceDataFunctions::~LAPriceDataFunctions(void)
{

}

//// QUERY ////
/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
LAPriceDataType*    
LAPriceDataFunctions::clone() const
{
    try {
        LAPriceDataFunctions*    pAttr = new LAPriceDataFunctions(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of function array which this object holds
    @return name of function array

*/
LAString      
LAPriceDataFunctions::convertToString(void) const
{
    LAString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<LAString>::const_iterator it;
        
		unsigned int i = 0;
		for (it = mNames.begin(); it != (mNames.end() - 1); ++it, i++)
        {
            ret += *it;
			if (mParamStrs.size() > i && mParamStrs[i] != "" && mParamStrs[i] != NULL_STR)
			ret += "(" + mParamStrs[i] + ")";
            ret += DATA_COLL_DEL;
        }
        ret += *it;
    }
    return ret;
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
	@param[in] pos index of function to be checked
    @return True or false

*/
bool
LAPriceDataFunctions::isTypeOf(function_t id, unsigned int pos) const
{
    return (! isNull() && getSize()>pos && mFnHolders[pos].isDefined()) ? 
                        mMethods[pos]->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
LAPriceDataFunctions::getType(unsigned int pos) const
{
    if (! isNull() && getSize()>pos && mFnHolders[pos].isDefined()) return mFnHolders[pos].getType();
	return FN_FUNCTION;
}
/*!
    @brief return the function object array which this class holds
    @return function object array
*/
const FunctionVector&
LAPriceDataFunctions::getFunctions(void) const 
{
    if (isNull() || getSize()==0)
    {
        throw LACoreInvalidData("NULL or No Methods sets", __FILE__, __LINE__);
    }
    return mMethods;
}


/*!
    @brief set function object array with string repsentation "str"
    @param[in] str string representation of function array
*/
void          
LAPriceDataFunctions::convertFromString(const LAString& str)
{
    vector<LAString> tokens = str.toToken( DATA_COLL_DEL );
    if(tokens.size() == 0 || str == NULL_STR)
	{
		update();
		setNull();
		mNames.clear();
	}
	else
	{
		setFunctions(tokens);
	}
}
/*!
    @brief set array of function objects with string repsentations "names"
    @param[in] names  array of string representation of function object
*/
void
LAPriceDataFunctions::setFunctions(const LAStringVector& names)
{
    mNames.resize(names.size());
	mParamStrs.clear();
	mParamStrs.resize(names.size(), "");
	bool null_flag = false;
	for (unsigned int i = 0; i < names.size(); i++)
	{
		int pos = names[i].findString('(');
		if(pos == (int)names[i].size() - 1 || pos == 0)
		{
			// exception
			throw LACoreInvalidData(names[i].getCString(), __FILE__, __LINE__);      
		}

		// setting of content
		if(pos == -1) //no exist'('
		{
			mNames[i] = names[i];
			mParamStrs[i] = "";
			if (names[i] == "")
			{
				null_flag = true;
			}
		}
		else
		{
			if (names[i].subString(names[i].size() - 1, names[i].size() - 1) != ")")
				throw LACoreInvalidData(names[i].getCString(), __FILE__, __LINE__);      
	                               
			mNames[i] = names[i].subString(0, pos - 1);
			mParamStrs[i] = names[i].subString(pos + 1, names[i].size() - 2);
		}		
	}
    if (null_flag)
		setNull(true);
	else
	{
		setNull(false);
		setFunctions();
	}

}
/*!
    @brief set function object array
    @param[in] b function array
    @param[in] names function name array
*/
void 
LAPriceDataFunctions::setFunctions(FunctionVector& b, 
                                         const LAStringVector& names)
{
    update();
	if(b.size()!=names.size())
	{
        throw LACoreInvalidData("Function size and names size is not same", __FILE__, __LINE__);
	}
	unsigned int size = names.size();
	mMethods.resize(size);
	mFnHolders.resize(size);
	mNames = names;
	mParamStrs.clear();
	mParamStrs.resize(names.size(), "");

	if(size>0)setNull(false);
	for(unsigned int i=0;i<size;i++)
	{
		if (b[i] != NULL && names[i].isDefined() && names[i] != "")
		{
			mFnHolders[i].set(b[i], true);
			mMethods[i] = b[i];
			mParamStrs[i] = b[i]->convertToString();
		}
		else
		{
			setNull(true);
		}
	}
}
////////////////////////////////////////////////




///////////////////////////////////////////
/*!
    @brief initialize this class with another
    @param[in] a object to be referenced for initialization
    @return initialized object(this object)
*/
LAPriceDataType& 
LAPriceDataFunctions::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_FUNCTIONS) 
    {
        LAString err = "Assignement error for LAPriceDataFunctions : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataFunctions& in = dynamic_cast<const LAPriceDataFunctions&>(a);
    setNull(a.isNull());
    
	unsigned int size = in.getSize();
	mMethods.resize(size);
	mFnHolders.resize(size);
	for(unsigned int i = 0; i < size; i++)
	{
		mMethods[i] = dynamic_cast<LAFunctionBase*>(in.mMethods[i]->clone());
		mFnHolders[i].set(mMethods[i],true);
	}
	mNames = in.mNames;




    return *this;
}
/*!
    @brief compare this object with another
	if argument is not LAPriceDataFunction, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal

*/
int          
LAPriceDataFunctions::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_FUNCTIONS) 
    {
        LAString err = "Compare error for LAPriceDataFunctions : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataFunctions& in = dynamic_cast<const LAPriceDataFunctions&>(a);
    if (isNull() && in.isNull()) return 0;
    if (isNull()) return -1;
    if (in.isNull()) return 1;

//  delete mpDataProvider;
//  mpDataProvider = NULL;

    return getSize() - in.getSize();
}
/*!
    @brief set pointer to data holder of this object
    @param[in] holder pointer to holder to be set

*/
void                
LAPriceDataFunctions::setHolder(LADataHolder* holder)
{
    LAPriceDataType::setHolder(holder);
    if(isNull())
	{
		if (getSize() != 0) setNull(false);
		setFunctions();
	}
}
/*!
    @brief inner method to set up AttrFunctions from function objects with name "mNames"
*/
void 
LAPriceDataFunctions::setFunctions(void)
{
    update();
	
	LAObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        LADataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            LAFunctionManager& em = dataInstance->getFunctionMaster();
            
			mMethods.resize(getSize());
			mFnHolders.resize(getSize());
			for(unsigned int i = 0; i < getSize(); i++)
			{
				const LACoreFunctionHolder& h = em.getFunction(mNames[i]);
				if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
				{
					mMethods[i] = NULL;
					LAString msg(mNames[i]);
					mNames[i] = "";
					msg += " is not found in Function Master as LAFunctionBase";
					throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
				}   
				mMethods[i] = dynamic_cast<LAFunctionBase*>(h.get().clone());
				if (mParamStrs[i] != "") mMethods[i]->convertFromString(mParamStrs[i]);			
				mFnHolders[i].set(mMethods[i], true);
			}
			setNull(false);
        }
    }
	else
	{
		setNull(true);
	}
}
/*!
	@brief add function object with name "b" to last component of function object array
	@param[in] b string representation of a function object added
*/
void
LAPriceDataFunctions::push_back(const LAString& b)
{
	update();
	
    LAString name, paramStr;
	int pos = b.findString('(');
    if(pos == (int)b.size() - 1 || pos == 0)
    {
        // exception
        throw LACoreInvalidData(b.getCString(), __FILE__, __LINE__);      
    }

    // setting of content
    if(pos == -1) //no exist'('
    {
		name = b;
		paramStr = "";
	}
    else
    {
		if (b.subString(b.size() - 1, b.size() - 1) != ")")
			throw LACoreInvalidData(b.getCString(), __FILE__, __LINE__);      
                               
        name = b.subString(0, pos - 1);
		paramStr = b.subString(pos + 1, b.size() - 2);
	}
	

	mNames.push_back(name);
	mParamStrs.push_back(paramStr);
    LAObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        LADataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
			for (unsigned int i = 0; i < mFnHolders.size(); i++)
				mFnHolders[i].setDeleteFlag(false);
			mFnHolders.resize(mNames.size());
			mMethods.resize(mNames.size());            
			LAFunctionManager& em = dataInstance->getFunctionMaster();
            const LACoreFunctionHolder& h = em.getFunction(b);
            if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
            {
                LAString msg(b);
                msg += " is not found in Function Master as LAFunctionBase";
                throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }            
            mMethods[mNames.size() - 1] = dynamic_cast<LAFunctionBase*>(h.get().clone());
 			for (unsigned int i = 0; i < mFnHolders.size() - 1; i++)
				mFnHolders[i].setDeleteFlag(true);
			mFnHolders[mNames.size() - 1].set(mMethods[mNames.size() - 1], true);			
			if (paramStr != "")  mMethods[mNames.size() - 1]->convertFromString(paramStr);
			if (getSize() == 1 && isNull()) setNull(false);
        }
    }
	else
	{
		setNull(true);
	}

}

/*!
	@brief add function object with name "name" to function object array
	@param[in] b pointer to function object added
	@param[in] name name of function object added
*/
void
LAPriceDataFunctions::push_back(LAFunctionBase* b,
											const LAString& name)
{
    if (b != NULL && name.isDefined() && name != "")
    {
        update();
		unsigned int size = getSize();
		for (unsigned int i = 0; i < size; i++)
			mFnHolders[i].setDeleteFlag(false);
		mFnHolders.resize(size + 1);
		mMethods.resize(size + 1);
		mMethods[size] = b;
		for (unsigned int i = 0; i < size; i++)
			mFnHolders[i].setDeleteFlag(true);
		mNames.push_back(name);		
		mFnHolders[size].set(b, true);
		mParamStrs.push_back(b->convertToString());
		if (size == 0 && isNull()) setNull(false);        
    }	
}
/*!
	@brief clear all data
*/
void
LAPriceDataFunctions::clear()
{
    update();
    setNull();
    mMethods.clear();
	mNames.clear();
	mFnHolders.clear();
}

/*
    @brief operator[]
    
    @param[in] i array index

    @return array component in the specified location
*/
const LAFunctionBase&         
LAPriceDataFunctions::operator[](int i) const
{
	return *mMethods[i];
}

/*
    @brief operator[]
    
    @param[in] i array index

    @return array component in the specified location
*/
LAFunctionBase&         
LAPriceDataFunctions::operator[](int i) 
{
    update();
	return *mMethods[i];
}
