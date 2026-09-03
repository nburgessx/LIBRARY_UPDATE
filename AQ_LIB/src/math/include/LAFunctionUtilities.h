#ifndef LAFunctionUtilities_h
#define LAFunctionUtilities_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
#include "LAString.h"
#include "LANumericalConstants.h"
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

#define chgrow(mat,str,c)(LAFunctionUtilities::findElement(mat,str,0,c,true))
#define chgcol(mat,str,r)(LAFunctionUtilities::findElement(mat,str,r,0,false))
#define upper(str)(LAFunctionUtilities::upperdelspace(str))
#define uppervec(vec)(LAFunctionUtilities::upperdelspace(vec))
#define lowervec(vec)(LAFunctionUtilities::lowerdelspace(vec))
#define uppermat(mat)(LAFunctionUtilities::upperdelspace(mat))

class LAFunctionUtilities
{
public:
	//change str delete space and upper 
	static void upperdelspace(LAString& input);
	//change str delete space and lower 
	static void lowerdelspace(LAString& input);
	//change str-vector delete space and upper 
	static void upperdelspace(LAStringVector& input);
	//change str-vector delete space and lower 
	static void lowerdelspace(LAStringVector& input);
	//change str-matrix delete space and upper 
	static void upperdelspace(LAStringMatrix& mat);
	static LAString getEntityPoolName(LADataInstance* dataInstance);
	//static void changeStringMatrixRows(LAStringMatrix &mat, LAString& inputs);
	static const int  findRowsNumber(const LAStringMatrix& defmat, const LAString& input);
	static const int  findColsNumber(const LAStringMatrix& defmat, const LAString& input);
	static LAString findElement(const LAStringMatrix& mat, const LAString& input, 
								const int row, const int col, bool ischangeroworcol, bool isemptyerror = true);
    // Overload with simplified inputs
    static LAString findElement(const LAStringMatrix& matrix, const LAString& key);

    static bool StringToBool(LAString s);

private:
	LAFunctionUtilities(void);
	~LAFunctionUtilities(void);
	LAFunctionUtilities(const LAFunctionUtilities &rhs);
	LAFunctionUtilities &operator=(const LAFunctionUtilities &rhs);
};

bool TryFind(const LAString& searchedItem, const vector<LAString>& inVector, size_t& foundIdx);

bool TryFind(const LAString& searchedItem, const LAString& searchedString, size_t& foundIdx);

template<typename T>
vector<T> MergeVectors(const vector<T>& v1, const vector<T>& v2)
{
    vector<T> target = v1;
    for (size_t i = 0; i < v2.size(); i++)
        target.push_back(v2[i]);
    return target;
}

template <typename T1=double, typename T2=double>
class EqualityOperator
{
public:
    explicit EqualityOperator(double eps_ = 10.0 * AQ_MACHINE_EPSILON) : eps(eps_){}
    bool operator()(const T1& x, const T2& y) const { return eq(x, y, eps); }

private:
    double eps;
};

template<typename T>
vector<T> EliminateDuplicates(const vector<T>& v, T eps)
{
    vector<T> target = v;
    EliminateDuplicates_(target, EqualityOperator<>(eps));
    return target;
}

template<typename T>
vector<T> EliminateDuplicates(const vector<T>& v)
{
    vector<T> target = v;
    EliminateDuplicates_(target);
    return target;
}

template<class C>
void EliminateDuplicates_(C& c)
{
    sort(c.begin(), c.end());
    typename C::iterator p = unique(c.begin(), c.end());
    c.erase(p, c.end());
}

template<class C, class Pr>
void EliminateDuplicates_(C& c, Pr comp)
{
    sort(c.begin(), c.end());
    typename C::iterator p = unique(c.begin(), c.end(), comp);
    c.erase(p, c.end());
}

#endif 
