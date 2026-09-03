#include <cassert>

#include "LAPriceIMMFwdRiskConversionMatrix.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

LAPriceIMMFwdRiskConversionMatrix::LAPriceIMMFwdRiskConversionMatrix(
	const LAStringVector& terms,
	const DateVector& dates,
	const DateVector& immGridDates,
	const DoubleMatrix& dfds,
	const DoubleMatrix& dsdf,
	const DoubleMatrix& dsdfExpanded) :
	mTerms(terms),
	mDates(dates),
	mIMMGridDates(immGridDates),
	mDfDs(dfds),
	mDsDf(dsdf),
	mDsDfExpanded(dsdfExpanded)
{
}

const LAStringVector& LAPriceIMMFwdRiskConversionMatrix::getTerms() const
{
	return mTerms;
}

const DoubleMatrix& LAPriceIMMFwdRiskConversionMatrix::getDsDfExpanded() const
{
	return mDsDfExpanded;
}

const DateVector& LAPriceIMMFwdRiskConversionMatrix::getDates() const
{
	return mDates;
}

const DateVector& LAPriceIMMFwdRiskConversionMatrix::getIMMGridDates() const
{
	return mIMMGridDates;
}

const DoubleMatrix& LAPriceIMMFwdRiskConversionMatrix::getDfDs() const
{
	return mDfDs;
}

const DoubleMatrix& LAPriceIMMFwdRiskConversionMatrix::getDsDf() const
{
	return mDsDf;
}

DoubleVector LAPriceIMMFwdRiskConversionMatrix::apply(const LAStringVector& riskTerms, const DoubleVector& riskValues) const
{
	DoubleVector immFwdDeltas(getDsDfExpanded().front().size());

	LAStringVector::size_type m = 0;
	DoubleVector::size_type riskValueStart = (riskTerms.size() == riskValues.size()) ? 0/* no parallel*/ : 1/* with parallel*/;
	for (LAStringVector::size_type l = 0, lend = getTerms().size(); l < lend; ++l)
	{
		while ((m < riskTerms.size()) && (riskTerms[m] != getTerms()[l]))
		{
			++m;
		}
		if (m == riskTerms.size())
		{
			continue;
		}
		for (DoubleVector::size_type k = 0, kend = immFwdDeltas.size(); k < kend; ++k)
		{
			immFwdDeltas[k] += riskValues[m + riskValueStart] * getDsDfExpanded()[l][k];
		}
	}

	return immFwdDeltas;
}

DoubleMatrix
LAPriceIMMFwdRiskConversionMatrix::calcInverseMatrix(const DoubleMatrix &original)
{
	using namespace boost::numeric;
	typedef ublas::permutation_matrix<size_t> pmatrix;

	// Copy original into a matrix of uBLAS
	ublas::matrix<double> mat(original.size(), original[0].size());
	for (DoubleMatrix::size_type i = 0U, iend = original.size(); i < iend; ++i)
	{
		const DoubleVector &row = original[i];
		for (DoubleVector::size_type j = 0U, jend = row.size(); j < jend; ++j)
		{
			assert(i < mat.size1());
			assert(j < mat.size2());
			mat(i, j) = row[j];
		}
	}

	bool bad = false;
	ublas::matrix<double> inverse(mat.size1(), mat.size1());
	try {
		pmatrix pm(mat.size1());
		ublas::lu_factorize(mat, pm);
		inverse.assign(ublas::identity_matrix<double>(mat.size1()));
		ublas::lu_substitute(mat, pm, inverse);

		// Sanity check
		double maxAbs = 0.0;
		for (ublas::matrix<double>::size_type i = 0U, iend = inverse.size1(); i < iend; ++i)
		{
			for (ublas::matrix<double>::size_type j = 0U, jend = inverse.size2(); j < jend; ++j)
			{
				maxAbs = std::max(maxAbs, std::fabs(inverse(i, j)));
			}
		}
		if (maxAbs >= 10.0)
		{
			bad = true;
		}
	}
	catch (const ublas::internal_logic&)
	{
		bad = true;
	}
	if (bad)
	{
		throw LACoreInvalidData("Arithmetic error occurred during creating the IMM forward risk conversion matrix",
			__FILE__, __LINE__);
	}

	// Copy inverse into a DoubleMatrix
	DoubleMatrix ret(inverse.size1());
	for (ublas::matrix<double>::size_type i = 0U, iend = inverse.size1(); i < iend; ++i)
	{
		ret.resize(i + 1);
		ret.back().resize(inverse.size2());
		for (ublas::matrix<double>::size_type j = 0U, jend = inverse.size2(); j < jend; ++j)
		{
			ret.back()[j] = inverse(i, j);
		}
	}

	return ret;
}

