#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
//#include "LACoreTemplateType.h"
#include "LAMathHWFuncMR.h"
#include "LAMathHWFuncTool.h"
#include "LAModelDynamicsAFFCurve.h"
#include "LAMathHWFuncSigma.h"
#include "LAGaussLegendre.h"
#include <map>


//+++++ DEFINE +++++
// ID for LARatesPathElementHW1FCurve
#define PE_HWCURVE	10032 

using namespace std;

//
//------------------------------ LARatesPathElementHW1FCurve ------------------------------
//

/*! 
    @brief Declaration of path element class that represents curve object for HW
*/
class LARatesPathElementHW1FCurve : public LARatesPathElementAFFCurve
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    LARatesPathElementHW1FCurve();

    LARatesPathElementHW1FCurve(double t,double a, double s);
                                //======================================
    	                        // copy constructor
	LARatesPathElementHW1FCurve(const LARatesPathElementHW1FCurve& v);
                                //======================================
                                // Destructor
	virtual ~LARatesPathElementHW1FCurve();
                                //======================================

								//======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LARatesPathElementBase*	
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
    virtual	LARatesPathElementHW1FCurve&
								operator = (const LARatesPathElementHW1FCurve& a);
								//======================================
                             	// set value for MC
	virtual void				set (const LARatesPathElementBase& a);
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
    
    const LARatesPathElementCurve* mpInitialCurve;
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
//------------------------------ LARatesPathElementHW1FCurveTMDPT ------------------------------
//


// ID for LARatesPathElementHW1FCurveTMDPT
#define PE_HWCURVETMDPT	10033 
class LAMathHWFuncMR;
class LAMathHWSigmaFunc;

class LARatesPathElementHW1FCurveTMDPT : public LARatesPathElementHW1FCurve
{
public:
//  LIFECYCLE
                                //======================================
	                            // constructor
    explicit 
    LARatesPathElementHW1FCurveTMDPT();
    
    LARatesPathElementHW1FCurveTMDPT( double t);
                                //======================================
	LARatesPathElementHW1FCurveTMDPT(double t, double r0, LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma,												LARatesPathElementCurve& initCurve);
								//======================================
    	                        // copy constructor
	LARatesPathElementHW1FCurveTMDPT(const LARatesPathElementHW1FCurveTMDPT& rhs);
                                //======================================
	                            // Destructor
	virtual ~LARatesPathElementHW1FCurveTMDPT();
                                //======================================
								// Check pathelement for this class ID
	virtual bool                isTypeOf(pathelement_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LARatesPathElementBase*	
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
    virtual	LARatesPathElementHW1FCurveTMDPT&
								operator = (const LARatesPathElementHW1FCurveTMDPT& rhs);
                                //=====================================
                                // create curve                                
    double                      E( double T ) const;
                                //=====================================
								// clear cache;
	void clear_cache(void){E_cache->clear();A_cache->clear();B_cache->clear();varp_cache->clear();};
    
	LAMathHWFuncTool* mpHWtoolMR;
	LAMathHWFuncTool* mpHWtoolVar;
	virtual double B( double T ) const;
	
protected:
	virtual double A( double T ) const;
    

private:
	LAGaussLegendre mGL;
	bool is_cloned;
    double varp( double T )  const;

    

	
	

};

