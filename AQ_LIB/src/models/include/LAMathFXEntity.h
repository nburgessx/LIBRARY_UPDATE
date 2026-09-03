#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LACoreAutoPtr.h"
#include "LAPriceDataDayCount.h"

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


class LADataInstance;
class LADate;
class LADataDoubles;
class LADataDouble;
class LADataDate;
class LAString;
class LADataString;
class LADataStrings;
class LADataReference;
class LADataMultiReference;

/*! 
    @brief Class to represent FX.
*/
class LAMathFXEntity : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathFXEntity(LADataInstance* dataInstance);
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
	const LADataString&	getName() const;
	// get this FX name. The setting of name is also possible. 
	LADataString&		getName();
    // get basedate
	const LADataDate&	getAsOfDate(void) const;
    // get basedate. The setting of basedate is also possible. 
	LADataDate&			getAsOfDate(void);
    // get fx type
	const LADataString&	getFXType(void) const;
    // get fx type. The setting of fx type is also possible. 
	LADataString&		getFXType(void);
	// get path object
	const LADataReference&
						getPathEntity() const;
	// get path object. The setting of path object is also possible. 
	LADataReference&
						getPathEntity();	
	// get yield entities
	const LADataMultiReference&
						getYieldCurves() const;
	// get yiled entities. The setting of yiled entities is also possible. 
	LADataMultiReference&
						getYieldCurves();
	// get currency names
	const LADataStrings&
						getCurrencys() const;
	// get currency names. The setting of get currency names is also possible. 
	LADataStrings&		getCurrencys();		
	// get spot rates
	const LADataDoubles&
						getSpotRates() const;
	// get spot rates. The setting of spot rates is also possible. 
	LADataDoubles&		getSpotRates();	
	// get calendar names
	const LADataStrings&
						getCalendarNames() const;
	// get calendar names. The setting of calendar names is also possible. 
	LADataStrings&		getCalendarNames();
	// set spot lag
	void setSpotLag(const LAString &ccy, unsigned int lag);
	// get spot lag
	unsigned int getSpotLag(const LAString &fx);

	// get fx rate (today base)
	double				getRate(const LAString& from, const LAString& to,
								const LADate& date) const;
	// get fx rate (today base)
	double				getRate(const LAString& from, const LAString& to,
								double t, DayCount dc = ACT_365_ISDA) const;	
	// get fx rate (spot base)
//	double				getSpotRate(const LAString& from, const LAString& to,
//								const LADate& date) const;
	// get fx rate (spot base)
//	double				getSpotRate(const LAString& from, const LAString& to,
//								double t, DayCount dc = ACT_365_ISDA) const;
	// get forward fx rate 
	double				getForwardRate(const LAString& from, const LAString& to,
								const LADate& date, const LADate& forward_date) const;
	// get forward fx rate
	double				getForwardRate(const LAString& from, const LAString& to,
								double t, double forward_t, DayCount dc = ACT_365_ISDA) const;
	// get spot date
	LADate				getSpotDate(const LAString& cur1, const LAString& cur2,
									const LADate& basedate) const;
	// get forward	
//	LADate				getForwardDate(const LAString& cur1, const LAString& cur2,
//									const LADate& basedate, const LAString& term) const;
	
	//	make copy(clone) of this FX object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data.If there is not Data to remove, do nothing. If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type = TYPE_NORMAL);

protected:
	// copy FX object	 
	virtual LAObject&	copy(const LAObject& e);
	

private:
	// set up this class
	void				setUp(void) const;	
	// calulate today rates from spot rates
	void				calcTodayRates() const;


	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);

	LADataHolder*				mpName;     // name (DATA_STRING)
	LADataHolder*               mpAsOfDate; // basedate (DATA_DATE)
	LADataHolder*               mpFXType;	// fx type (DATA_STRING)
	LADataHolder*				mpPathEntity;// path object (DATA_REFERENCE)
	LADataHolder*				mpYieldCurves;// yield object (DATA_MULTIREFERENCE)
	LADataHolder*				mpCurrencys;// currency name (DATA_STRINGS)
	LADataHolder*				mpSpotRates;// rates (DATA_DOUBLES) 
	LADataHolder*				mpCalendarNames;// calendar names (DATA_STRINGS)  
	std::map<LAString, unsigned int> mSpotLag;// spot lag 
	// variables for performance up
	mutable int									mFXVersion;		// version
	mutable std::map<LAString, unsigned int>	mCurrencyPosMap;// position of each currency
	mutable int									mFXType;// fx type
	mutable DoubleArray							mTodayRates;// today rates
	mutable std::vector<std::vector<std::vector<std::pair<unsigned int, bool> > > >
												mFXSDEPosInfo;// positions of fx sdes  
	mutable IntArray							mIRSDEPosInfo;// positions of ir sde
	mutable DayCount							mDCPath;// daycount of path object


};
