/*! @file
    @brief Source of data class to hold a function array
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDataFunctions.h"
#include "AQLFunctionManager.h"
#include "AQLDataInstance.h"
#include "AQLDataVector.h"

using namespace std;

//======================= IMPLEMENTATION =============================
/*!
    @brief default constructor
*/
AQLPriceDataFunctions::AQLPriceDataFunctions(void) : 
    AQLPriceDataType(DATA_FUNCTIONS)
{
}
/*!
    @brief copy constructor
*/
AQLPriceDataFunctions::AQLPriceDataFunctions(const AQLPriceDataFunctions& attr) :
    AQLPriceDataType(attr),mFnHolders(attr.mFnHolders)
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
					mMethods[i] = dynamic_cast<AQLFunctionBase*>(attr.mFnHolders[i].get().clone());					
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
AQLPriceDataFunctions::AQLPriceDataFunctions(const FunctionHolderVector& h, 
                                    const AQLStringVector& names) :
    AQLPriceDataType(DATA_FUNCTIONS),mFnHolders(h)
{
    if(h.size()!=names.size())
	{
        throw AQLCoreInvalidData("Function size and names size is not same", __FILE__, __LINE__);
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
				mMethods[i] = dynamic_cast<AQLFunctionBase*>(h[i].get().clone());
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
AQLPriceDataFunctions::AQLPriceDataFunctions(FunctionVector& b,
                                         const AQLStringVector& names) :
    AQLPriceDataType(DATA_FUNCTIONS)
{
    setFunctions(b, names);
}
/*!
    @brief destructor
*/
AQLPriceDataFunctions::~AQLPriceDataFunctions(void)
{

}

/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
AQLPriceDataType*    
AQLPriceDataFunctions::clone() const
{
    try {
        AQLPriceDataFunctions*    pAttr = new AQLPriceDataFunctions(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of function array which this object holds
    @return name of function array

*/
AQLString      
AQLPriceDataFunctions::convertToString(void) const
{
    AQLString ret;
    if (isNull() )
    {
        ret = NULL_STR;
    }
    else
    {
        ret = "";
        vector<AQLString>::const_iterator it;
        
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
AQLPriceDataFunctions::isTypeOf(function_t id, unsigned int pos) const
{
    return (! isNull() && getSize()>pos && mFnHolders[pos].isDefined()) ? 
                        mMethods[pos]->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
AQLPriceDataFunctions::getType(unsigned int pos) const
{
    if (! isNull() && getSize()>pos && mFnHolders[pos].isDefined()) return mFnHolders[pos].getType();
	return FN_FUNCTION;
}
/*!
    @brief return the function object array which this class holds
    @return function object array
*/
const FunctionVector&
AQLPriceDataFunctions::getFunctions(void) const 
{
    if (isNull() || getSize()==0)
    {
        throw AQLCoreInvalidData("NULL or No Methods sets", __FILE__, __LINE__);
    }
    return mMethods;
}


/*!
    @brief set function object array with string repsentation "str"
    @param[in] str string representation of function array
*/
void          
AQLPriceDataFunctions::convertFromString(const AQLString& str)
{
    vector<AQLString> tokens = str.toToken( DATA_COLL_DEL );
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
AQLPriceDataFunctions::setFunctions(const AQLStringVector& names)
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
			throw AQLCoreInvalidData(names[i].getCString(), __FILE__, __LINE__);      
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
				throw AQLCoreInvalidData(names[i].getCString(), __FILE__, __LINE__);      
	                               
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
AQLPriceDataFunctions::setFunctions(FunctionVector& b, 
                                         const AQLStringVector& names)
{
    update();
	if(b.size()!=names.size())
	{
        throw AQLCoreInvalidData("Function size and names size is not same", __FILE__, __LINE__);
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
AQLPriceDataType& 
AQLPriceDataFunctions::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_FUNCTIONS) 
    {
        AQLString err = "Assignement error for AQLPriceDataFunctions : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataFunctions& in = dynamic_cast<const AQLPriceDataFunctions&>(a);
    setNull(a.isNull());
    
	unsigned int size = in.getSize();
	mMethods.resize(size);
	mFnHolders.resize(size);
	for(unsigned int i = 0; i < size; i++)
	{
		mMethods[i] = dynamic_cast<AQLFunctionBase*>(in.mMethods[i]->clone());
		mFnHolders[i].set(mMethods[i],true);
	}
	mNames = in.mNames;




    return *this;
}
/*!
    @brief compare this object with another
	if argument is not AQLPriceDataFunction, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal

*/
int          
AQLPriceDataFunctions::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_FUNCTIONS) 
    {
        AQLString err = "Compare error for AQLPriceDataFunctions : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataFunctions& in = dynamic_cast<const AQLPriceDataFunctions&>(a);
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
AQLPriceDataFunctions::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
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
AQLPriceDataFunctions::setFunctions(void)
{
    update();
	
	AQLObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        AQLDataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            AQLFunctionManager& em = dataInstance->getFunctionMaster();
            
			mMethods.resize(getSize());
			mFnHolders.resize(getSize());
			for(unsigned int i = 0; i < getSize(); i++)
			{
				const AQLCoreFunctionHolder& h = em.getFunction(mNames[i]);
				if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
				{
					mMethods[i] = NULL;
					AQLString msg(mNames[i]);
					mNames[i] = "";
					msg += " is not found in Function Master as AQLFunctionBase";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
				}   
				mMethods[i] = dynamic_cast<AQLFunctionBase*>(h.get().clone());
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
AQLPriceDataFunctions::push_back(const AQLString& b)
{
	update();
	
    AQLString name, paramStr;
	int pos = b.findString('(');
    if(pos == (int)b.size() - 1 || pos == 0)
    {
        // exception
        throw AQLCoreInvalidData(b.getCString(), __FILE__, __LINE__);      
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
			throw AQLCoreInvalidData(b.getCString(), __FILE__, __LINE__);      
                               
        name = b.subString(0, pos - 1);
		paramStr = b.subString(pos + 1, b.size() - 2);
	}
	

	mNames.push_back(name);
	mParamStrs.push_back(paramStr);
    AQLObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        AQLDataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
			for (unsigned int i = 0; i < mFnHolders.size(); i++)
				mFnHolders[i].setDeleteFlag(false);
			mFnHolders.resize(mNames.size());
			mMethods.resize(mNames.size());            
			AQLFunctionManager& em = dataInstance->getFunctionMaster();
            const AQLCoreFunctionHolder& h = em.getFunction(b);
            if (! h.isDefined() || ! h.isTypeOf(FN_FUNCTION)) 
            {
                AQLString msg(b);
                msg += " is not found in Function Master as AQLFunctionBase";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }            
            mMethods[mNames.size() - 1] = dynamic_cast<AQLFunctionBase*>(h.get().clone());
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
AQLPriceDataFunctions::push_back(AQLFunctionBase* b,
											const AQLString& name)
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
AQLPriceDataFunctions::clear()
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
const AQLFunctionBase&         
AQLPriceDataFunctions::operator[](int i) const
{
	return *mMethods[i];
}

/*
    @brief operator[]
    
    @param[in] i array index

    @return array component in the specified location
*/
AQLFunctionBase&         
AQLPriceDataFunctions::operator[](int i) 
{
    update();
	return *mMethods[i];
}
