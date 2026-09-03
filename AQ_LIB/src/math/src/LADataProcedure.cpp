/*! @file
    @brief Implementation of data to refer a specific class that implements procedure.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataProcedure.h"
#include "LAObject.h"
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"

using namespace std;

// NULL status is not necessary in this class

/*!
    @brief default constructor
*/
LADataProcedure::LADataProcedure(void) : 
    LAPriceDataType(DATA_PROCEDURE),  
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(), mName("")
{
}

/*!
    @brief copy constructor

    @param[in] v original object
*/
LADataProcedure::LADataProcedure(const LADataProcedure& v) : 
    LAPriceDataType(v), 
    mVersion(v.mVersion), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(v.mFuncHolder), mName("")
{
    if (! v.isNull())
    {
        mpMethods = &(dynamic_cast<const class LACoreProcedure&>(mFuncHolder.get()));
        mName = v.mName; // copy of function name
    }
}

/*!
    @brief constructor

    @param[in] h holder of function class
    @param[in] name function name
*/
LADataProcedure::LADataProcedure(const LACoreFunctionHolder& h, 
                                            const LAString& name): 
    LAPriceDataType(DATA_PROCEDURE), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(h), mName("")
{
    if (h.isDefined() && name.isDefined() && name != "" )
    {
        mVersion = 0;
        mName = name;
        mpMethods = &(dynamic_cast<const class LACoreProcedure&>(mFuncHolder.get()));
        setNull(false);
    }
}

/*!
    @brief constructor

    @param[in] v function class
    @param[in] name function name
*/
LADataProcedure::LADataProcedure(const LACoreProcedure* v, const LAString& name) : 
    LAPriceDataType(DATA_PROCEDURE), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(), mName("")
{
    setMethod(v, name);
}

