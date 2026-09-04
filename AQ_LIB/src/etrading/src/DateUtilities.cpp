//
// @File: DateUtilities.cpp
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.


// Includes: Standard Library
#include <time.h>
#include <string>
#include <algorithm>
#include <utility>
#include <map>
#include <iterator>
#include <vector>
#include <cmath>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


// Includes: This Library
#include "DateUtilities.h"
#include "AQLDateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "ContainerUtilities.h"
#include "ETradingException.h"
#include "AQLDate.h"
#include "TypeHelpers.h"
#include "AQLDateScheduleHelpers.h"      // Helper methods create date from string


namespace etrading
{
    /* Brief    Function to round a double to the nearest integer
    */
    int roundToNearest( double number )
    {
        // Use floor( d + 0.5 ) for positive doubles and ceil( d - 0.5 ) for negative doubles
        return number < 0.0 ? int( ceil( number - 0.5 ) ) : int( floor( number + 0.5 ) );
    }

    const int firstNonBusinessDayIdx( const std::vector<boost::gregorian::date>& dates, const AQLMathCalendar& cal )
    {
        auto loc_of_non_business_day = std::find_if( dates.cbegin(), dates.cend(), [&cal] ( const boost::gregorian::date & date )
        {
            return !isBusinessDay( date, cal );
        }  );
        if( loc_of_non_business_day == dates.cend() )
        {
            return -1;
        }
        else
        {
            return std::distance( dates.cbegin(), loc_of_non_business_day );
        }
    }

    const bool isWeekend( const boost::gregorian::date& date )
    {
        return ( date.day_of_week() ==  boost::date_time::Saturday || date.day_of_week() ==  boost::date_time::Sunday );
    }

    const bool isBusinessDay( const boost::gregorian::date& d, const AQLMathCalendar& cal )
    {
        AQLDate mlibDate( toYYYYMMDDFromGregorianDate( d ).c_str() );
        return !( cal.isHoliday( mlibDate ) );
    }

    std::vector<boost::gregorian::date>
    dayAdjust(	const std::vector<boost::gregorian::date>& inputDates,
                const BusinessDayAdjustmentEnum bdAdj,
                const AQLMathCalendar& cal )
    {
        std::vector<boost::gregorian::date> retDatesUsed;
        std::for_each( inputDates.cbegin(), inputDates.cend(), [&retDatesUsed, &cal, &bdAdj]( const boost::gregorian::date & inputDate )
        {
            retDatesUsed.emplace_back( etrading::dayAdjust( inputDate, bdAdj, cal ) );
        } );
        return retDatesUsed;
    };

    boost::gregorian::date dayAdjust(	const boost::gregorian::date& d,
                                        const BusinessDayAdjustmentEnum busDayAdjust,
                                        const AQLMathCalendar& cal )
    {

        boost::gregorian::date ret = d;
        bool isHoliday = false;

        //const set<const AQLMathCalendar *> &calSet = c.getCalendarSet();
        //set<const AQLMathCalendar *>::const_iterator it = calSet.begin();

        if ( !isBusinessDay( ret, cal ) )
        {
            switch( busDayAdjust )
            {
                case MOD_FOLLOWING:
                    ret += boost::gregorian::days( 1 );
                    while( !isBusinessDay( ret, cal ) )
                    {
                        ret += boost::gregorian::days( 1 );
                    }
                    // was the month changed?
                    if ( ret.month() != d.month() )
                    {
                        ret = d;
                        ret += boost::gregorian::days( -1 );
                        while( !isBusinessDay( ret, cal ) )
                        {
                            ret += boost::gregorian::days( -1 );
                        }
                    }
                    break;
                case FOLLOWING:
                    ret += boost::gregorian::days( 1 );
                    while( !isBusinessDay( ret, cal ) )
                    {
                        ret += boost::gregorian::days( 1 );
                    }
                    break;
                case PRECEDING:
                    ret += boost::gregorian::days( -1 );
                    while( !isBusinessDay( ret, cal ) )
                    {
                        ret += boost::gregorian::days( -1 );
                    }
                    break;
                case MOD_PRECEDING:
                    ret += boost::gregorian::days( -1 );
                    while( !isBusinessDay( ret, cal ) )
                    {
                        ret += boost::gregorian::days( -1 );
                    }
                    // change month ?
                    if ( ret.month() != d.month() )
                    {
                        ret = d;
                        ret += boost::gregorian::days( 1 );
                        while( !isBusinessDay( ret, cal ) )
                        {
                            ret += boost::gregorian::days( 1 );
                        }
                    }
                    break;
                case NO_CHANGE: //no change
                    break;
            }
        }
        return ret;
    }

