#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LACoreAutoPtr.h"
#include "LAMathAttrSDE.h"

//// DEFINES ////
// Entitiy id of LAMathIndexEntity
#define ENTITY_INDEX 11


#ifndef IR_MODEL_DATA_INDEXTYPE
#define IR_MODEL_DATA_INDEXTYPE				"IndexType"				//  data name of index type
#endif
#ifndef IR_MODEL_DATA_ACCESSORY
#define IR_MODEL_DATA_ACCESSORY				"Accessory"				//  data name of index accessory
#endif
#ifndef IR_MODEL_DATA_CURRENCY
#define IR_MODEL_DATA_CURRENCY				"Currency"				//  data name of currency
#endif
#ifndef IR_MODEL_DATA_FREQUENCY
#define IR_MODEL_DATA_FREQUENCY				"Frequency"				//  data name of frequency
#endif
#ifndef IR_MODEL_DATA_DAYCOUNT
#define IR_MODEL_DATA_DAYCOUNT				"DayCount"				//  data name of daycount 
#endif
#ifndef IR_MODEL_DATA_PATHENTITY
#define IR_MODEL_DATA_PATHENTITY				"PathEntity"			//  data name of path object
#endif
#ifndef IR_MODEL_DATA_FXRATE
#define IR_MODEL_DATA_FXRATE					"FXRate"				//  data name of fx object
#endif
#ifndef IR_MODEL_DATA_CACHESIZE
#define IR_MODEL_DATA_CACHESIZE				"CacheSize"				//  data name of cachesize
#endif
#ifndef PRICING_DATA_ROLLDAYFORINDEXGENERATE
#define PRICING_DATA_ROLLDAYFORINDEXGENERATE "RollDayForIndexGenerate"  //  data name of RollDayForIndexGenerate
#endif
#ifndef PRICING_DATA_BASISCURVE
#define PRICING_DATA_BASISCURVE             "BasisCurve"                 //  data name of Basis
#endif
#ifndef PRICING_DATA_BASISINTERPOLATION
#define PRICING_DATA_BASISINTERPOLATION     "BasisInterpolation"    //  data name of BasisInterpolation
#endif
#ifndef PRICING_DATA_ISFWDINTERPOLATION
#define PRICING_DATA_ISFWDINTERPOLATION      "IsFWDInterpolation"         //  data name of IsFWDInterpolation
#endif
#ifndef PRICING_DATA_FWDINTERPOLATION
#define PRICING_DATA_FWDINTERPOLATION        "FWDInterpolation"    //  data name of FWDInterpolation
#endif
#ifndef PRICING_DATA_DISCOUNTCURVE
#define PRICING_DATA_DISCOUNTCURVE           "DiscountCurve"                 //  data name of DiscountCurve
#endif
#ifndef PRICING_DATA_VOLATILITYUNDERLYING
#define PRICING_DATA_VOLATILITYUNDERLYING      "VolatilityUnderlying"                 //  data name of VolatilityUnderlying
#endif
#ifndef PRICING_DATA_CAMODEL
#define PRICING_DATA_CAMODEL      "ConvexityAdjustModel"                 //  data name of ConvexityAdjustModel
#endif
#ifndef PRICING_DATA_CATHRESHOLD
#define PRICING_DATA_CATHRESHOLD      "ConvexityAdjustThreshold"                 //  data name of ConvexityAdjustThreshold
#endif
#ifndef PRICING_DATA_CAINTEGRALCONDITION
#define PRICING_DATA_CAINTEGRALCONDITION      "ConvexityAdjustIntegralCondition"                 //  data name of ConvexityAdjustIntegralCondition
#endif
#ifndef PRICING_DATA_CAPREMIUMINTEGRALSTEP
#define PRICING_DATA_CAPREMIUMINTEGRALSTEP      "ConvexityAdjustPremiumIntegralStep"                 //  data name of ConvexityAdjustPremiumIntegralStep
#endif
#ifndef PRICING_DATA_CFDAYCOUNT
#define PRICING_DATA_CFDAYCOUNT			"CFDayCount"        // data name of CFDayCount
#endif
#ifndef PRICING_DATA_CFFREQUENCY
#define PRICING_DATA_CFFREQUENCY			"CFFrequency"        // data name of CFCalcEndDates
#endif
#ifndef PRICING_DATA_PAYMENTLAG
#define PRICING_DATA_PAYMENTLAG			"PaymentLag"        // data name of PaymentLag
#endif




class LADataInstance;
class LADate;
class LADataBool;
class LADataString;
class LADataStrings;
class LADataReference;
class LAPriceDataCalendar;
class LAPriceDataSlidingRule;
class LAPriceDataDayCount;
class LAPriceDataInterpolation;
class LAMathFXEntity;
class LAMathPathEntity;
class LAMathPlainVanillaEntity;
class LA1DDataSet;
class LAObjectPool;
class LAMathYieldCurve;
class LAMathYieldCurvePro;
/*! 
    @brief Class to represent index.
*/
class LAMathIndexEntity : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathIndexEntity(LADataInstance* dataInstance);
    // copy constructor
	LAMathIndexEntity(const LAMathIndexEntity& irse);
    // destructor
	virtual ~LAMathIndexEntity();

