#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLRatesBM.h"
#include "AQLRatesNumeraireBase.h"
#include "AQLRatesPEInterpolationBase.h"

// ID for AQLRatesSDEBase
#define FN_SDEBASE	1701 

class AQLRatesPathElementBase;
class AQLRatesSDEIntegralBase;

// sde type
enum SDE_TYPE {	
	dX,
	DIVIDEdXbyX
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde class

*/
class AQLRatesSDEBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesSDEBase(SDE_TYPE type);
	//	Copy constructor
	AQLRatesSDEBase(const AQLRatesSDEBase& v);
	// Destructor
	virtual ~AQLRatesSDEBase();
//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// get sde type
								/*!
									@return sde type
								*/
	SDE_TYPE					getSDEType() const {return mType;}								
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual AQLRatesNumeraireBase*	getNumeraire(void) {return mpNumeraire;}
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual const AQLRatesNumeraireBase*
								getNumeraire(void) const {return mpNumeraire;}
								//==========================================
	                            // get interpolation class
								/*!
									@return interpolation class
								*/		
	AQLRatesPEInterpolationBase*
								getInterpolationMethod(void) const {return mpInter;}
								//==========================================
	                            // get integralfunction class
								/*!
									@return integralfunction class
								*/		
	AQLRatesSDEIntegralBase*
								getIntegralFunction(void) const {return mpIntegral;}

								//==========================================
	                            // get drift
								/*!
									@return drift
								*/		
	const std::vector<AQLFunctionBase*>&	
								getDrift(void) const {return mDrift;}
								//==========================================
	                            // get volatility
								/*!
									@return volatility
								*/	
	const std::vector<std::vector<AQLFunctionBase*> >& 
								getVolatility(void) const {return mVolatility;}
								//==========================================
	                            // get Brownian Motion class
								/*!
									@return Brownian Motion class
								*/	
	AQLRatesBM*		 				getBM() const {return mpBM;}
								//==========================================
	                            // get time grid
								/*!
									@return time grid
								*/	
	const DoubleArray&			getTimeGrid(void) const {return mTimeGrid;}		
    							//==========================================
	                            // get path
	virtual const	std::vector<AQLRatesPathElementBase*>&
								getPath(void);
   								//==========================================
							    // get path element
	virtual const AQLRatesPathElementBase*
								getPathElement(unsigned int pos) = 0;
 								//==========================================
								// return string representaion
//    virtual AQLString			convertToString(void) const;
//	OPERATION
								//==========================================
								// transform from string representaion
//     virtual void				convertFromString(const AQLString& str);
								//==========================================
	                            // set numeraire		
	virtual void				setNumeraire(AQLRatesNumeraireBase* pnumeraire);
								//==========================================
	                            // set drift
	void						setDrift(std::vector<AQLFunctionBase*>& drift);								
								//==========================================
	                            // set volatility				
	void						setVolatility(std::vector<std::vector<AQLFunctionBase*> >& volatility);
								//==========================================
	                            // set Brownian Motion class
								/*!
									@param[in] pbm	pointer to Brownian motion
									@note this class is pointer owner of the Brownian motion
								*/	
	void						setBM(AQLRatesBM* pbm) {delete mpBM; mpBM = pbm;}
								//==========================================
	                            // set interpolation class
								/*!
									@param[in] pinter pointer to interpolation class
									@note this class is pointer owner of the interpolation class
								*/	
	void						setInterpolationMethod(AQLRatesPEInterpolationBase* pinter) {delete mpInter; mpInter = pinter;}
								//==========================================
								// set initial value
	void						setInitialValue (const AQLRatesPathElementBase& initial);
								//==========================================
	                            // set output time grid
								/*!
									@param[in] timegrid	time grid
									@note timegrid[0] is the initial time
								*/	
	void						setTimeGrid(const DoubleArray& timegrid) {mTimeGrid.clear(); mTimeGrid = timegrid;}
								//==========================================
	                            // set sde integral function							
	void						setIntegralFunction(AQLRatesSDEIntegralBase* pintegral);
								//==========================================
	                            // set templete of output path element	
	void						setOutputTemplate(const AQLRatesPathElementBase* pelement);
								//==========================================
								// set up this class for path calculation
	virtual	void				setUp();
								//==========================================
								//	set antithetic or not
								/*!
									@brief set antithetic or not
									@param[in] flag antithetic flag
									
									@note setBM method must be called before calling this method
								*/	
	void						setAntithetic(bool flag = true) {mpBM->setAntithetic(flag);}
								//==========================================
								// get initial path for set skew
	const AQLRatesPathElementBase*	getInitialValue() const {return mpInitial;};
								//==========================================
								// get Template for set skew
	const AQLRatesPathElementBase*  getTemplate() const {return mpTemplate;};

protected:
								//==========================================
	                            // calculate path
								/*!
							
									@param[in] pos position of sde integral grid				
								*/	
	virtual void				calcPath(unsigned int pos) = 0;
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;


	SDE_TYPE									mType;				// sde type
	unsigned int								mID;				// id of path
	AQLRatesNumeraireBase*							mpNumeraire;		// numeraire
	AQLRatesBM*										mpBM;				// brownian motion
	AQLRatesPEInterpolationBase*					mpInter;			// interpolation method
	DoubleArray									mTimeGrid;			// time grid
	std::vector<AQLRatesPathElementBase*>			mPath;				// path
	std::vector<AQLFunctionBase*>				mDrift;				// drift
	std::vector<std::vector<AQLFunctionBase*> >	mVolatility;		// volatility
	AQLRatesSDEIntegralBase*						mpIntegral;			// sde integral function
	const AQLRatesPathElementBase*					mpInitial;			// initial value
	const AQLRatesPathElementBase*					mpTemplate;			// templete of output path element
	unsigned int								mPos;				// position of current path element

};

