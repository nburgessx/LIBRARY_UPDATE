#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLCoreTemplateType.h"


// LAMathAttrSDE's Data ID
#define DATA_SDE 61

// SDE path Type
enum SDEPATH_TYPE {
	IR,		// IR
	FX,		// FX
	//furuya//
	sVOL		// VOL

};




class LARatesSDEBase;
class AQLIntegralBase;

//============== DECLARE LAMathAttrSDE ============================
/*! 
    @brief Declaration of class representing a sde data
*/
class LAMathAttrSDE : public AQLPriceDataType
{
public:
//  LIFECYCLE
	// default constructor
    LAMathAttrSDE(void);

	// copy constructor
    LAMathAttrSDE(const LAMathAttrSDE& attr);

	// constructor
    LAMathAttrSDE(const AQLCoreFunctionHolder& h, const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);

    // constructor
	LAMathAttrSDE(LARatesSDEBase* b, const AQLString& name, SDEPATH_TYPE type, const AQLString& currency);
	// destructor
    virtual ~LAMathAttrSDE(void);

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
    const LARatesSDEBase&			getSDE(void) const;
	// return the SDE object which this class holds
    LARatesSDEBase&				getSDE(void);
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
    void                        setSDE(LARatesSDEBase* b, 
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

    LARatesSDEBase*				mpSDE;			// pointer to SDE object 
	AQLCoreFunctionHolder            mFnHolder;		// pointer to function holder
	AQLString                    mName;			// SDE name
	AQLString					mCurrency;		// currency of sde path
	SDEPATH_TYPE				mType;			// type of path (IR, FX,...)

};
