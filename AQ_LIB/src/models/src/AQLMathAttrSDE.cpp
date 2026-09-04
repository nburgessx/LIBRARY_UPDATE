/*! @file
    @brief Source code of class representing a sde data
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathAttrSDE.h"
#include "AQLFunctionManager.h"
#include "AQLRatesSDEBase.h"
#include "AQLIntegralBase.h"
#include "AQLDataInstance.h"

using namespace std;

//furuya//
//#define SDEPATH_TYPE_SIZE 2
#define SDEPATH_TYPE_SIZE 3
// SDE PATH Type Strings
static const char_t* gSDEPathType[SDEPATH_TYPE_SIZE] =
{
//    "IR", "FX"
	"IR", "FX", "VOL"
};

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
AQLMathAttrSDE::AQLMathAttrSDE(void) : 
    AQLPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(), mName(""), mCurrency("")
{
}
/*!
    @brief copy constructor
*/
AQLMathAttrSDE::AQLMathAttrSDE(const AQLMathAttrSDE& attr) :
    AQLPriceDataType(attr),
    mpSDE(NULL), mFnHolder(attr.mFnHolder), mName(""), mCurrency(attr.mCurrency), mType(attr.mType)
{
    if (attr.mName != "")
    {
        if (mFnHolder.isDefined())
        {
            mpSDE = dynamic_cast<AQLRatesSDEBase*>(attr.mFnHolder.get().clone());
            mFnHolder.set(mpSDE, true);
        }
        mName = attr.mName;
    } 
}
/*!
    @brief constructor

    @param[in] h function's holder to be set
    @param[in] name name of SDE
    @param[in] type SDE type (IR or FX)
    @param[in] currency currency
*/
AQLMathAttrSDE::AQLMathAttrSDE(const AQLCoreFunctionHolder& h, 
                                         const AQLString& name,
										 SDEPATH_TYPE type, const AQLString& currency) :
    AQLPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(h), mName(""), mCurrency(currency), mType(type)
{
    if (mFnHolder.isDefined() && name.isDefined() && name != "")
    {
        setNull(false);
        mpSDE = dynamic_cast<AQLRatesSDEBase*>(h.get().clone());
        mFnHolder.set(mpSDE,true);


        mName = name;
//		mParamStr = mpSDE->convertToString();
    } 
}
/*!
    @brief constructor
    @param[in] b pointer to SDE object
    @param[in] name name of SDE object
*/
AQLMathAttrSDE::AQLMathAttrSDE(AQLRatesSDEBase* b, 
                                         const AQLString& name,
										 SDEPATH_TYPE type, const AQLString& currency) :
    AQLPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(), mName("")
{
    setSDE(b, name, type, currency);
}
/*!
    @brief destructor
*/
AQLMathAttrSDE::~AQLMathAttrSDE(void)
{

}

