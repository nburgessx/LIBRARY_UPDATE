/*! @file
    @brief Implementation to generate random numbers.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceDataRand.h"
#include "LAFunctionManager.h"
#include "LADataInstance.h"

using namespace std;

//======================= IMPLEMENTATION =============================
//// LIFECYCLE ////
/*!
    @brief default constructor
*/
LAPriceDataRand::LAPriceDataRand(void) : 
	LAPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(), mName("")
{
}
/*!
    @brief copy constructor
*/
LAPriceDataRand::LAPriceDataRand(const LAPriceDataRand& attr) :
	LAPriceDataType(attr),
	mpMethods(NULL), mFnHolder(attr.mFnHolder), mName("")
{
	if (attr.mName != "")
	{
		if (mFnHolder.isDefined())
		{
			mpMethods = dynamic_cast<LARandBase*>(attr.mFnHolder.get().clone());
			mFnHolder.set(mpMethods,true);

//			mpMethods = &(dynamic_cast<const class LARandBase&>
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
LAPriceDataRand::LAPriceDataRand(const LACoreFunctionHolder& h, 
										 const LAString& name) :
	LAPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(h), mName("")
{
	if (mFnHolder.isDefined() && name.isDefined() && name != "")
	{
		setNull(false);

		mpMethods = dynamic_cast<LARandBase*>(h.get().clone());
		mFnHolder.set(mpMethods,true);
//		mpMethods = &(dynamic_cast<const class LARandBase&>
//													(mFnHolder.get()));
		mName = name;
	} 
}
/*!
    @brief constructor
    @param[in] b pointer to random generator function
    @param[in] name name of random generator function
*/
LAPriceDataRand::LAPriceDataRand(LARandBase* b, 
										 const LAString& name) :
	LAPriceDataType(DATA_RAND),
	mpMethods(NULL), mFnHolder(), mName("")
{
	setMethod(b, name);
}
/*!
    @brief destructor
*/
LAPriceDataRand::~LAPriceDataRand(void)
{
	
}

//// QUERY ////
/*!
    @brief deep copy of the object
    @return pointer to the Data object newly created
*/
LAPriceDataType*
LAPriceDataRand::clone() const
{
    try {
        LAPriceDataRand*	pAttr = new LAPriceDataRand(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief random generators method in the string representation
    @return random generators method
*/
LAString      
LAPriceDataRand::convertToString(void) const
{
	return (isNull() ? NULL_STR : mName);
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function id
    @return True or false
*/
bool                
LAPriceDataRand::isTypeOf(function_t id) const
{
	return (! isNull() && mFnHolder.isDefined()) ? 
						mpMethods->isTypeOf(id) : false;
}
/*!
    @brief get function type
    @return function type
*/
function_t
LAPriceDataRand::getType(void) const
{
	if (! isNull() && mFnHolder.isDefined()) return mFnHolder.getType();
	return FN_RANDBASE;
}
/*!
    @brief return Method function to be set
    @return function to be set(LARandBase)
*/
const LARandBase&
LAPriceDataRand::getMethod(void) const 
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	return *mpMethods;
}


/*!
    @brief function to get dimension information to be set
    @return dimension to be set
*/
const UintArray&                
LAPriceDataRand::getDim(void)const
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	return mpMethods->getDim();
}

/*!
    @brief function to get seed
    @return seed
*/
const UlongArray&                
LAPriceDataRand::getSeed(void)const
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
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
LAPriceDataRand::getUniforms(DoubleArray& variates) 
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
LAPriceDataRand::getGaussians(DoubleArray& variates) 
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
LAPriceDataRand::convertFromString(const LAString& str)
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
		setMethod(data);
	}
}

 
/*!
    @brief function to set dimension information(first element should be the number of dimensionj
    @param[in] dimValue dimension information
*/
void
LAPriceDataRand::setDim(const UintArray& dimValue)
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	mpMethods->setDim(dimValue);
}

/*!
    @brief function to set seed
    @param[in] seedValue seed
*/
void
LAPriceDataRand::setSeed(const UlongArray& seedValue)
{
	if (isNull() || ! mFnHolder.isDefined())
	{
		throw LACoreInvalidData("No Method sets", __FILE__, __LINE__);
	}
	mpMethods->setSeed(seedValue);
}
/*!
    @brief function to set parameters
    @param[in] param paremeters

*/								
void
LAPriceDataRand::setParam(const DoubleMatrix& param)
{
	mpMethods->setParam(param);
}

/*!
    @brief set the method of function from the name
    @param[in] name function name
*/
void
LAPriceDataRand::setMethod(const LAString& name)
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
LAPriceDataRand::setMethod(LARandBase* b, 
										 const LAString& name)
{
	if (b != NULL && name.isDefined() && name != "")
	{
		update();//added by matsumura 20060105
		
		mFnHolder.set(b, true);
		setNull(false);

		mpMethods = b;
//		mpMethods = &(dynamic_cast<const class LARandBase&>
//													(mFnHolder.get()));
		mName = name;
	}
}

/*!
    @brief the contents of the object to be initialized with those of another object
    
    LAPriceDataType is supposed to be set on the argument

    @param[in] a object used by the reference
    @return reference to the LAPriceDataType
*/
LAPriceDataType& 
LAPriceDataRand::assignment(const LAPriceDataType& a)
{
	if (this == &a) return *this;

	if (a.getType() != DATA_RAND) 
	{
		LAString err = "Assignement error for LAPriceDataRand : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const LAPriceDataRand& in = dynamic_cast<const LAPriceDataRand&>(a);
	setNull(a.isNull());
	mpMethods = dynamic_cast<LARandBase*>(in.mpMethods->clone());
	mFnHolder.set(mpMethods,true);
	mName = in.mName;

	return *this;
}
/*!
    @brief compare the contents against the other objecet

	error if "a" is not LAPriceDataRand.

    @param[in] a object to compare

    @return 0 if the same, and not 0 otherwise. Comparison about set info is performed.
*/
int          
LAPriceDataRand::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_RAND) 
	{
		LAString err = "Compare error for LAPriceDataRand : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	const LAPriceDataRand& in = dynamic_cast<const LAPriceDataRand&>(a);
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
LAPriceDataRand::setHolder(LADataHolder* holder)
{
	LAPriceDataType::setHolder(holder);
	if(mpMethods == NULL)
	{
		setMethod();
	}
}
/*!
    @brief set the object from the function name
*/
void 
LAPriceDataRand::setMethod(void)
{
	// update
    update();
	
	LAObject* e = getObject();
	if (e != NULL && ! isNull())
	{
		LADataInstance* dataInstance = e->getDataInstance();
		if (dataInstance != NULL)
		{
			LAFunctionManager& em = dataInstance->getFunctionMaster();
			const LACoreFunctionHolder& h = em.getFunction(mName);
			if (! h.isDefined() || ! h.isTypeOf(FN_RANDBASE)) 
			{
			/*	mpMethods = NULL;
				mName = "";
				setNull(true);
				return;*/
				mpMethods = NULL;
				LAString msg(mName);
				mName = "";
				msg += " is not found in Function Master as LARandBase";
				throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
			}
			mpMethods = dynamic_cast<LARandBase*>(h.get().clone());
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
