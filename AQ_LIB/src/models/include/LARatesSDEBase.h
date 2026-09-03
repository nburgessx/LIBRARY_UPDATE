#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LARatesBM.h"
#include "LARatesNumeraireBase.h"
#include "LARatesPEInterpolationBase.h"

// ID for LARatesSDEBase
#define FN_SDEBASE	1701 

class LARatesPathElementBase;
class LARatesSDEIntegralBase;

// sde type
enum SDE_TYPE {	
	dX,
	DIVIDEdXbyX
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde class

*/
class LARatesSDEBase : public LACoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesSDEBase(SDE_TYPE type);
	//	Copy constructor
	LARatesSDEBase(const LARatesSDEBase& v);
	// Destructor
	virtual ~LARatesSDEBase();
//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
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
	virtual LARatesNumeraireBase*	getNumeraire(void) {return mpNumeraire;}
								//==========================================
	                            // get numeraire
								/*!
									@return numeraire
								*/		
	virtual const LARatesNumeraireBase*
								getNumeraire(void) const {return mpNumeraire;}
								//==========================================
	                            // get interpolation class
								/*!
									@return interpolation class
								*/		
	LARatesPEInterpolationBase*
								getInterpolationMethod(void) const {return mpInter;}
								//==========================================
	                            // get integralfunction class
								/*!
									@return integralfunction class
								*/		
	LARatesSDEIntegralBase*
								getIntegralFunction(void) const {return mpIntegral;}

								//==========================================
	                            // get drift
								/*!
									@return drift
								*/		
	const std::vector<LAFunctionBase*>&	
								getDrift(void) const {return mDrift;}
								//==========================================
	                            // get volatility
								/*!
									@return volatility
								*/	
	const std::vector<std::vector<LAFunctionBase*> >& 
								getVolatility(void) const {return mVolatility;}
								//==========================================
	                            // get Brownian Motion class
								/*!
									@return Brownian Motion class
								*/	
	LARatesBM*		 				getBM() const {return mpBM;}
								//==========================================
	                            // get time grid
								/*!
									@return time grid
								*/	
	const DoubleArray&			getTimeGrid(void) const {return mTimeGrid;}		
    							//==========================================
	                            // get path
	virtual const	std::vector<LARatesPathElementBase*>&
								getPath(void);
   								//==========================================
							    // get path element
	virtual const LARatesPathElementBase*
								getPathElement(unsigned int pos) = 0;
 								//==========================================
								// return string representaion
//    virtual LAString			convertToString(void) const;
//	OPERATION
								//==========================================
								// transform from string representaion
//     virtual void				convertFromString(const LAString& str);
								//==========================================
	                            // set numeraire		
	virtual void				setNumeraire(LARatesNumeraireBase* pnumeraire);
								//==========================================
	                            // set drift
	void						setDrift(std::vector<LAFunctionBase*>& drift);								
								//==========================================
	                            // set volatility				
	void						setVolatility(std::vector<std::vector<LAFunctionBase*> >& volatility);
								//==========================================
	                            // set Brownian Motion class
								/*!
									@param[in] pbm	pointer to Brownian motion
									@note this class is pointer owner of the Brownian motion
								*/	
	void						setBM(LARatesBM* pbm) {delete mpBM; mpBM = pbm;}
								//==========================================
	                            // set interpolation class
								/*!
									@param[in] pinter pointer to interpolation class
									@note this class is pointer owner of the interpolation class
								*/	
	void						setInterpolationMethod(LARatesPEInterpolationBase* pinter) {delete mpInter; mpInter = pinter;}
								//==========================================
								// set initial value
	void						setInitialValue (const LARatesPathElementBase& initial);
								//==========================================
	                            // set output time grid
								/*!
									@param[in] timegrid	time grid
									@note timegrid[0] is the initial time
								*/	
	void						setTimeGrid(const DoubleArray& timegrid) {mTimeGrid.clear(); mTimeGrid = timegrid;}
								//==========================================
	                            // set sde integral function							
	void						setIntegralFunction(LARatesSDEIntegralBase* pintegral);
								//==========================================
	                            // set templete of output path element	
	void						setOutputTemplate(const LARatesPathElementBase* pelement);
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
	const LARatesPathElementBase*	getInitialValue() const {return mpInitial;};
								//==========================================
								// get Template for set skew
	const LARatesPathElementBase*  getTemplate() const {return mpTemplate;};

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
	LARatesNumeraireBase*							mpNumeraire;		// numeraire
	LARatesBM*										mpBM;				// brownian motion
	LARatesPEInterpolationBase*					mpInter;			// interpolation method
	DoubleArray									mTimeGrid;			// time grid
	std::vector<LARatesPathElementBase*>			mPath;				// path
	std::vector<LAFunctionBase*>				mDrift;				// drift
	std::vector<std::vector<LAFunctionBase*> >	mVolatility;		// volatility
	LARatesSDEIntegralBase*						mpIntegral;			// sde integral function
	const LARatesPathElementBase*					mpInitial;			// initial value
	const LARatesPathElementBase*					mpTemplate;			// templete of output path element
	unsigned int								mPos;				// position of current path element

};

