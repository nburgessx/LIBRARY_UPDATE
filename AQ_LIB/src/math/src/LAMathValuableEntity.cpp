/*! @file
    @brief Implementation to represent the Object with the evaluation, such as pricing. \n

		This class has Data(LADataValuation) as member variables
		to reprecent valuefunction such as pricing derived from LAObject class.

		LAMathObjectValue set Data name automatically as follows <BR>
		1. CALIBRATION_DATA_NAME(LADataString) <BR>
		2. CALIBRATION_DATA_VALUE(LADataValuation)

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LAMathValuableEntity.h"
#include "LAMathDefine.h"
#include "LADataValuation.h"
#include "LAPriceDataType.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LADataBasics.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataVector.h"

//#include <afx.h>
using namespace std;

/*!
    @brief default constructor

    @param[in] dataInstance LADataInstance object that this class belongs to
*/
LAMathObjectValue::LAMathObjectValue(LADataInstance* dataInstance) 
: LAObject()
{
    setDataInstance(dataInstance);

	LAPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_VALUE, DATA_VALUATION);

    mpName              = &add(CALIBRATION_DATA_NAME);
    mpMethod            = &add(CALIBRATION_DATA_VALUE);
}

/*!
    @brief copy constructor

    @param[in] inst original object
*/
LAMathObjectValue::LAMathObjectValue(const LAMathObjectValue& inst) 
: LAObject(inst)
{
    // DataValues are copied automatically by LAObject
    // get a pointer to the copied Data 
    mpName              = &getData(CALIBRATION_DATA_NAME);
    mpMethod            = &getData(CALIBRATION_DATA_VALUE);
}

/*!
    @brief destructor
*/
LAMathObjectValue::~LAMathObjectValue()
{
}

/*!
    @brief  return Object ID of this class

    @return Object ID
*/
object_t    
LAMathObjectValue::getType(void) const
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
LAMathObjectValue::isTypeOf(object_t id) const
{
    return (id == ENTITY_VENTITY ? true : LAObject::isTypeOf(id));
}

