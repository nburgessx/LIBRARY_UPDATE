/*! @file
    @brief Implementation to represent the Object with the evaluation, such as pricing. \n

		This class has Data(AQLDataValuation) as member variables
		to reprecent valuefunction such as pricing derived from AQLObject class.

		AQLMathObjectValue set Data name automatically as follows <BR>
		1. CALIBRATION_DATA_NAME(AQLDataString) <BR>
		2. CALIBRATION_DATA_VALUE(AQLDataValuation)

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLMathValuableEntity.h"
#include "AQLMathDefine.h"
#include "AQLDataValuation.h"
#include "AQLPriceDataType.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataVector.h"

//#include <afx.h>
using namespace std;

/*!
    @brief default constructor

    @param[in] dataInstance AQLDataInstance object that this class belongs to
*/
AQLMathObjectValue::AQLMathObjectValue(AQLDataInstance* dataInstance) 
: AQLObject()
{
    setDataInstance(dataInstance);

	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_VALUE, DATA_VALUATION);

    mpName              = &add(CALIBRATION_DATA_NAME);
    mpMethod            = &add(CALIBRATION_DATA_VALUE);
}

/*!
    @brief copy constructor

    @param[in] inst original object
*/
AQLMathObjectValue::AQLMathObjectValue(const AQLMathObjectValue& inst) 
: AQLObject(inst)
{
    // DataValues are copied automatically by AQLObject
    // get a pointer to the copied Data 
    mpName              = &getData(CALIBRATION_DATA_NAME);
    mpMethod            = &getData(CALIBRATION_DATA_VALUE);
}

/*!
    @brief destructor
*/
AQLMathObjectValue::~AQLMathObjectValue()
{
}

/*!
    @brief  return Object ID of this class

    @return Object ID
*/
object_t    
AQLMathObjectValue::getType(void) const
{
    return ENTITY_VENTITY;
}

/*!
    @brief find out whether the object or the inherited object matches a specified object id or not

    @param[in] id Object ID

    @retval true if this (inherited) Object matchs the selected Object ID
    @retval false otherwise
*/
bool
AQLMathObjectValue::isTypeOf(object_t id) const
{
    return (id == ENTITY_VENTITY ? true : AQLObject::isTypeOf(id));
}

