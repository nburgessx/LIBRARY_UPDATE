// AQLPriceDataDayCount.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLDate.h"
#include "CoreEnumerations.h"


// Data ID of AQLMathDaycount
#define DATA_DAYCOUNT 60

//  DayCount-rule
enum DayCount
{
	ACT_ACT,		    // Actual/Actual
    ACT_365,		    // Actual/365(Fixed)
    ACT_360,		    // Actual/360
    N30_360,		    // 30/360
	E30_360,		    // 30E/360
	ACT_365_ISDA,       // Actual/365(ISDA RULE)
	ACT_365_FJ,		    // Actual/365FJ
	DAYCOUNT_ONE,       // 1/1
	DAYCOUNT_HALF,		// 1/2
	DAYCOUNT_QUARTER,   // 1/4
	ACT_ACT_ICMA		// Actual/Actual(ICMA RULE)
};

/*! 
    @brief  Data-CLASS(DayCount rule)
*/
class AQLPriceDataDayCount : public AQLPriceDataType
{
public:

	// default_constructor 
	explicit AQLPriceDataDayCount(const staticData_t attr = DATA_DAYCOUNT);
	
	// copy_constructor
    AQLPriceDataDayCount(const AQLPriceDataDayCount& attr);
	
	// constructor
	AQLPriceDataDayCount(const DayCount dc);
	
	// destructor
    virtual ~AQLPriceDataDayCount(void);

    // Copy Object of this Data
    virtual AQLPriceDataType*    clone() const; 


	// Return DayCount-rule by AQLString
    virtual AQLString        convertToString(void) const;

    // Return Term between fromDate to toDate by year-count
	double	                getTerm(const AQLDate& fromDate,	const AQLDate& toDate, bool includelast = true) const;

    // Return Term(day-count) by year-count
	double	                getTerm(const AQLDate& fromDate,	const double& termD, bool includelast = true) const;

    // Return Term(year-count) by day-count
	double	                getDayTerm(const AQLDate& fromDate,	const double& termY, bool includelast = true) const;
    
	// Return Numerator for the specified Daycount
    int                     getNumerator(const AQLDate& fromDate, const AQLDate& toDate, bool includelast= true) const; 
    
	// Return Denominator for the specified Daycount
    int                     getDenominator() const;
    
	// Get DayCount-rule by DayCount
    DayCount                getDayCount(void) const { return mDayCount; }
	
	// Static Method to get etrading DayCountEnum
	static etrading::DayCountEnum dayCountEnum( const DayCount & daycount );
	
	// Get etrading DayCountEnum
	etrading::DayCountEnum dayCountEnum() const;

	// Get Coupon Start Dates
	std::vector<AQLDate>		getCouponStartDates(void) const {return mCouponStartDates;}
	
	// Get Coupon End Dates
	std::vector<AQLDate>		getCouponEndDates(void) const {return mCouponEndDates;}

    // Set Data(DayCount-rule) from AQLString
    virtual void            convertFromString(const AQLString& str);

    // Set DayCount-rule
	void                    setDayCount(const DayCount dc);
    
	// Set DayCount-rule
	void                    setDayCount(const AQLString& dc);
	
	// Set CouponStartDates (for AC_AC_ICMA)
	void					setCouponStartDates(std::vector<AQLDate>& dates) {mCouponStartDates = dates;}
	
	// Set CouponEndDates (for AC_AC_ICMA)
	void					setCouponEndDates(std::vector<AQLDate>& dates) {mCouponEndDates = dates;}
	
	// Set the number of coupons in one year (for AC_AC_ICMA)
	void					setCouponsInYear(const int& n) {mCouponsPerYear = n;}


protected:

    // Compare Objects(DayCount-rule)
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // Compare Objects(DayCount-rule)
	virtual int				compare(const AQLPriceDataType& a) const;


private:
	
	DayCount				mDayCount;			// DayCount-rule
	std::vector<AQLDate>		mCouponStartDates;  // for AC_AC_ICMA
	std::vector<AQLDate>		mCouponEndDates;    // for AC_AC_ICMA
	unsigned int			mCouponsPerYear;    // for AC_AC_ICMA	
};


