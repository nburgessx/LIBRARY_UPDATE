/*! @file
    @brief Implementation of data for valuation.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataValuation.h"
#include "AQLObject.h"
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"

using namespace std;

// NULL status is not necessary in this class

/*!
    @brief default constructor
*/
AQLDataValuation::AQLDataValuation(void) : 
    AQLPriceDataType(DATA_VALUATION),  
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL), mValue(0),
    mDate(), mFuncHolder(), mName("")
{
}

/*!
    @brief copy constructor

    @param[in] v original object
*/
AQLDataValuation::AQLDataValuation(const AQLDataValuation& v) : 
    AQLPriceDataType(DATA_VALUATION), 
    mVersion(v.mVersion), mpMethods(NULL), mpDataProvider(NULL), mValue(0),
    mDate(), mFuncHolder(v.mFuncHolder), mName("")
{
    if (! v.isNull())
    {
        if (mVersion != -1)
        {
            mpMethods = &(dynamic_cast<const class AQLCoreValuation&>(mFuncHolder.get()));
            mName = v.mName; // copy of function name
        }
        mValue = v.mValue;
        setNull(false);
    }
}

/*!
    @brief constructor

    @param[in] h holder of function class
    @param[in] name function name
*/
AQLDataValuation::AQLDataValuation(const AQLCoreFunctionHolder& h, const AQLString& name) : 
    AQLPriceDataType(DATA_VALUATION), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL), mValue(0),
    mDate(), mFuncHolder(h), mName("")
{
    if (h.isDefined() && name.isDefined() && name != "" )
    {
        mVersion = 0;
        mName = name;
        mpMethods = &(dynamic_cast<const class AQLCoreValuation&>(mFuncHolder.get()));
        setNull(false);
    }
}

/*!
    @brief constructor

    @param[in] v function class
    @param[in] name function name
*/
AQLDataValuation::AQLDataValuation(const AQLCoreValuation* v, const AQLString& name) : 
    AQLPriceDataType(DATA_VALUATION), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL), mValue(0),
    mDate(), mFuncHolder(), mName("")
{
    setMethod(v, name);
}

