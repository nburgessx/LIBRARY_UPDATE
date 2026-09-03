#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LAModelDynamicsCurve.h"



// ID for LARatesPathElementLMMCurve
#define PE_LMMCURVE	4 



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of path element class that represents curve object for libor market model
*/
class LARatesPathElementLMMCurve : public LARatesPathElementCurve
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesPathElementLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t = 0);
	// Destructor
	virtual ~LARatesPathElementLMMCurve();
	// copy constructor
	LARatesPathElementLMMCurve(const LARatesPathElementLMMCurve& v);

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
    /*!
	    @param[in] a source object
        @return copy object
    */
    virtual	LARatesPathElementLMMCurve&
								operator = (const LARatesPathElementLMMCurve& a);

	// get discount bond price
	virtual double				getP (double T) const;
	// get value
	/*!
		@return value
	*/
//	virtual const void*			get(void) const {return &mData_L;}
	// set value
	/*!
		@param[in] pdata pointer to value to set
	*/
/*	virtual void				set(const void* pdata) 
								{	
									const SCALARARRAY* p_array = reinterpret_cast<const SCALARARRAY*>(pdata); 
				
									int i, j;
									for (i = p_array->size() - 1, j = mData_L.size() - 1; j >= 0; j--, i--)
										mData_L[j] = (*p_array)[i];
								}*/
	// set value
	/*!
		@param[in] a value to set
	*/	
	virtual void				set(const SCALARARRAY& a) 
								{	
									int i, j;
									for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
										mValue[j] = a[i];
									mPos = 10000;
								}
	// set value
	virtual void				set (const LARatesPathElementBase& a);


	// set start time of this curve
	virtual void				set_t (double t); 
	// get libor rate
	double						getL (double ts, double te, double delta, const LARatesPathElementCurve& curve) const;
	// get tenor
	const DoubleArray*			getTenor() const {return mpTenor;}
	// get delta tenor
	const DoubleArray*			getDeltaTenor() const {return mpDeltaTenor;}
	// get delta tenor
	const DoubleArray*			getInitialLibor() const {return mpInitialData_L;}

private:
	// clear data
	void						clear();

	int*						mpRefCount;		// reference counter of common data   

protected:
//	SCALARARRAY					mData_L;		// libor
	DoubleArray*				mpTenor;		// tenor
	DoubleArray*				mpDeltaTenor;	// delta tenor
	SCALARARRAY*				mpInitialData_L;// initial libor
	const LARatesPathElementCurve*	mpInitialCurve;	// initial curve
	std::map<double, double>*   mpInitialCache; // cache
	double						mtP;			// DF(mt)
	mutable double              mlt;            // lt for cache
	mutable double              mlt_;            // lt_ for cache
	mutable unsigned int        mPos;             // mPos for cache

};
