#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathCashFlowSchedules.h"
#include "LAFunctionUtilities.h"
#include "LAMathDateUtilities.h"

//================ Single Flow ===================================
CashFlowTiming LAMathScheduleUtility::CashFlowSchedule(LADate valDate, LAString mtyTerm, LAStringMatrix legScheduler, LAStringMatrix indexScheduler)
{
    ////// Input properties ////
    LAString spotLag = LAFunctionUtilities::findElement(legScheduler, "SpotLag");
    LAString payCalendar_ = LAFunctionUtilities::findElement(legScheduler, "PaymentCalendar");
    LAString paySlidingRule_ = LAFunctionUtilities::findElement(legScheduler, "PaymentSlidingRule");
    //LAString payDelay = LAFunctionUtilities::findElement(legScheduler, "PaymentDelay");
    LAString fixingTiming = LAFunctionUtilities::findElement(legScheduler, "FixingTiming");
    LAString fixingLag = LAFunctionUtilities::findElement(legScheduler, "FixingLag");
    LAString fixingCalendar = LAFunctionUtilities::findElement(legScheduler, "FixingCalendar");
    LAString daycountConvention = LAFunctionUtilities::findElement(legScheduler, "Daycount");
    upper(fixingTiming);
    bool isAdvance = (fixingTiming == "ADVANCE");
    bool overrideToArrear = true;
    if (overrideToArrear)
        isAdvance = false;
    LAString indexFixingLag = LAFunctionUtilities::findElement(indexScheduler, "FixingLag");
    LAString indexFixingCalendar = LAFunctionUtilities::findElement(indexScheduler, "FixingCalendar");

    // Conversions
    LAPriceDataSlidingRule paySlidingRule;
    paySlidingRule.convertFromString(paySlidingRule_);
    LAPriceDataCalendar payCalendar;
    upper(payCalendar_);
    payCalendar.convertFromString(payCalendar_);

    // No change
    LAPriceDataSlidingRule noChangeSlidingRule;
    noChangeSlidingRule.convertFromString("no_change");
    LAPriceDataCalendar noChangeCalendar;
    noChangeCalendar.convertFromString("");

    //// Calculation ////
    //LADate startDate = CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    LADate swapStartDate = CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    LADate endDate = CalendarAdvance(swapStartDate, mtyTerm, noChangeSlidingRule, noChangeCalendar);
    LADate payDate = endDate;
    LADate startDate = endDate;
    //LADate payDate = CalendarAdvance(endDate, payDelay, paySlidingRule, payCalendar);
    // Fixing Date
    LADate refDate = (isAdvance ? startDate : payDate);
    LAPriceDataSlidingRule fixingSr;
    fixingSr.convertFromString("Preceding");
    LAPriceDataCalendar fixingCdr;
    fixingCdr.convertFromString(fixingCalendar);
    LADate fixingDate = CalendarAdvance(refDate, "-" + fixingLag, fixingSr, fixingCdr);
    // Index settlement
    LAPriceDataSlidingRule idxSr;
    idxSr.convertFromString("Following");
    LAPriceDataCalendar idxFixingCdr;
    idxFixingCdr.convertFromString(indexFixingCalendar);
    LADate idxSettlDate = CalendarAdvance(fixingDate, indexFixingLag, idxSr, idxFixingCdr);
    // Accrual
    LAPriceDataDayCount daycount = Daycount(daycountConvention);
    double accrual = YearFraction(daycount, startDate, endDate);

    return CashFlowTiming { fixingDate, startDate, endDate, payDate, idxSettlDate, accrual };
}