//// QUERY ////
/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
AQLPriceDataType*    
AQLMathAttrSDE::clone() const
{
    try {
        AQLMathAttrSDE*    pAttr = new AQLMathAttrSDE(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of sde which this object holds
    @return name of SDE
*/
AQLString      
AQLMathAttrSDE::convertToString(void) const
{
	if (isNull()) return NULL_STR;
/*	const AQLString& param = mpSDE->convertToString();
	if (param == "" || param == NULL_STR) return mName;
	return mName + "(" + mpSDE->convertToString() + ")";
*/
	AQLString ret = gSDEPathType[mType];
	ret += ":";
	ret += getCurrency();	
	ret += ":";
	ret += mName;
	return ret;
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool
AQLMathAttrSDE::isTypeOf(function_t id) const
{
    return (! isNull() && mFnHolder.isDefined()) ? 
                        mpSDE->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
AQLMathAttrSDE::getType(void) const
{
    if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
    return FN_FUNCTION;
}
/*!
    @brief return the SDE object which this class holds
    @return SDE object
*/
const AQLRatesSDEBase&
AQLMathAttrSDE::getSDE(void) const 
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpSDE;
}

/*!
    @brief return the SDE object which this class holds
    @return SDE object
*/
AQLRatesSDEBase&
AQLMathAttrSDE::getSDE(void)
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	update();
    return *mpSDE;
}

/*!
    @brief set SDE object with string repsentation "str"
    @param[in] str string representation of SDE
*/
void          
AQLMathAttrSDE::convertFromString(const AQLString& str)
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
		const AQLStringVector& strs = data.toToken(':');
		if (strs.size() != 3)
		{
			//error
			AQLString msg = str;
			msg += " is wrong format";
	        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

		}
        int i;
		for (i = 0; i < SDEPATH_TYPE_SIZE; i++)
        {
            if (strs[0] == gSDEPathType[i])
            {
                mType = SDEPATH_TYPE(i);
                break;
            }
        }
        if (i == SDEPATH_TYPE_SIZE)
        {
            AQLString msg("Invalid String for convertFromString : ");
            msg += strs[0];
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
//		mCurrency = strs[1];
        setSDE(strs[2], mType, strs[1]);
    }
}
/*!
    @brief set SDE object specified by "name"
    @param[in] name SDE name to be specified
    @param[in] type SDE type (IR or FX)
    @param[in] currency currency
*/
void
AQLMathAttrSDE::setSDE(const AQLString& name, SDEPATH_TYPE type, const AQLString& currency)
{
	mType = type;
	mCurrency = currency;
	mName = name;
	if (name != "")
	{
		setNull(false);
	}
	setSDE();

/*
    int pos = name.findString('(');
    if(pos == (int)name.size() - 1 || pos == 0)
    {
        //
        throw AQLCoreInvalidData(name.getCString(), __FILE__, __LINE__);      
    }

    // 
    if(pos == -1) //'(' 
    {
		mName = name;
//		mParamStr = "";
		if (name != "")
		{
			setNull(false);
		}
		setSDE();

    }
    else
    {
		if (name.subString(name.size() - 1, name.size() - 1) != ")")
			throw AQLCoreInvalidData(name.getCString(), __FILE__, __LINE__);      
                               
        mName = name.subString(0, pos - 1);
//		mParamStr = name.subString(pos + 1, name.size() - 2);
		setNull(false);
		setSDE();		   
	}
*/
}
/*!
    @brief set SDE object specified by pointer as "name". This class controls pointer.
    @param[in] b specified SDE
    @param[in] name SDE name
    @param[in] type SDE type (IR or FX)
    @param[in] currency currency
*/
void 
AQLMathAttrSDE::setSDE(AQLRatesSDEBase* b, 
                                         const AQLString& name
										 , SDEPATH_TYPE type, const AQLString& currency)
{
	mType = type;
	mCurrency = currency;
	if (b != NULL && name.isDefined() && name != "")
    {
        update();
		mFnHolder.set(b, true);
        setNull(false);
        mpSDE = b;
        mName = name;
//		mParamStr = b->convertToString();
    }
}
////////////////////////////////////////////////
// VALUATION METHODS



///////////////////////////////////////////
/*!
    @brief initialize this class with another
    @param[in] a object to be referenced for initialization
    @return initialized object(this object)
*/
AQLPriceDataType& 
AQLMathAttrSDE::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_SDE) 
    {
        AQLString err = "Assignement error for AQLMathAttrSDE : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLMathAttrSDE& in = dynamic_cast<const AQLMathAttrSDE&>(a);
    setNull(a.isNull());
    mpSDE = dynamic_cast<AQLRatesSDEBase*>(in.mpSDE->clone());
    mFnHolder.set(mpSDE,true);
    mName = in.mName;

    return *this;
}
/*!
    @brief compare this object with another
	if argument is not AQLMathAttrSDE, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal
*/
int          
AQLMathAttrSDE::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_SDE) 
    {
        AQLString err = "Compare error for AQLMathAttrSDE : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLMathAttrSDE& in = dynamic_cast<const AQLMathAttrSDE&>(a);
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
AQLMathAttrSDE::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
	if(mpSDE == NULL)//added by matsumura 20060112 
	{
		setSDE();
	}
}
/*!
    @brief inner method to set up AttrSDE with name "mName"
*/
void 
AQLMathAttrSDE::setSDE(void)
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
            if (! h.isDefined() || ! h.isTypeOf(FN_SDEBASE)) 
            {
                mpSDE = NULL;
                AQLString msg(mName);
                mName = "";
                msg += " is not found in Function Master as AQLRatesSDEBase";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            
            mpSDE = dynamic_cast<AQLRatesSDEBase*>(h.get().clone());
            mFnHolder.set(mpSDE, true);
//			if (mParamStr != "") mpSDE->convertFromString(mParamStr);			
			setNull(false);//added by matsumura 20060112
        }
    }
	else //added by matsumura 20060112
	{
		if(mpSDE != NULL)
		{
			mFnHolder.set(NULL, false);
			mpSDE=NULL;
			setNull(true);//added by matsumura 20060112
		}
	}
}
