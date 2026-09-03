/*! @file
    @brief definition of the delta/gammna conversion matrix type.
 */

#ifndef AQLPriceIMMFwdRiskConversionMatrix_h
#define AQLPriceIMMFwdRiskConversionMatrix_h


#include "AQLCoreTemplateType.h"

class AQLPriceIMMFwdRiskConversionMatrix
{
public:
	AQLPriceIMMFwdRiskConversionMatrix(
		const AQLStringVector& terms,
		const DateVector& dates,
		const DateVector& immGridDates,
		const DoubleMatrix& dfds,
		const DoubleMatrix& dsdf,
		const DoubleMatrix& dsdfExpanded);

	const AQLStringVector& getTerms() const;
	const DateVector& getDates() const;
	const DateVector& getIMMGridDates() const;
	const DoubleMatrix& getDfDs() const;
	const DoubleMatrix& getDsDf() const;
	const DoubleMatrix& getDsDfExpanded() const;

	DoubleVector apply(const AQLStringVector& riskTerms, const DoubleVector& riskValues) const;

	static DoubleMatrix calcInverseMatrix(const DoubleMatrix &original);
	static DoubleMatrix expand(const DoubleMatrix& dsdf, const DateVector& gridDates, const DateVector& immGridDates);

	// @Description: Re-distributes a certain quantity attributed to original intervals to other intervals
	//  @param original [in] The array of original quantity;
	//                       original[i] is attributed to the interval [gridDates[i], gridDates[i + 1]) for i = 0, 1, ..., gridDates.size() - 2,
	//                       and original[i] is ignored for i which is greater than gridDates.size() - 2
	//  @param gridDates [in] The array of dates which are the start dates and/or the end dates of the intervals
	//                        to which original[i] are attributed in the way above
	//  @param immGridDates[in] The array of dates which are the start dates and/or the end dates of the intervals
	//                          to which returnValue[i] are attributed in the way below
	//  @return The array of re-distributed quantity whose size is equal to immGridDates.size() - 1;
	//          returnValue[0] is attributed to the interval (-Inf, immGridDates[1]),
	//          returnValue[i] is attributed to the interval[immGridDates[i], immGridDates[i + 1]) for i = 1, ..., immGridDates.size() - 3, and
	//          returnValue[immGridDates.size() - 2] is attributed to the interval [immGridDates[immGridDates.size() - 2], +Inf)
	static DoubleVector expand(const DoubleVector& original, const DateVector& gridDates, const DateVector& immGridDates);

private:
	AQLStringVector mTerms;
	DateVector mDates;
	DateVector mIMMGridDates;
	DoubleMatrix mDfDs;
	DoubleMatrix mDsDf;
	DoubleMatrix mDsDfExpanded;
};

#endif
