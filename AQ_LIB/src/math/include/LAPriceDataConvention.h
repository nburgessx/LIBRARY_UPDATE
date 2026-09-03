#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LADate.h"
#include "LAPriceDataDayCount.h"

// LAPriceDataConvention Data ID
#define DATA_CONVENTION 42

// RateConvention
enum RateConvention
{
	CONT,  // Continuous Compound
    MNT,   // Monthly Compound
    QTR,   // Quarterly Compound
    SA,    // Semi-annual Compound
    ANN,   // Annual Compounded
    SIMPL  // Simple Interest Rate 
};


/*! 
    @brief Data class to represent the data (type rate convention).
*/
class LAPriceDataConvention : public LAPriceDataType
{
public:
//	LIFECYCLE
	// default constructor
	LAPriceDataConvention(void);
	// copy constructor
    LAPriceDataConvention(const LAPriceDataConvention& attr);
	// constructor
	LAPriceDataConvention(const DayCount dc, const RateConvention rc);
	// destructor
    virtual ~LAPriceDataConvention(void);

//  QUERY
    // deep copy of the data object
    virtual LAPriceDataType*    clone() const; 

    // return (interest rate calculation method) in the string representation
    virtual LAString        convertToString(void) const;

    // get terms in years between fromDate and toDate
	double	                getTerm(const LADate& fromDate,	const LADate& toDate) const;

    // get terms in years (number of days per period) from the start date
	double	                getTerm(const LADate& fromDate,	const double& termD) const;

    // get terms in days (number of years per period) from the start date
	double	                getDayTerm(const LADate& fromDate,	const double& termY) const;
    
	// return discount factor based on the period and interest rate (annual interest)
	double					getDF(const double& rate, 
									const LADate& fromDate,
									const LADate& toDate) const;
    // return discount factor based on the period and interest rate (annual interest)
	double					getDF(const double& rate, const LADate& fromDate,
											const double& term) const;												

    // return yield to maturity based on the period and interest rate (annual interest)
	double					getCompound(const double& rate, 
									const LADate& fromDate,
									const LADate& toDate) const;
    // return yield to maturity based on the period and interest rate (annual interest)
	double					getCompound(const double& rate,
									const LADate& fromDate, 
									const double& term) const;

    // return interest rate (annual interest) based on the period and discount factor
	double					getRate(const double& df, const LADate& fromDate,
									const LADate& toDate) const;
    // return interest rate (annual interest) based on the period and discount factor
	double					getRate(const double& df, 
									const LADate& fromDate,
									const double& term) const;

    // convert interest rate
	double					convertTo(const double& rate, 
									const LADate& fromDate,
									const LADate& toDate,
									const LAPriceDataConvention& cv) const;

    // get rate convention that the class holds
    /* 
        @return rate convention that the class holds
    */
	RateConvention          getConvention(void) const { return mRateConvention; }

    // get day count convention that the class holds
    /* 
        @return day count convention that the class holds
    */
    DayCount                getDayCount(void) const { return mDayCount.getDayCount(); }

    // calculate the rate of return from the period and interest rate (annual interest) based on the rate convention
	static double			rateToRet(const double& rate, 
								const double& term,
								const LAPriceDataConvention& conv);

    // calculate interest rate (annual interest) from the period and the rate of return based on the rate convention
	static double			retToRate(const double& ret, 
								const double& term,
								const LAPriceDataConvention& conv);


//  OPERATION
    // set the rate convention Data from the specified string
    virtual void            convertFromString(const LAString& str);

    // set the interest calculation rules
    void                    setConvention(const RateConvention rc);
    // set the interest calculation rules
	void                    setConvention(const LAString& rc);

    // set day count convention
	void                    setDayCount(const DayCount dc);
    // set day count convention
	void                    setDayCount(const LAString& dc);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int				compare(const LAPriceDataType& a) const;
private:
//	DayCount             mDayCount;       // day count convention
	RateConvention       mRateConvention; // rate convention
	LAPriceDataDayCount		 mDayCount;		  // day count convention	
};