/*!
    @brief deep copy of the object

    @return the copied Object
*/
AQLObject* 
AQLMathObjectValue::clone() const
{
    try {
        AQLMathObjectValue*   pInst = new AQLMathObjectValue(*this);
        return pInst;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief get valuable function name

    @return valuable function name
*/
const AQLDataString&     
AQLMathObjectValue::getName() const
{
    if (! mpName->isDefined()) 
    {
        AQLString msg("Name is not set");
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return dynamic_cast<const AQLDataString&>(mpName->get());
}

/*!
    @brief get valuable function name

    @return valuable function name
*/
AQLDataString&       
AQLMathObjectValue::getName()
{
    if (! mpName->isDefined()) 
    {
        AQLString msg("Name is not set");
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return dynamic_cast<AQLDataString&>(mpName->get());
}

// const AQLPriceCurrency&        
// AQLMathObjectValue::getCurrency()
// {
//  if (! mpCurrency->isDefined()) 
//  {
//      AQLString msg("Currency is not set");
//      throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
//  }
//  return dynamic_cast<const AQLPriceCurrency&>
//                                  (mpCurrency->get());
// }
// AQLPriceCurrency&      
// AQLMathObjectValue::getCurrency()
// {
//  if (! mpCurrency->isDefined()) 
//  {
//      AQLString msg("Currency is not set");
//      throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
//  }
//  return dynamic_cast<AQLPriceCurrency&>
//                                  (mpCurrency->get());
// }

/*!
    @brief find out whether the function class that this class has or the inherited class matches a specified function id or not

    @return true if match, and else otherwise.
*/
const bool
AQLMathObjectValue::isMethodTypeOf(function_t id) const
{
    if (! mpMethod->isDefined())
    {
        return false;
    }
    return dynamic_cast<const AQLDataValuation&>(mpMethod->get()).isTypeOf(id);
}

/*!
    @brief return Function ID this class holds

    @return Function ID
*/
function_t          
AQLMathObjectValue::getMethodType(void) const
{
    if (! mpMethod->isDefined())
    {
        return 0;
    }
    return dynamic_cast<const AQLDataValuation&>(mpMethod->get()).getType();
}

/*!
    @brief register the Data into the Object

    @param[in] name Data name

    @return Data Holder
*/
AQLDataHolder&
AQLMathObjectValue::add(const AQLString& name)
{
    // name search
    AQLDataInstance* dataInstance = getDataInstance();
    AQLPriceDataManager& dm = dataInstance->getDataMaster();
    const AQLDataHolder& dh = dm.getData(name);
    return AQLObject::add(name, dh);
} 

/*!
    @brief return the result of evaluation of the Object

    @param[in] date the date to be evaluated

    @return valuation result
*/
double
AQLMathObjectValue::value(const AQLDate& date)
{
    double ret;
    try {
        ret = dynamic_cast<AQLDataValuation&>(mpMethod->get()).value(date);
    }
    catch(AQLCoreError& e)
    {
		cout << e.getMsg() << endl;
        AQLString msg("Evaluation Error at ");
        msg += getName();
        AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
        err += e;
		setErrMsgAttr(err);
        throw err;
    }
/*#ifdef _MSC_VER
    catch (CException* e)
    {
        char_t str[256];
        e->GetErrorMessage(str, 255);
        AQLString msg("Evaluation Error at ");
        msg += getName().get() + "[";
        msg += str;
        msg += "]";
        e->Delete();
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
#else*/
    catch (...)
    {
        AQLString msg("Evaluation Error at ");
        msg += getName().get();
        AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		setErrMsgAttr(err);
        throw err;
    }
/*#endif*/
    return ret;
}   

/*!
    @brief return the result of foreced re-evaluation of the Object

    @return valuation result
*/
double
AQLMathObjectValue::revalue(void)
{
    double ret;
    try {
        ret = dynamic_cast<AQLDataValuation&>(mpMethod->get()).revalue();
    }
    catch(AQLCoreError& e)
    {
        AQLString msg("Evaluation Error at ");
        msg += getName().get();
        AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
        err += e;
		setErrMsgAttr(err);
        throw err;
    }
/*#ifdef _MSC_VER
    catch (CException* e)
    {
        char_t str[256];
        e->GetErrorMessage(str, 255);
        AQLString msg("Evaluation Error at ");
        msg += getName().get() + "[";
        msg += str;
        msg += "]";
        e->Delete();
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
#else*/
    catch (...)
    {
        AQLString msg("Evaluation Error at ");
        msg += getName().get();
		AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		setErrMsgAttr(err);
        throw err;
    }
/*#endif*/
    return ret;
}   

/*!
    @brief set function name and Function class to implement the valuation function

    retain ownership of the AQLCoreValuation object

    @param[in] a function class
    @param[in] name function name
*/
void     
AQLMathObjectValue::setValuationMethod(const AQLCoreValuation* a, const AQLString& name)
{
    dynamic_cast<AQLDataValuation&>(mpMethod->get()).setMethod(a, name);
}

/*!
    @brief set the result of a function and the function name from the specified string

     You can specify the following string. <br>
	 1. If only the number (Eg "1.25 ()" or "1.25")<br> 
	    Only set (1.25) result. Function name is not set. <br>
	 2. If only the function name (Eg "fn_EuropeanOption" "(fn_EuropeanOption)") <br>
	    Set (fn_EuropeanOption) the function name. The results of a calculation is set to 0.0. <br>
	 3. If the name of the function + result (Ex "1.25 (fn_EuropeanOption)"). <br>
	    Set (fn_EuropeanOption) name and function (1.25) result. <br>

    @param[in] name string consisting of the result of a function and function name
*/
void     
AQLMathObjectValue::setValuationMethod(const AQLString& name)
{
    dynamic_cast<AQLDataValuation&>(mpMethod->get()).setMethod(name);
}

/*!
    @brief delete Data

    do nothing is the specified Data is not registered
	Data that holds as a member variable ("Value" and "Name") is not deleted even if you specify.
    
    @param[in] dataName Data name to be deleted
*/
void                
AQLMathObjectValue::remove(const AQLString& dataName)
{
    //not remove a particular data
//  if(dataName == CALIBRATION_DATA_CURRENCY 
//      || dataName == CALIBRATION_DATA_NAME
    if(dataName == CALIBRATION_DATA_NAME
        || dataName == CALIBRATION_DATA_VALUE) return; 
    AQLObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
AQLMathObjectValue::reset(void)
{
	clear();
    mpName  = &add(CALIBRATION_DATA_NAME);
    mpMethod = &add(CALIBRATION_DATA_VALUE);	
}

/*!
    @brief shallow copy of the project

    @param[in] e original object

    @return the copied object
*/
AQLObject&
AQLMathObjectValue::copy(const AQLObject& e)
{
    if (this == &e) return *this;

    if (! e.isTypeOf(ENTITY_VENTITY))
    {
        AQLString err = "Assignement error for AQLMathObjectValue : from ";
        err += AQLString(e.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    AQLObject::copy(e);  

    mpMethod            =  &getData(CALIBRATION_DATA_VALUE);
    mpName              =  &getData(CALIBRATION_DATA_NAME);

    return *this;
}
/*!
	@brief set error message attr

	@param[in] err error message
*/
void 
AQLMathObjectValue::setErrMsgAttr(AQLCoreError &err)
{
	AQLString add = "";
	AQLString attr = "";
	for (unsigned int i = 0;i < err.getSize();++i)
	{
		AQLString tmp = AQLString(err.getMsg(i)) + "[" + AQLString(err.getFile(i)) + ":" + AQLString(err.getLine(i)) + "]" + "\n";
		attr += add + tmp;
		add += "<---";
	}
	//regist 
	AQLDataHolder &dh = getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		dynamic_cast<AQLDataStrings &>(dh.get()).push_back(attr);
	}
	else
	{
		AQLStringVector msgs(1, attr);
		AQLObject::add(CALIBRATION_DATA_ERRORMESSAGES, new AQLDataStrings(msgs));
	}
}