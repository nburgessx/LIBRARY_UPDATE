/*! @file
    @brief Source code of class representing a sde data

*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathAttrSDE.cpp
//
//  DESCRIPTION :       Source code of class representing a sde data
//                      
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathAttrSDE.h"
#include "LAFunctionManager.h"
#include "LARatesSDEBase.h"
#include "LAIntegralBase.h"
#include "LADataInstance.h"

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
LAMathAttrSDE::LAMathAttrSDE(void) : 
    LAPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(), mName(""), mCurrency("")
{
}
/*!
    @brief copy constructor
*/
LAMathAttrSDE::LAMathAttrSDE(const LAMathAttrSDE& attr) :
    LAPriceDataType(attr),
    mpSDE(NULL), mFnHolder(attr.mFnHolder), mName(""), mCurrency(attr.mCurrency), mType(attr.mType)
{
    if (attr.mName != "")
    {
        if (mFnHolder.isDefined())
        {
            mpSDE = dynamic_cast<LARatesSDEBase*>(attr.mFnHolder.get().clone());
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
LAMathAttrSDE::LAMathAttrSDE(const LACoreFunctionHolder& h, 
                                         const LAString& name,
										 SDEPATH_TYPE type, const LAString& currency) :
    LAPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(h), mName(""), mCurrency(currency), mType(type)
{
    if (mFnHolder.isDefined() && name.isDefined() && name != "")
    {
        setNull(false);
        mpSDE = dynamic_cast<LARatesSDEBase*>(h.get().clone());
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
LAMathAttrSDE::LAMathAttrSDE(LARatesSDEBase* b, 
                                         const LAString& name,
										 SDEPATH_TYPE type, const LAString& currency) :
    LAPriceDataType(DATA_SDE),
    mpSDE(NULL), mFnHolder(), mName("")
{
    setSDE(b, name, type, currency);
}
/*!
    @brief destructor
*/
LAMathAttrSDE::~LAMathAttrSDE(void)
{

}

//// QUERY ////
/*!
    @brief copy(clone) this object
    @return pointer to object produced
*/
LAPriceDataType*    
LAMathAttrSDE::clone() const
{
    try {
        LAMathAttrSDE*    pAttr = new LAMathAttrSDE(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief get string representation of sde which this object holds
    @return name of SDE
*/
LAString      
LAMathAttrSDE::convertToString(void) const
{
	if (isNull()) return NULL_STR;
/*	const LAString& param = mpSDE->convertToString();
	if (param == "" || param == NULL_STR) return mName;
	return mName + "(" + mpSDE->convertToString() + ")";
*/
	LAString ret = gSDEPathType[mType];
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
LAMathAttrSDE::isTypeOf(function_t id) const
{
    return (! isNull() && mFnHolder.isDefined()) ? 
                        mpSDE->isTypeOf(id) : false;
}
/*!
    @brief return class type
    @return class type
*/
function_t
LAMathAttrSDE::getType(void) const
{
    if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
    return FN_FUNCTION;
}
/*!
    @brief return the SDE object which this class holds
    @return SDE object
*/
const LARatesSDEBase&
LAMathAttrSDE::getSDE(void) const 
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpSDE;
}

/*!
    @brief return the SDE object which this class holds
    @return SDE object
*/
LARatesSDEBase&
LAMathAttrSDE::getSDE(void)
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
	update();
    return *mpSDE;
}

/*!
    @brief set SDE object with string repsentation "str"
    @param[in] str string representation of SDE
*/
void          
LAMathAttrSDE::convertFromString(const LAString& str)
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
		const LAStringVector& strs = data.toToken(':');
		if (strs.size() != 3)
		{
			//error
			LAString msg = str;
			msg += " is wrong format";
	        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

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
            LAString msg("Invalid String for convertFromString : ");
            msg += strs[0];
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
LAMathAttrSDE::setSDE(const LAString& name, SDEPATH_TYPE type, const LAString& currency)
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
        //—áŠO
        throw LACoreInvalidData(name.getCString(), __FILE__, __LINE__);      
    }

    //@“à—e‚ÌÝ’è
    if(pos == -1) //'('‚ª‚È‚¢ 
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
			throw LACoreInvalidData(name.getCString(), __FILE__, __LINE__);      
                               
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
LAMathAttrSDE::setSDE(LARatesSDEBase* b, 
                                         const LAString& name
										 , SDEPATH_TYPE type, const LAString& currency)
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
LAPriceDataType& 
LAMathAttrSDE::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_SDE) 
    {
        LAString err = "Assignement error for LAMathAttrSDE : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAMathAttrSDE& in = dynamic_cast<const LAMathAttrSDE&>(a);
    setNull(a.isNull());
    mpSDE = dynamic_cast<LARatesSDEBase*>(in.mpSDE->clone());
    mFnHolder.set(mpSDE,true);
    mName = in.mName;

    return *this;
}
/*!
    @brief compare this object with another
	if argument is not LAMathAttrSDE, then error
    @param[in] a objecto to be compared

    @return 1 :equal, 0 :not-equal
*/
int          
LAMathAttrSDE::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_SDE) 
    {
        LAString err = "Compare error for LAMathAttrSDE : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAMathAttrSDE& in = dynamic_cast<const LAMathAttrSDE&>(a);
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
LAMathAttrSDE::setHolder(LADataHolder* holder)
{
    LAPriceDataType::setHolder(holder);
	if(mpSDE == NULL)//added by matsumura 20060112 
	{
		setSDE();
	}
}
/*!
    @brief inner method to set up AttrSDE with name "mName"
*/
void 
LAMathAttrSDE::setSDE(void)
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
            if (! h.isDefined() || ! h.isTypeOf(FN_SDEBASE)) 
            {
                mpSDE = NULL;
                LAString msg(mName);
                mName = "";
                msg += " is not found in Function Master as LARatesSDEBase";
                throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            
            mpSDE = dynamic_cast<LARatesSDEBase*>(h.get().clone());
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