/*!
    @brief deep copy of the object

    @return the copied Object
*/
LAObject* 
LAMathObjectValue::clone() const
{
    try {
        LAMathObjectValue*   pInst = new LAMathObjectValue(*this);
        return pInst;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief get valuable function name

    @return valuable function name
*/
const LADataString&     
LAMathObjectValue::getName() const
{
    if (! mpName->isDefined()) 
    {
        LAString msg("Name is not set");
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return dynamic_cast<const LADataString&>(mpName->get());
}

/*!
    @brief get valuable function name

    @return valuable function name
*/
LADataString&       
LAMathObjectValue::getName()
{
    if (! mpName->isDefined()) 
    {
        LAString msg("Name is not set");
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return dynamic_cast<LADataString&>(mpName->get());
}

// const LAPriceCurrency&        
// LAMathObjectValue::getCurrency()
// {
//  if (! mpCurrency->isDefined()) 
//  {
//      LAString msg("Currency is not set");
//      throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
//  }
//  return dynamic_cast<const LAPriceCurrency&>
//                                  (mpCurrency->get());
// }
// LAPriceCurrency&      
// LAMathObjectValue::getCurrency()
// {
//  if (! mpCurrency->isDefined()) 
//  {
//      LAString msg("Currency is not set");
//      throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
//  }
//  return dynamic_cast<LAPriceCurrency&>
//                                  (mpCurrency->get());
// }

/*!
    @brief find out whether the function class that this class has or the inherited class matches a specified function id or not

    @return true if match, and else otherwise.
*/
const bool
LAMathObjectValue::isMethodTypeOf(function_t id) const
{
    if (! mpMethod->isDefined())
    {
        return false;
    }
    return dynamic_cast<const LADataValuation&>(mpMethod->get()).isTypeOf(id);
}

/*!
    @brief return Function ID this class holds

    @return Function ID
*/
function_t          
LAMathObjectValue::getMethodType(void) const
{
    if (! mpMethod->isDefined())
    {
        return 0;
    }
    return dynamic_cast<const LADataValuation&>(mpMethod->get()).getType();
}

/*!
    @brief register the Data into the Object

    @param[in] name Data name

    @return Data Holder
*/
LADataHolder&
LAMathObjectValue::add(const LAString& name)
{
    // name search
    LADataInstance* dataInstance = getDataInstance();
    LAPriceDataManager& dm = dataInstance->getDataMaster();
    const LADataHolder& dh = dm.getData(name);
    return LAObject::add(name, dh);
} 

/*!
    @brief return the result of evaluation of the Object

    @param[in] date the date to be evaluated

    @return valuation result
*/
double
LAMathObjectValue::value(const LADate& date)
{
    double ret;
    try {
        ret = dynamic_cast<LADataValuation&>(mpMethod->get()).value(date);
    }
    catch(LACoreError& e)
    {
		cout << e.getMsg() << endl;
        LAString msg("Evaluation Error at ");
        msg += getName();
        LACoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
        err += e;
		setErrMsgAttr(err);
        throw err;
    }
/*#ifdef _MSC_VER
    catch (CException* e)
    {
        char_t str[256];
        e->GetErrorMessage(str, 255);
        LAString msg("Evaluation Error at ");
        msg += getName().get() + "[";
        msg += str;
        msg += "]";
        e->Delete();
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
#else*/
    catch (...)
    {
        LAString msg("Evaluation Error at ");
        msg += getName().get();
        LACoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
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
LAMathObjectValue::revalue(void)
{
    double ret;
    try {
        ret = dynamic_cast<LADataValuation&>(mpMethod->get()).revalue();
    }
    catch(LACoreError& e)
    {
        LAString msg("Evaluation Error at ");
        msg += getName().get();
        LACoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
        err += e;
		setErrMsgAttr(err);
        throw err;
    }
/*#ifdef _MSC_VER
    catch (CException* e)
    {
        char_t str[256];
        e->GetErrorMessage(str, 255);
        LAString msg("Evaluation Error at ");
        msg += getName().get() + "[";
        msg += str;
        msg += "]";
        e->Delete();
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
#else*/
    catch (...)
    {
        LAString msg("Evaluation Error at ");
        msg += getName().get();
		LACoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		setErrMsgAttr(err);
        throw err;
    }
/*#endif*/
    return ret;
}   

/*!
    @brief set function name and Function class to implement the valuation function

    retain ownership of the LACoreValuation object

    @param[in] a function class
    @param[in] name function name
*/
void     
LAMathObjectValue::setValuationMethod(const LACoreValuation* a, const LAString& name)
{
    dynamic_cast<LADataValuation&>(mpMethod->get()).setMethod(a, name);
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
LAMathObjectValue::setValuationMethod(const LAString& name)
{
    dynamic_cast<LADataValuation&>(mpMethod->get()).setMethod(name);
}

/*!
    @brief delete Data

    do nothing is the specified Data is not registered
	Data that holds as a member variable ("Value" and "Name") is not deleted even if you specify.
    
    @param[in] dataName Data name to be deleted
*/
void                
LAMathObjectValue::remove(const LAString& dataName)
{
    //not remove a particular data
//  if(dataName == CALIBRATION_DATA_CURRENCY 
//      || dataName == CALIBRATION_DATA_NAME
    if(dataName == CALIBRATION_DATA_NAME
        || dataName == CALIBRATION_DATA_VALUE) return; 
    LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathObjectValue::reset(void)
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
LAObject&
LAMathObjectValue::copy(const LAObject& e)
{
    if (this == &e) return *this;

    if (! e.isTypeOf(ENTITY_VENTITY))
    {
        LAString err = "Assignement error for LAMathObjectValue : from ";
        err += LAString(e.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    LAObject::copy(e);  

    mpMethod            =  &getData(CALIBRATION_DATA_VALUE);
    mpName              =  &getData(CALIBRATION_DATA_NAME);

    return *this;
}
/*!
	@brief set error message attr

	@param[in] err error message
*/
void 
LAMathObjectValue::setErrMsgAttr(LACoreError &err)
{
	LAString add = "";
	LAString attr = "";
	for (unsigned int i = 0;i < err.getSize();++i)
	{
		LAString tmp = LAString(err.getMsg(i)) + "[" + LAString(err.getFile(i)) + ":" + LAString(err.getLine(i)) + "]" + "\n";
		attr += add + tmp;
		add += "<---";
	}
	//regist 
	LADataHolder &dh = getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		dynamic_cast<LADataStrings &>(dh.get()).push_back(attr);
	}
	else
	{
		LAStringVector msgs(1, attr);
		LAObject::add(CALIBRATION_DATA_ERRORMESSAGES, new LADataStrings(msgs));
	}
}