/*!
    @brief destructor
*/
AQLDataValuation::~AQLDataValuation()
{
    if (mpDataProvider != NULL) delete mpDataProvider; 
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*    
AQLDataValuation::clone() const
{
    try {
        AQLDataValuation*    pAttr = new AQLDataValuation(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool                
AQLDataValuation::isTypeOf(function_t id) const
{
    if (isNull() || mVersion == -1) return false;
    return mFuncHolder.isTypeOf(id);
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t                  
AQLDataValuation::getType() const
{
    if (! isNull() && mVersion != -1) return mFuncHolder.getType();
    return FN_VALUATION;
}

/*!
    @brief return Method function to be set
    @return function to be set(AQLCoreValuation)
*/
const AQLCoreValuation&
AQLDataValuation::getMethod(void) const 
{
    if (isNull() || ! mFuncHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}


/*!
    @brief result value and function name in the string representation
    
	function name is enclosed in "()" 
    
    @return result value +(function name) e.x. "3.245678(fn_stock)"
*/
AQLString          
AQLDataValuation::convertToString(void) const
{
    if (isNull()) return NULL_STR;

    AQLString ret(mValue, DOUBLE_PRECISION);
    if(mName != "")
    {
        ret+="(";
        ret+=mName;
        ret+=")";
    }
    return ret;
}

/*!
    @brief set the Data from string representation(with function name and the resultof the function)

	 You can specify the following string. <br>
	 1. If only the number (Eg "1.25 ()" or "1.25")<br> 
	    Only set (1.25) result. Function name is not set. <br>
	 2. If only the function name (Eg "fn_EuropeanOption" "(fn_EuropeanOption)") <br>
	    Set (fn_EuropeanOption) the function name. The results of a calculation is set to 0.0. <br>
	 3. If the name of the function + result (Ex "1.25 (fn_EuropeanOption)"). <br>
	    Set (fn_EuropeanOption) name and function (1.25) result. <br>

    @param[in] str string consisting of the result of a function and function name
*/
void          
AQLDataValuation::convertFromString(const AQLString& str)
{
    AQLString data;
    bool ret = strToData(str, data);
    if (ret || data == "")
    {
        // clear Buggage
        setDataProvider(NULL);
        // update
        update();
        setNull();
        mVersion = -1;
        mName = "";
        mValue = 0;
    }
    else
    {
        setMethod(data);
    }
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

    @param[in] str string consisting of the result of a function and function name
*/
void          
AQLDataValuation::setMethod(const AQLString& str)
{
    AQLString eqstr;
    double value=0.0;
    int pos=str.findString('(');
    if(pos > (int)str.size()-2)
    {
        // last character is '(' or str size is 0
        // exception
        throw AQLCoreInvalidData(str.getCString(), __FILE__, __LINE__);      
    }

    // setting of content
    if(pos==-1) // no exist'(' 
    {
        AQLString firststr = str.subString(0,0);
        if(firststr == AQLString('-') || (firststr >= AQLString('0') &&  firststr <= AQLString('9'))) // str has numerical
        {
            setValue(str.getDoubleValue());  // set calculation result
            // not set function name
            return;         
        }
        // str is function name
        value=0.0;
        eqstr = str;
    }
    else if(pos==0)  // in case of starting from '('
    {
        // str="(eq_..)"
        value=0.0;
        eqstr=str.subString(pos+1,str.size()-2);
    }
    else
    {
        value=str.subString(0,pos-1).getDoubleValue();  // calculation result
        if(pos == static_cast<int>(str.size()-2)){
            // str="0.0001()" case
            setValue(value);  // only set calculation result
            return;                 
        }               
        eqstr=str.subString(pos+1,str.size()-2); //  function name
    }
    // name and the value of the function has been known so far
    mValue = value;
    mName = eqstr;
    // from now, set object from the function name
    // clear Buggage
    setDataProvider(NULL);
    if (mName != "")
    {
        setNull(false);
        setMethod();
    }
    else
    {
        setNull();
    }
}

/*!
    @brief set function name and Function class to implement the function

    let both calculation result and mVersion be 0

    @param[in] v Function class to implement the function (which holds the ownership)
    @param[in] name function name
*/
void 
AQLDataValuation::setMethod(const AQLCoreValuation* v, const AQLString& name) 
{
    // update
    update();
    // clear Buggage
    setDataProvider(NULL);
    // value clear
    mValue = 0;
    // set function
    if (name.isDefined() && v != NULL && name != "") 
    {
        setNull(false);
        mName=name;
        mpMethods = v;
        mFuncHolder.set(v, true);
        mVersion = 0;
    }
    else
    {
        setNull();
        mName = "";
        mVersion = -1;
    }
}

/*!
    @brief set the calculation results

    to -1 mVersion, to "" the function name

    @param[in] v calculation results
*/
void                
AQLDataValuation::setValue(const double v)
{
    // update
    update();
    // clear Buggage
    setDataProvider(NULL);
    // set flag by False
    mVersion = -1;
    // set Value
    mValue = v;
    // release Null
    setNull(false);
    // clear name
    mName = "";
}

/*!
    @brief not calculate when called calibrateModel()

   let mVersion set by -1, and set it from AQLCoreValuation in case the Object does not have necessary Data
*/
void                
AQLDataValuation::setNoValuation(void) const
{
    mVersion  = -1;
}

/*!
    @brief unset the set of setNoValuation(), and calculation is performed when called value()

	set mVersion 0
*/
void                
AQLDataValuation::setValuation(void) const
{
    mVersion = 0;
}

/*!
    @brief perform the evaluation method of the Function class(value), returning the result of the calculation

    @param[in] basedate the date to be evaluated

    @return result from valuable function
*/
double              
AQLDataValuation::value(const AQLDate& basedate)
{
    if (mVersion != -1) 
    { // calculation mode
        // get Instruemnt
        AQLObject* inst = getObject();
        if ( inst != NULL )
        {
// 28JUN05:ysuzuki          if (mVersion != inst->getModel() || basedate != mDate)
            // modify to perform value() if the valuation flag of the Object that thsi Data belongs to is true
            if (inst->IsValuated() && ( mVersion != inst->getModel() || basedate != mDate ) )
            {
                mDate = basedate;
                // mpMethods is also set if inst is set
                mValue = mpMethods->value(basedate, *inst, *this);  // calculation result
                mVersion = inst->getModel();
            }
        }
        else
        {
            // error
            throw AQLCoreInvalidData("instrument is not set for AQLDataValuation",
                            __FILE__, __LINE__);
        }
    }
    return mValue;
}

/*!
    @brief perform the re-evaluation method of the Function class, returning the result of the calculation

    setNoValuation() even if the method is running, re-evaluation is carried out forcibly.

    @return result from valuable function
*/
double              
AQLDataValuation::revalue(void)
{
    // set AQLDataProvider by NULL
    setDataProvider(NULL);
    // recalculate
    // get Instruemnt
    AQLObject* inst = getObject();
    if ( inst != NULL )
    {
        mValue = mpMethods->value(mDate, *inst, *this);
        mVersion = inst->getModel();
    }
    else
    {
        // error
        throw AQLCoreInvalidData("instrument is not set for AQLDataValuation",
                        __FILE__, __LINE__);
    }
    return mValue;
}

/*!
    @brief set the AQLDataProvider to member functions

    Used in this class to have a class to hold the result produced as a byproduct of the calculation.
	It holds ownership, and it must be pointer allocated by new.

    @param[in] dataProvider AQLDataProvider to be set
*/
void                
AQLDataValuation::setDataProvider(AQLDataProvider* dataProvider) const
{
    delete mpDataProvider;
    mpDataProvider = dataProvider;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType& 
AQLDataValuation::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_VALUATION) 
    {
        AQLString err = "Assignment error for AQLDataValuation : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    const AQLDataValuation& in = dynamic_cast<const AQLDataValuation&>(a);
    
    // set AQLDataProvider NULL
    setDataProvider(NULL);
    // set mumber functions
    mName=in.mName;
    mFuncHolder = in.mFuncHolder;
    mValue = in.mValue;
    mDate = in.mDate;
    mVersion = (in.mVersion != -1 ? 0 : -1);
    if (mVersion != -1)
    {
        mpMethods = &(dynamic_cast<const class AQLCoreValuation&>(mFuncHolder.get()));
    }
    setNull(in.isNull());
    return *this;
}

/*!
    @brief compare function (with respect to the other object), and it compares whether Functio ID is bigger or smaller

    @param[in] a AQLDataValuation object to be compared

    @return  (Function ID of the Function class that this Data has) - (Function ID of the Function class that Data "a" has)
*/
int          
AQLDataValuation::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_VALUATION) 
    {
        AQLString err = "Compare error for AQLDataValuation : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLDataValuation& in = dynamic_cast<const AQLDataValuation&>(a);
    if (mVersion == -1 && in.mVersion == -1) return 0;
    if (mVersion == -1) return -1;
    if (in.mVersion == -1) return 1;

    return getType() -  in.getType();
}

/*!
    @brief set a pointer to the Holder of the Data

    able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void                
AQLDataValuation::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
    setMethod();
}

/*!
    @brief get Function object from Function Master based on mName, and set it to mpMethods

    set mVersion 0
*/
void                        
AQLDataValuation::setMethod()
{
    update();
    AQLObject* e = getObject();
    if (e != NULL && mName != "")
    {
        AQLDataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            AQLFunctionManager& eq = dataInstance->getFunctionMaster();
            const AQLCoreFunctionHolder& h = eq.getFunction(mName);
            if (! h.isDefined() || !h.get().isTypeOf(FN_VALUATION)) 
            {
                AQLString msg(mName);
                mVersion = -1;
                mName = "";
                msg += " is not found in Function Master as AQLCoreValuation";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            mFuncHolder = h;
            mVersion = 0;
            mpMethods =  &(dynamic_cast<const class AQLCoreValuation&>(mFuncHolder.get()));
        }
    }
}
