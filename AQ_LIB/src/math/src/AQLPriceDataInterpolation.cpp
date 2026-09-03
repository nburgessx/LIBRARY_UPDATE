
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDataInterpolation.h"
#include "AQLFunctionManager.h"
#include "AQLDataInstance.h"

using namespace std;

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
AQLPriceDataInterpolation::AQLPriceDataInterpolation(void) : 
    AQLPriceDataType(DATA_INTERPOLATION),
    mpMethods(NULL), mFnHolder(), mName("")/*, mpDataProvider(NULL)*/
{
}
/*!
    @brief copy constructor
*/
AQLPriceDataInterpolation::AQLPriceDataInterpolation(const AQLPriceDataInterpolation& attr) :
    AQLPriceDataType(attr),
    mpMethods(NULL), mFnHolder(attr.mFnHolder), mName("")/*, mpDataProvider(NULL)*/
{
    if (attr.mName != "")
    {
        if (mFnHolder.isDefined())
        {
            mpMethods = dynamic_cast<AQLInterpolationBase*>(attr.mFnHolder.get().clone());
            mFnHolder.set(mpMethods,true);
            
//          mpMethods = &(dynamic_cast<const class AQLInterpolationBase&>
//              (mFnHolder.get()));
        }
        mName = adjustInterpolationName(attr.mName);
    } 
}
/*!
    @brief constructor

    @param[in] h function holder
    @param[in] name function name

    @return period from the start date (in years)
*/
AQLPriceDataInterpolation::AQLPriceDataInterpolation(const AQLCoreFunctionHolder& h, 
                                         const AQLString& name) :
    AQLPriceDataType(DATA_INTERPOLATION),
    mpMethods(NULL), mFnHolder(h), mName("")/*, mpDataProvider(NULL)*/
{
    if (mFnHolder.isDefined() && name.isDefined() && name != "")
    {
        setNull(false);
        mpMethods = dynamic_cast<AQLInterpolationBase*>(h.get().clone());
        mFnHolder.set(mpMethods,true);

//      mpMethods = &(dynamic_cast<const class AQLInterpolationBase&>
//                                                  (mFnHolder.get()));
        mName = adjustInterpolationName(name);
    } 
}
/*!
    @brief constructor
    @param[in] b pointer to interpolation function
    @param[in] name name of interpolation function
*/
AQLPriceDataInterpolation::AQLPriceDataInterpolation(AQLInterpolationBase* b, 
                                         const AQLString& name) :
    AQLPriceDataType(DATA_INTERPOLATION),
    mpMethods(NULL), mFnHolder(), mName("")/*, mpDataProvider(NULL)*/
{
    setMethod(b, name);
}
/*!
    @brief destructor
*/
AQLPriceDataInterpolation::~AQLPriceDataInterpolation(void)
{
//  delete mpDataProvider;
}

