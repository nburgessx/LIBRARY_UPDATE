#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"
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




class AQLDataInstance;
class AQLDate;
class AQLDataBool;
class AQLDataString;
class AQLDataStrings;
class AQLDataReference;
class AQLPriceDataCalendar;
class AQLPriceDataSlidingRule;
class AQLPriceDataDayCount;
class AQLPriceDataInterpolation;
class LAMathFXEntity;
class LAMathPathEntity;
class LAMathPlainVanillaEntity;
class AQL1DDataSet;
class AQLObjectPool;
class LAMathYieldCurve;
class LAMathYieldCurvePro;
/*! 
    @brief Class to represent index.
*/
class LAMathIndexEntity : public AQLObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathIndexEntity(AQLDataInstance* dataInstance);
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
	const AQLDataString&	getName() const;
	// get this index name
	AQLDataString&		getName();
	// get index type
	const AQLDataString&	getIndexType() const;
	// get index type
	AQLDataString&		getIndexType();	
	// get index accessory
	const AQLDataString&	getAccessory() const;
	// get index accessory
	AQLDataString&		getAccessory();	
	// get index currency
	const AQLDataString&	getCurrency() const;
	// get index currency
	AQLDataString&		getCurrency();
	// get index daycount
	const AQLPriceDataDayCount&	getDayCount() const;
	// get index daycount
	AQLPriceDataDayCount&		getDayCount();
	// get index frequency
	const AQLDataString&	getFrequency() const;
	// get index frequency
	AQLDataString&		getFrequency();	
	// get index calendar
	const AQLPriceDataCalendar&
						getCalendar() const;
	// get index calendar
	AQLPriceDataCalendar&		getCalendar();		
	// get index sliding rule
	const AQLPriceDataSlidingRule&
						getSlidingRule() const;
	// get index sliding rule
	AQLPriceDataSlidingRule&	getSlidingRule();		
	// get path object
	const AQLDataReference&
						getPathEntity() const;
	// get path object
	AQLDataReference&
						getPathEntity();
	// get fx object
	const AQLDataReference&
						getFXEntity() const;
	// get fx object
	AQLDataReference&
						getFXEntity();
	// get basis
	const AQLDataString&
						getBasis() const;
	// get basis
	AQLDataString&
						getBasis();
	// get discount curve
	const AQLDataString&
						getDiscountCurve() const;
	// get discount curve
	AQLDataString&
						getDiscountCurve();
	// get basis interpolation
	const AQLPriceDataInterpolation&
						getBasisInterpolation() const;
	// get basis interpolation
	AQLPriceDataInterpolation&
						getBasisInterpolation();
	// get isfwdinter
	const AQLDataBool&
						getIsFWDInterpolation() const;
	// get isfwdinter
	AQLDataBool&
						getIsFWDInterpolation();
	// get fwd interpolation
	const AQLPriceDataInterpolation&
						getFWDInterpolation() const;
	// get fwd interpolation
	AQLPriceDataInterpolation&
						getFWDInterpolation();
	// get volatility underlying
	const AQLDataString&
						getVolUnderlying() const;
	// get volatility underlying
	AQLDataString&
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
	void				addGrid(const AQLDate& date);
	// add to time grids used for calculation of convexity adjustment
	void				addConvexityGrids(const AQLDate& fixingdate, const AQLDate& paymentdate, const AQLDate& enddate);
	// clear grid
	void				clearGrid(void);
	//	make copy(clone) of this index object object.
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%

//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	// set up index for MC calculaion
	void				setUpforMC(void);
    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type = TYPE_NORMAL);
	// set up this class for index calculation
	virtual	void		setUpforPlainVanilla(void);
	// get spot lag
	void setSpotLag(const AQLString &ccy, unsigned int lag);
	// set dates for indexgenerate
	void setDatesForIndexGenerates(const std::map<AQLDate, DateVector>& datesmap);
	// get convexity adjust 
	const DoubleArray& getConvexityAdjust() const {return mConvexityAdjust;};
	// get convexity adjust vol
	const DoubleArray& getConvexityAdjustVol() const {return mConvexityAdjustVol;};
	// calculate index
	virtual	void		calcIndexforPlainVanilla();	

	// is the cashflow value of this index in this instance convexity adjusted for delayed libor
	bool				isDelayedConvexityAdjusted() const;

	// is caModel the word that means convexity adjustment model for delayed libor
	static bool			isDelayedConvexityAdjustModel(const AQLString& caModel);

protected:
	// copy index object	 
	virtual AQLObject&	copy(const AQLObject& e);
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
	AQLDataHolder&				add(const AQLString& name);
	// calc SpreadMat
	void						calcSpreadMat(const LAMathYieldCurve &curve, const DoubleMatrix &gridMat);

	AQLDataHolder*				mpName;     // name (DATA_STRING)
	AQLDataHolder*				mpIndexType;// index type (DATA_STRING)
	AQLDataHolder*				mpAccessory;// index accessory (DATA_STRING)
	AQLDataHolder*				mpCurrency;	// currency (DATA_STRING)
	AQLDataHolder*				mpDC;		// daycount (DATA_DAYCOUNT)
	AQLDataHolder*				mpFrequency;// daycount (DATA_STRING)
	AQLDataHolder*				mpCalendar;	// calendar (DATA_CALENDAR)
	AQLDataHolder*				mpSlidingRule;// slidingrule (DATA_SLIDINGRULE)
	AQLDataHolder*				mpPathEntity;// path object(DATA_REFERENCE)
	AQLDataHolder*				mpFXEntity;// fx object(DATA_REFERENCE)
	AQLDataHolder*				mpCacheSize;// cache size
	AQLDataHolder*				mpDiscountCurve;    // discount curve (DATA_STRINGS)
	AQLDataHolder*				mpBasis;    // basis (DATA_STRINGS)
	AQLDataHolder*				mpBasisInter; // basis (DATA_INTERPOLATION)
	AQLDataHolder*				mpIsFWDInter;    // basis (DATA_BOOL)
	AQLDataHolder*				mpFWDInter; // basis (DATA_INTERPOLATION)
	AQLDataHolder*				mpVolUnderlying; // volatility underlying (DATA_STRING)


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
	AQLString					mCurveType;

	//for analytic
	DoubleMatrix				mDFMat;// DF grid of each index observation date which can be used when analytic risk
	// for fx
	DoubleArray					mForwardTimes;// forward times
	AQLString					mFromCurrency;// currency (from)
	AQLString					mToCurrency;// currency (to)

	DateVector					mDateGrid_old;// date grid (last use) 
	LAMathPlainVanillaEntity*		mpVanilla; // pointer to plain vanilla object
	std::map<AQLString, unsigned int> mSpotLag;// spot lag
	std::map<AQLDate, DateVector> mDatesForGenerate;// dates for indexgenerate

	double	getLIBORConvexityAdjust(double forward, double fixingterm, double timingterm, double dfEnd, double dfPayment, unsigned int curpos);
	double	getConvexityAdjust(double forward, double optionterm, unsigned int curpos);
	DoubleArray					mConvexityAdjust; // convexity adjust 
	DoubleArray					mConvexityAdjustVol; // convexity adjust volatility
	AQLString					mConvexityAdjustModel; // convexity adjust model
	double						mConvexityAdjustThreshold; // convexity adjust thresholds
	DoubleArray					mCouponTerm; // calculation period of coupon
	DoubleArray					mConvexityAdjustIC; // integral conditons: (lower bound ofintegral range, upper bound ofintegral range, integral steps)
	int					        mConvexityAdjustPremIntegralStep; // convexity adjust premium integral step
};
