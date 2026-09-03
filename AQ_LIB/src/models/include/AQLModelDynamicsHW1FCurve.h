#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
//#include "AQLCoreTemplateType.h"
#include "AQLMathHWFuncMR.h"
#include "AQLMathHWFuncTool.h"
#include "AQLModelDynamicsAFFCurve.h"
#include "AQLMathHWFuncSigma.h"
#include "AQLGaussLegendre.h"
#include <map>


//+++++ DEFINE +++++
// ID for AQLRatesPathElementHW1FCurve
#define PE_HWCURVE	10032 

using namespace std;

//
//------------------------------ AQLRatesPathElementHW1FCurve ------------------------------
//

/*! 
    @brief Declaration of path element class that represents curve object for HW
*/
class AQLRatesPathElementHW1FCurve : public AQLRatesPathElementAFFCurve
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    AQLRatesPathElementHW1FCurve();

    AQLRatesPathElementHW1FCurve(double t,double a, double s);
                                //======================================
    	                        // copy constructor
	AQLRatesPathElementHW1FCurve(const AQLRatesPathElementHW1FCurve& v);
                                //======================================
                                // Destructor
	virtual ~AQLRatesPathElementHW1FCurve();
                                //======================================

								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLRatesPathElementBase*	
								clone() const;
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
								//======================================
	                            // equal operator
                                /*!
	                                @param[in] a source object
                                    @return copy object
                                */
    virtual	AQLRatesPathElementHW1FCurve&
								operator = (const AQLRatesPathElementHW1FCurve& a);
								//======================================
                             	// set value for MC
	virtual void				set (const AQLRatesPathElementBase& a);
	// set start time of this curve
	virtual void				set_t (double t);
                                //======================================
    virtual double E( double T ) const;
    virtual double varp(double T) const;
	virtual double B( double T ) const;
	virtual double initialF() const {return mInitialF;}                            
	virtual double initialF(double T) const;
                                //======================================   
protected:
    
    const AQLRatesPathElementCurve* mpInitialCurve;
    virtual double A( double T ) const;
	mutable map<double, double>* E_cache;
    mutable map<double, double>* A_cache;
    mutable map<double, double>* B_cache;
    mutable map<double, double>* varp_cache;
    mutable bool delete_cache;
private:
                                //======================================
                                //
    void						clear();
	bool is_cloned;
	double mMR;
    double mSigma;
    double s_SQa;
	double mInitialF;			


};

//
//------------------------------ AQLRatesPathElementHW1FCurveTMDPT ------------------------------
//


// ID for AQLRatesPathElementHW1FCurveTMDPT
#define PE_HWCURVETMDPT	10033 
class AQLMathHWFuncMR;
class AQLMathHWSigmaFunc;

class AQLRatesPathElementHW1FCurveTMDPT : public AQLRatesPathElementHW1FCurve
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    AQLRatesPathElementHW1FCurveTMDPT();
    
    AQLRatesPathElementHW1FCurveTMDPT( double t);
                                //======================================
	AQLRatesPathElementHW1FCurveTMDPT(double t, double r0, AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma,												AQLRatesPathElementCurve& initCurve);
								//======================================
    	                        // copy constructor
	AQLRatesPathElementHW1FCurveTMDPT(const AQLRatesPathElementHW1FCurveTMDPT& rhs);
                                //======================================
	                            // Destructor
	virtual ~AQLRatesPathElementHW1FCurveTMDPT();
                                //======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLRatesPathElementBase*	
								clone() const;
								//======================================
								// Return this class ID
	virtual pathelement_t		getType() const;
                                //=====================================
                        	    // equal operator
                                /*!
	                                @param[in] a source object
                                    @return copy object
                                */
    virtual	AQLRatesPathElementHW1FCurveTMDPT&
								operator = (const AQLRatesPathElementHW1FCurveTMDPT& rhs);
                                //=====================================
                                // create curve                                
    double                      E( double T ) const;
                                //=====================================
								// clear cache;
	void clear_cache(void){E_cache->clear();A_cache->clear();B_cache->clear();varp_cache->clear();};
    
	AQLMathHWFuncTool* mpHWtoolMR;
	AQLMathHWFuncTool* mpHWtoolVar;
	virtual double B( double T ) const;
	
protected:
	virtual double A( double T ) const;
    

private:
	AQLGaussLegendre mGL;
	bool is_cloned;
    double varp( double T )  const;

    

	
	

};

