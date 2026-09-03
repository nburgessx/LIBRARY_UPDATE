#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLInterpolationBase.h"



// ID for AQLRatesPathElementHJMCurve
#define PE_HJMCURVE	9 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for HJM
*/
class AQLRatesPathElementHJMCurve : public AQLRatesPathElementCurve
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesPathElementHJMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// Destructor
	virtual ~AQLRatesPathElementHJMCurve();
	// copy constructor
	AQLRatesPathElementHJMCurve(const AQLRatesPathElementHJMCurve& v);

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
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	AQLRatesPathElementHJMCurve&
								operator = (const AQLRatesPathElementHJMCurve& a);

	// get discount bond price
	virtual double				getP (double T) const;
	// set value
	/*!
		@param[in] a value to set
	*/	
	virtual void				set(const SCALARARRAY& a) 
								{	
									int i, j;
									for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
										mValue[j] = a[i];
								}
	// set value
	virtual void				set (const AQLRatesPathElementBase& a);


	// set start time of this curve
	virtual void				set_t (double t); 
	void						setInterpolationMethod(AQLInterpolationBase* pinter); 

private:
	// set start time of this curve
	void						set_t (void); 
	// clear data
	void						clear();

	int*						mpRefCount;		// reference counter of common data   

protected:
//	SCALARARRAY					mData_L;		// libor
	DoubleArray*				mpTenor;		// tenor
	DoubleArray*				mpDeltaTenor;	// delta tenor
	SCALARARRAY*				mpInitialData_L;// initial libor
	const AQLRatesPathElementCurve*	mpInitialCurve;	// initial curve
	AQLInterpolationBase*		mpInter;	// pointer to interpolation function
};
