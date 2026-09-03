// LAPriceDataDayCount.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LADate.h"
#include "CoreEnumerations.h"


// Data ID of LAMathDaycount
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
class LAPriceDataDayCount : public LAPriceDataType
{
public:

	// default_constructor 
	explicit LAPriceDataDayCount(const staticData_t attr = DATA_DAYCOUNT);
	
	// copy_constructor
    LAPriceDataDayCount(const LAPriceDataDayCount& attr);
	
	// constructor
	LAPriceDataDayCount(const DayCount dc);
	
	// destructor
    virtual ~LAPriceDataDayCount(void);

    // Copy Object of this Data
    virtual LAPriceDataType*    clone() const; 


	// Return DayCount-rule by LAString
    virtual LAString        convertToString(void) const;

    // Return Term between fromDate to toDate by year-count
	double	                getTerm(const LADate& fromDate,	const LADate& toDate, bool includelast = true) const;

    // Return Term(day-count) by year-count
	double	                getTerm(const LADate& fromDate,	const double& termD, bool includelast = true) const;

    // Return Term(year-count) by day-count
	double	                getDayTerm(const LADate& fromDate,	const double& termY, bool includelast = true) const;
    
	// Return Numerator for the specified Daycount
    int                     getNumerator(const LADate& fromDate, const LADate& toDate, bool includelast= true) const; 
    
	// Return Denominator for the specified Daycount
    int                     getDenominator() const;
    
	// Get DayCount-rule by DayCount
    DayCount                getDayCount(void) const { return mDayCount; }
	
	// Static Method to get etrading DayCountEnum
	static etrading::DayCountEnum dayCountEnum( const DayCount & daycount );
	
	// Get etrading DayCountEnum
	etrading::DayCountEnum dayCountEnum() const;

	// Get Coupon Start Dates
	std::vector<LADate>		getCouponStartDates(void) const {return mCouponStartDates;}
	
	// Get Coupon End Dates
	std::vector<LADate>		getCouponEndDates(void) const {return mCouponEndDates;}

    // Set Data(DayCount-rule) from LAString
    virtual void            convertFromString(const LAString& str);

    // Set DayCount-rule
	void                    setDayCount(const DayCount dc);
    
	// Set DayCount-rule
	void                    setDayCount(const LAString& dc);
	
	// Set CouponStartDates (for AC_AC_ICMA)
	void					setCouponStartDates(std::vector<LADate>& dates) {mCouponStartDates = dates;}
	
	// Set CouponEndDates (for AC_AC_ICMA)
	void					setCouponEndDates(std::vector<LADate>& dates) {mCouponEndDates = dates;}
	
	// Set the number of coupons in one year (for AC_AC_ICMA)
	void					setCouponsInYear(const int& n) {mCouponsPerYear = n;}


protected:

    // Compare Objects(DayCount-rule)
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // Compare Objects(DayCount-rule)
	virtual int				compare(const LAPriceDataType& a) const;


private:
	
	DayCount				mDayCount;			// DayCount-rule
	std::vector<LADate>		mCouponStartDates;  // for AC_AC_ICMA
	std::vector<LADate>		mCouponEndDates;    // for AC_AC_ICMA
	unsigned int			mCouponsPerYear;    // for AC_AC_ICMA	
};


