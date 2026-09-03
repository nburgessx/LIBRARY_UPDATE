#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceQuantAdjustmentFuncBase.h"

// ID for LAPriceQuantAdjustmentHWFXDD
#define FN_QUANTADJUSTMENTFXDD	30101 

class LAMathPathEntity;
class LAMathVolFuncFXDD;
class LARatesSpotSDE;
class LARatesNumeraireBase;
class LAPriceDriftHW; 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of quanto adjustment function class

*/
class LAPriceQuantAdjustmentHWFXDD : public LAPriceQuantAdjustmentFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceQuantAdjustmentHWFXDD(const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, const bool isAdjustInPath = false, double fxCriteria = 0.01, const double quantoCriteria = 10.0);
	//	Copy constructor
	LAPriceQuantAdjustmentHWFXDD(const LAPriceQuantAdjustmentHWFXDD& v);
	// Destructor
	virtual ~LAPriceQuantAdjustmentHWFXDD();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;

//	OPERATION
								//======================================
								// set up this class
								/*!
									@param[in] path path object 
								*/
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// get quanto adjustment
	virtual double 				getQuantAdjust(const double ts) const;
								//======================================
								// calc quant adjust
	virtual void				calcQuantAdjust(const double ts, const double te, const double r) const;
								//======================================
								// return quant adjust
	virtual double				getFXQuantAdjustNoFX(const double ts, const double te) const;

								//======================================
								// return quant adjust
	virtual double				getFXQuantAdjustFXMulti(const double ts, const double te) const;

private:

protected:
	virtual void				setUp() const;
	void                        searchTimeGridPos(const double t, unsigned int &pos)  const;

	AQLString					mSDEAttrNameIR_F;	// data name of ir(foreign currency) sde
	AQLString					mSDEAttrNameFX;	    // data name of fx sde
	bool                        mIsAdjustInPath;
	double						mFXCriteria;	     // fx criteria for avoid zero divide
	double                      mQuantoCriteria;    // quanto adjust  criteria for avoid zero divide
	LAPriceDriftHW*				mpDriftIR;		// drift function of foregin ir 
	LARatesNumeraireBase*	        mpNumeraireF;	// numerarire of foreign currency
	LARatesSpotSDE*				mpSDEFX;		// fx sde
	const LAMathVolFuncFXDD*		mpFxVolatility;	// fx volatility	
	mutable	unsigned int		mPos_old;		// last cache position
	mutable	bool				mIsSetUped;		// setuped or not

	DoubleArray					mCorrelation;	// correlation data
	mutable DoubleArray			mECache;		 // cache
	mutable DoubleArray			mBCache;		// cache
	mutable DoubleArray			mIRVolCorCache;	// cache
	mutable DoubleArray			mIRVolIntCorCache;	// cache
	mutable DoubleArray			mX1;		// cache
	mutable DoubleArray			mX2;		// cache
	mutable double			    mIRCache1;		// cache
	mutable double			    mIRCache2;		// cache
	mutable DoubleArray			mFXCache1;		// cache
	mutable DoubleArray			mFXCache2;		// cache
	mutable DoubleArray			mMinFX;			// minimum fx for avoid zero divide
	mutable DoubleArray			mIRQuantAdjustment;		// cache
	

};