    // loosely adapted from AQLDateHelpers.cpp renamed to AQLDateHelpers.cpp (around line 528) etrading::AQLDateHelpers::termStrtoYMDW
    std::tuple<int, int, int, int> tenorStringToYMDW( const std::string& tenorString )
    {
        size_t pl_y, pl_m, pl_d, pl_w;		//place of year, month, day, week
        std::string str = trim_to_upper( tenorString.c_str() );

        if ( str == "ON" || str == "O/N" || str == "O\\N" )
        {
            str = "1D";
        }
        else if ( str == "TN" || str == "T/N" || str == "T\\N" )
        {
            str = "2D";
        }

        //find place of...
        pl_y = std::string::npos; 
        pl_m = std::string::npos;
        pl_d = std::string::npos;
        pl_w = std::string::npos;

        pl_y = str.find_first_of( "Y" );
        pl_m = str.find_first_of( "M" );
        pl_d = str.find_first_of( "D" );
        pl_w = str.find_first_of( "W" );

        // std::string::npos is -1 ...
        if ( pl_y == std::string::npos && pl_m == std::string::npos && pl_d == std::string::npos && pl_w == std::string::npos )
        {
            throw ETradingException( ( boost::format( "#Error Invalid Tenor String: TenorString must be ON, TN, Y, M, D or W but found %s" ) % str.c_str() ).str() );
        }
        // week
        if ( ( pl_y != std::string::npos || pl_m != std::string::npos || pl_d != std::string::npos ) && pl_w != std::string::npos )
        {
            throw ETradingException( ( boost::format( "#Error Invalid Tenor String: TenorString must not be a combination of W and one of ON, TN, Y, M, D but found %s" ) % str.c_str() ).str() );
        }

        std::string tmp = str;
        removeAllSubstrings( tmp, "CO-TERM" );
        tmp.erase( std::remove_if( tmp.begin(), tmp.end(), boost::is_any_of( "1234567890YMDW-" ) ), tmp.end() );
        if ( tmp.size() != 0 )
        {
            throw ETradingException( ( boost::format( "#Error Invalid Tenor String: TenorString must be ON, TN, Y, M, D or W but found %s" ) % str.c_str() ).str() );
        }

        // assume YMDW
        std::vector<int> mp = boost::assign::list_of( pl_y )( pl_m )( pl_d )( pl_w );

        std::string str_y, str_m, str_d, str_w = "0";
        str_y = str_m = str_d = str_w;

        // the entry has to be in the right order xxxYxxxxMxxxxD
        for( unsigned int charCounter = 0u; charCounter < mp.size(); charCounter++ )
        {
            int idxInString = mp[charCounter];
            if( idxInString > 0 ) // ignore 0 because leading YMD or W is irrelevant
            {
                if( charCounter == 0 )
                {
                    str_y = str.substr( 0, idxInString );
                }
                else if( charCounter == 1 )
                {
                    int startOfNumberInString = ( mp[0] > 0 ) ? mp[0] + 1 : 0;
                    str_m = 	str.substr( startOfNumberInString , idxInString - startOfNumberInString );
                }
                else if( charCounter == 2 )
                {
                    int startOfNumberInString = ( mp[1] > 0 ) ? mp[1] + 1 : ( ( mp[0] > 0 ) ? mp[0] + 1 : 0 );
                    str_d = 	str.substr( startOfNumberInString , idxInString - startOfNumberInString );
                }
                else if( charCounter == 3 )
                {
                    str_w = str.substr( 0, idxInString );
                }
            }
        };

        return std::make_tuple( boost::lexical_cast<int>( str_y ), boost::lexical_cast<int>( str_m ), boost::lexical_cast<int>( str_d ), boost::lexical_cast<int>( str_w ) );
    }

    const bool canBeTenorString(const std::string& tenorString)
    {
        std::string str = trim_to_upper( tenorString.c_str() );
        if ( str == "ON" || str == "O/N" || str == "O\\N" )
        {
            str = "1D";
        }
        else if ( str == "TN" || str == "T/N" || str == "T\\N" )
        {
            str = "2D";
        }
        boost::regex regX( "(\\d+[Y])?(\\d+[M])?(\\d+[D])?");
        boost::cmatch matches;
        return boost::regex_match( str.c_str(), matches, regX ) && str.size() > 0;
    };


    boost::gregorian::date addTenorString( const boost::gregorian::date& input, const std::string& tenorString )
    {
        auto date = input;
        auto ymdw = tenorStringToYMDW( tenorString );

        // set W only
        const int w = std::get<3>( ymdw );
        if ( w != 0 ) // cannot use W with other terms
        {
            date += boost::gregorian::weeks( w );
            return date;
        }

        const int y = std::get<0>( ymdw );
        if( y != 0 )
        {
            date += boost::gregorian::years( y );
        }
        const int m = std::get<1>( ymdw );
        if( m != 0 )
        {
            date += boost::gregorian::months( m );
        }
        const int d = std::get<2>( ymdw );
        if( d != 0 )
        {
            date += boost::gregorian::days( d );
        }
        return date;
    };

