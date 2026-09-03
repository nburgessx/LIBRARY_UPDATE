#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

#include <limits>
#include <iomanip>
#include <map>
#include <sstream>


#include "AQLCoreAppError.h"
#include "AQLString.h"

using namespace std;

const double eps_1d = 1./367.;
const double eps_1W = 7./367.;

class LAModelUtilities
{
	public:
		// from algorithm_pls.h

		//
		template<typename T> static T bound(T a, T x, T b)
		{
			return std::max(a,std::min(x,b));
		}

		// finding index in a vector corresponsding to a number
		template<typename T1, typename T2> static size_t getIndex(T1 x, const vector<T2>& X_vec)
		{
			if( x < X_vec.front() )
			{
				return 0; //throw out of the range (earlier than expiry)
			}
			else if( X_vec.back() < x )
			{
				return X_vec.size()-1;//throw out of the range (later than maturity)
			}
			else
			{
				return (--upper_bound(X_vec.begin(), X_vec.end(), x) - X_vec.begin());
			}

		}

		//
		template<class In, class T> static vector<T> accumulateVec( In first, In last, T init )
		{
			vector<T> ret;
			while ( first != last )
			{
				init = init + *first++;
				ret.push_back( init );
			}
			return ret;
		}

		// Sorts sequence and then eliminates duplicate factors.
		template<class C> static void eliminateDuplicates(C& c)
		{
			sort(c.begin(), c.end());
			typename C::iterator p = unique(c.begin(), c.end());
			c.erase(p, c.end());
		}

		template<class C> static C insertData(const C& c, int n)
		{
			if ( n < 0 ) n = 0;
			if ( n == 0 ) return c;
			C tmp = c;
			C ret;
			eliminate_duplicates(tmp);
			typename C::iterator p = tmp.begin();
			while ( p != tmp.end() )
			{
				ret.insert(ret.end(), *p);
				if ( ++p != tmp.end() )
				{
					double dt = (*p - *(p-1))/(n+1);
					for ( size_t i = 0; i < static_cast<size_t>(n); i++ )
						ret.insert(ret.end(), *(ret.end()-1) + dt);
				}
			}
			eliminateDuplicates(ret);
			return ret;
		}

		// Sort two sequences by one of the sequence.
		template<typename T> static void sortVectors( const vector<T>& in_vec1, const vector<T>& in_vec2,
				vector<T>& out_vec1, vector<T>& out_vec2)
		{
			size_t n = in_vec1.size();
			if ( in_vec2.size() != n ) throw AQLCoreInvalidData("size inconsistent", __FILE__, __LINE__);
			vector<vector<T*> > data(n, vector<T*>(2));
			for ( size_t i = 0; i < n; i++ )
			{
				data[i][0] = const_cast<double*>(&(in_vec1[i]));
				data[i][1] = const_cast<double*>(&(in_vec2[i]));
			}
			class comp {public: bool operator()(const vector<T*>& u1,const vector<T*>& u2){return *u1[0]<*u2[0];}};
			sort(data.begin(), data.end(), comp());
			out_vec1.resize(n);
			out_vec2.resize(n);
			for ( size_t i = 0; i < n; i++ )
			{
				out_vec1[i] = *data[i][0];
				out_vec2[i] = *data[i][1];
			}
		}

		//
		template <typename T> static double mean(const vector<T>& x)
		{
			if (x.size() == 0) return 0.0;
			return accumulate(x.begin(), x.end(), 0.0) / x.size();
		}

		//
		template <typename T> static double stdDev(const vector<T>& x)
		{
			if (x.size() == 0) return 0.0;
			double Mean = mean(x);
			double MeanSQ = inner_product(x.begin(), x.end(), x.begin(), 0.0) / x.size();

			return sqrt(MeanSQ - Mean * Mean);
		}

		//
		template <typename T> static T gcd(T u, T v)
		{
			if (v <= u)
			{
				if (u % v == 0) return v;
				else return gcd(v, u % v);
			}
			else
			{
				if (v % u == 0) return u;
				else return gcd(u, v % u);
			}
		}

		//
		template <typename T> static T abs_( T x )
		{
			return 0 <= x ? x : -x;
		}

		//
		template <typename T1, typename T2> static T1 sign(T1 a, T2 b)
		{
			return 0 <= b ? abs_(a) : -abs_(a);
		}

		//
		template <typename T> static T sign(T x)
		{
			return sign(1., x);
		}

