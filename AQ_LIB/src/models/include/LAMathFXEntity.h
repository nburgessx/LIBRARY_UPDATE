#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"
#include "AQLPriceDataDayCount.h"

//// DEFINES ////
// Entitiy id of LAMathFXEntity
#define ENTITY_FX 13

#ifndef IR_MODEL_DATA_FXTYPE
#define IR_MODEL_DATA_FXTYPE			"FXType"		//  data name of fx type  
#endif
#ifndef IR_MODEL_DATA_PATHENTITY
#define IR_MODEL_DATA_PATHENTITY		"PathEntity"	//  data name of path object
#endif
#ifndef IR_MODEL_DATA_YIELDCURVES
#define IR_MODEL_DATA_YIELDCURVES	"YieldCurves"	//  data name of yield curves
#endif
#ifndef IR_MODEL_DATA_CURRENCYS
#define IR_MODEL_DATA_CURRENCYS		"Currencys"		//  data name of currency names
#endif
#ifndef IR_MODEL_DATA_SPOTRATES
#define IR_MODEL_DATA_SPOTRATES		"SpotRates"		//  data name of spot rates
#endif
#ifndef IR_MODEL_DATA_CALENDARNAMES
#define IR_MODEL_DATA_CALENDARNAMES	"CalendarNames" //  data name of calendar names
#endif
#ifndef IR_MODEL_DATA_SPOTLAG
#define IR_MODEL_DATA_SPOTLAG		"SpotLag"		//  data name of spot lag
#endif


class AQLDataInstance;
class AQLDate;
class AQLDataDoubles;
class AQLDataDouble;
class AQLDataDate;
class AQLString;
class AQLDataString;
class AQLDataStrings;
class AQLDataReference;
class AQLDataMultiReference;

/*! 
    @brief Class to represent FX.
*/
class LAMathFXEntity : public AQLObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathFXEntity(AQLDataInstance* dataInstance);
    // copy constructor
	LAMathFXEntity(const LAMathFXEntity& fx);
    // destructor
	virtual ~LAMathFXEntity();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
	// get this FX name
	const AQLDataString&	getName() const;
	// get this FX name. The setting of name is also possible. 
	AQLDataString&		getName();
    // get basedate
	const AQLDataDate&	getAsOfDate(void) const;
    // get basedate. The setting of basedate is also possible. 
	AQLDataDate&			getAsOfDate(void);
    // get fx type
	const AQLDataString&	getFXType(void) const;
    // get fx type. The setting of fx type is also possible. 
	AQLDataString&		getFXType(void);
	// get path object
	const AQLDataReference&
						getPathEntity() const;
	// get path object. The setting of path object is also possible. 
	AQLDataReference&
						getPathEntity();	
	// get yield entities
	const AQLDataMultiReference&
						getYieldCurves() const;
	// get yiled entities. The setting of yiled entities is also possible. 
	AQLDataMultiReference&
						getYieldCurves();
	// get currency names
	const AQLDataStrings&
						getCurrencys() const;
	// get currency names. The setting of get currency names is also possible. 
	AQLDataStrings&		getCurrencys();		
	// get spot rates
	const AQLDataDoubles&
						getSpotRates() const;
	// get spot rates. The setting of spot rates is also possible. 
	AQLDataDoubles&		getSpotRates();	
	// get calendar names
	const AQLDataStrings&
						getCalendarNames() const;
	// get calendar names. The setting of calendar names is also possible. 
	AQLDataStrings&		getCalendarNames();
	// set spot lag
	void setSpotLag(const AQLString &ccy, unsigned int lag);
	// get spot lag
	unsigned int getSpotLag(const AQLString &fx);

	// get fx rate (today base)
	double				getRate(const AQLString& from, const AQLString& to,
								const AQLDate& date) const;
	// get fx rate (today base)
	double				getRate(const AQLString& from, const AQLString& to,
								double t, DayCount dc = ACT_365_ISDA) const;	
	// get fx rate (spot base)
//	double				getSpotRate(const AQLString& from, const AQLString& to,
//								const AQLDate& date) const;
	// get fx rate (spot base)
//	double				getSpotRate(const AQLString& from, const AQLString& to,
//								double t, DayCount dc = ACT_365_ISDA) const;
	// get forward fx rate 
	double				getForwardRate(const AQLString& from, const AQLString& to,
								const AQLDate& date, const AQLDate& forward_date) const;
	// get forward fx rate
	double				getForwardRate(const AQLString& from, const AQLString& to,
								double t, double forward_t, DayCount dc = ACT_365_ISDA) const;
	// get spot date
	AQLDate				getSpotDate(const AQLString& cur1, const AQLString& cur2,
									const AQLDate& basedate) const;
	// get forward	
//	AQLDate				getForwardDate(const AQLString& cur1, const AQLString& cur2,
//									const AQLDate& basedate, const AQLString& term) const;
	
	//	make copy(clone) of this FX object object.
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing. If member variable is specified to remove, do not remove it.
	virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type = TYPE_NORMAL);

protected:
	// copy FX object	 
	virtual AQLObject&	copy(const AQLObject& e);
	

private:
	// set up this class
	void				setUp(void) const;	
	// calulate today rates from spot rates
	void				calcTodayRates() const;


	// set Data specified by the name.
	AQLDataHolder&				add(const AQLString& name);

	AQLDataHolder*				mpName;     // name (DATA_STRING)
	AQLDataHolder*               mpAsOfDate; // basedate (DATA_DATE)
	AQLDataHolder*               mpFXType;	// fx type (DATA_STRING)
	AQLDataHolder*				mpPathEntity;// path object (DATA_REFERENCE)
	AQLDataHolder*				mpYieldCurves;// yield object (DATA_MULTIREFERENCE)
	AQLDataHolder*				mpCurrencys;// currency name (DATA_STRINGS)
	AQLDataHolder*				mpSpotRates;// rates (DATA_DOUBLES) 
	AQLDataHolder*				mpCalendarNames;// calendar names (DATA_STRINGS)  
	std::map<AQLString, unsigned int> mSpotLag;// spot lag 
	// variables for performance up
	mutable int									mFXVersion;		// version
	mutable std::map<AQLString, unsigned int>	mCurrencyPosMap;// position of each currency
	mutable int									mFXType;// fx type
	mutable DoubleArray							mTodayRates;// today rates
	mutable std::vector<std::vector<std::vector<std::pair<unsigned int, bool> > > >
												mFXSDEPosInfo;// positions of fx sdes  
	mutable IntArray							mIRSDEPosInfo;// positions of ir sde
	mutable DayCount							mDCPath;// daycount of path object


};