// dsdf (param) : a column vector is for a scenario (that is, a period between two dates' elements) and a row vector for a forward rate period
// returned matrix: a column vector is for a forward rate period (that is, an IMM period) and a row vector for a scenario
DoubleMatrix
LAPriceIMMFwdRiskConversionMatrix::expand(const DoubleMatrix& dsdf, const DateVector& gridDates, const DateVector& immGridDates)
{
	DoubleMatrix dsdfExpanded(dsdf.front().size());
	for (DoubleMatrix::iterator i = dsdfExpanded.begin(), iend = dsdfExpanded.end(); i != iend; ++i)
	{
		i->resize(immGridDates.size());
	}

	for (DateVector::size_type i = 1U, iend = gridDates.size(); i < iend; ++i)
	{
		const LADate startDate = gridDates[i - 1];
		const LADate endDate = gridDates[i];
		DateVector::size_type s = std::upper_bound(immGridDates.begin(), immGridDates.end(), startDate) - immGridDates.begin();

		// For each i that
		//  - [immGridDates[i - 1], immGridDates[i]) (i = s, s + 1, ..., immGridDates.size() - 1) or
		//  - [immGridDates[i - 1], +Inf           ) (i =                immGridDates.size()    )
		// has an intersection with [startDate, endDate),
		// collect i and the length of the intersection
		std::vector<std::pair<DateVector::size_type, int>> gridsConcerned;
		int allIntersectionDays = 0;
		while ((s <= immGridDates.size()) && (immGridDates[s - 1] < endDate))
		{
			const int intersectionDays = (s < immGridDates.size()) ?
				std::max(immGridDates[s - 1], startDate).intervalDays(std::min(immGridDates[s], endDate)) :
				std::max(immGridDates[s - 1], startDate).intervalDays(                          endDate ) ;
			gridsConcerned.push_back(std::make_pair(s - 1, intersectionDays));
			allIntersectionDays += intersectionDays;
			++s;
		}

		if (allIntersectionDays > 0)
		{
			// Divide ds/df proportionally to the intersection lengths
			for (DateVector::size_type j = 1, jend = gridDates.size(); j < jend; ++j)
			{
				for (std::vector<std::pair<DateVector::size_type, int>>::const_iterator
					k = gridsConcerned.begin(), kend = gridsConcerned.end(); k != kend; ++k)
				{
					dsdfExpanded[j - 1][k->first] += dsdf[i - 1][j - 1] * k->second / allIntersectionDays;
				}
			}
		}
		else
		{
			// Impose only to the first/last IMM grid
			DoubleVector::size_type k = (endDate <= immGridDates.front()) ? 0 : immGridDates.size() - 1;
			for (DateVector::size_type j = 1, jend = gridDates.size(); j < jend; ++j)
			{
				dsdfExpanded[j - 1][k] += dsdf[i - 1][j - 1];
			}
		}
	}
	return dsdfExpanded;
}

/*!
    @Description: Re-distributes a certain quantity attributed to original intervals to other intervals
    @param original [in] The array of original quantity;
                         original[i] is attributed to the interval [gridDates[i], gridDates[i + 1]) for i = 0, 1, ..., gridDates.size() - 2,
                         and original[i] is ignored for i which is greater than gridDates.size() - 2
    @param gridDates [in] The array of dates which are the start dates and/or the end dates of the intervals
                          to which original[i] are attributed in the way above
    @param immGridDates[in] The array of dates which are the start dates and/or the end dates of the intervals
                            to which returnValue[i] are attributed in the way below
    @return The array of re-distributed quantity whose size is equal to immGridDates.size();
            returnValue[0] is attributed to the interval (-Inf, immGridDates[1]),
            returnValue[i] is attributed to the interval[immGridDates[i], immGridDates[i + 1]) for i = 1, ..., immGridDates.size() - 2, and
            returnValue[immGridDates.size() - 1] is attributed to the interval [immGridDates[immGridDates.size() - 1], +Inf)
*/
DoubleVector
LAPriceIMMFwdRiskConversionMatrix::expand(const DoubleVector& original, const DateVector& gridDates, const DateVector& immGridDates)
{
	DoubleVector expanded(immGridDates.size());

	for (DateVector::size_type i = 1, iend = gridDates.size(); i < iend; ++i)
	{
		// Original interval
		const LADate startDate = gridDates[i - 1];
		const LADate endDate = gridDates[i];

		// Original value
		const double v = original[i - 1];

		// The smallest number which meets immGridDates[s - 1] <= startDate < immGridDates[s]
		DateVector::size_type s = std::upper_bound(immGridDates.begin(), immGridDates.end(), startDate) - immGridDates.begin();

		// For each i that
		//  - [immGridDates[i - 1], immGridDates[i]) (i = s, s + 1, ..., immGridDates.size() - 1) or
		//  - [immGridDates[i - 1], +Inf           ) (i =                immGridDates.size()    )
		// has an intersection with [startDate, endDate),
		// collect i and the length of the intersection

		// Collect all i for which [immGridDates[i - 1], immGridDates[i]) has an intersection
		// with [startDate, endDate) and the lengths of the intersections
		std::vector<std::pair<DateVector::size_type, int>> gridsConcerned;
		int totalIntersectionDays = 0;
		while ((s <= immGridDates.size()) && (immGridDates[s - 1] < endDate))
		{
			const int intersectionDays = (s < immGridDates.size()) ?
				std::max(immGridDates[s - 1], startDate).intervalDays(std::min(immGridDates[s], endDate)) :
				std::max(immGridDates[s - 1], startDate).intervalDays(                          endDate ) ;
			gridsConcerned.push_back(std::make_pair(s - 1, intersectionDays));
			totalIntersectionDays += intersectionDays;
			++s;
		}

		if (totalIntersectionDays > 0)
		{
			// Divide original quantity proportionally to the intersection lengths
			for (std::vector<std::pair<DateVector::size_type, int>>::const_iterator
				k = gridsConcerned.begin(), kend = gridsConcerned.end(); k != kend; ++k)
			{
				expanded[k->first] += v * k->second / totalIntersectionDays;
			}
		}
		else if (endDate <= immGridDates[0])
		{
			// Impose only to the first IMM grid
			expanded[0] += v;
		}
		else
		{
			// Impose only to the last IMM grid
			expanded[expanded.size() - 1] += v;
		}
	}

	return expanded;
}