//// QUERY ////
/*!
    @brief deep copy of the data object - calling code is resposible for memory clean-up.
    @Calling code must delete AQLPriceDataType when finished with the object to avoid memory leaks.
    @return pointer to the Data object newly created
*/
AQLPriceDataType*    
AQLPriceDataInterpolation::clone() const
{
    try {
        AQLPriceDataInterpolation*    pAttr = new AQLPriceDataInterpolation(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief interpolation method in the string representation
    @return interpolations method
*/
AQLString      
AQLPriceDataInterpolation::convertToString(void) const
{
    return (isNull() ? NULL_STR : mName);
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool
AQLPriceDataInterpolation::isTypeOf(function_t id) const
{
    return (! isNull() && mFnHolder.isDefined()) ? 
                        mpMethods->isTypeOf(id) : false;
}
/*!
    @brief get function type
    @return function type
*/
function_t
AQLPriceDataInterpolation::getType(void) const
{
	if (!isNull() && mFnHolder.isDefined())
	{
		return mFnHolder.getType();
	}
    return FN_INTERPOLATION;
}

// Check if Interpolator is Null
bool AQLPriceDataInterpolation::isNullOrUndefined() const
{
	// Check if the object, the interpolation method or function holder is null
	const bool result = ( isNull() || !mFnHolder.isDefined() || mpMethods == NULL || mpMethods->interpolationData() == nullptr ) ? true : false;
	return result;
}

/*!
    @brief return Method function to be set
    @return function to be set(AQLRandBase)
*/
const AQLInterpolationBase&
AQLPriceDataInterpolation::getMethod(void) const 
{
    if (isNull() || ! mFnHolder.isDefined())
    {
        throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
    }
    return *mpMethods;
}
/*!
    @brief set specified interpolation method from string format
    @param[in] str string to represent the interpolation to be selected preferably 
*/
void          
AQLPriceDataInterpolation::convertFromString(const AQLString& str)
{
    AQLString data;
    bool ret = strToData(str, data);
    if (ret || data == "")
    {
		update();
//      delete mpDataProvider;
//      mpDataProvider = NULL;
        setNull();
        mName = "";
    }
    else
    {
        setMethod(data);
    }
}
/*!
    @brief set function specified by name
    @param[in] name function name
*/
void
AQLPriceDataInterpolation::setMethod(const AQLString& name)
{
	if (mName != name)
	{
			
		mName = adjustInterpolationName(name);
		if (name != "")
		{
			setNull(false);
			setMethod();
		}
		else
		{
			update();
		}
	}

}
/*!
    @brief set name and function
    @param[in] b function
    @param[in] name function name
*/
void 
AQLPriceDataInterpolation::setMethod(AQLInterpolationBase* b, const AQLString& name)
{
    if (b != NULL && name.isDefined() && name != "")
    {
		update();
        mFnHolder.set(b, true);
        setNull(false);
        mpMethods = b;
        mName = adjustInterpolationName(name);
    }
}
////////////////////////////////////////////////
// VALUATION METHODS

/*!
    @brief return a value of one-dimensional curve
    @param[in] x1 interpolation point
    @return interpolation value (double)
*/
double       
AQLPriceDataInterpolation::value(const double x1) const
{
    if (isNull() == true /*|| mpDataProvider == NULL*/) return 0;
    return mpMethods->value(x1/*, mpDataProvider*/);
}
//
/*!
    @brief set a value of one-dimensional curve
    @param[in] index x-axis value
    @param[in] value y-axis value
*/
void
AQLPriceDataInterpolation::set(const DoubleArray& index, const DoubleArray& value)
{
    if (isNull() == true) return;
//  delete mpDataProvider;
//  mpDataProvider = NULL;
    mpMethods->set(index, value);
}

// set the value of the interpolation join date double
void
AQLPriceDataInterpolation::setJoinDateAsDouble( const double& joinDateAsDouble )
{
    if (isNull() == true) return;
    mpMethods->setJoinDateAsDouble(joinDateAsDouble);
}

// return the value of the interpolation join date double
double
AQLPriceDataInterpolation::getJoinDateAsDouble() const
{
	return mpMethods->getJoinDateAsDouble();
}

// return true if it's hybrid interpolation, e.g. linearSpline
bool
AQLPriceDataInterpolation::isHybrid() const
{
	return mpMethods->isHybrid();
}

///////////////////////////////////////////
/*!
    @brief the contents of the object to be initialized with those of another object
    
	AQLPriceDataType is supposed to be set on the argument

    @param[in] a object used by the reference
    @return reference to the AQLPriceDataType
*/
AQLPriceDataType& 
AQLPriceDataInterpolation::assignment(const AQLPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_INTERPOLATION) 
    {
        AQLString err = "Assignement error for AQLPriceDataInterpolation : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataInterpolation& in = dynamic_cast<const AQLPriceDataInterpolation&>(a);
    setNull(a.isNull());
//  mFnHolder = in.mFnHolder;
//  mpMethods = in.mpMethods;
    mpMethods = dynamic_cast<AQLInterpolationBase*>(in.mpMethods->clone());
    mFnHolder.set(mpMethods,true);
    mName = adjustInterpolationName(in.mName);

//  delete mpDataProvider;
//  mpDataProvider = NULL;


    return *this;
}
/*!
    @brief compare the contents against the other objecet

    error if "a" is not AQLPriceDataInterpolation.
	
    @param[in] a object to compare

    @return 0 if the same, and not 0 otherwise. Comparison about set info is performed.
*/
int          
AQLPriceDataInterpolation::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_INTERPOLATION) 
    {
        AQLString err = "Compare error for AQLPriceDataInterpolation : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const AQLPriceDataInterpolation& in = dynamic_cast<const AQLPriceDataInterpolation&>(a);
    if (isNull() && in.isNull()) return 0;
    if (isNull()) return -1;
    if (in.isNull()) return 1;

//  delete mpDataProvider;
//  mpDataProvider = NULL;

    return getType() - in.getType();
}
/*!
    @brief set up the pointer to the data holder
    @param[in] holder pointer of Holder to set
*/
void                
AQLPriceDataInterpolation::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
	if(mpMethods == NULL)
	{
		setMethod();
	}

}
/*!
    @brief set the object from the function name
*/
void 
AQLPriceDataInterpolation::setMethod(void)
{

	// update
    update();

    AQLObject* e = getObject();
    if (e != NULL && ! isNull())
    {
        AQLDataInstance* dataInstance = e->getDataInstance();
        if (dataInstance != NULL)
        {
            AQLFunctionManager& em = dataInstance->getFunctionMaster();
            const AQLCoreFunctionHolder& h = em.getFunction(mName);
            if (! h.isDefined() || ! h.isTypeOf(FN_INTERPOLATION)) 
            {
                mpMethods = NULL;
                AQLString msg(mName);
                mName = "";
                msg += " is not found in Function Master as AQLInterpolationBase";
                throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            
            mpMethods = dynamic_cast<AQLInterpolationBase*>(h.get().clone());
            mFnHolder.set(mpMethods,true);
			setNull(false);
        }
    }
	else
	{
		if(mpMethods!=NULL)
		{
			mFnHolder.set(NULL,false);
			mpMethods=NULL;
			setNull(true);
		}
	}
}

AQLString AQLPriceDataInterpolation::adjustInterpolationName(AQLString candidateName)
{
	const AQLString fnString = "fn_";
	const AQLString interpolationString = "interpolation";

	AQLString retName = candidateName;
	retName.toLower();
	if(retName.subString(0,2) != fnString)
	{
		retName = fnString + retName;
	}

	int idxInString = retName.findString( interpolationString );
	if(idxInString > 0)
	{
		retName = retName.subString(0,idxInString+interpolationString.size());
	} else
	{
		retName +=  interpolationString;
	}

	return retName;
}


