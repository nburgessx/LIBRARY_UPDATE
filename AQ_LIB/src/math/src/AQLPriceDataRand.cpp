/*! @file
    @brief Implementation to generate random numbers.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDataRand.h"
#include "AQLFunctionManager.h"
#include "AQLDataInstance.h"

using namespace std;

//======================= IMPLEMENTATION =============================
/*!
    @brief default constructor
*/
AQLPriceDataRand::AQLPriceDataRand(void) : 
	AQLPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(), mName("")
{
}
/*!
    @brief copy constructor
*/
AQLPriceDataRand::AQLPriceDataRand(const AQLPriceDataRand& attr) :
	AQLPriceDataType(attr),
	mpMethods(NULL), mFnHolder(attr.mFnHolder), mName("")
{
	if (attr.mName != "")
	{
		if (mFnHolder.isDefined())
		{
			mpMethods = dynamic_cast<AQLRandBase*>(attr.mFnHolder.get().clone());
			mFnHolder.set(mpMethods,true);

//			mpMethods = &(dynamic_cast<const class AQLRandBase&>
//				(mFnHolder.get()));
		}
		mName = attr.mName;
	} 
}
/*!
    @brief constructor

    @param[in] h Functon Holder
    @param[in] name function name

    @return period from start date (in years)
*/
AQLPriceDataRand::AQLPriceDataRand(const AQLCoreFunctionHolder& h, 
										 const AQLString& name) :
	AQLPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(h), mName("")
{
	if (mFnHolder.isDefined() && name.isDefined() && name != "")
	{
		setNull(false);

		mpMethods = dynamic_cast<AQLRandBase*>(h.get().clone());
		mFnHolder.set(mpMethods,true);
//		mpMethods = &(dynamic_cast<const class AQLRandBase&>
//													(mFnHolder.get()));
		mName = name;
	} 
}
/*!
    @brief constructor
    @param[in] b pointer to random generator function
    @param[in] name name of random generator function
*/
AQLPriceDataRand::AQLPriceDataRand(AQLRandBase* b, 
										 const AQLString& name) :
	AQLPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(), mName("")
{
	setMethod(b, name);
}
/*!
    @brief destructor
*/
AQLPriceDataRand::~AQLPriceDataRand(void)
{
	
}

/*!
    @brief deep copy of the object
    @return pointer to the Data object newly created
*/
AQLPriceDataType*
AQLPriceDataRand::clone() const
{
    try {
        AQLPriceDataRand*	pAttr = new AQLPriceDataRand(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief random generators method in the string representation
    @return random generators method
*/
AQLString      
AQLPriceDataRand::convertToString(void) const
{
	return (isNull() ? NULL_STR : mName);
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool                
AQLPriceDataRand::isTypeOf(function_t id) const
{
	return (! isNull() && mFnHolder.isDefined()) ? 
						mpMethods->isTypeOf(id) : false;
}
/*!
    @brief get function type
    @return function type
*/
function_t
AQLPriceDataRand::getType(void) const
{
	if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
	return FN_RANDBASE;
}
/*!
    @brief return Method function to be set
    @return function to be set(AQLRandBase)
*/
const AQLRandBase&
AQLPriceDataRand::getMethod(void) const 
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	return *mpMethods;
}


/*!
    @brief function to get dimension information to be set
    @return dimension to be set
*/
const UintArray&                
AQLPriceDataRand::getDim(void)const
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	return mpMethods->getDim();
}

/*!
    @brief function to get seed
    @return seed
*/
const UlongArray&                
AQLPriceDataRand::getSeed(void)const
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	return mpMethods->getSeed();
}

/*!
    @brief function to generate uniform random numbers

	A uniform random number is generated based on dimensional information that you set in advance.
	By generating a random number, usually change the seed.

    @param[in] variates store the random number on 1 trial of Monte Carlo of generated uniform random numbers 
*/
void
AQLPriceDataRand::getUniforms(DoubleArray& variates) 
{
	//error throw ??
	if (isNull()) return;
	mpMethods->getUniforms(variates);
}
/*!
    @brief function to generate normal random numbers
    
    A normal random number is generated based on dimensional information that you set in advance.
	By generating a random number, usually change the seed.

    @param[in] variates store the random number on 1 trial of Monte Carlo of generated uniform random numbers
*/
void
AQLPriceDataRand::getGaussians(DoubleArray& variates) 
{
	//error throw ??
	if (isNull()) return;
	mpMethods->getGaussians(variates);
}
/*!
    @brief set the method of function to generate rondom numbers from the selected string format
    @param[in] str string to represent the random number generation method to be selected preferably
*/
void          
AQLPriceDataRand::convertFromString(const AQLString& str)
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
		setMethod(data);
	}
}

 
/*!
    @brief function to set dimension information(first element should be the number of dimension)
    @param[in] dimValue dimension information
*/
void
AQLPriceDataRand::setDim(const UintArray& dimValue)
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	mpMethods->setDim(dimValue);
}

