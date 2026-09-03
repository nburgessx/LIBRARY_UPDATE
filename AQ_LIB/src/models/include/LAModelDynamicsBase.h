#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include <vector>


#define pathelement_t int
// ID for LARatesPathElementBase
#define PE_BASE	1 

#ifdef __SCALAR_FLOAT__
	typedef float SCALAR;
#else
	typedef double SCALAR;
#endif
typedef std::vector<SCALAR> SCALARARRAY;



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of path element

*/
class LARatesPathElementBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPathElementBase();
	// Destructor
	virtual ~LARatesPathElementBase();

//  QUERY
								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LARatesPathElementBase*	
								clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
								//======================================
								// get value
//	virtual const void*			get(void) const = 0; 
								//======================================
								// get value
	virtual const SCALARARRAY&	get(void) const {return mValue;} 
								//======================================
								// set value
//	virtual void				set(const void* pdata) = 0; 
								//======================================
								// set value
	virtual void				set(SCALAR a);
								//======================================
								// set value
	virtual void				set(const SCALARARRAY& a);
								//======================================
								// set value
	virtual void				set(const LARatesPathElementBase& a) = 0; 


private:
	
protected:
	SCALARARRAY mValue;// data
};