//================ Multiple Flows ===================================
vector<CashFlowTiming> LAMathScheduleUtility::LegSchedule(LADate valDate, LAString mtyTerm, LAStringMatrix legScheduler, LAStringMatrix indexScheduler)
{
    // Find input properties
    LAString spotLag = LAFunctionUtilities::findElement(legScheduler, "SpotLag");
    LAString frequency = LAFunctionUtilities::findElement(legScheduler, "Frequency");
    LAString accCalendar = LAFunctionUtilities::findElement(legScheduler, "AccrualCalendar");
    LAString accSlidingRule = LAFunctionUtilities::findElement(legScheduler, "AccrualSlidingRule");
    LAString payDelay = LAFunctionUtilities::findElement(legScheduler, "PaymentDelay");
    LAString payCalendar_ = LAFunctionUtilities::findElement(legScheduler, "PaymentCalendar");
    LAString paySlidingRule_ = LAFunctionUtilities::findElement(legScheduler, "PaymentSlidingRule");
    LAString fixingLag = LAFunctionUtilities::findElement(legScheduler, "FixingLag");
    LAString fixingCalendar = LAFunctionUtilities::findElement(legScheduler, "FixingCalendar");
    LAString fixingTiming = LAFunctionUtilities::findElement(legScheduler, "FixingTiming");
    LAString daycountConvention = LAFunctionUtilities::findElement(legScheduler, "Daycount");
    upper(fixingTiming);
    bool isAdvance = (fixingTiming == "ADVANCE");
    LAString indexFixingLag = LAFunctionUtilities::findElement(indexScheduler, "FixingLag");
    LAString indexFixingCalendar = LAFunctionUtilities::findElement(indexScheduler, "FixingCalendar");

    // Conversions
    LAPriceDataSlidingRule paySlidingRule;
    paySlidingRule.convertFromString(paySlidingRule_);
    LAPriceDataCalendar payCalendar;
    upper(payCalendar_);
    payCalendar.convertFromString(payCalendar_);
    LAPriceDataSlidingRule noChangeSlidingRule;
    noChangeSlidingRule.convertFromString("no_change");
    LAPriceDataCalendar noChangeCalendar;
    noChangeCalendar.convertFromString("");

    // Preliminary
    LADate startDate = CalendarAdvance(valDate, spotLag, paySlidingRule, payCalendar);
    LADate endDate = CalendarAdvance(startDate, mtyTerm, noChangeSlidingRule, noChangeCalendar);
    int day = endDate.dayOfMonth();

    // Calculate schedule
    DateVector baseDates = BaseDates(frequency, accCalendar, accSlidingRule, startDate, endDate, &day);
    DateVector startDates = StartDates(baseDates);
    DateVector endDates = EndDates(baseDates);
    DateVector payDates = PayDates(endDates, payDelay, paySlidingRule, payCalendar);
    DateVector fixingDates = FixingDates(startDates, payDates, isAdvance, fixingLag, fixingCalendar);
    DateVector idxSettlDates = IndexSettlementDates(fixingDates, indexFixingLag, indexFixingCalendar);
    LAPriceDataDayCount daycount = Daycount(daycountConvention);
    vector<double> accruals = Accruals(daycount, startDates, endDates);

    size_t n = payDates.size();
    vector<CashFlowTiming> schedule(n);
    for (size_t i = 0; i < n; i++)
        schedule[i] = CashFlowTiming{ fixingDates[i], startDates[i], endDates[i], payDates[i], idxSettlDates[i], accruals[i] };

    return schedule;
}

DateVector LAMathScheduleUtility::BaseDates(LAString frequency, LAString calendar, LAString slidingRule,
                     LADate startDate, LADate endDate, int* day)
{
    return LAMathDateUtilities::generateSchedule(startDate, endDate, frequency, slidingRule, calendar, 0, 0, day, true, 0);
}

DateVector LAMathScheduleUtility::StartDates(DateVector baseDates)
{
    size_t n = baseDates.size() - 1;
    DateVector startDates(n);
    for (size_t i = 0; i < n; i++)
        startDates[i] = baseDates[i];

    return startDates;
}

DateVector LAMathScheduleUtility::EndDates(DateVector baseDates)
{
    size_t n = baseDates.size() - 1;
    DateVector endDates(n);
    for (size_t i = 0; i < n; i++)
        endDates[i] = baseDates[i + 1];

    return endDates;
}

DateVector LAMathScheduleUtility::PayDates(DateVector endDates, LAString term, LAPriceDataSlidingRule slidingRule, LAPriceDataCalendar calendar)
{
    size_t n = endDates.size();
    DateVector payDates(n);
    for (size_t i = 0; i < n; i++)
        payDates[i] = CalendarAdvance(endDates[i], term, slidingRule, calendar);

    return payDates;
}

DateVector LAMathScheduleUtility::FixingDates(DateVector startDates, DateVector payDates, bool isAdvance, LAString fixingLag, LAString calendar)
{
    DateVector refDates = (isAdvance ? startDates : payDates);
    size_t n = refDates.size();
    DateVector fixingDates(n);
    LAPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("Preceding");
    LAPriceDataCalendar cdr;
    cdr.convertFromString(calendar);
    for (size_t i = 0; i < n; i++)
        fixingDates[i] = CalendarAdvance(refDates[i], "-" + fixingLag, slidingRule, cdr);

    return fixingDates;
}

DateVector LAMathScheduleUtility::IndexSettlementDates(DateVector fixingDates, LAString settlLag, LAString calendar)
{
    size_t n = fixingDates.size();
    DateVector idxSettlDates(n);
    LAPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("Following");
    LAPriceDataCalendar cdr;
    cdr.convertFromString(calendar);
    for (size_t i = 0; i < n; i++)
        idxSettlDates[i] = CalendarAdvance(fixingDates[i], settlLag, slidingRule, cdr);

    return idxSettlDates;
}

DoubleVector LAMathScheduleUtility::Accruals(LAPriceDataDayCount daycount, DateVector startDates, DateVector endDates)
{
    size_t n = startDates.size();
    DoubleVector acc(n);
    for (size_t i = 0; i < n; i++)
        acc[i] = YearFraction(daycount, startDates[i], endDates[i]);

    return acc;
}

LAPriceDataSlidingRule LAMathScheduleUtility::ModelSlidingRule()
{
    LAPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString("MOD_FOLLOWING");
    return slidingRule;
}

LAPriceDataCalendar LAMathScheduleUtility::ModelCalendar()
{
    LAPriceDataCalendar calendar;
    calendar.convertFromString("TKB:LNB");
    return calendar;
}
