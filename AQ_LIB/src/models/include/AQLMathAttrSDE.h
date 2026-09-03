#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLCoreTemplateType.h"


// AQLMathAttrSDE's Data ID
#define DATA_SDE 61

// SDE path Type
enum SDEPATH_TYPE {
	IR,		// IR
	FX,		// FX
	//furuya//
	sVOL		// VOL

};




class AQLRatesSDEBase;
class AQLIntegralBase;

//============== DECLARE AQLMathAttrSDE ============================
/*! 
    @brief Declaration of class representing a sde data
*/
class AQLMathAttrSDE : public AQLPriceDataType
{
public:
//  LIFECYCLE
	// default constructor
    AQLMathAttrSDE(void);

	// copy constructor
    AQLMathAttrSDE(const AQLMathAttrSDE& attr);

	// constructor
    AQLMathAttrSDE(const AQLCoreFunctionHolder& h, const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);

    // constructor
	AQLMathAttrSDE(AQLRatesSDEBase* b, const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);
	// destructor
    virtual ~AQLMathAttrSDE(void);

//  QUERY
	// copy this object
    virtual AQLPriceDataType*        clone() const;// %%% COVARIANT RETURN %%% 

	// get string representation of SDE which this object holds
    virtual AQLString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;
    
	// return class type
    function_t                  getType() const;

	// return the function object which this class holds
    const AQLRatesSDEBase&			getSDE(void) const;
	// return the SDE object which this class holds
    AQLRatesSDEBase&				getSDE(void);
	// return currency
	/*!
		@return currency
	*/
	const AQLString&				getCurrency(void) const {return mCurrency;}
	// return sde path type
	/*!
		@return sde path type (IR, FX)
	*/
	SDEPATH_TYPE				getSDEPathType(void) const {return mType;}
	
	//  OPERATOR    
    // set SDE object with string repsentation "str"
    virtual void                convertFromString(const AQLString& str);
        
	// set SDE object specified by "name"
    void                        setSDE(const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);
	// set SDE object with name "name"
    void                        setSDE(AQLRatesSDEBase* b, 
                                            const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);

//  VARIATION METHODS



//  OPERATION

protected:
	// initialize this class with another
    virtual AQLPriceDataType&        assignment(const AQLPriceDataType& a);
	// compare this object with another
    virtual int                 compare(const AQLPriceDataType& a) const;

	//	set up the pointer to data holder
    virtual void                setHolder(AQLDataHolder* holder);

private:
                                //=========================================
	// inner method to set up AttrSDE with name "mName"
    void                        setSDE();

    AQLRatesSDEBase*				mpSDE;			// pointer to SDE object 
	AQLCoreFunctionHolder            mFnHolder;		// pointer to function holder
	AQLString                    mName;			// SDE name
	AQLString					mCurrency;		// currency of sde path
	SDEPATH_TYPE				mType;			// type of path (IR, FX,...)

};
