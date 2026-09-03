/*! @file
    @brief Implementation of data to refer a specific class that implements procedure.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataProcedure.h"
#include "AQLObject.h"
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"

using namespace std;

// NULL status is not necessary in this class

/*!
    @brief default constructor
*/
AQLDataProcedure::AQLDataProcedure(void) : 
    AQLPriceDataType(DATA_PROCEDURE),  
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(), mName("")
{
}

/*!
    @brief copy constructor

    @param[in] v original object
*/
AQLDataProcedure::AQLDataProcedure(const AQLDataProcedure& v) : 
    AQLPriceDataType(v), 
    mVersion(v.mVersion), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(v.mFuncHolder), mName("")
{
    if (! v.isNull())
    {
        mpMethods = &(dynamic_cast<const class AQLCoreProcedure&>(mFuncHolder.get()));
        mName = v.mName; // copy of function name
    }
}

/*!
    @brief constructor

    @param[in] h holder of function class
    @param[in] name function name
*/
AQLDataProcedure::AQLDataProcedure(const AQLCoreFunctionHolder& h, 
                                            const AQLString& name): 
    AQLPriceDataType(DATA_PROCEDURE), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(h), mName("")
{
    if (h.isDefined() && name.isDefined() && name != "" )
    {
        mVersion = 0;
        mName = name;
        mpMethods = &(dynamic_cast<const class AQLCoreProcedure&>(mFuncHolder.get()));
        setNull(false);
    }
}

/*!
    @brief constructor

    @param[in] v function class
    @param[in] name function name
*/
AQLDataProcedure::AQLDataProcedure(const AQLCoreProcedure* v, const AQLString& name) : 
    AQLPriceDataType(DATA_PROCEDURE), 
    mVersion(-1), mpMethods(NULL), mpDataProvider(NULL),
    mDate(), mFuncHolder(), mName("")
{
    setMethod(v, name);
}

/*!
    @brief destructor
*/
AQLDataProcedure::~AQLDataProcedure()
{
    if (mpDataProvider != NULL) delete mpDataProvider; 
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*    
AQLDataProcedure::clone() const
{
    try {
        AQLDataProcedure*    pAttr = new AQLDataProcedure(*this);
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
AQLDataProcedure::isTypeOf(function_t id) const
{
    if (isNull()) return false;
    return mFuncHolder.isTypeOf(id);
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t                  
AQLDataProcedure::getType() const
{
    if (! isNull()) return mFuncHolder.getType();
    return FN_PROCEDURE;
}

/*!
    @brief return Method function to be set
    @return function to be set(AQLCoreProcedure)
*/
const AQLCoreProcedure&
AQLDataProcedure::getMethod(void) const
{
    if (isNull() || ! mFuncHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}

/*!
    @brief  function name in the string representation
    
    @return function name
*/
AQLString          
AQLDataProcedure::convertToString(void) const
{
    return (isNull() ? NULL_STR : mName);
}

/*!
    @brief set the Data from string representation(function name)


    @param[in] str function name
*/
void          
AQLDataProcedure::convertFromString(const AQLString& str)
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
AQLDataProcedure::setMethod(const AQLString& str)
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
AQLDataProcedure::setMethod(const AQLCoreProcedure* v, const AQLString& name) 
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

    let mVersion set by -1, and set it from AQLCoreValuation in case the Object does not have necessary Data
*/
void                
AQLDataProcedure::setNoValuation(void) const
{
    mVersion  = -1;
}

/*!
    @brief unset the set of setNoValuation(), and calculation is performed when called value()

    set mVersion 0
*/
void                
AQLDataProcedure::setValuation(void) const
{
    mVersion = 0;
}

// Calibrate Model
void AQLDataProcedure::calibrateModel(const AQLDate& basedate)
{
    if (mVersion != -1) 
    { 
        // Get Model
        AQLObject* model = getObject();
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
            throw AQLCoreInvalidData("#Error: Model Error - Unable to initialize the model within AQLDataProcedure", __FILE__, __LINE__);
        }
    }
}

// calibrate curve model
void AQLDataProcedure::calibrateOISAndSwapCurve(const AQLDate& basedate, const AQLString & curveCollection, const AQLString & curveIndex )
{
    if (mVersion != -1) 
    { 
        // Get Model
        AQLObject* model = getObject();
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
            throw AQLCoreInvalidData("#Error: Model Error - Unable to initialize the OIS and Swap Curve within AQLDataProcedure", __FILE__, __LINE__);
        }
    }
}

// Re-calibrate Model
void AQLDataProcedure::reCalibrateModel(void)
{
    // set AQLDataProvider by NULL
    setDataProvider(NULL);
    // recalculate
    // get Instruemnt
    AQLObject* inst = getObject();
    if ( inst != NULL )
    {
        mpMethods->calibrateModel(mDate, *inst, *this);
        mVersion = inst->getModel();
    }
    else
    {
        // Error
        throw AQLCoreInvalidData("instrument is not set for AQLDataProcedure",
                        __FILE__, __LINE__);
    }
}

/*!
    @brief set the AQLDataProvider to member functions

	Used in this class to have a class to hold the result produced as a byproduct of the calculation.
	It holds ownership, and it must be pointer allocated by new.

    @param[in] dataProvider AQLDataProvider to be set
*/
void                
AQLDataProcedure::setDataProvider(AQLDataProvider* dataProvider) const
{
    delete mpDataProvider;
    mpDataProvider = dataProvider;
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType& 
AQLDataProcedure::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_PROCEDURE) 
    {
        AQLString err = "Assignment error for AQLDataProcedure : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    const AQLDataProcedure& in = dynamic_cast<const AQLDataProcedure&>(a);
    
    // set AQLDataProvider NULL
    setDataProvider(NULL);
    // set mumber functions 
    mName=in.mName;
    mFuncHolder = in.mFuncHolder;
    mDate = in.mDate;
    mVersion = in.mVersion;
    if (mVersion != -1)
    {
        mpMethods = &(dynamic_cast<const class AQLCoreProcedure&>(mFuncHolder.get()));
    }
    setNull(in.isNull());
    return *this;
}

/*!
    @brief compare function (with respect to the other object), and it compares whether Functio ID is bigger or smaller

    @param[in] a AQLDataProcedure object to be compared

	@return  (Function ID of the Function class that this Data has) - (Function ID of the Function class that Data "a" has)

*/
int          
AQLDataProcedure::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_PROCEDURE) 
    {
        AQLString err = "Compare error for AQLDataProcedure : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLDataProcedure& in = dynamic_cast<const AQLDataProcedure&>(a);
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
AQLDataProcedure::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
    setMethod();
}

/*!
    @brief get Function object from Function Master based on mName, and set it to mpMethods

    set mVersion 0
*/
void                        
AQLDataProcedure::setMethod()
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
            if (! h.isDefined() || !h.get().isTypeOf(FN_PROCEDURE)) 
            {
                AQLString msg(mName);
                mVersion = -1;
                mName = "";
                msg += " is not found in Function Master as AQLCoreProcedure";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            mFuncHolder = h;
            mVersion = 0;
            mpMethods =  &(dynamic_cast<const class AQLCoreProcedure&>(mFuncHolder.get()));
            setNull(false);
        }
    }
}