//  QUERY
   // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
	// get this index name
	const LADataString&	getName() const;
	// get this index name
	LADataString&		getName();
	// get index type
	const LADataString&	getIndexType() const;
	// get index type
	LADataString&		getIndexType();	
	// get index accessory
	const LADataString&	getAccessory() const;
	// get index accessory
	LADataString&		getAccessory();	
	// get index currency
	const LADataString&	getCurrency() const;
	// get index currency
	LADataString&		getCurrency();
	// get index daycount
	const LAPriceDataDayCount&	getDayCount() const;
	// get index daycount
	LAPriceDataDayCount&		getDayCount();
	// get index frequency
	const LADataString&	getFrequency() const;
	// get index frequency
	LADataString&		getFrequency();	
	// get index calendar
	const LAPriceDataCalendar&
						getCalendar() const;
	// get index calendar
	LAPriceDataCalendar&		getCalendar();		
	// get index sliding rule
	const LAPriceDataSlidingRule&
						getSlidingRule() const;
	// get index sliding rule
	LAPriceDataSlidingRule&	getSlidingRule();		
	// get path object
	const LADataReference&
						getPathEntity() const;
	// get path object
	LADataReference&
						getPathEntity();
	// get fx object
	const LADataReference&
						getFXEntity() const;
	// get fx object
	LADataReference&
						getFXEntity();
	// get basis
	const LADataString&
						getBasis() const;
	// get basis
	LADataString&
						getBasis();
	// get discount curve
	const LADataString&
						getDiscountCurve() const;
	// get discount curve
	LADataString&
						getDiscountCurve();
	// get basis interpolation
	const LAPriceDataInterpolation&
						getBasisInterpolation() const;
	// get basis interpolation
	LAPriceDataInterpolation&
						getBasisInterpolation();
	// get isfwdinter
	const LADataBool&
						getIsFWDInterpolation() const;
	// get isfwdinter
	LADataBool&
						getIsFWDInterpolation();
	// get fwd interpolation
	const LAPriceDataInterpolation&
						getFWDInterpolation() const;
	// get fwd interpolation
	LAPriceDataInterpolation&
						getFWDInterpolation();
	// get volatility underlying
	const LADataString&
						getVolUnderlying() const;
	// get volatility underlying
	LADataString&
						getVolUnderlying();

	// get time grid
	const DateVector&	getGrid() const {return mDateGrid;};
	// get index 
	/*!
		@return index array
	*/
	// get index
	const DoubleArray&	getIndex() const {return *mpIndexArray;};
	// get index
	const DoubleArray&	getTimeGrid() const {return mTimeGrid;};
	// getGridMat
	const DoubleMatrix& getGridMat() const {return mGridMat;};
	//for analytic
	// getDFMat
	const DoubleMatrix& getDFMat() const {return mDFMat;};
	// getTermMat
	const DoubleMatrix& getTermMat() const {return mTermMat;};
	// getYieldCurvePro
	//const LAMathYieldCurvePro& getYieldCurvePro (void) const;


	// set next index
	void				setNextIndex();
	// get cache size
	int					getCacheSize() const;	
	// set cache size
	void				setCacheSize(unsigned int size);	
	// add to time grid
	void				addGrid(const LADate& date);
	// add to time grids used for calculation of convexity adjustment
	void				addConvexityGrids(const LADate& fixingdate, const LADate& paymentdate, const LADate& enddate);
	// clear grid
	void				clearGrid(void);
	//	make copy(clone) of this index object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%

//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	// set up index for MC calculaion
	void				setUpforMC(void);
    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type = TYPE_NORMAL);
	// set up this class for index calculation
	virtual	void		setUpforPlainVanilla(void);
	// get spot lag
	void setSpotLag(const LAString &ccy, unsigned int lag);
	// set dates for indexgenerate
	void setDatesForIndexGenerates(const std::map<LADate, DateVector>& datesmap);
	// get convexity adjust 
	const DoubleArray& getConvexityAdjust() const {return mConvexityAdjust;};
	// get convexity adjust vol
	const DoubleArray& getConvexityAdjustVol() const {return mConvexityAdjustVol;};
	// calculate index
	virtual	void		calcIndexforPlainVanilla();	

	// is the cashflow value of this index in this instance convexity adjusted for delayed libor
	bool				isDelayedConvexityAdjusted() const;

	// is caModel the word that means convexity adjustment model for delayed libor
	static bool			isDelayedConvexityAdjustModel(const LAString& caModel);

protected:
	// copy index object	 
	virtual LAObject&	copy(const LAObject& e);
	// calculate index
	virtual	void		calcIndex();	
	// set up this class for index calculation
	virtual	void		setUp(void);
	// set up this class for ir index calculation
	void				setUpforIR(void);
	// set up this class for fx index calculation
	void				setUpforFX(void);
	//// set up this class for index calculation
	//virtual	void		setUpforPlainVanilla(void);
	// set up this class for ir index calculation
	void				setUpforIRforPlainVanilla(void);
	// set up this class for fx index calculation
	void				setUpforFXforPlainVanilla(void);