    int daysBetween( const boost::gregorian::date& startDate, const boost::gregorian::date& endDate )
    {
        boost::gregorian::date_period period( startDate, endDate );
        auto daysInBetween = period.length().days();
        return daysInBetween;
    };

    boost::gregorian::date addYearFraction( const boost::gregorian::date& input, const double yearFraction, const etrading::DayCountEnum dayCount ) // assumes ACT_365
    {
        if( yearFraction < 0.0 )
        {
            throw ETradingException( ( boost::format( "#Error Invalid Year Fraction: Only positive yearFraction arguments are allowed (supplied: %f)" ) % yearFraction ).str() );
        }

        if( yearFraction > 1000.0 )
        {
            throw ETradingException( ( boost::format( "#Error Invalid Year Fraction: Only yearFraction arguments less than 1000 are allowed (supplied: %f)" ) % yearFraction ).str() );
        }

        if( dayCount == etrading::ACT_ACT_DAYCOUNT )
        {

            // this is the incorrect implementation of this day count but we need
            // an incorrect implementation to match AlgoQuantLib (this was requested with full knowledge of the trading desk, tokyo quants, etc. cfr. Nicholas Burgess)

            boost::gregorian::date nextYear( input.year() + 1, 1, 1 );
            auto daysToEndOfYear = daysBetween( input, nextYear );

            double td1 = static_cast<double>( daysToEndOfYear );
            double ty1 = static_cast<double>( 365 + ( isLeapYear( input.year() ) ? 1 : 0 ) );

            double ret = 0.0;
            if ( td1 / ty1 >= yearFraction )
            {
                ret = yearFraction * ty1;
            }
            else
            {
                boost::gregorian::date tmpDate = input;
                tmpDate += boost::gregorian::days( daysToEndOfYear );
                double term = yearFraction - td1 / ty1;
                int year = static_cast<int>( term );

                tmpDate += boost::gregorian::years( year );
                term -= static_cast<double>( year );

                tmpDate += boost::gregorian::days( 1 );

                int ty2 = 365 + ( isLeapYear( tmpDate.year() ) ? 1 : 0 );
                int days = static_cast<int>( term * ty2 );
                tmpDate += boost::gregorian::days( days - 1 );

                ret = daysBetween( input, tmpDate ) + term * ty2 - days;
            }

            int daysToAdd = roundToNearest( ret ); // Amended from Ceil to roundToNearest
            return input + boost::gregorian::days( daysToAdd );

            /*
            // this is the correct implementation of this day count but we need
            // an incorrect implementation to match AlgoQuantLib (this was requested with full knowledge of the trading desk, tokyo quants, etc. cfr. Nicholas Burgess)

            double fractionLeft = yearFraction;
            boost::gregorian::date iteratorDate = input;
            double currentDenominator = isLeapYear(iteratorDate.year()) ? 366.0 : 365.0;

            boost::gregorian::date nextYear(iteratorDate.year()+1,1,1);
            boost::gregorian::date_period period( iteratorDate, nextYear );
            auto daysBetween = period.length().days();

            double toNextYearFraction = static_cast<double>(daysBetween)/currentDenominator;
            if(toNextYearFraction < fractionLeft)
            {
            		fractionLeft -= toNextYearFraction;
            		iteratorDate = nextYear;
            }
            else
            {
            		int numOfDaysToAdd = static_cast<int>(currentDenominator*fractionLeft);
            		return (iteratorDate + boost::gregorian::days( numOfDaysToAdd ));
            		fractionLeft = 0.0;
            }

            while(fractionLeft > 1.0)
            {
            	int numberOfYearsToAdd = static_cast<int>(std::floor(fractionLeft));
            	iteratorDate += boost::gregorian::years( numberOfYearsToAdd );
            	int iterYear = iteratorDate.year();
            	fractionLeft -= static_cast<double>(numberOfYearsToAdd);
            }

            // update currentDenominator & iteratorDate
            currentDenominator = isLeapYear(iteratorDate.year()) ? 366.0 : 365.0;

            int numOfDaysToAdd = static_cast<int>(currentDenominator*fractionLeft);
            iteratorDate += boost::gregorian::days( numOfDaysToAdd );
            fractionLeft = 0.0;

            return iteratorDate;
            */

        }
        else if( dayCount == etrading::ACT_360_DAYCOUNT || dayCount == etrading::ACT_365_DAYCOUNT )
        {
            // cfr. double AQLPriceDataDayCount::getDayTerm(const AQLDate& fromDate, const double& termY, bool includelast) const
            //  No consensus exists on whether this is right or wrong in the original "common" project but placed here for backward compatability
            // (I think this is wrong because a yearFraction  > 1.0 it could be interpreted as less than a year e.g. 1.0+(1.0/365) )
            int daysToAdd = static_cast<int>( yearFraction ) * ( ( dayCount == etrading::ACT_360_DAYCOUNT ) ? 360 : 365 );
            return ( input + boost::gregorian::days( daysToAdd ) );
        }
        else
        {
            // cfr. double AQLPriceDataDayCount::getDayTerm(const AQLDate& fromDate, const double& termY, bool includelast) const
            throw ETradingException( "N30_360, E30_360 and ACT_365_FJ are not supported this method" );
        }

    };