/*!
    @brief function to set seed
    @param[in] seedValue seed
*/
void
AQLPriceDataRand::setSeed(const UlongArray& seedValue)
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw AQLCoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	mpMethods->setSeed(seedValue);
}
/*!
    @brief function to set parameters
    @param[in] param paremeters

*/								
void
AQLPriceDataRand::setParam(const DoubleMatrix& param)
{
	mpMethods->setParam(param);
}

/*!
    @brief set the method of function from the name
    @param[in] name function name
*/
void
AQLPriceDataRand::setMethod(const AQLString& name)
{
	mName = name;
	if (name != "")
	{
		setNull(false);
	}
	setMethod();
}
/*!
    @brief set name and the method of function, and holds ownership of the pointer
    @param[in] b function
	@param[in] name function name
*/
void 
AQLPriceDataRand::setMethod(AQLRandBase* b, 
										 const AQLString& name)
{
	if (b != NULL && name.isDefined() && name != "")
	{
		update();//added by matsumura 20060105
		
		mFnHolder.set(b, true);
		setNull(false);

		mpMethods = b;
//		mpMethods = &(dynamic_cast<const class AQLRandBase&>
//													(mFnHolder.get()));
		mName = name;
	}
}

/*!
    @brief the contents of the object to be initialized with those of another object
    
    AQLPriceDataType is supposed to be set on the argument

    @param[in] a object used by the reference
    @return reference to the AQLPriceDataType
*/
AQLPriceDataType& 
AQLPriceDataRand::assignment(const AQLPriceDataType& a)
{
	if (this == &a) return *this;

	if (a.getType() != DATA_RAND) 
	{
		AQLString err = "Assignement error for AQLPriceDataRand : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const AQLPriceDataRand& in = dynamic_cast<const AQLPriceDataRand&>(a);
	setNull(a.isNull());
	mpMethods = dynamic_cast<AQLRandBase*>(in.mpMethods->clone());
	mFnHolder.set(mpMethods,true);
	mName = in.mName;

	return *this;
}
/*!
    @brief compare the contents against the other objecet

	error if "a" is not AQLPriceDataRand.

    @param[in] a object to compare

    @return 0 if the same, and not 0 otherwise. Comparison about set info is performed.
*/
int          
AQLPriceDataRand::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_RAND) 
	{
		AQLString err = "Compare error for AQLPriceDataRand : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const AQLPriceDataRand& in = dynamic_cast<const AQLPriceDataRand&>(a);
	if (isNull() && in.isNull()) return 0;
	if (isNull()) return -1;
	if (in.isNull()) return 1;


	return getType() - in.getType();
}
/*!
    @brief set a pointer to the Holder of the Data
    @param[in] holder pointer of Holder to set
*/
void				
AQLPriceDataRand::setHolder(AQLDataHolder* holder)
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
AQLPriceDataRand::setMethod(void)
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
			if (! h.isDefined() || ! h.isTypeOf(FN_RANDBASE)) 
			{
			/*	mpMethods = NULL;
				mName = "";
				setNull(true);
				return;*/
				mpMethods = NULL;
				AQLString msg(mName);
				mName = "";
				msg += " is not found in Function Master as AQLRandBase";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
			}
			mpMethods = dynamic_cast<AQLRandBase*>(h.get().clone());
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
