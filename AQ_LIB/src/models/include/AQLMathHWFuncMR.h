/*! @file
    @brief Declaration of path element class that represents curve object for HW

*/
#ifndef AQLMATHHWFUNCMR_H
#define AQLMATHHWFUNCMR_H

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMathHWFuncMR.h
//
//  SYNOPSIS    :       AQLMathHWFuncMR
//  DESCRIPTION :       Declaration of path element class that represents curve object for HW
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
#include "AQLCoreTemplateType.h"
#include "AQLFunctionBase.h"
#include "AQLModelDynamicsAFFCurve.h"
#include "AQLInterpolationBase.h"
#include <map>
#include "AQLGaussLegendre.h"
#include "AQLCoreSystemError.h"


const int  HWGAUSSLEGENDRENUM = 20;
const int  HWINTEGRATEDNUM = 2;



//
//------------------------------ AQLMathHWFuncMR ------------------------------
//
//

//+++++ DEFINE +++++
// Funciton ID of AQLMathHWFuncMR
#define FN_HWFUNCMR	10018
// Function Name of AQLMathHWFuncMR
#define FN_HWFUNCMR_STR	"fn_hwfuncmr"

class AQLMathHWFuncMR : public AQLFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
	AQLMathHWFuncMR();
    explicit 
    AQLMathHWFuncMR(double a) : mMR(a) {};
                                //======================================
                                // copy constructor
	AQLMathHWFuncMR(const AQLMathHWFuncMR &rhs);
                                //======================================
                                // destructor
	virtual ~AQLMathHWFuncMR(void);
                                //======================================
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*    clone() const;
								//======================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const
								{
									return mMR;
									//throw AQLCoreInvalidData("this operator is not supported",__FILE__,__LINE__);
								};
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	                            // return a value
    virtual double              get_a(double t) const { return mMR; }
                                //==========================================
                                // integrate from 0 to t
    virtual double              integrate( double t ) const { return mMR * t; }
                                //==========================================
                                // integrate from t_s to t_e
    virtual double              integrate( double ts, double te ) const { return mMR * (te - ts); }
                                //==========================================

private :
	
    // a
    double mMR;		     	
	
};

//+++++ DEFINE +++++
// Funciton ID of AQLMathHWFuncMRTMDPT
#define FN_HWFUNCMRTMDPT	10019
// Function Name of AQLMathHWFuncMRTMDPT
#define FN_HWFUNCMRTMDPT_STR	"fn_hwfuncmrtmdpt"

class AQLMathHWFuncMRTMDPT : public AQLMathHWFuncMR
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
   /* AQLMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                  const DoubleArray&    a_grid_,
                  Interpolation&        interpolation_,
                  int                   integrate_n_
                );*/
	 AQLMathHWFuncMRTMDPT( const DoubleArray&    T_grid_,
                  const DoubleArray&    a_grid_,
				  AQLInterpolationBase&	interpolation_);
                                //======================================
                            	// copy constructor
	AQLMathHWFuncMRTMDPT(const AQLMathHWFuncMRTMDPT &rhs);
                                //======================================
                                // destructor
	virtual ~AQLMathHWFuncMRTMDPT(void);
                                //======================================	
//  QUERY
								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*   clone() const;
								//======================================
                            	// equal operator
                                /*!
	                                @param[in] a source object
                                    @return copy object
                                */
    //virtual AQLMathHWFuncMRTMDPT&       operator =( const AQLMathHWFuncMRTMDPT& rhs );
	
								// return function value
	virtual double				operator()(const DoubleArray& x) const
								{
									return mpInter->value(x[0]);
								};
								//======================================
								// return this class type
    virtual function_t          getType() const;
	                            //==========================================
	                            // return a value at t
    virtual double              get_a(double t) const;
                                //======================================
                                // integrate from 0 to t 
    virtual double              integrate( double t ) const;
                                //======================================
                                // integrate from t_s to t_e
    virtual double              integrate( double ts, double te ) const;
                                //======================================
  
private:

    
    DoubleArray a_grid;		// a grid
	DoubleArray T_grid;		 // time grid    
	AQLInterpolationBase* mpInter;
    bool is_cloned;
   	AQLGaussLegendre mGL;
	
	
};


#endif