		//Computes sqrt(a^2+b^2) without destructive underflow or overflow
		template <typename T1, typename T2> static double pythag(T1 a, T2 b)
		{
			T1 absa = abs_(a);
			T2 absb = abs_(b);
			double temp;
			if (absb < absa)
			{
				temp = static_cast<double>( absb ) / absa;
				return absa * sqrt(1.0 + temp * temp);
			}
			else
			{
				if ( absb == 0 ) return 0.0;
				temp = static_cast<double>( absa ) / absb;
				return absb * sqrt(1.0 + temp * temp);
			}
		}

		//
		template <typename T1, typename T2> static void shiftVec(vector<T1>& x, T2 shiftX)
		{
			typedef typename vector<T1>::iterator T1_iterator;
			T1_iterator iter;
			for (iter = x.begin(); iter != x.end(); iter++)  *iter += static_cast<T1>( shiftX );
		}

		//
		//startPos = 0 (default) for scaling from the beginning of vector
		template <typename T1, typename T2> static void scaleVec(vector<T1>& x, T2 scaleX, int startPos = 0)
		{
			typedef typename vector<T1>::iterator T1_iterator;
			T1_iterator iter;
			for (iter = x.begin() + startPos; iter != x.end(); iter++)  *iter *= static_cast<T1>( scaleX );        
		}

		//
		template <typename T> static void takeColRow(vector<vector<T> >& x, int take_cols = 0, int take_rows = 0)
		{
			int i;
			int row_num, col_num;
			row_num = static_cast<int>(x.size());
			col_num = static_cast<int>(x[0].size());

			if ((abs_(take_rows) < row_num) && (0 < take_rows))
			{
				x.erase(x.begin() + take_rows, x.end());    
				row_num = take_rows;
			}

			if ((abs_(take_rows) < row_num) && (take_rows < 0))
			{
				x.erase(x.begin(), x.end() - 1 + take_rows);    
				row_num = abs_(take_rows);    
			}

			if ((abs_(take_cols) < col_num) && (0 < take_cols))
			{
				for ( i = 0; i < row_num; i++ )
				{
					x[i].erase(x[i].begin() + take_cols, x[i].end());    
				}
			}

			if ((abs_(take_cols) < col_num) && (take_cols < 0))
			{
				for ( i = 0; i < row_num; i++ )
				{
					x[i].erase(x[i].begin(), x[i].end() - 1 + take_cols);    
				}
			}
		}

		//
		template <typename T> static void takeColRow(const vector<vector<T> >& x, vector<vector<T> >& y, int take_cols = 0, int take_rows = 0)
		{
			y = x;
			takeColRow(y, take_cols, take_rows);
		}


		// from util.h
		template<typename T> inline bool isNan(T value)
		{
			return value != value;
		}

		template<typename T> inline bool isInf(T value)
		{
			return numeric_limits<T>::has_infinity && value == numeric_limits<T>::infinity();
		}

		template <typename T1, typename T2> static bool eq(T1 x, T2 y, double eps = 10. * numeric_limits<double>::epsilon() )
		{
			return fabs( static_cast<double>(x - y) ) <= eps;
		}

		template <typename T> static T max_(T x, T y, double eps = 10. * numeric_limits<double>::epsilon() )
		{
			return !eq(x,y,eps) && x <= y ? y : x;
		}

		template <typename T> static T min_(T x, T y, double eps = 10. * numeric_limits<double>::epsilon() )
		{
			return eq(x,y,eps) || x > y ? y : x;
		}

		//
		template <typename T1, typename T2> static vector<T2> map2vector( const map<T1, T2>& x )
		{
			vector<T2> ret;
			typedef typename map<T1, T2>::const_iterator Map_T1_T2_iterator;
			Map_T1_T2_iterator itr = x.begin();
			while ( itr != x.end() )
			{
				ret.push_back( itr->second );
				itr++;
			}
			if ( ret.size() == 0 ) ret.resize(1,0);
			return ret;
		}

		template <typename T> static void printVector( const vector<T> x, const string s = "" )
		{
			cout << s << "\t";
			for ( size_t i = 0; i < x.size(); i++)
			{
				cout << setw(9) << x[i] << "\t";
			}
			cout << endl;
		}

		// Creates string from given stream. Especially useful to convert number to string.
		//template<typename T> static const AQLString n2s(const T x)
		template<typename T> static const string n2s(const T x)
		{
			stringstream str_stream;
			str_stream.precision(15); 
			str_stream << x;
			return str_stream.str().c_str();
		}
		
};