    // copy from double AQLPriceDataDayCount::getTerm(const AQLDate& fromDate, const AQLDate& toDate, bool includelast) const
    // in AQLPriceDataDayCount.cpp (line 116)
    double getYearFractionFromDayCount(	const DayCountEnum dayCountEnum,
                                        const boost::gregorian::date& fromDate,
                                        const boost::gregorian::date& toDate,
                                        bool include_toDate )
    {
        if ( toDate < fromDate )
        {
            return -getYearFractionFromDayCount( dayCountEnum, toDate, fromDate, include_toDate );
        }
        if ( fromDate == toDate )
        {
            return 0.0;
        }

        boost::gregorian::date_period period( fromDate, toDate );
        auto daysBetween = period.length().days();

        double ret = 0.0;
        switch ( dayCountEnum )
        {
            case ACT_360_DAYCOUNT:
                ret = daysBetween / 360.0;
                break;
            case ACT_365_DAYCOUNT:
                ret = daysBetween / 365.0;
                break;
            case N30_360_DAYCOUNT:
            {
                int d1 = ( fromDate.day() == 31 ) ? 30 : (int) fromDate.day();
                int d2 = ( ( toDate.day() == 31 ) && ( d1 == 30 ) ) ? 30 : (int) toDate.day();

                ret = ( ( toDate.year() - fromDate.year() ) * 360.0
                        + ( toDate.month() - fromDate.month() ) * 30.0
                        + ( d2 - d1 ) ) / 360.0;
            }
            break;
            case E30_360_DAYCOUNT:
            {
                int d1 = ( fromDate.day() == 31 ) ? 30 : (int) fromDate.day();
                int d2 = ( toDate.day() == 31 ) ? 30 : (int) toDate.day();
                /*
                // the original function was also checking an odd settable termination date.... ?
                if( (toDate.monthOfYear() == 2) && (d2 == 29 || (!toDate.isLeapYear() && d2 == 28)) &&
                (toDate < mTerminationDate) ) d2 = 30;
                */
                if( ( toDate.month() == 2 ) && ( d2 == 29 || ( !isLeapYear( toDate.year() ) && d2 == 28 ) ) )
                {
                    d2 = 30;
                }

                ret = ( ( toDate.year() - fromDate.year() ) * 360.0
                        + ( toDate.month() - fromDate.month() ) * 30.0
                        + ( d2 - d1 ) ) / 360.0;
            }
            break;
            case ACT_ACT_DAYCOUNT:
            {
                //TODO: FIX ME

                //// https://en.wikipedia.org/wiki/Day_count_convention#Actual.2FActual_ISDA
                //int y1 = fromDate.year();
                //int y2 = toDate.year();
                //if( y2 == y1 )
                //{
                //    double denominator = isLeapYear( y1 ) ? 366.0 : 365.0;
                //    double includeAdjusment = include_toDate ? 1.0 : 0.0;
                //    return ( boost::gregorian::date_period( fromDate, toDate ).length().days() + includeAdjusment ) / denominator;
                //}

                //auto daysFromDateToEOY	= boost::gregorian::date_period( fromDate, boost::gregorian::date( fromDate.year() + 1, 1, 1 ) ).length().days();
                //auto daysToDateToEOY	= boost::gregorian::date_period( boost::gregorian::date( toDate.year(), 1, 1 ), toDate ).length().days();
                //
                //if( !include_toDate )
                //{
                //    daysToDateToEOY -= 1;
                //}

                //int daysInLeapYear = ( isLeapYear( y1 ) ? daysFromDateToEOY - 1 : 0 ) + ( isLeapYear( y2 ) ? daysToDateToEOY : 0 );
                //int daysNotInLeapYear = ( !isLeapYear( y1 ) ? daysFromDateToEOY - 1 : 0 ) + ( !isLeapYear( y2 ) ? daysToDateToEOY : 0 );

                //int yearCounter = y1 + 1;
                //while( yearCounter < y2 )
                //{
                //    if( isLeapYear( yearCounter ) )
                //    {
                //        daysInLeapYear += 366.0;
                //    }
                //    else
                //    {
                //        daysInLeapYear += 365.0;
                //    }
                //    yearCounter++;
                //}
                //return ( daysInLeapYear / 366.0 + daysNotInLeapYear / 365.0 );


                // ALTERNATIVE: UNTIL THE ABOVE IS FIXED
                const bool includeLast = true;
				AQLString dayCount("ACT/365");
                double yearFraction = AQLDateScheduleHelpers::getTerm( toAQLDateFromGregorianDate( fromDate ), toAQLDateFromGregorianDate( toDate ), dayCount, includeLast );

                return yearFraction;

            }
            break;
            case ACT_365_FJ_DAYCOUNT:
            {
                int y1 = fromDate.month();
                int y2 = toDate.month();
                int d1 = toDate.year() - fromDate.year() ;

                auto tmpDate = fromDate;
                int f = 0;
                int i;

                if ( y1 < 3 && isLeapYear( fromDate.year() ) )
                {
                    f++;
                }
                //add followed condition : && fromDate.stringWithFormat("MMDD") != "0229" (a comment from the AlgoQuantLib library)
                if ( y2 > 2 && isLeapYear( toDate.year() ) && fromDate.year() != toDate.year() )
                {
                    f++;
                }
                if ( include_toDate  && y1 == 2 && fromDate.day() == 29 )
                {
                    f--;
                }
                if ( !include_toDate && y2 == 2 && toDate.day()   == 29 )
                {
                    f--;
                }

                for ( i = 0; i < d1 - 1; i++ )
                {
                    tmpDate += boost::gregorian::years( 1 );
                    if ( isLeapYear( tmpDate.year() ) )
                    {
                        f++;
                    }
                }
                ret = ( daysBetween - f ) / 365.0;
            }
            break;
			case ONE_DAYCOUNT:
			{
				ret = 1.0;
			}
			break;
			case HALF_DAYCOUNT:
			{
				ret = 0.5;
			}
			break;
			case QUARTER_DAYCOUNT:
			{
				ret = 0.25;
			}
			break;
        }
        return ret;
    };

