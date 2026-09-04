/*!
    \file Utility.h
    \brief Declares and implements various utility functions.

*/

#ifndef AQLCoreUtil_h
#define AQLCoreUtil_h

#ifdef _MSC_VER
//    #pragma warning (disable : 4786)
    #pragma warning (disable : 4996)
#endif

#include <vector>
#include <map>

//#ifdef __GNUC__
//#include <ext/unordered_map>
//using namespace __gnu_cxx;
//#else
//#include <unordered_map>
//#endif

#include <unordered_map>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string.h>

using namespace std;

//namespace util
//{



const double eps_1d = 1./367.;
const double eps_1W = 7./367.;

template <typename T1, typename T2>
bool eq(T1 x, T2 y, double eps = 10. * std::numeric_limits<double>::epsilon() )
{
    return fabs( static_cast<double>(x - y) ) <= eps;
}

template <typename T>
T max_(T x, T y, double eps = 10. * std::numeric_limits<double>::epsilon() )
{
    return !eq(x,y,eps) && x <= y ? y : x;
}

template <typename T>
T min_(T x, T y, double eps = 10. * std::numeric_limits<double>::epsilon() )
{
    return eq(x,y,eps) || x > y ? y : x;
}

//
template <typename T1, typename T2>
std::vector<T2> map_to_vector( const std::map<T1, T2>& x )
{
    std::map<double, double>::iterator itr_;
    std::vector<T2> ret;
    typename std::map<T1, T2>::const_iterator itr = x.begin();
    while ( itr != x.end() )
    {
        ret.push_back( itr->second );
        itr++;
    }
    if ( ret.size() == 0 ) ret.resize(1,0);
    return ret;
}

//
template <typename T1, typename T2>
std::vector<T2> map_to_vector( const unordered_map<T1, T2>& x )
{
    std::vector<T2> ret;
    typename std::map<T1, T2>::const_iterator itr = x.begin();
    while ( itr != x.end() )
    {
        ret.push_back( itr->second );
        itr++;
    }
    if ( ret.size() == 0 ) ret.resize(1,0);
    return ret;
}

template <typename T>
void print_vector( const std::vector<T> x, const std::string s = "" )
{
    std::cout << s << "\t";
    for ( size_t i = 0; i < x.size(); i++)
    {
        std::cout << std::setw(9) << x[i] << "\t";
    }
    std::cout << std::endl;
}

//
template <typename T>
size_t search_Idx(T x, const std::vector<T>& y)
{
     size_t i = upper_bound(y.begin(), y.end(), x) - y.begin();
     if( i != 0 && i < y.size() )
     {
         i = eq(x, y[i], 1. / 367. ) ? i : --i;
     }
     else if( i == y.size() )
     {
         --i;
     }
     return i;
}

// Creates string from given stream. Especially useful to convert number to string.
template<typename T>
const std::string n2s(const T x)
{
    std::stringstream str_stream;
    str_stream << x;
    return str_stream.str();
}

// Get integer part of double.
const double IntOfDouble(const double n);

// Get decimal part of double.
const double DecOfDouble(const double n);

// Transforms the int/double to string that includes comma in 3 digit.
template<typename T>
const std::string commaStr(T number, unsigned short dec = 6)
{
//    number = static_cast<double>(number);
    double abs_number = fabs( static_cast<double>(number) );
    double abs_number_int = IntOfDouble(abs_number);
    double abs_number_dec = DecOfDouble(abs_number);
    if (abs_number_dec == 0) dec = 0;

    if (abs_number < 1000 && dec <= 0)
    {
        return n2s(static_cast<int>(number));
    }
    else
    {
        std::string abs_number_dec_str = "";
        if (0 < dec)
        {
            char tmp[100];
            std::string option = "%#." + n2s(dec) + 'f';
            sprintf(tmp, option.c_str(), abs_number_dec);
            abs_number_dec_str = strchr(tmp, '.');
        }

        std::string abs_number_int_str = "";
        std::string str;
        if (abs_number < 1000)
        {
            abs_number_int_str = n2s(static_cast<int>(abs_number_int));
        }
        else
        {
            std::vector<std::string> tmp_abs_number_int;
            double tmp_int = abs_number_int;
            while (1000 <= tmp_int)
            {
                double tmp_int2 = tmp_int;
                tmp_int = IntOfDouble(tmp_int / 1000.0);
                str = n2s( static_cast<int>(tmp_int2 - tmp_int * 1000) );

                if (str.size() == 1) str = "00" + str;
                else if (str.size() == 2) str = "0" + str;
                tmp_abs_number_int.push_back(str);
            }
            tmp_abs_number_int.push_back(n2s( static_cast<int>(tmp_int) ));

            for (size_t i = tmp_abs_number_int.size() - 1; 0 < i; i--)
            {
                abs_number_int_str += tmp_abs_number_int[i] + ',';
            }
            abs_number_int_str += tmp_abs_number_int[0];
        }

        std::string sgn;
        if (0<number) sgn = "";
        else sgn = '-';

        return sgn + abs_number_int_str + abs_number_dec_str;
    }
}

//}// end namespace

//using namespace util;

#endif
