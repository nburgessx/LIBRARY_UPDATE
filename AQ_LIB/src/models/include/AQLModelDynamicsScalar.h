#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelDynamicsBase.h"


// ID for AQLRatesPathElementScalar
#define PE_SCALAR	2 




///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents SCALAR value
*/
class AQLRatesPathElementScalar : public AQLRatesPathElementBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesPathElementScalar(SCALAR a = 0);
	// copy constructor
	AQLRatesPathElementScalar(const AQLRatesPathElementScalar& v);
	// Destructor
	virtual ~AQLRatesPathElementScalar();

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLRatesPathElementBase*	
								clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;

	// equal operator
    virtual	AQLRatesPathElementScalar&
								operator = (const AQLRatesPathElementScalar& a);
	// equal operator
    /*!	
    	@param[in] a scalar value
    	@return this object
    */
	virtual	AQLRatesPathElementScalar&
								operator = (SCALAR a) {mValue[0] = a; return *this;}
	// get value
    /*!
	    @return value
    */
	//virtual const void*		get() const {return &mValue;}
	
	// set value
    /*!
		@param[in] pdata pointer to value to set
    */	
//	virtual void				set(const void *pdata) 
//								{mValue = *reinterpret_cast<const SCALAR*>(pdata);}
	void						set(SCALAR a) {mValue[0] = a;}
	// set value
	void						set(const SCALARARRAY& a);
	// set value
	virtual void				set(const AQLRatesPathElementBase& a); 

private:
//	SCALARARRAY mValue;// value
protected:

};

