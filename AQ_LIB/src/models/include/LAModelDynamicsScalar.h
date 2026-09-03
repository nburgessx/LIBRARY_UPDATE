#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAModelDynamicsBase.h"


// ID for LARatesPathElementScalar
#define PE_SCALAR	2 




///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents SCALAR value
*/
class LARatesPathElementScalar : public LARatesPathElementBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPathElementScalar(SCALAR a = 0);
	// copy constructor
	LARatesPathElementScalar(const LARatesPathElementScalar& v);
	// Destructor
	virtual ~LARatesPathElementScalar();

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LARatesPathElementBase*	
								clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;

	// equal operator
    virtual	LARatesPathElementScalar&
								operator = (const LARatesPathElementScalar& a);
	// equal operator
    /*!	
    	@param[in] a scalar value
    	@return this object
    */
	virtual	LARatesPathElementScalar&
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
	virtual void				set(const LARatesPathElementBase& a); 

private:
//	SCALARARRAY mValue;// value
protected:

};