/*!
    @brief destructor
*/
LADataProcedure::~LADataProcedure()
{
    if (mpDataProvider != NULL) delete mpDataProvider; 
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
LAPriceDataType*    
LADataProcedure::clone() const
{
    try {
        LADataProcedure*    pAttr = new LADataProcedure(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool                
LADataProcedure::isTypeOf(function_t id) const
{
    if (isNull()) return false;
    return mFuncHolder.isTypeOf(id);
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t                  
LADataProcedure::getType() const
{
    if (! isNull()) return mFuncHolder.getType();
    return FN_PROCEDURE;
}

/*!
    @brief return Method function to be set
    @return function to be set(LACoreProcedure)
*/
const LACoreProcedure&
LADataProcedure::getMethod(void) const
{
    if (isNull() || ! mFuncHolder.isDefined())
    {
        throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}

/*!
    @brief  function name in the string representation
    
    @return function name
*/
LAString          
LADataProcedure::convertToString(void) const
{
    return (isNull() ? NULL_STR : mName);
}

/*!
    @brief set the Data from string representation(function name)


    @param[in] str function name
*/
void          
LADataProcedure::convertFromString(const LAString& str)
{
    LAString data;
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
    }
    else
    {
        setMethod(data);
    }
}

/*!
    @brief set function name

    @param[in] str function name
*/
void          
LADataProcedure::setMethod(const LAString& str)
{
    // clear Buggage
    setDataProvider(NULL);
    mName = str;
    // set Object from function name
    setMethod();
}

/*!
    @brief set function name and Function class to implement the procedure function

    let both calculation result and mVersion be 0

    @param[in] v Function class to implement procedure (which holds the ownership)
    @param[in] name function name
*/
void 
LADataProcedure::setMethod(const LACoreProcedure* v, const LAString& name) 
{
    // update
    update();
    // clear Buggage
    setDataProvider(NULL);
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
    @brief not calculate when called calibrateModel()

    let mVersion set by -1, and set it from LACoreValuation in case the Object does not have necessary Data
*/
void                
LADataProcedure::setNoValuation(void) const
{
    mVersion  = -1;
}

/*!
    @brief unset the set of setNoValuation(), and calculation is performed when called value()

    set mVersion 0
*/
void                
LADataProcedure::setValuation(void) const
{
    mVersion = 0;
}

// Calibrate Model
void LADataProcedure::calibrateModel(const LADate& basedate)
{
    if (mVersion != -1) 
    { 
        // Get Model
        LAObject* model = getObject();
        if ( model != NULL )
        {
            if (model->IsValuated() && ( mVersion != model->getModel() || basedate != mDate ))
			{
                mpMethods->calibrateModel(basedate, *model, *this);
                mDate = basedate;

#ifndef USE_MODEL_VERSIONING
				mVersion = model->getModelVersion();
#else
				mVersion = model->getModel();
#endif

			}
        }
        else
        {
            throw LACoreInvalidData("#Error: Model Error - Unable to initialize the model within LADataProcedure", __FILE__, __LINE__);
        }
    }
}

// calibrate curve model
void LADataProcedure::calibrateOISAndSwapCurve(const LADate& basedate, const LAString & curveCollection, const LAString & curveIndex )
{
    if (mVersion != -1) 
    { 
        // Get Model
        LAObject* model = getObject();
        if ( model != NULL )
        {
            if (model->IsValuated() && ( mVersion != model->getModel() || basedate != mDate ))
			{
                mpMethods->calibrateOISAndSwapCurve(basedate, *model, *this, curveCollection, curveIndex);
                mDate = basedate;

#ifndef USE_MODEL_VERSIONING
				mVersion = model->getModelVersion();
#else
				mVersion = model->getModel();
#endif

			}
        }
        else
        {
            throw LACoreInvalidData("#Error: Model Error - Unable to initialize the OIS and Swap Curve within LADataProcedure", __FILE__, __LINE__);
        }
    }
}

// Re-calibrate Model
void LADataProcedure::reCalibrateModel(void)
{
    // set LADataProvider by NULL
    setDataProvider(NULL);
    // recalculate
    // get Instruemnt
    LAObject* inst = getObject();
    if ( inst != NULL )
    {
        mpMethods->calibrateModel(mDate, *inst, *this);
        mVersion = inst->getModel();
    }
    else
    {
        // Error
        throw LACoreInvalidData("instrument is not set for LADataProcedure",
                        __FILE__, __LINE__);
    }
}

/*!
    @brief set the LADataProvider to member functions

	Used in this class to have a class to hold the result produced as a byproduct of the calculation.
	It holds ownership, and it must be pointer allocated by new.

    @param[in] dataProvider LADataProvider to be set
*/
void                
LADataProcedure::setDataProvider(LADataProvider* dataProvider) const
{
    delete mpDataProvider;
    mpDataProvider = dataProvider;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
LAPriceDataType& 
LADataProcedure::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_PROCEDURE) 
    {
        LAString err = "Assignment error for LADataProcedure : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    const LADataProcedure& in = dynamic_cast<const LADataProcedure&>(a);
    
    // set LADataProvider NULL
    setDataProvider(NULL);
    // set mumber functions 
    mName=in.mName;
    mFuncHolder = in.mFuncHolder;
    mDate = in.mDate;
    mVersion = in.mVersion;
    if (mVersion != -1)
    {
        mpMethods = &(dynamic_cast<const class LACoreProcedure&>(mFuncHolder.get()));
    }
    setNull(in.isNull());
    return *this;
}

/*!
    @brief compare function (with respect to the other object), and it compares whether Functio ID is bigger or smaller

    @param[in] a LADataProcedure object to be compared

	@return  (Function ID of the Function class that this Data has) - (Function ID of the Function class that Data "a" has)

*/
int          
LADataProcedure::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_PROCEDURE) 
    {
        LAString err = "Compare error for LADataProcedure : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LADataProcedure& in = dynamic_cast<const LADataProcedure&>(a);
    if (isNull())
    {
        if (in.isNull()) return 0;
        else return -1;
    }
    if (in.isNull()) return 1;

    return getType() -  in.getType();
}

/*!
    @brief set a pointer to the Holder of the Data

   able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void                
LADataProcedure::setHolder(LADataHolder* holder)
{
    LAPriceDataType::setHolder(holder);
    setMethod();
}

/*!
    @brief get Function object from Function Master based on mName, and set it to mpMethods

    set mVersion 0
*/
void                        
LADataProcedure::setMethod()
{
    update();
    LAObject* e = getObject();
    if (e != NULL && mName != "")
    {
        LADataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            LAFunctionManager& eq = dataInstance->getFunctionMaster();
            const LACoreFunctionHolder& h = eq.getFunction(mName);
            if (! h.isDefined() || !h.get().isTypeOf(FN_PROCEDURE)) 
            {
                LAString msg(mName);
                mVersion = -1;
                mName = "";
                msg += " is not found in Function Master as LACoreProcedure";
                throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            mFuncHolder = h;
            mVersion = 0;
            mpMethods =  &(dynamic_cast<const class LACoreProcedure&>(mFuncHolder.get()));
            setNull(false);
        }
    }
}