    const bool isLeapYear( int year )
    {
        if ( year % 4 != 0 )
        {
            return false;
        }
        if ( year % 100 != 0 )
        {
            return true;
        }
        if ( year % 400 == 0 )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::string getCurrentDateTime()
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        return ( boost::format( "%04i%02i%02i %02i:%02i:%02i" ) % now.date().year()
                                                                % now.date().month().as_number()
                                                                % now.date().day()
                                                                % now.time_of_day().hours()
                                                                % now.time_of_day().minutes()
                                                                % now.time_of_day().seconds() ).str();
    };

    AQLDate getCurrentMLibDate()
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        
        AQLDate currentDate;
        currentDate.setYear( now.date().year() );
        currentDate.setMonth( now.date().month().as_number() );
        currentDate.setDay( now.date().day() );
        
        return currentDate;
    }

    unsigned int getCurrentDecade()
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        
        unsigned int currentDecade = now.date().year() - ( now.date().year() % 10 );

        return currentDecade;
    }

    unsigned int getCurrentCentury()
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        
        unsigned int currentCentury = now.date().year() - ( now.date().year() % 100 );

        return currentCentury;
    }

    std::string getCurrentTime()
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        return ( boost::format( "%02i:%02i:%02i" ) % now.time_of_day().hours()
                                                   % now.time_of_day().minutes()
                                                   % now.time_of_day().seconds() ).str();
    };

    AQLDate toAQLDateFromGregorianDate( const boost::gregorian::date& gregorian_date )
    {
        AQLDate laDate;
		laDate.setYear( gregorian_date.year() );
		laDate.setMonth( gregorian_date.month() );
		laDate.setDay( gregorian_date.day() );
		laDate.dateToJulius();

        return laDate;
    };

    std::vector<AQLDate> toAQLDatesFromGregorianDates( const std::vector<boost::gregorian::date>& gregorian_dates )
    {
        std::vector<AQLDate> AQLDates;
        std::for_each( gregorian_dates.cbegin(), gregorian_dates.cend(),
                       [&AQLDates]( const boost::gregorian::date & gDate )
        {
            AQLDates.emplace_back( toAQLDateFromGregorianDate( gDate ) );
        }
                     );
        return AQLDates;
    };

    // returns a date string "YYYYMMDD" from a boost::gregorian::date
    std::string toYYYYMMDDFromGregorianDate( const boost::gregorian::date& gregorian_date )
    {
        return std::string(  ( boost::format( "%04i%02i%02i" ) % static_cast<short>( gregorian_date.year() ) % static_cast<short>( gregorian_date.month() ) % static_cast<short>( gregorian_date.day() ) ).str() ) ;
    };

    // returns a date string "YYYYMMDD" from an AQLDate
    std::string toYYYYMMDDFromDate( const AQLDate& date )
    {
        boost::gregorian::date gregorianDate = toGregorianDateFromLADate( date );
        std::string dateString = toYYYYMMDDFromGregorianDate( gregorianDate );
        return dateString;
    }

    boost::gregorian::date toGregorianDateFromYYYYMMDD( const std::string& inputDate ) // only supplied as a sister method
    {
        return boost::gregorian::from_undelimited_string( inputDate );
    } ;

    boost::gregorian::date toGregorianDateFromLADate( const AQLDate& mbd )
    {
        return boost::gregorian::date( mbd.yearOfEra(), mbd.monthOfYear(), mbd.dayOfMonth() );
    };

    bool couldBeDate( const std::string& inString )
    {
        boost::cmatch matches;
        int idx = -1;
        for( size_t regexCounter = 0 ; regexCounter < DATE_REGEX.size(); regexCounter++ )
        {
            if( boost::regex_match( inString.c_str(), matches, DATE_REGEX[regexCounter ] ) )
            {
                idx =  regexCounter;
                break;
            }
        }
        return idx >= 0;
    };

    // Converts a std::string to a boost:gregorian::date using boost regular expression logic.
    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp operates on the specific position of elements REGEX expression list.
    boost::gregorian::date toGregorianDateFromREGEX( const std::string& inString )
    {
        // assume the couldBeDate has already been checked - no need to do double checking so throw an exception if not valid
        auto cit = std::find_if( DATE_REGEX.cbegin(), DATE_REGEX.cend(), [&inString] ( const boost::regex & regX ) -> bool
        {
            boost::cmatch matches;
            return boost::regex_match( inString.c_str(), matches, regX );
        } );
        int idx = ( cit != DATE_REGEX.cend() ? cit - DATE_REGEX.cbegin() : -1 );
        if( idx < 0 )
        {
            throw ETradingException( ( boost::format( "#Error: Unable to convert string to date: %s" ) % inString.c_str() ).str() );
        }

        // ("\\d{5}")("\\d{8}")("\\d{4}-\\d{2}-\\d{2}")("(\\d{2,4})\\/(\\d{2})(?:\\/?(\\d{2}))?")("\\d{4}\\\\\\d{2}\\\\\\d{2}")("(\\d{2})\\/(\\d{2})(?:\\/?(\\d{2,4}))?")("\\d{2}-\\d{2}-\\d{4}")("\\d{2}\\\\\\d{2}\\\\\\d{4}")
        if( idx == 0 )
        {
			return toGregorianDateFromExcelDate(StringToNumber<int>(inString)); 
        }
        if( idx == 1 )
        {
            return boost::gregorian::from_undelimited_string( inString );
        }
        else if( idx == 2 || idx == 3 )
        {
            return boost::gregorian::from_string( inString );
        }
        else
        {
            std::string yearString = ( idx == 4 ) ? inString.substr( 0, 4 ) : inString.substr( inString.length() - 4, 4 );
            std::string dayString = ( idx == 4 ) ? inString.substr( inString.length() - 2, 2 ) : inString.substr( 0, 2 );
            std::string monthString = ( idx == 4 ) ? inString.substr( 5, 2 ) : inString.substr( 3, 2 );
            int year = boost::lexical_cast<int>( yearString );
            int month = boost::lexical_cast<int>( monthString );
            int day = boost::lexical_cast<int>( dayString );
            if( year >= 1400 && year <= 3000 && day >= 1 && day <= 31 && month >= 1 && month <= 12 )
            {
                return boost::gregorian::date( year, month, day );
            }
            else
            {
                throw ETradingException( (  boost::format( "#Error: Unable to generate date from year %i month %i day %i" ) %  year % month % day ).str() );
            }
        }
        return boost::gregorian::date( boost::gregorian::min_date_time ); // should never get here
    };

    // Converts a std::string to an AQLDate using boost regular expression logic.
    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp operates on the specific position of elements REGEX expression list.
    AQLDate toAQLDateFromREGEX( const std::string& inString )
    {
        boost::gregorian::date gregorianDate = toGregorianDateFromREGEX( inString );
        AQLDate laDate = toAQLDateFromGregorianDate( gregorianDate );
        return laDate;
    }

    // http://www.codeproject.com/Articles/2750/Excel-serial-date-to-Day-Month-Year-and-vise-versa
    const int toExcelDateFromGregorianDate( const boost::gregorian::date& gregorian_date )
    {
        const int year_int = gregorian_date.year();
        const int month_int = gregorian_date.month();
        const int day_int = gregorian_date.day();

        // Excel/Lotus 123 have a bug with 29-02-1900. 1900 is not a
        // leap year, but Excel/Lotus 123 think it is...
        if ( day_int == 29 && month_int == 02 && year_int == 1900 )
        {
            return 60;
        }
        // DMY to Modified Julian calculatie with an extra substraction of 2415019.
        long nSerialDate =
            int( ( 1461 * ( year_int + 4800 + int( ( month_int - 14 ) / 12 ) ) ) / 4 ) +
            int( ( 367 * ( month_int - 2 - 12 * ( ( month_int - 14 ) / 12 ) ) ) / 12 ) -
            int( ( 3 * ( int( ( year_int + 4900 + int( ( month_int - 14 ) / 12 ) ) / 100 ) ) ) / 4 ) +
            day_int - 2415019 - 32075;
        if ( nSerialDate < 60 )
        {
            // Because of the 29-02-1900 bug, any serial date
            // under 60 is one off... Compensate.
            nSerialDate--;
        }

        return static_cast<const int>( nSerialDate );
    };

    boost::gregorian::date toGregorianDateFromExcelDate( int excelDate )
    {
        int nDay;
        int nMonth;
        int nYear;

        // Excel/Lotus 123 have a bug with 29-02-1900. 1900 is not a
        // leap year, but Excel/Lotus 123 think it is...
        if ( excelDate == 60 )
        {
            nDay    = 29;
            nMonth    = 2;
            nYear    = 1900;
            return boost::gregorian::date( nYear, nMonth, nDay );
        }
        else if ( excelDate < 60 )
        {
            // Because of the 29-02-1900 bug, any serial date
            // under 60 is one off... Compensate.
            excelDate++;
        }

        // Modified Julian to DMY calculation with an addition of 2415019
        int l = excelDate + 68569 + 2415019;
        int n = int( ( 4 * l ) / 146097 );
        l = l - int( ( 146097 * n + 3 ) / 4 );
        int i = int( ( 4000 * ( l + 1 ) ) / 1461001 );
        l = l - int( ( 1461 * i ) / 4 ) + 31;
        int j = int( ( 80 * l ) / 2447 );

        nDay = l - int( ( 2447 * j ) / 80 );
        l = int( j / 11 );

        nMonth = j + 2 - ( 12 * l );
        nYear = 100 * ( n - 49 ) + i + l;
        return boost::gregorian::date( nYear, nMonth, nDay );
    }

    double getRateFromReturn( const double returnValue, const double yearFraction, const CompoundingFrequencyEnum compFreq )
    {
        double rate = 0;
        switch ( compFreq )
        {
            case SIMPLE_COMPOUNDING:
                rate = ( returnValue - 1.0 ) / yearFraction;
                break;
            case MONTHLY_COMPOUNDING:
            case LUNAR_COMPOUNDING:
                rate = ( std::pow( returnValue, 1.0 / ( yearFraction * 12.0 ) ) - 1.0 ) * 12.0;
                break;
            case SEMI_ANNUAL_COMPOUNDING:
                rate = ( std::pow( returnValue, 1.0 / ( yearFraction * 2.0 ) ) - 1.0 ) * 2.0;
                break;
            case ANNUAL_COMPOUNDING:
                rate = std::pow( returnValue, 1.0 / yearFraction ) - 1.0;
                break;
            case QUARTERLY_COMPOUNDING:
                rate = ( std::pow( returnValue, 1.0 / ( yearFraction * 4.0 ) ) - 1.0 ) * 4.0;
                break;
            case WEEKLY_COMPOUNDING:
                rate = ( std::pow( returnValue, 1.0 / ( yearFraction * 52.0 ) ) - 1.0 ) * 52.0;
                break;
            default:
                throw ETradingException( ( boost::format( "getRateFromReturn: unhandled CompoundingFrequency: %s" ) % toString( compFreq ) ).str() );
                break;
        }
        return rate;
    }

    double convertRateFromAnnualTo( const double inputRate, const CompoundingFrequencyEnum targetCompoundingFreq )
    {
        double rate = inputRate;
        switch ( targetCompoundingFreq )
        {
            case MONTHLY_COMPOUNDING:
            case LUNAR_COMPOUNDING:
                rate = 12.0 * ( std::pow( inputRate + 1.0, 1.0 / 12.0 ) - 1.0 );
                break;
            case SEMI_ANNUAL_COMPOUNDING:
                rate = 2.0 * ( std::pow( inputRate + 1.0, 0.5 ) - 1.0 );
                break;
            case QUARTERLY_COMPOUNDING:
                rate = 4.0 * ( std::pow( inputRate + 1.0, 0.25 ) - 1.0 );
                break;
            case WEEKLY_COMPOUNDING:
                rate = 52.0 * ( std::pow( inputRate + 1.0, 1.0 / 52.0 ) - 1.0 );
                break;
            case SIMPLE_COMPOUNDING:
            case ANNUAL_COMPOUNDING:
                // nothing to be done
                break;
            default:
                throw ETradingException( ( boost::format( "convertRateFromAnnualTo: unhandled CompoundingFrequency: %s" ) % toString( targetCompoundingFreq ) ).str() );
                break;
        }
        return rate;
    }

    /* @brief			Parses a tenor string of the format "nY" and return the number of years
    *  @param [in]		tenor			The tenor string to parse
    *  @param [in]		throwOnFailure	Controls whether the function throws an exception if there is a parsing failure
	*  @return			The length of the tenor in years
    */
	unsigned int parseTenorYears( const std::string& tenor, const bool throwOnFailure  )
	{
		// riskCutOffTenor should be in the format "nY", for example "10Y"
		unsigned int tenorYears = 0;
		bool parseSuccessful = false;

		if ( ! tenor.empty() )
		{
			// a tenor string has been provided. Check that the format is valid.
			// It should be at least 2 chars long
			const unsigned int tenorStringLength = tenor.size();
			if (tenorStringLength > 1)
			{
				// Get the final character of the string
				std::string tenorUnit = tenor.substr( tenorStringLength - 1, 1);
				if (tenorUnit == "Y" || tenorUnit == "y" )
				{
					// This looks like a candidate year tenor.
					// Attempt to get the number of years
					std::string tenorYearsString = tenor.substr( 0, tenorStringLength - 1);
					try
					{
						// Here we only support positive tenors
						int years = std::stoi( tenorYearsString );
						if ( years >= 0 )
						{
							tenorYears = years;
							parseSuccessful = true;
						}
					}
					catch ( const std::exception& e )
					{
						// Failed to convert to a year integer. We throw an informative message below.
						(void)e;
					}
				}
			}

			if ( ! parseSuccessful && throwOnFailure )
			{
				throw AQLCoreInvalidData( ( boost::format("#Error: Invalid tenor \"%s\". Expecting format \"nnY\" ." )
									   % tenor ).str().c_str(), __FILE__, __LINE__ );
			}
		}
		return tenorYears;
	}

    /* @brief			Function to populate Date and Value vectors from a DateValue matrix with 2 columns of type AQLStringMatrix. Dates must be in ascending order with no duplicates.
    *  @param [out]		dateOutput			The date results output vector
    *  @param [out]		valueOutput	        The value results output vector
    *  @param [in]		inputMatrix	        Input String Matrix: Must have 2 columns with the first column containing dates and the second containing double values
    */
    void populateDateValueVectorsFromStringMatrix( std::vector<AQLDate> & dateOutput,
                                                   std::vector<double> & valueOutput,
                                                   const AQLStringMatrix & inputMatrix )
    {
        AQ_REQUIRE( inputMatrix.size() > 0, "DateValue input matrix is empty" )
        AQ_REQUIRE( inputMatrix[0].size() == 2, "DateValue input matrix should have exactly 2 columns; The first column should have dates and the second column values" )
        
        dateOutput.resize( inputMatrix.size() );
        valueOutput.resize( inputMatrix.size() );
        
        AQLDate lastPaymentDate;

        for( size_t i = 0; i< inputMatrix.size(); ++i )
        {
            dateOutput[i]   = etrading::AQLDateScheduleHelpers::getLADate( inputMatrix[i][0].getCString() );
            char * pFirstNonNumber;
            valueOutput[i]  = std::strtod( inputMatrix[i][1].getCString(), &pFirstNonNumber );
            
            if( i > 0 && dateOutput[i] <= lastPaymentDate )
            {
                std::string currentPaymentDateAsString  = dateOutput[i].convertDateToString("DD-MM-YYYY").getCString();
                std::string lastPaymentDateAsString     = lastPaymentDate.convertDateToString("DD-MM-YYYY").getCString();
                std::string currentIndex                = std::to_string( static_cast<long long>(i + 1) );
                AQ_THROW( "Dates must be sorted in ascending order with no duplicates - Date '" + currentPaymentDateAsString + "' in position '" + currentIndex + "' is invalid and less than or equal to the previous date'" + lastPaymentDateAsString + "'" )
            }

            lastPaymentDate = dateOutput[i];            
        }
    }
}
