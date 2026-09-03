#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "LAEvent.h"


// Funciton ID of MACalibrationFunc
#define FN_CALIBRATIONFUNC	110011
// Function Name of MACalibrationFunc
#define FN_CALIBRATIONFUNC_STR	"fn_calibrationfunc"


class MACalibrationFunc : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit MACalibrationFunc(void);
	// destructor
	virtual ~MACalibrationFunc(void);
	// copy constructor
	MACalibrationFunc(const MACalibrationFunc &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;	

		                        //==========================================
	                            // return function value
	virtual double				operator()(double t) const;	

			                    //==========================================
	                            // set real function
	void				         setRealFunction(const AQLFunctionBase &method);	
			                    //==========================================
	                            // set ready on
	void				         setOn();	


	
protected :
	const AQLFunctionBase *mpRealFunc;		// pointer to real function
	bool  mIsReady;                         // ready flg
	mutable MAEvent mEvent;                 // event
#ifdef __HAS_MIC__
	mutable common_lib::Mutex mMutex;
#endif
};