//private:
	// clear cash data
	void						clearCache(void);
	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);
	// calc SpreadMat
	void						calcSpreadMat(const LAMathYieldCurve &curve, const DoubleMatrix &gridMat);

	LADataHolder*				mpName;     // name (DATA_STRING)
	LADataHolder*				mpIndexType;// index type (DATA_STRING)
	LADataHolder*				mpAccessory;// index accessory (DATA_STRING)
	LADataHolder*				mpCurrency;	// currency (DATA_STRING)
	LADataHolder*				mpDC;		// daycount (DATA_DAYCOUNT)
	LADataHolder*				mpFrequency;// daycount (DATA_STRING)
	LADataHolder*				mpCalendar;	// calendar (DATA_CALENDAR)
	LADataHolder*				mpSlidingRule;// slidingrule (DATA_SLIDINGRULE)
	LADataHolder*				mpPathEntity;// path object(DATA_REFERENCE)
	LADataHolder*				mpFXEntity;// fx object(DATA_REFERENCE)
	LADataHolder*				mpCacheSize;// cache size
	LADataHolder*				mpDiscountCurve;    // discount curve (DATA_STRINGS)
	LADataHolder*				mpBasis;    // basis (DATA_STRINGS)
	LADataHolder*				mpBasisInter; // basis (DATA_INTERPOLATION)
	LADataHolder*				mpIsFWDInter;    // basis (DATA_BOOL)
	LADataHolder*				mpFWDInter; // basis (DATA_INTERPOLATION)
	LADataHolder*				mpVolUnderlying; // volatility underlying (DATA_STRING)


//	int							mCacheSize;	// cash size 
	DoubleMatrix				mCache;		// cash	
	DoubleMatrix				mAntiCache;	// cash for antithetic
	int							mCachePos;	// cash position
	int							mAntiCachePos;	// antithetic cash position
	int							mPos;		// path position number
	DateVector					mDateGrid;	// date grid
	DoubleArray					mTimeGrid;	// time grid
	DoubleArray					mIndexArray;// index array
	DoubleArray*				mpIndexArray;// pointer to index array
	bool						mIsAntithetic;	// antithetic flag
	int							mSDEPos;	// position number of sde correspond to this index
	LAMathPathEntity*				mpPath;		// pointer to path object
	LAMathFXEntity*				mpFX;		// pointer to fx object
	int							mIndexVersion;	// version
    
	SDEPATH_TYPE				mSDEType;// sde path type (IR,FX)
	// for ir
	DoubleMatrix				mGridMat;// tenor grid of each index observation date
	DoubleMatrix				mTermMat;// term grid of each index observation date
	DoubleMatrix				mFloatTermMat;// calculation period of floating side of each index observation date 
	DoubleMatrix				mSpreadMat;// spread grid of each index observation date
	DoubleMatrix				mSpreadMat_DF;// spread grid of each index observation date
	DoubleArray					mFwds;// forward rate grid
	DoubleArray					mFwdsGrid;// forward rate
	DoubleArray					mFwdsStartGrid;// forward rate startdate grid
	DateMatrix					mFwdsStartEndDate;// startdate and enddate for forward rate
	DoubleMatrix				mFwdsGridMat; // forward rate grid mat
	DoubleMatrix				mFwdsTermMat; // forward rate term mat
	DoubleArray					mFixingGrid;// fixingdate grid
	DoubleArray					mPaymentGrid;// paymentdate grid
	DoubleArray					mTimingTerm;// timing term
	bool                        mIsSameFwds; // forward rate same accessary
	bool                        mIsDaycountAdj; // flag for determining whether forward rate is adjusted by daycount
	LAString					mCurveType;

	//for analytic
	DoubleMatrix				mDFMat;// DF grid of each index observation date which can be used when analytic risk
	// for fx
	DoubleArray					mForwardTimes;// forward times
	LAString					mFromCurrency;// currency (from)
	LAString					mToCurrency;// currency (to)

	DateVector					mDateGrid_old;// date grid (last use) 
	LAMathPlainVanillaEntity*		mpVanilla; // pointer to plain vanilla object
	std::map<LAString, unsigned int> mSpotLag;// spot lag
	std::map<LADate, DateVector> mDatesForGenerate;// dates for indexgenerate

	double	getLIBORConvexityAdjust(double forward, double fixingterm, double timingterm, double dfEnd, double dfPayment, unsigned int curpos);
	double	getConvexityAdjust(double forward, double optionterm, unsigned int curpos);
	DoubleArray					mConvexityAdjust; // convexity adjust 
	DoubleArray					mConvexityAdjustVol; // convexity adjust volatility
	LAString					mConvexityAdjustModel; // convexity adjust model
	double						mConvexityAdjustThreshold; // convexity adjust thresholds
	DoubleArray					mCouponTerm; // calculation period of coupon
	DoubleArray					mConvexityAdjustIC; // integral conditons: (lower bound ofintegral range, upper bound ofintegral range, integral steps)
	int					        mConvexityAdjustPremIntegralStep; // convexity adjust premium integral step
};
