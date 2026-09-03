#ifndef LAMathCashFlowSchedules_h
#define LAMathCashFlowSchedules_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LAString.h"
#include "LADate.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include <vector>

using namespace std;

struct CashFlowTiming
{
    LADate fixing;
    LADate start;
    LADate end;
    LADate payment;
    LADate indexSettlement;
    double accrual;
};

class LAMathScheduleUtility
{
public:
//================ Single Flow ===================================
static CashFlowTiming CashFlowSchedule(LADate valDate, LAString mtyTerm, LAStringMatrix legScheduler, LAStringMatrix indexScheduler);

//================ Multiple Flows ===================================
static vector<CashFlowTiming> LegSchedule(LADate valDate, LAString mtyTerm, LAStringMatrix legScheduler, LAStringMatrix indexScheduler);

static DateVector BaseDates(LAString frequency, LAString calendar, LAString slidingRule,
                     LADate startDate, LADate endDate, int* day);

static DateVector StartDates(DateVector baseDates);

static DateVector EndDates(DateVector baseDates);

static DateVector PayDates(DateVector endDates, LAString term, LAPriceDataSlidingRule slidingRule, LAPriceDataCalendar calendar);

static DateVector FixingDates(DateVector startDates, DateVector payDates, bool isAdvance, LAString fixingLag, LAString calendar);

static DateVector IndexSettlementDates(DateVector fixingDates, LAString settlLag, LAString calendar);

static DoubleVector Accruals(LAPriceDataDayCount daycount, DateVector startDates, DateVector endDates);

static LAPriceDataSlidingRule ModelSlidingRule();

static LAPriceDataCalendar ModelCalendar();
};

#endif