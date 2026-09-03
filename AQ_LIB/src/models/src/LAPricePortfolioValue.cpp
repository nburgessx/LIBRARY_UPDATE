/*! @file
    @brief Source code for class to evaluate portfolio.

*/
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPricePortfolioValue.h"
#include "LAPriceTradeValue.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADate.h"
#include "LADataValuation.h"
#include "LADataProcedure.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObjectHolder.h"
#include "LAMathDefine.h"
#include "LAPriceDataFunction.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathValuableEntity.h"

#include "LABasic.h"
#include "LAFunctionBase.h"
#include "LAMathFXEntity.h"
#include "LAMathDateUtilities.h"

#include "LAMatrix.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathVolatility.h"
#include "LAMathVolFuncIRSABR.h"
#include "LAMathSABR.h"
#include "LABlackScholesBaseFunc.h"
#include "LAPriceIMMFwdRiskConversionMatrix.h"
#include "LAPriceIRVegaUnderlyingAsset.h"
#include "LACoreComponentManager.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
#include <time.h>

using namespace std;

#define PV	"PV"
#define PVANDRISK	"PVANDRISK"
#define RISK	"RISK"

#define ANALYTIC "ANALYTIC"
#define SEMIANALYTIC "SEMIANALYTIC"

#ifndef SWAP
#define SWAP		"SWAP"
#endif

namespace
{
	/*!
	    @Description: Deduces the IMM dates from the risk object
	    @param info [in] The risk object which shall contain "IMMFwdRateTerm" data
	    @param baseDate [in] The base date
	    @return A vector which contains all IMM dates, which does not contain the base date itself
	*/
	DateVector
	deduceImmDates(const LAObject& info, const LADate& baseDate)
	{
		const LADataHolder* dh;

		dh = &info.getData(PRICING_DATA_IMMFWDRATETERM, ISNOTNULL);
		const IntVector& immIndices = dynamic_cast<const LADataInts&>(dh->get()).get();

		DateVector dates;
		LAString sld = "NO_CHANGE";
		LAString cal = "";

		dates.resize(immIndices.size());
		for (DateVector::size_type i = 0, iend = dates.size(); i < iend; ++i)
		{
			dates[i] = LAMathDateUtilities::getIMMDate3(baseDate, immIndices[i], cal, sld);
		}

		return dates;
	}

	/*!
	    @Description: Deduce the start and end dates of the period which is bound to the specified term
	    @param term [in] A string expression of the term, for example "ON", "1M", "1X7", and "10Y"
	    @param objPool [in] The object pool
	    @param baseYieldName [in] The name of the yield curve object, for example "YIELD_SDE_JPY_IR"
	    @param baseDate [in] The base date
	    @param spotDate [in] The spot-corresponding date of the base date
	    @param yieldCurve [in] The yield curve
	    @return A pair of the start date and the end date of the period
	*/
	std::pair<LADate, LADate>
	getPeriod(const LAString& term, const LAObjectPool& objPool,
		const LAString& baseYieldName, const LADate& baseDate, const LADate& spotDate, const LAMathYieldCurve* yieldCurve)
	{
		if (isdigit(static_cast<int>(term.getCString()[0])))
		{
			// Starts with a digit
			const char* x = strchr(term.getCString(), 'X');
			if (x)
			{
				// Maybe FRA
				const std::size_t ix = x - term.getCString();
				LAString start = term.subString(0, ix - 1) + "M";
				LAString end = term.subString(ix + 1, term.size() - 1) + "M";
				const LADate startDate = LAMathDateCalculations::getDate(
					spotDate, start, yieldCurve->getSlidingRule(), &yieldCurve->getCalendar(), true);
				const LADate endDate = LAMathDateCalculations::getDate(
					spotDate, end, yieldCurve->getSlidingRule(), &yieldCurve->getCalendar(), true);
					startDate.dayOfWeek();
				return std::make_pair(startDate, endDate);
			}
			else
			{
				const char unit = term.getCString()[term.size() - 1];
				switch (unit)
				{
				case 'D':
					{
						LADataInt n;
						n.convertFromString((term + "").subString(0, term.size() - 2));
						LADate d = spotDate;
						d.addDays(n.get());
						return std::make_pair(LADate(), yieldCurve->getSlidingRule().getDate(d, yieldCurve->getCalendar()));
					}
				default:	// W, M or Y
					return std::make_pair(LADate(), LAMathDateCalculations::getDate(
						spotDate, term, yieldCurve->getSlidingRule(), &yieldCurve->getCalendar(), true));
				}
			}
		}
		else if ((term == "ON") || (term == "TN"))
		{
			return std::make_pair(LADate(), LAMathDateCalculations::getDate(
				baseDate, term, yieldCurve->getSlidingRule(), &yieldCurve->getCalendar(), true));
		}
		else
		{
			// Maybe future
			for (EntityConstIter j = objPool.begin(); j != objPool.end(); ++j)
			{
				LAString x = j->first.subString(baseYieldName.size(), baseYieldName.size() + 12);
				if ((j->first.subString(0, baseYieldName.size() - 1) == baseYieldName)
					&& j->second.getData("Term").isDefined()
					&& j->second.getData("StartDate").isDefined()
					&& j->second.getData("EndDate").isDefined())
				{
					if (dynamic_cast<const LADataString&>(j->second.get().getData("Term").get()).get() == term)
					{
						return std::make_pair(
							dynamic_cast<const LADataDate&>(j->second.get().getData("StartDate").get()),
							dynamic_cast<const LADataDate&>(j->second.get().getData("EndDate").get()));
					}
				}
			}
		}
		return std::make_pair(LADate(), LADate());
	}

	/*!
	    @Description: Deduces the risk grids from the risk object
	    @param info [in] The risk object which shall contain "RiskGrid" data
	    @param outputName [in] The output name, for example "jpy_delta"
	    @return A vector which contains all risk grid string expressions
	*/
	LAStringVector
	getRiskGrids(const LAObject& info, const LAString& outputName)
	{
		LAString outputNameU(outputName);
		LAString outputNameL(outputName);
		outputNameU.toUpper();
		outputNameL.toLower();

		const LADataHolder* dh;
		dh = &info.getData(outputNameL + AP_CALIBRATION_DATA_RISK_GRID);
		if (!dh->isDefined())
		{
			dh = &info.getData(outputNameU + AP_CALIBRATION_DATA_RISK_GRID, ISDEFINED);
		}
		LAStringVector riskGrids = dynamic_cast<const LADataStrings&>(dh->get()).get();
		for (LAStringVector::size_type i = 0, iend = riskGrids.size(); i < iend; ++i)
		{
			LAString& riskGrid = riskGrids[i];
			size_t justAfterLastUnderscore = std::find(
				std::reverse_iterator<const char*>(riskGrid.getCString() + riskGrid.size()),
				std::reverse_iterator<const char*>(riskGrid.getCString()),
				'_').base() - riskGrid.getCString();
			riskGrid = riskGrid.subString(justAfterLastUnderscore, riskGrid.size() - 1).toUpper();
		}
		return riskGrids;
	}

	/*!
	    @Description: Tells whether two curves gives different DFs at term t
	    @param curve1 [in] Yield curve #1
	    @param curve2 [in] Yield curve #2
	    @param t [in] The term on which two DFs are compared, in years
	    @return true if two curves gives different DFs at term t, false otherwise
	*/
	bool
	curvesDifferAt(const LAMathYieldCurve &curve1, const LAMathYieldCurve &curve2, double t)
	{
		const double df1 = curve1.getDF(t);
		const double df2 = curve2.getDF(t);
		return std::fabs(df2 - df1) > 1.0e-15;
	}

	/*!
	    @Description: Calculates forward rates between all adjacent elements of dates
	    @param dates [in] The array of the dates, which shall be sorted in ascending order
	    @param yieldCurve [in] The yield curve
	    @return The calculated forward rates; This vector is shorter than dates (parameter)
	            by one
	*/
	DoubleVector
	calcFwdSimpleRates(const DateVector& dates, const LAMathYieldCurve& yieldCurve)
	{
		LADate date0 = dates.front();
		DateVector::const_iterator i = dates.begin();
		++i;
		DateVector::const_iterator iend = dates.end();
		DoubleVector fwdRates;
		while (i != iend)
		{
			const LADate date1 = *i;
			const double d = date0.intervalDays(date1) / 365.0;
			const double df = (date0.cmp(date1) < 0) ? yieldCurve.getDF(date0, date1) : yieldCurve.getDF(date1, date0);
			fwdRates.push_back((1.0 / df - 1.0) / d);
			if (_isnan(fwdRates.back()))
			{
				throw LACoreError(
					("NaN forward rate: " + yieldCurve.getName().get() +
						" at (" + date0.convertDateToString().getCString() + ", "
						+ date1.convertDateToString().getCString() + ")").getCString(),
					__FILE__, __LINE__);
			}
			date0 = date1;
			++i;
		}
		return fwdRates;
	}

	/*!
	    @Description: Collects used (= the bumped curve gives different DFs from the pre-bump one) grids
	                  and their corresponding curves
	    @param info [in] The risk object
	    @param scenarios [in] A reference which points the scenario entities
	    @return A vector of pairs whose first elements are string expressions of the terms and
	            the second elements are the corresponding yield curves;
	            Pairs are sorted in the same order as "terms" and have "Base" as the first element
	*/
	std::pair<std::vector<std::pair<LAString, LAMathYieldCurve*> >, LAString>
	getUsedTerms(const LAObject& info, const LADataMultiReference& scenarios, const LAStringVector& terms)
	{
		const LADataHolder* dh;

		// For example, "YIELD_SDE_JPY_IR"
		dh = &info.getData(PRICING_DATA_TARGETNAMES, ISDEFINED);
		LAString targetName = dynamic_cast<const LADataStrings&>(dh->get()).get().front();

		// For example, "JPYOIS"
		dh = &info.getData(PRICING_DATA_IMMRISKYIELDCURVENAME, ISDEFINED);
		LAStringVector curveNames = dynamic_cast<const LADataStrings&>(dh->get()).get();
		if (curveNames.size() == 1)
		{
			LAString curveName = curveNames.front();
			curveNames.assign(terms.size(), curveName);
		}
		else if (curveNames.size() != terms.size())
		{
			throw LACoreInvalidData("Risk grids and risk curve names do not match in terms of their number of members", __FILE__, __LINE__);
		}

		dh = &info.getData(PRICING_DATA_ISWAVE, ISDEFINED);
		bool isWave = dynamic_cast<const LADataBool&>(dh->get()).get();

		LADataInstance& dataInstance = *info.getDataInstance();
		LAObjectPool& objPool = dataInstance.getObjectPool();

		// terms[i]'s corresponding post-bumping curves
		std::vector<LAMathYieldCurve*> curves(terms.size());
		int scenarioStart = (terms.size() == scenarios.getSize()) ? 0/* no parallel */ : 1/*with parallel*/; 
		for (LAStringVector::size_type i = 0, iend = terms.size(); i < iend; ++i)
		{
			curves[i] = &dynamic_cast<LAMathYieldCurve&>(scenarios.get(i + scenarioStart).get());
		}

		LAMathYieldCurve* curveBase = &dynamic_cast<LAMathYieldCurve&>(objPool.getObject(targetName, ENCHKTYPE_ISDEFINED).get());

		// terms[i]'s corresponding pre-bumping curves
		std::vector<LAMathYieldCurve*> preBumpCurves(terms.size());
		if (isWave)
		{
			preBumpCurves[0] = curveBase;
			std::copy(curves.begin(), curves.end() - 1, preBumpCurves.begin() + 1);
		}
		else
		{
			std::fill(preBumpCurves.begin(), preBumpCurves.end(), curveBase);
		}

		LAString possibleRiskFloorTerm = "";

		// Whether used or not is judged by differences of DFs of bumped curves;
		// take care the diffs must be investigated in accordance with bump modes (for example, "wave")
		std::vector<std::pair<LAString, LAMathYieldCurve*> > usedTerms;
		LAStringVector riskYieldNames;
		usedTerms.push_back(std::make_pair("Base", curveBase));
		riskYieldNames.push_back(curveBase->getCurveType());
		for (LAStringVector::size_type j = 0, jend = terms.size(); j < jend; ++j)
		{
			if (curveNames[j] == "")
			{
				if (usedTerms.size() == 1)	// "Base" only
				{
					possibleRiskFloorTerm = terms[j];
				}
				continue;
			}

			LAMathYieldCurve& prebumpCurve = *preBumpCurves[j];
			LAMathYieldCurve& yieldCurve = *curves[j];

			prebumpCurve.setCurveType(curveNames[j]);
			yieldCurve.setCurveType(curveNames[j]);

			bool used = false;

			// Do DFs differ at nM where n = 1, 2,..., 18?
			for (int i = 1; i <= 18; ++i)
			{
				if (curvesDifferAt(prebumpCurve, yieldCurve, i / 12.0))
				{
					used = true;
					break;
				}
			}

			if (!used)
			{
			// Do DFs differ at nY where n = 2, 3,..., 60?
			for (int i = 2; i <= 60; ++i)
			{
				if (curvesDifferAt(prebumpCurve, yieldCurve, static_cast<double>(i)))
				{
					used = true;
						break;
					}
				}
			}

			if (used)
			{
				usedTerms.push_back(std::make_pair(terms[j], &yieldCurve));
				riskYieldNames.push_back(curveNames[j]);
			}
		}

		// Again set the curve type including curveBase
		for (LAStringVector::size_type i = 0, iend = usedTerms.size(); i < iend; ++i)
		{
			usedTerms[i].second->setCurveType(riskYieldNames[i]);
		}

		return std::make_pair(usedTerms, possibleRiskFloorTerm);
	}

	/*!
	    @Description: make an LAPriceIMMFwdRiskConversionMatrix object
	    @param info [in] THe risk object
	    @param scenarios [in] A reference which points the scenario entities
	*/
	LAPriceIMMFwdRiskConversionMatrix
	calcIMMFwdRiskConversionMatrix(const LAObject& info, const LADataMultiReference& scenarios)
	{
		const LADataHolder* dh;

		// For example, "JPY_DELTA"
		dh = &info.getData(PRICING_DATA_OUTPUTNAME, ISNOTNULL);
		LAString outputName = dynamic_cast<const LADataString&>(dh->get()).get();
		outputName.toUpper();

		// For example, { "1M", "BOJ01", ... }
		LAStringVector terms = getRiskGrids(info, outputName);

		// For example, "JPYOIS"
		dh = &info.getData(PRICING_DATA_IMMRISKYIELDCURVENAME, ISDEFINED);
		LAStringVector curveNames = dynamic_cast<const LADataStrings&>(dh->get()).get();
		if (curveNames.empty())
		{
			dh = &info.getData(PRICING_DATA_IMMFWDRATETERM, ISNOTNULL);
			const IntVector& immIndices = dynamic_cast<const LADataInts&>(dh->get()).get();
			return LAPriceIMMFwdRiskConversionMatrix(
				LAStringVector(), DateVector(), DateVector(), DoubleMatrix(), DoubleMatrix(), DoubleMatrix(1, DoubleVector(immIndices.size())));
		}
		else if (curveNames.size() == 1)
		{
			LAString curveName = curveNames.front();
			curveNames.assign(terms.size(), curveName);
		}
		else if (curveNames.size() != terms.size())
		{
			throw LACoreInvalidData("Risk grids and risk curve names do not match in terms of their number of members", __FILE__, __LINE__);
		}

		LAStringVector originalCurveTypes;
		for (unsigned int i = 0, iend = scenarios.getSize(); i < iend; ++i)
		{
			originalCurveTypes.push_back(dynamic_cast<LAMathYieldCurve&>(scenarios.get(i).get()).getCurveType());
		}

		// Pairs of used term names their corresponding yield curves,
		// ordered in the same way as terms, and have "Base" in their front
		std::pair<std::vector<std::pair<LAString, LAMathYieldCurve*> >, LAString> p
			= getUsedTerms(info, scenarios, terms);
		std::vector<std::pair<LAString, LAMathYieldCurve*> >& usedTerms = p.first;
		LAString& riskFloorTerm = p.second;

		// Deduce the spot date: there may be more direct-forward ways to get it
		LADate baseDate = dynamic_cast<const LADataDate&>(usedTerms.front().second->getData(CALIBRATION_DATA_ASOFDATE).get());
		baseDate.dayOfWeek();

		// For example, "YIELD_SDE_JPY_IR"
		dh = &info.getData(PRICING_DATA_TARGETNAMES, ISNOTNULL);
		LAString targetName = dynamic_cast<const LADataStrings&>(dh->get()).get().front();
		const LAObjectPool& objPool = usedTerms.front().second->getDataInstance()->getObjectPool();
		LADate spotDate;
		for (EntityConstIter i = objPool.begin(), iend = objPool.end(); i != iend; ++i)
		{
			if (i->first.subString(0, targetName.size() + 4) == targetName + "_DATA")
			{
				const LADataHolder& a = i->second.getData("SpotDate");
				if (a.isDefined())
				{
					spotDate = dynamic_cast<const LADataDate&>(a.get()).get();
					break;
				}
			}
		}

		// Grid dates: grids which are not used do not appear in them
		// index 0: base date
		// index 1: 1st used grid date
		// index 2: 2nd used grid date
		// ...
		// Note that ON and TN are not included
		DateVector gridDates;
		gridDates.push_back(baseDate);
		for (std::vector<std::pair<LAString, LAMathYieldCurve*> >::const_iterator
			i = usedTerms.begin() + 1, iend = usedTerms.end();	// begin() is for "base"
			i != iend; ++i)
		{
			const LAString& term = i->first;
			if ((term != "ON") && (term != "TN"))
			{
				const std::pair<LADate, LADate> period = getPeriod(term, objPool, targetName, baseDate, spotDate, i->second);
				if (period.second != LADate())
				{
					period.second.dayOfWeek();
					gridDates.push_back(period.second);
				}
			}
		}

		dh = &info.getData(PRICING_DATA_IMMRISKFLOORTERM);
		if (dh->isDefined() && !dh->isNull())
		{
			riskFloorTerm = dynamic_cast<const LADataString&>(dh->get()).get();
		}
		if (riskFloorTerm != "")
		{
			LAStringVector::size_type i = std::find(terms.begin(), terms.end(), riskFloorTerm) - terms.begin();
			LADate riskFloor = getPeriod(riskFloorTerm, objPool, targetName, baseDate, spotDate, usedTerms.front().second).second;
			if (riskFloor < gridDates[1])
			{
				riskFloor.dayOfWeek();
				gridDates.front() = riskFloor;
			}
		}

		// Calculate fwd rate diffs attributed to the corresponding bumps;
		// fwdRateDeltaMatrix[i][j] is j-th fwd rate diff by i-th market bump
		DoubleMatrix fwdRateDeltaMatrix;
		dh = &info.getData(PRICING_DATA_ISWAVE, ISNOTNULL);
		bool isWave = dynamic_cast<const LADataBool&>(dh->get()).get();
		dh = &info.getData(PRICING_DATA_IMMACTUALGRIDSHIFTVAL, ISNOTNULL);
		DoubleVector shiftVals = dynamic_cast<const LADataDoubles&>(dh->get()).get();

		DoubleVector::size_type j = 0;
		for (std::vector<std::pair<LAString, LAMathYieldCurve*> >::size_type i = 1, iend = usedTerms.size(); i < iend; ++i)	// index 0 is for "Base"
		{
			LAMathYieldCurve& prebumpCurve = *usedTerms[isWave ? (i - 1) : 0].second;
			LAMathYieldCurve& bumpedCurve = *usedTerms[i].second;
			prebumpCurve.setCurveType(bumpedCurve.getCurveType());

			// Fwd rates between gridDates[j] and gridDates[j + 1]
			DoubleVector prebumpFwdRates = calcFwdSimpleRates(gridDates, prebumpCurve);
			DoubleVector bumpedFwdRates = calcFwdSimpleRates(gridDates, bumpedCurve);

			fwdRateDeltaMatrix.resize(fwdRateDeltaMatrix.size() + 1);
			DoubleVector& fwdRateDeltas = fwdRateDeltaMatrix.back();
			fwdRateDeltas.resize(bumpedFwdRates.size());

			// Deltas are bumped fwd rates minus pre-bump fwd rates
			std::transform(bumpedFwdRates.begin(), bumpedFwdRates.end(),
				prebumpFwdRates.begin(), fwdRateDeltas.begin(), std::minus<double>());

			// ... and then divided by the shift amounts
			while (usedTerms[i].first != terms[j])
			{
				++j;
			}
			for (auto& d : fwdRateDeltas) d /= shiftVals[j];
		}

		// Hereafter we don't have any interests on Base, ON and TN
		usedTerms.erase(usedTerms.begin());	// "Base"
		for (std::vector<std::pair<LAString, LAMathYieldCurve*> >::size_type i = 0; i != usedTerms.size();)
		{
			if ((usedTerms[i].first == "ON") || (usedTerms[i].first == "TN"))
			{
				usedTerms.erase(usedTerms.begin() + i);
				fwdRateDeltaMatrix.erase(fwdRateDeltaMatrix.begin() + i);
			}
			else
			{
				++i;
			}
		}

		// df/ds matrix -> ds/df matrix
		// swapRateDeltaMatrix[i][j] is i-th swap rate diffs by j-th fwd rate bump
		DoubleMatrix swapRateDeltaMatrix = LAPriceIMMFwdRiskConversionMatrix::calcInverseMatrix(fwdRateDeltaMatrix);

		// Expand ds/df matrix to imm grids
		DateVector immGridDates = deduceImmDates(info, baseDate);
		immGridDates.insert(immGridDates.begin(), baseDate);
		DoubleMatrix swapRateDeltaMatrixExpanded = LAPriceIMMFwdRiskConversionMatrix::expand(swapRateDeltaMatrix, gridDates, immGridDates);

		LAStringVector gridTerms(usedTerms.size());
		for (LAStringVector::size_type k = 0, kend = gridTerms.size(); k < kend; ++k)
		{
			gridTerms[k] = usedTerms[k].first;
		}

		for (unsigned int i = 0, iend = scenarios.getSize(); i < iend; ++i)
		{
			dynamic_cast<LAMathYieldCurve&>(scenarios.get(i).get()).setCurveType(originalCurveTypes[i]);
		}

		return LAPriceIMMFwdRiskConversionMatrix(gridTerms, gridDates, immGridDates, fwdRateDeltaMatrix, swapRateDeltaMatrix, swapRateDeltaMatrixExpanded);
	}

	/*!
	    @Description: Retrieves market rate bump risk values from the risk object, converts them into IMM forward rate bump ones,
	                  and write them back into the object
	    @param info [in] The risk object
	    @param unders [in]
	    @param outputName [in] The output name of the market rate bump risk values, for example "jpy_delta"
	    @param matrix [in] The conversion matrix object
	*/
	void
	putIMMFwdRisksOnMarketRateBump(const LAObject& info, const LADataMultiReference &unders, const LAString& outputName, const LAPriceIMMFwdRiskConversionMatrix& matrix)
	{
		LAString outputNameU(outputName);
		outputNameU.toUpper();

		LAStringVector riskGrids = getRiskGrids(info, outputName);

		for (unsigned int i = 0, iend = unders.getSize(); i < iend; ++i)
		{
			LAObjectHolder& objHolder = unders.get(i);

			const LADataHolder* dh;
			dh = &objHolder.get().getData((outputName + "").toLower());
			const DoubleVector& riskValues = dynamic_cast<const LADataDoubles&>(dh->get()).get();

			LADataHolder aa(new LADataDoubles(matrix.apply(riskGrids, riskValues)));
			objHolder.get().add(outputNameU + "IMMFwd", aa);
		}
	}

	/*!
	    @Description: Retrieves zero rate bump risk values from the risk object, converts them into IMM forward rate bump ones,
	                  and write them back into the object
	    @param info [in] The risk object
	    @param unders [in]
	    @param outputName [in] The output name of the market rate bump risk values, for example "jpy_delta"
	*/
	void
	putIMMFwdRisksOnZeroRateBump(const LAObject& info, const LADataMultiReference &unders, const LAString& outputName)
	{
		LAObjectPool& objPool = info.getDataInstance()->getObjectPool();

		// For example, "YIELD_SDE_JPY_IR"
		const LADataHolder* dh = &info.getData(PRICING_DATA_TARGETNAMES, ISDEFINED);
		LAString targetName = dynamic_cast<const LADataStrings&>(dh->get()).get().front();

		// Get the intact yield curve
		const LAMathYieldCurve* const yieldCurve = dynamic_cast<const LAMathYieldCurve*>(&objPool.getObject(targetName).get());

		// Get the base date
		LADate baseDate = dynamic_cast<const LADataDate&>(yieldCurve->getData(CALIBRATION_DATA_ASOFDATE).get()).get();
		baseDate.dayOfWeek();

		// Get the spot date
		LADate spotDate;
		for (EntityConstIter j = objPool.begin(), jend = objPool.end(); j != jend; ++j)
		{
			if (j->first.subString(0, targetName.size() + 4) == targetName + "_DATA")
			{
				const LADataHolder& a = j->second.getData("SpotDate");
				if (a.isDefined())
				{
					spotDate = dynamic_cast<const LADataDate&>(a.get()).get();
					break;
				}
			}
		}
		spotDate.dayOfWeek();

		// Risk grids: hereafter we will call its length N
		LAStringVector riskGrids = getRiskGrids(info, outputName);

		// Dates of risk grids: its length is N+1 (the first element is the base date)
		DateVector gridDates;
		gridDates.push_back(baseDate);
		for (std::size_t j = 0, je = riskGrids.size(); j < je; ++j)
		{
			const LAString& term = riskGrids[j];
			const std::pair<LADate, LADate> period = getPeriod(term, objPool, targetName, baseDate, spotDate, yieldCurve);
			if (period.second != LADate())
			{
				period.second.dayOfWeek();
				gridDates.push_back(period.second);
			}
		}

		// Dates of IMM grids: the first element is the base date
		DateVector immGridDates = deduceImmDates(info, baseDate);
		immGridDates.insert(immGridDates.begin(), baseDate);

		for (unsigned int j = 0, iend = unders.getSize(); j < iend; ++j)
		{
			LAObjectHolder& objHolder = unders.get(j);

			// Zero rate bump risk values: its length is N (without parallel) or N+1 (with parallel)
			dh = &objHolder.get().getData(LAString(outputName).toLower());
			const DoubleVector& riskValues = dynamic_cast<const LADataDoubles&>(dh->get()).get();

			// Forward rate bump risk values: its length is unconditionally N+1
			DoubleVector fwdRiskValues(riskGrids.size() + 1);

			// The first element of the grid-wise zero rate bump risk values:
			// we define this to exclude parallel
			const double* zeroRiskValues = &riskValues.front() + riskValues.size() - riskGrids.size();

			// The first element of the risk grid dates:
			// we define this to exclude the base date
			const LADate* ds = &gridDates[0] + 1;	// To exclude baseDate

			const std::size_t n = riskGrids.size();

			// Convert zero rate bump risk values into forward rate bump risk values grid-wisely
			fwdRiskValues[n    ] = 0.0;
			fwdRiskValues[n - 1] = zeroRiskValues[n - 1] * ds[n - 2].intervalDays(ds[n - 1]) / baseDate.intervalDays(ds[n - 1]);
			for (std::size_t k = n - 2; k > 0; --k)
			{
				fwdRiskValues[k] = (zeroRiskValues[k] / baseDate.intervalDays(ds[k]) + fwdRiskValues[k + 1] / ds[k].intervalDays(ds[k + 1])) * ds[k - 1].intervalDays(ds[k]);
			}
			fwdRiskValues[0] = zeroRiskValues[0] + fwdRiskValues[1] * baseDate.intervalDays(ds[0]) / ds[0].intervalDays(ds[1]);

			// Expand forward rate bump risk values into IMM grids
			const DoubleVector immFwdRiskValues = LAPriceIMMFwdRiskConversionMatrix::expand(fwdRiskValues, gridDates, immGridDates);

			// Now we have done
			LADataHolder aa(new LADataDoubles(immFwdRiskValues));
			objHolder.get().add(LAString(outputName).toUpper() + "IMMFwd", aa);
		}
	}

	/*!
	    @Description: Retrieves IR vega values from the underlying entities,
	                  converts them in accordance with instructions contained the risk object,
	                  write the converted IR vegas back into the underlying entities
	                  and modify some dataValues of the risk object so that they reflects the conversion correctly.
	    @param info [in] The risk object
	    @param unders [in] The underlying entities
	    @param outputName [in] The output name of the risk values, for example "jpy_vega"
	*/
	void
	putConvertedVega(LAObject& info, const LADataMultiReference &unders, const LAString& outputName)
	{
		LADataInstance* dataInstance = info.getDataInstance();
		LAObjectPool& objPool = dataInstance->getObjectPool();

		// "JPY_VEGA" and "jpy_vega"
		LAString outputNameU(outputName);
		outputNameU.toUpper();
		LAString outputNameL(outputName);
		outputNameL.toLower();

		// "JPY"
		LAString currencyU = dynamic_cast<const LADataString&>(info.getData("Name").get()).get().subString(0, 2);
		currencyU.toUpper();

		// AsOfDate
		const LADate baseDate = dynamic_cast<const LADataDate&>(
			objPool.getObject("YIELD_SDE_" + currencyU + "_IR_DATA").get().getData(CALIBRATION_DATA_ASOFDATE).get()).get();

		// "tibor"
		const LAString underlyingNameL = LAString(dynamic_cast<const LADataString&>(
			info.getData(PRICING_DATA_RISKCURVETYPENAME, ISNOTNULL).get()).get()).toLower();

		const LAMathVolFuncIRSABR* irVol;
		double shift;
		{
			const LAString volNameBase =
				dynamic_cast<const LADataStrings&>(info.getData("TargetNames").get()).get().front();
			const LAMathVolatility& scenarioBase =
				dynamic_cast<const LAMathVolatility&>(info.getDataInstance()->getObjectPool().getObject(volNameBase).get());
			irVol = dynamic_cast<const LAMathVolFuncIRSABR*>(scenarioBase.getVolatilityFunc());
			shift = irVol->getForwardShiftValue();
		}

		// Collect info about converted-after vols
		LADataHolder dh = info.getData(PRICING_DATA_VEGACONVERTEDSHIFTVAL);
		double shiftConverted;
		if (dh.isDefined() && !dh.isNull())
		{
			shiftConverted = dynamic_cast<const LADataDouble&>(dh.get()).get();
		}
		else
		{
			shiftConverted = shift;
		}
		const auto getDataStringL = [&info](const LAString& name, const LAString& defaultValue) -> LAString
		{
			LADataHolder ahu = info.getData(name);
			if (ahu.isDefined() && !ahu.isNull())
			{
				return LAString(dynamic_cast<const LADataString&>(ahu.get()).get()).toLower();
			}
			else
			{
				return defaultValue;
			}
		};
		// "libor"
		const LAString underlyingNameConvertedL = getDataStringL(PRICING_DATA_VEGACONVERTEDUNDERLYING, underlyingNameL);
		// "6m"
		const LAString capletTenorConvertedL = getDataStringL(PRICING_DATA_VEGACONVERTEDCAPLETTENOR, "");

		double ratioUpperBound = std::numeric_limits<double>::max();
		{
			LADataHolder ahu = info.getData(PRICING_DATA_VEGACONVERTEDRATIOUPPERBOUND);
			if (ahu.isDefined() && !ahu.isNull())
			{
				ratioUpperBound = dynamic_cast<const LADataDouble&>(ahu.get()).get();
			}
		};

		const bool shiftsDiffer = LAMath::abs(shift - shiftConverted) > 0.0001;

		const LAPriceOriginalIRSABRUnderlyingAsset underlying(baseDate, currencyU, underlyingNameL, dataInstance);

		// Black vega calcuration engine
		LABlackScholesBase* const engine = LACoreComponentManager::getBlackComponentMap().find(BK VEGA CALL)->second;

		std::shared_ptr<const LAPriceIRVegaUnderlyingAsset> underlyingConverted;
		if (underlyingNameL != underlyingNameConvertedL)
		{
			underlyingConverted.reset(new LAPriceAnotherUnderlyingAsset(baseDate, underlying.getCurveID(), info));
		}

		const LAPriceIRVegaUnderlyingAsset* underlyingTarget = underlyingConverted ?
			underlyingConverted.get() : &underlying;

		for (unsigned int j = 0, iend = unders.getSize(); j < iend; ++j)
		{
			LAObjectHolder& objHolder = unders.get(j);

			LADataHolder* dh;
			dh = &objHolder.get().getData(outputNameL);
			const DoubleVector& riskValues = dynamic_cast<const LADataDoubles&>(dh->get()).get();

			dh = &objHolder.get().getData(outputNameU + "RiskGrid");
			const LAStringVector& riskGrids = dynamic_cast<const LADataStrings&>(dh->get()).get();

			std::size_t gridRiskOffset = 0;
			if (riskValues.size() > riskGrids.size())
			{
				// Parallel vega, which is to be ignored
				gridRiskOffset = 1;
			}

			DoubleVector riskValuesConverted(riskValues.size() - gridRiskOffset);	// Each element is initialized with zero

			std::deque<bool> isCaps(riskGrids.size());
			LAStringVector opts(riskGrids.size());
			LAStringVector underlyingLs(riskGrids.size());

			// Grid vega
			for (std::size_t i = 0, ie = riskGrids.size(); i < ie; ++i)
			{
				// "Swaption_1Y_7Y" -> ("1Y", "7Y")
				const int firstDemiliter = riskGrids[i].findString('_');
				const LAString gridSpec = riskGrids[i].subString(firstDemiliter + 1, riskGrids[i].size() - 1);
				const int secondDemiliter = gridSpec.findString('_');
				opts[i] = gridSpec.subString(0, secondDemiliter - 1);
				underlyingLs[i] = gridSpec.subString(secondDemiliter + 1, gridSpec.size() - 1).toLower();

				LAString forName;
				std::tie(forName, isCaps[i]) = underlying.getCurveName(underlyingLs[i]);

				if (riskValues[i + gridRiskOffset] == 0.0)
				{
					continue;
				}

				double forward = underlying.getForward(
					opts[i], underlyingLs[i], std::make_pair(forName, isCaps[i]));

				// STEP A: conversion in terms of shift amount
				double shiftFactor = 1.0;
				if (shiftsDiffer)
				{
					double optTerm;
					double underlyingTerm;
					std::tie(optTerm, underlyingTerm) = underlying.getTerms(opts[i], underlyingLs[i], isCaps[i]);

					const double alpha = irVol->getSABRParam(SABR_ALPHA, optTerm, underlyingTerm);
					const double beta = irVol->getSABRParam(SABR_BETA, optTerm, underlyingTerm);
					const double nu = irVol->getSABRParam(SABR_NU, optTerm, underlyingTerm);
					const double rho = irVol->getSABRParam(SABR_RHO, optTerm, underlyingTerm);
					LAMathSABR_Hagan sabr(alpha, beta, nu, rho);

					// Calculation engine of shift -> premium
					auto shiftedVegaEngine = [&engine, &sabr, optTerm, forward](double shiftValue) -> double
					{
						const double shiftedForward = LAMath::max(forward + shiftValue, MIN_RATE);
						const double shiftedStrike = LAMath::max(forward + shiftValue, MIN_RATE);		// at the money

						const double vol = sabr.getSABRVol(optTerm, shiftedForward, shiftedStrike);
						AnalyticBKParam param;
						param.F = shiftedForward;
						param.K = shiftedStrike;
						param.Te = optTerm;
						param.Td = param.Te;
						param.actT = param.Te;
						param.Nu = 1.0;
						param.rd = 0.0;
						param.Vol = vol;
						return engine->calc(param);
					};

					const double undiscountedVegaFrom = shiftedVegaEngine(shift);
					const double undiscountedVegaTo = shiftedVegaEngine(shiftConverted);
					shiftFactor = undiscountedVegaTo / undiscountedVegaFrom;
				}

				// STEP B: conversion in terms of group; that is, from TIBOR to LIBOR
				double groupFactor = 1.0;
				if (underlyingConverted)
				{
					// If both TIBOR->LIBOR and 3M->6M is to take place,
					// use the final target tenor curve for caplets
					// so as not to prevent "JPY1MLFORECAST is not built" errors for TIBOR 1M vegas
					const LAString underlyingL = (isCaps[i] && (capletTenorConvertedL.size() > 0)) ?
						capletTenorConvertedL : underlyingLs[i];
					const double forwardConverted = underlyingConverted->getForward(
						opts[i], underlyingL, underlyingConverted->getCurveName(underlyingL));
					groupFactor = (forwardConverted + shiftConverted) / (forward + shiftConverted);
					forward = forwardConverted;	// may used in the next step as a "converted-from" rate
				}

				// STEP C: conversion in terms of tenor; that is, from 3M to 6M
				double tenorFactor = 1.0;
				if (isCaps[i] && (capletTenorConvertedL.size() > 0) && (underlyingLs[i] != capletTenorConvertedL))
				{
					const auto curveName = underlyingTarget->getCurveName(capletTenorConvertedL);
					const double forwardConverted =
						underlyingConverted->getForward(opts[i], capletTenorConvertedL, curveName);
					tenorFactor = (forwardConverted + shiftConverted) / (forward + shiftConverted);
				}

				const double ratio = std::min(shiftFactor * groupFactor * tenorFactor, ratioUpperBound);
				riskValuesConverted[i] = riskValues[i + gridRiskOffset] * ratio;
			}

			// Collect caplet vegas into the specified tenor
			if (capletTenorConvertedL.size() > 0)
			{
				std::map<LAString, double> riskMap;
				LAStringVector riskGridsConverted;
				for (std::size_t i = 0, ie = riskGrids.size(); i < ie; ++i)
				{
					LAString riskGridConverted = riskGrids[i];
					if (isCaps[i] && (underlyingLs[i] != capletTenorConvertedL))
					{
						for (int j = riskGridConverted.size() - 1; j >= 0; --j)
						{
							if (riskGridConverted[j] == '_')
							{
								riskGridConverted = riskGridConverted.subString(0, j) + capletTenorConvertedL;
								break;
							}
						}
					}
					const auto j = riskMap.find(riskGridConverted);
					if (j == riskMap.end())
					{
						riskMap.insert(std::make_pair(riskGridConverted, riskValuesConverted[i]));
						riskGridsConverted.push_back(riskGridConverted);
					}
					else
					{
						j->second += riskValuesConverted[i];
					}
				}

				riskValuesConverted.clear();
				for (const auto& riskGridConverted : riskGridsConverted)
				{
					riskValuesConverted.push_back(riskMap[riskGridConverted]);
				}

				// Replace the risk grids in the object
				objHolder.get().remove(outputNameU + "RiskGrid");
				objHolder.get().add(outputNameU + "RiskGrid", new LADataStrings(std::move(riskGridsConverted)));
			}

			objHolder.get().remove(outputNameU + "isParallelShift");
			objHolder.get().add(outputNameU + "isParallelShift", new LADataBool(false));
			objHolder.get().remove(outputNameL);
			objHolder.get().add(outputNameL, new LADataDoubles(std::move(riskValuesConverted)));
		}

		if (underlyingConverted)
		{
			info.remove(PRICING_DATA_RISKCURVETYPENAME);
			info.add(PRICING_DATA_RISKCURVETYPENAME, new LADataString(LAString(underlyingNameConvertedL).toUpper()));
			info.remove(PRICING_DATA_RISKBASESHIFTCURVETYPENAME);
			info.add(PRICING_DATA_RISKBASESHIFTCURVETYPENAME, new LADataString(LAString(underlyingNameConvertedL).toUpper()));
		}
	}

} // end unnamed

const double LAPricePortfolioValue::FAIL_VALUE = DBL_MAX;
const LAString LAPricePortfolioValue::ERROR    = "_ERROR";
const LAString LAPricePortfolioValue::PV_ERROR = "PV_ERROR";
const unsigned int LAPricePortfolioValue::EXTRASCENARIO_MAX = 10;

/*!
    @brief constructor
*/
LAPricePortfolioValue::LAPricePortfolioValue() :
LACoreValuation()
{
}
/*!
    @brief destructor

*/
LAPricePortfolioValue::~LAPricePortfolioValue()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LAPricePortfolioValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_PORTFOLIOVALUE ? true : LACoreValuation::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
LACoreFunctionBase*
LAPricePortfolioValue::clone() const
{
    try 
	{
        return new LAPricePortfolioValue();
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
LAPricePortfolioValue::getType() const
{
	return FN_IR_PORTFOLIOVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPricePortfolioValue::registerData(LAPriceDataManager& dm) const
{	
	dm.setData(CALIBRATION_DATA_UNDERLYINGS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_RISKCALCINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_TARGETNAMES, DATA_STRINGS);
	dm.setData(PRICING_DATA_BASEEXTRATARGETNAMES, DATA_STRINGS);
	dm.setData(PRICING_DATA_BASESCENARIO, DATA_STRINGS);
	dm.setData(PRICING_DATA_SCENARIO1, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_SCENARIO2, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_BASEEXTRASCENARIO, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_BASEOUTPUTNAME, DATA_STRING);
	dm.setData(PRICING_DATA_OUTPUTNAME, DATA_STRING);
	dm.setData(PRICING_DATA_OUTPUTNAME2, DATA_STRING);
	dm.setData(PRICING_DATA_BASEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_WAVEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_OPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_OPERATOR2, DATA_FUNCTION);
	dm.setData(PRICING_DATA_BASECOEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_WAVECOEFFICIENT, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_COEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_COEFFICIENT2, DATA_DOUBLES);
	dm.setData(PRICING_DATA_ISGRIDSENSITIVITY, DATA_BOOL);
	dm.setData(PRICING_DATA_ISCALCRISK, DATA_BOOL);
	dm.setData(PRICING_DATA_ISWAVE, DATA_BOOL);
	dm.setData(PRICING_DATA_VALUETYPE, DATA_STRING);
	dm.setData(PRICING_DATA_CLEANPRICE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ERRORSTATUS, DATA_STRINGS);
	dm.setData(PRICING_DATA_ISSETUPPAYOFF, DATA_BOOL);
	dm.setData(PRICING_DATA_VALUATIONCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_ISRESULTOUTPUT, DATA_BOOL);
	dm.setData(PRICING_DATA_ZEROCALC, DATA_BOOL);
	dm.setData(PRICING_DATA_OMITNOTIONALEXPOSURE, DATA_BOOL);
	dm.setData(PRICING_DATA_IMMRISKYIELDCURVENAME, DATA_STRINGS);
	dm.setData(PRICING_DATA_IMMFWDRATETERM, DATA_INTS);
	dm.setData(CALIBRATION_DATA_FEEAMOUNTS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_FEEPAYMENTDATES, DATA_DATES);
	dm.setData(CALIBRATION_DATA_FEECURRENCIES, DATA_STRINGS);
	dm.setData(CALIBRATION_DATA_FEEDISCOUNTCURVES, DATA_STRINGS);
}
/*!
	@brief value portfolio

	@param[in] basedate evaluate day
	@param[in,out] object Portfolio object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return sum of present values of each trades
	
*/
double
LAPricePortfolioValue::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	(void)att; //20061018--David--Fix warning:C4100 20070411--Nagase--gcc

	LADataHolder* dh;
	//value type
	LAString value_type = "";
	dh = &object.getData(PRICING_DATA_VALUETYPE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		value_type = dynamic_cast<const LADataString&>(dh->get()).get();
		value_type.toUpper();
		if (value_type != PV && value_type != PVANDRISK && value_type != RISK)
		{
			//error
			LAString msg = "ValueType=";
			msg += value_type;
			msg += " is not supported";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	
	//object pool
	LAObjectPool& objPool = object.getDataInstance()->getObjectPool();
	// reference pool
	LACoreReferencePool& rpool = object.getDataInstance()->getReferencePool();
	
	// underlyings
	dh = &object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL);
	LADataMultiReference& unders = dynamic_cast<LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < unders.getSize(); i++)
		unders.get(i).remove(PRICING_DATA_ISCALCRISK);

	//is analytic 
	bool isanalyticOnahead = false;
	// risk info
	if (value_type != PV)
	{
		dh = &object.getData(PRICING_DATA_RISKCALCINFOS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const LADataMultiReference& infos = dynamic_cast<const LADataMultiReference&>(dh->get());
			for (unsigned int i = 0; i < infos.getSize(); i++)
			{
				//dh = &(infos.get(i).getData(PRICING_DATA_ISANALYTIC,NOCHECK));
				dh = &(infos.get(i).getData(PRICING_DATA_ANALYTICCALCTYPE,NOCHECK));
				//if (dh->isDefined () && !dh->isNull() && dynamic_cast<const LADataBool &>(dh->get()))
				if (dh->isDefined () && !dh->isNull() && dynamic_cast<const LADataString &>(dh->get()).get() == ANALYTIC)
				{
					isanalyticOnahead = true;
					break;
				}
			}
		}
	}
	
	
	
	vector<LAMathObjectValue*> trades(unders.getSize());
	DoubleArray pv_base(unders.getSize());
	double ret = 0.0;
	for (unsigned int i = 0; i < unders.getSize(); i++)
	{
		trades[i] = &dynamic_cast<LAMathObjectValue&>(unders.get(i).get());
		trades[i]->remove(PRICING_DATA_ERRORSTATUS);
		trades[i]->add(PRICING_DATA_ERRORSTATUS);
		try
		{
			if (isanalyticOnahead)
			{	
				trades[i]->remove(PRICING_DATA_ISANALYTIC);
				trades[i]->LAObject::add(PRICING_DATA_ISANALYTIC, new LADataBool(true));
			}
			
cout << "LAPricePortfolioValue calc trade value called.." << endl;
cout << "[Scenario = PV, TradeNum = " << i << "]" << endl;
clock_t cstart = clock();
			if (value_type == RISK)
			{
				dh = &trades[i]->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL);
				pv_base[i] = dynamic_cast<const LADataDouble&>(dh->get()).get();
			}
			else
			{
				pv_base[i] = trades[i]->value(basedate);
			}
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LAPricePortfolioValue calc trade value end.." << endl;
cout << "-> time = " << time << endl;
			ret += pv_base[i];
		}
		catch(...)
		{
			pv_base[i] = FAIL_VALUE;
			dynamic_cast<LADataStrings &>(trades[i]->getData(PRICING_DATA_ERRORSTATUS, ISDEFINED).get()).push_back(PV_ERROR);
			if (value_type != RISK)
			{
				LAObject &e_trade = *trades[i];
				// remove value data
				e_trade.remove(PRICING_DATA_CLEANPRICE);
				e_trade.remove(PRICING_DATA_DIRTYPRICE);
				e_trade.remove(PRICING_DATA_ACCRUEDINTEREST);
				e_trade.remove(PRICING_DATA_CLEANPRICESQUARE);
				e_trade.remove(PRICING_DATA_CLEANPRICEDEVIATION);
				e_trade.remove(PRICING_DATA_CALLTRIGGERVALUE);
				e_trade.remove(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER);
				e_trade.remove(PRICING_DATA_PVCURRENCY);

				// set value data FAILE VALUE  
				e_trade.add(PRICING_DATA_CLEANPRICE, new LADataDouble(FAIL_VALUE));
				e_trade.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(FAIL_VALUE));
				e_trade.add(PRICING_DATA_ACCRUEDINTEREST, new LADataDouble(FAIL_VALUE));
				e_trade.add(PRICING_DATA_CLEANPRICESQUARE, new LADataDouble(FAIL_VALUE));
				e_trade.add(PRICING_DATA_CLEANPRICEDEVIATION, new LADataDouble(FAIL_VALUE));

				//PV Currency
				LAString pvCcy;
				dh = &(e_trade.getData(PRICING_DATA_CURRENCY, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					pvCcy = dynamic_cast<const LADataString&>(dh->get()).get();
					dh = &(e_trade.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						pvCcy = dynamic_cast<const LADataString&>(dh->get()).get();
				}
				else
				{
					dh = &(e_trade.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						pvCcy = dynamic_cast<const LADataString&>(dh->get()).get();
				}
				e_trade.add(PRICING_DATA_PVCURRENCY, new LADataString(pvCcy));

				// check isdetailoutput
				LADataHolder &detailOut = e_trade.getData(PRICING_DATA_ISDETAILOUTPUT);
				if (detailOut.isDefined() && !detailOut.isNull() 
					&& dynamic_cast<LADataBool &>(detailOut.get()).get())
				{
					e_trade.add(PRICING_DATA_CALLTRIGGERVALUE, new LADataDouble(FAIL_VALUE));
					e_trade.add(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER, new LADataDouble(FAIL_VALUE));
				}
			}
		}

	}	

	for (unsigned int i = 0; i < unders.getSize(); i++)
		unders.get(i).add(PRICING_DATA_ISCALCRISK, new LADataBool(true));

	// risk info
	dh = &object.getData(PRICING_DATA_RISKCALCINFOS, NOCHECK);
	if (!dh->isDefined() || dh->isNull())
	{
		if (value_type != "" && value_type != PV)
		{
			//error
			throw LACoreInvalidData("RiskCalcInfos is not exsist", __FILE__, __LINE__);
		}
        return ret;
	}

	if (value_type == PV) return ret;

	DoubleArray pv_base_orig = pv_base;
	LAStringVector pvCurrency_orig;
	for (unsigned int j = 0; j < unders.getSize(); j++)
	{
		dh = &(unders.get(j).getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pvCurrency_orig.push_back( dynamic_cast<const LADataString &>(dh->get()).get() );
		}
		else
		{
			dh = &(unders.get(j).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
			pvCurrency_orig.push_back( dynamic_cast<const LADataString &>(dh->get()).get() );
		}
	}

	dh = &object.getData(PRICING_DATA_RISKCALCINFOS, NOCHECK);
	LADataMultiReference& infos = dynamic_cast<LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < infos.getSize(); i++)
	{
		pv_base = pv_base_orig;
		// issetuppayoff
		bool isSetUpPayOff = false;
		dh = &infos.get(i).getData(PRICING_DATA_ISSETUPPAYOFF, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isSetUpPayOff = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		for (unsigned int j = 0; j < unders.getSize(); j++)
		{
			unders.get(j).remove(PRICING_DATA_ISSETUPPAYOFF);
			unders.get(j).add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(isSetUpPayOff));
		}
		// omit notional exposure or not
		bool omitNotionalExposure = false;
		dh = &infos.get(i).getData(PRICING_DATA_OMITNOTIONALEXPOSURE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			omitNotionalExposure = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		for (unsigned int j = 0; j < unders.getSize(); j++)
		{
			unders.get(j).remove(PRICING_DATA_OMITNOTIONALEXPOSURE);
			unders.get(j).add(PRICING_DATA_OMITNOTIONALEXPOSURE, new LADataBool(omitNotionalExposure));
		}

		//analytic mode or not
		//dh = &(infos.get(i).getData(PRICING_DATA_ISANALYTIC, ISNOTNULL));
		dh = &(infos.get(i).getData(PRICING_DATA_ANALYTICCALCTYPE, ISNOTNULL));
		LAString analytictype = dynamic_cast<const LADataString &>(dh->get());
		/*bool isanalytic = dynamic_cast<LADataBool &>(dh->get());
		if (isanalytic)*/
		if (analytictype.toUpper() == ANALYTIC)
		{
			throw LACoreInvalidData("Analytic risk is not supported now!",__FILE__,__LINE__);
			//// we stopped to calculate the analytic risk on 2012/11/26 because of bad performance of the linepricer
			//// output attr name
			//dh = &infos.get(i).getData(PRICING_DATA_OUTPUTNAME, ISNOTNULL);
			//const LAString& outputname = dynamic_cast<const LADataString&>(dh->get()).get();

			//dh = &(infos.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			//LAString keyname = dynamic_cast<LADataString &>(dh->get());

			//dh = &(infos.get(i).getData(PRICING_DATA_SHIFTVALS_ANALYTIC, ISNOTNULL));
			//DoubleVector shiftvals = dynamic_cast<const LADataDoubles &>(dh->get()).get();
			//
			//dh = &(infos.get(i).getData(PRICING_DATA_DIVUNIT_ANALYTIC, ISNOTNULL));
			//double divunit = dynamic_cast<const LADataDouble &>(dh->get()).get();
			//if (0.0 == divunit)
			//	throw LACoreInvalidData("divuniti must not be 0",__FILE__,__LINE__);

			//DoubleVector multivec(shiftvals.size(), 1.0/divunit);
			//
			//dh = &(infos.get(i).getData(PRICING_DATA_SHIFTTYPE_ANALYTIC, ISNOTNULL));
			//LAString shifttype = dynamic_cast<const LADataString &>(dh->get());
			//if (shifttype.toUpper () == "DIFF")
			//{
			//	transform(shiftvals.begin(), shiftvals.end(), multivec.begin(), shiftvals.begin(), multiplies<double>());
			//}
			//else if (shifttype == "RATIO")
			//{
			//	transform(shiftvals.begin(), shiftvals.end(), multivec.begin(), shiftvals.begin(), multiplies<double>());

			//	dh = &(infos.get(i).getData(PRICING_DATA_RISKCURVETYPENAME, ISNOTNULL));
			//	LAString keycurve = dynamic_cast<const LADataString &>(dh->get());

			//	if (STD == keycurve)
			//		keycurve = SWAP;

			//	dh = &(infos.get(i).getData(PRICING_DATA_RISKCURVETYPECURRENCY, ISNOTNULL));
			//	LAString keyccy = dynamic_cast<const LADataString &>(dh->get());

			//	//get asof
			//	dh = &(unders.get(0).getData("PathEntity", NOCHECK));
			//	if (!dh->isDefined() || dh->isNull())
			//	{
			//		dh = &(unders.get(0).getData("MarketParam", ISNOTNULL));
			//	}
			//	const LADataReference& pathref = dynamic_cast<const LADataReference &>(dh->get());
			//	if (!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
			//	{
			//		//const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(pathref.get().get());
			//		throw LACoreInvalidData("SemiAnalyticError",__FILE__,__LINE__);
			//	}
			//	
			//	const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get());
			//	const LAMathYieldCurvePro& yldPro = path.getIRCurvePro(keyccy);
			//	DoubleVector underlyingrates;
			//	IntVector omitposvec;
			//	const std::map<LAString, LAString>& assignedCurveMktMap = yldPro.getAssignedCurveMktMap();
			//	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
			//	{
			//		if (keycurve == it->second)
			//		{
			//			underlyingrates = yldPro.getConversionMarketRates(it->first).get();
			//			if (yldPro.isOmitGridsExist(it->first))
			//				omitposvec = yldPro.getConversionOmitGrids(it->first).get();
			//			break;
			//		}
			//	}
			//	if (underlyingrates.size() + omitposvec.size() != shiftvals.size())
			//		throw LACoreInvalidData("Analytic Grid Size Error",__FILE__,__LINE__);

			//	//imput tmprate;
			//	for (unsigned int k = 0; k < omitposvec.size(); k++)
			//	{
			//		unsigned int pos = omitposvec[k];
			//		underlyingrates.insert(underlyingrates.begin()+pos, 0.0);
			//	}

			//	//multiple underlying
			//	transform(shiftvals.begin(), shiftvals.end(), underlyingrates.begin(), shiftvals.begin(), multiplies<double>());
			//}
			//else
			//	throw LACoreInvalidData("Analytic Shifttype Error",__FILE__,__LINE__);

			//for (unsigned int j = 0; j < trades.size(); j++)
			//{
			//	try
			//	{
			//		double tmpret = trades[j]->value(basedate);
			//		/*dh = &(trades[j]->getData(keyname, ISNOTNULL));
			//		DoubleVector out = dynamic_cast<const LADataDoubles &>(dh->get()).get();*/
			//		dh = &(infos.get(i).getData(PRICING_DATA_RISKCURVETYPECURRENCY, ISNOTNULL));
			//		LAString keyccy = dynamic_cast<const LADataString &>(dh->get());
			//		//get yield curve pro
			//		dh = &(unders.get(0).getData("PathEntity", NOCHECK));
			//		if (!dh->isDefined() || dh->isNull())
			//		{
			//			dh = &(unders.get(0).getData("MarketParam", ISNOTNULL));
			//		}
			//		const LADataReference& pathref = dynamic_cast<const LADataReference &>(dh->get());
			//		if (!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
			//		{
			//			throw LACoreInvalidData("SemiAnalyticError",__FILE__,__LINE__);
			//		}
			//		
			//		const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get());
			//		const LAMathYieldCurvePro& yldPro = path.getIRCurvePro(keyccy);
			//		const std::map<LAString, LAString>& assignedCurveMktMap = yldPro.getAssignedCurveMktMap();
			//		dh = &(infos.get(i).getData(PRICING_DATA_RISKCURVETYPENAME, ISNOTNULL));
			//		LAString keyMarket = dynamic_cast<const LADataString &>(dh->get());
			//		if (keyMarket == STD) keyMarket = SWAP;
			//		LAString tmpKeyname = keyname;
			//		LAStringVector riskInfo = tmpKeyname.toToken('_');
			//		DoubleVector out;
			//		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
			//		{
			//			if (keyMarket == it->second)
			//			{
			//				LAString keyCurve = riskInfo[0] + "_" + riskInfo[1] + "_" + it->first;
			//				dh = &(trades[j]->getData(keyCurve, ISNOTNULL));
			//				if (out.size() == 0)
			//				{
			//					out = dynamic_cast<const LADataDoubles &>(dh->get()).get();
			//				}
			//				else
			//				{
			//					const DoubleVector& out_tmp = dynamic_cast<const LADataDoubles &>(dh->get()).get();
			//					if (out_tmp.size() != out.size())
			//						throw LACoreInvalidData("out put sizes are different in analytic risk!",__FILE__,__LINE__);
			//					for (size_t k=0; k<out.size(); k++)
			//					{
			//						out[k] += out_tmp[k];
			//					}
			//				}
			//			}
			//		}
			//		//LAString tmpRiskName = outputname;
			//		if (out.size() != shiftvals.size())
			//			throw LACoreInvalidData("Analytic Grid Size Error",__FILE__,__LINE__);
			//		
			//		//multiple out
			//		transform(out.begin(), out.end(), shiftvals.begin(), out.begin(), multiplies<double>());
			//		double gridsum = std::accumulate(out.begin(), out.end(), 0.0);

			//		dh = &(infos.get(i).getData(PRICING_DATA_ISGRIDSENSITIVITY_ANALYTIC, ISNOTNULL));
			//		bool isgrid = dynamic_cast<const LADataBool &>(dh->get()).get();
			//		if (isgrid)
			//		{
			//			out.insert(out.begin(), gridsum);
			//			unders.get(j).remove(outputname);
			//			unders.get(j).add(outputname, new LADataDoubles(out));
			//		}
			//		else
			//		{
			//			unders.get(j).remove(outputname);
			//			unders.get(j).add(outputname, new LADataDouble(gridsum));
			//		}
			//	}
			//	catch(...)
			//	{
			//		DoubleVector out(1,FAIL_VALUE);
			//		unders.get(j).remove(outputname);
			//		unders.get(j).add(outputname, new LADataDoubles(out));
			//	}
			//	
			//}
			//continue;
		}
		
		
		//reset valcurrency for each trades
		for (unsigned int j = 0; j < unders.getSize(); j++)
			unders.get(j).remove(PRICING_DATA_VALUATIONCURRENCY);
			

		dh = &infos.get(i).getData(PRICING_DATA_RISKOUTPUTCURRENCY, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			LAString valcurrency = dynamic_cast<const LADataString &>(dh->get()).get();
			//LAString pvcurrency;
			double exchangerate = 1.0;

			LAMathFXEntity* pfx = 0;
			LAString b_outputname;
			dh = &infos.get(i).getData(PRICING_DATA_BASEOUTPUTNAME, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
				b_outputname =  dynamic_cast<LADataString &>(dh->get()).get();
			
			for (unsigned int j = 0; j < unders.getSize(); j++)
			{

				bool isspotcalc = (pvCurrency_orig[j] != valcurrency);
				if (isspotcalc && pfx == 0)
				{
					dh = &(object.getData(PRICING_DATA_FXRATE, ISNOTNULL));
					LAObject& e = dynamic_cast<LADataReference &>(dh->get()).get().get();
					pfx = &(dynamic_cast<LAMathFXEntity &>(e));
				}

				double valueTerm = 0.;
				dh = &(unders.get(j).getData(PRICING_DATA_VALUEDATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const LADate& valueDate = dynamic_cast<const LADataDate&>(dh->get()).get();
					LADataReference pathref;
					dh = &(unders.get(j).getData("PathEntity", NOCHECK));
					if (!dh->isDefined() || dh->isNull())
					{
						dh = &(unders.get(j).getData("MarketParam", ISNOTNULL));
						pathref = dynamic_cast<LADataReference &>(dh->get());	
					}
					else
					{
						pathref = dynamic_cast<LADataReference &>(dh->get());
					}
					
					if (!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
					{
						const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(pathref.get().get());
						valueTerm = path.getDayCount().getTerm(path.getAsOfDate().get(), valueDate);
					}
					else
					{
						const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get());
						valueTerm = path.getDayCount().getTerm(path.getAsOfDate().get(), valueDate);
					}
				}

				exchangerate = (isspotcalc) ? pfx->getRate(pvCurrency_orig[j],valcurrency,valueTerm) : 1.0;
				if(pv_base[j] != FAIL_VALUE)
				{
					pv_base[j] *= exchangerate;
				}
				unders.get(j).remove(PRICING_DATA_VALUATIONCURRENCY);
				unders.get(j).add(PRICING_DATA_VALUATIONCURRENCY, new LADataString(valcurrency));
				unders.get(j).remove(PRICING_DATA_ISSETUPPAYOFF);
				unders.get(j).add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(false));

				unders.get(j).remove(b_outputname + "_" + PRICING_DATA_RISKOUTPUTCURRENCY);
				unders.get(j).add(b_outputname + "_" + PRICING_DATA_RISKOUTPUTCURRENCY, new LADataString(valcurrency));
			}
		}

		// operator
		dh = &(infos.get(i).getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

		// coefficient
		dh = &(infos.get(i).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
		const DoubleArray& coeff = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		method.setParam(coeff);		
		
		// grid sensitivity?
		dh = &infos.get(i).getData(PRICING_DATA_ISGRIDSENSITIVITY, ISNOTNULL);
		bool isgrid = dynamic_cast<const LADataBool&>(dh->get()).get();

		// output attr name
		dh = &infos.get(i).getData(PRICING_DATA_OUTPUTNAME, ISNOTNULL);
		const LAString& outputname = dynamic_cast<const LADataString&>(dh->get()).get();
		for (unsigned int j = 0; j < unders.getSize(); j++)
			unders.get(j).remove(outputname);

		// target names
		dh = &infos.get(i).getData(PRICING_DATA_TARGETNAMES, ISNOTNULL);
		const LAStringVector& targets = dynamic_cast<const LADataStrings&>(dh->get()).get();

		// scenario1
		dh = &infos.get(i).getData(PRICING_DATA_SCENARIO1, NOCHECK);
		if (!dh->isDefined() || dh->isNull())
		{
			infos.remove(infos.get(i).getName());
			continue;
		}
		LADataMultiReference& scenario1 = dynamic_cast<LADataMultiReference&>(dh->get());
		if (targets.size() != scenario1.getSize())
		{
			//error
			throw LACoreInvalidData("target name size and scenario1 size are not same", __FILE__, __LINE__);
		}

		LADataMultiReference* pscenario2 = 0;
		dh = &infos.get(i).getData(PRICING_DATA_SCENARIO2, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{			
			pscenario2 = &dynamic_cast<LADataMultiReference&>(dh->get());
			if (targets.size() != pscenario2->getSize())
			{
				//error
				throw LACoreInvalidData("target name size and scenario2 size are not same", __FILE__, __LINE__);
			}		
		}
		
		// extra target
		vector<LAStringVector> ex_targets1;
		vector<LAStringVector> ex_targets2;
		// extra scenario
		vector<LADataMultiReference*> ex_scenarios1;
		vector<LADataMultiReference*> ex_scenarios2;

		for (unsigned int j = 0; j < EXTRASCENARIO_MAX; ++j)
		{
			dh = &infos.get(i).getData(PRICING_DATA_EXTRATARGETNAMES1 + LAString("_") + LAString(static_cast<int>(j + 1)), NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				const LAStringVector& target1 = dynamic_cast<const LADataStrings&>(dh->get()).get();
				dh = &infos.get(i).getData(PRICING_DATA_EXTRASCENARIO1 + LAString("_") + LAString(static_cast<int>(j + 1)), ISNOTNULL);
				LADataMultiReference& ex_scenario1 = dynamic_cast<LADataMultiReference&>(dh->get());

				if (isgrid)
				{
					if (target1.size() != targets.size() || ex_scenario1.getSize() != targets.size())
					{
						//error
						throw LACoreInvalidData("grid risk. target name size and extra target1 size are not same", __FILE__, __LINE__);
					}
				}
				else
				{
					if (target1.size() != 1 || ex_scenario1.getSize() != 1)
					{
						//error
						throw LACoreInvalidData("parallel risk. target name size and extra target1 size are not 1", __FILE__, __LINE__);
					}
				}
				ex_targets1.push_back(target1);
				ex_scenarios1.push_back(&ex_scenario1);				
			}
			else
			{
				break;
			}

			// for scenario 2
			if (pscenario2)
			{
				dh = &infos.get(i).getData(PRICING_DATA_EXTRATARGETNAMES2 + LAString("_") + LAString(static_cast<int>(j + 1)), NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const LAStringVector& target2 = dynamic_cast<const LADataStrings&>(dh->get()).get();
					dh = &infos.get(i).getData(PRICING_DATA_EXTRASCENARIO2 + LAString("_") + LAString(static_cast<int>(j + 1)), ISNOTNULL);
					LADataMultiReference& ex_scenario2 = dynamic_cast<LADataMultiReference&>(dh->get());
					
					if (isgrid)
					{
						if (targets.size() != target2.size() || targets.size() != ex_scenario2.getSize())
						{
							//error
							throw LACoreInvalidData("grid risk. target name size and extra target2 size are not same", __FILE__, __LINE__);
						}
					}
					else
					{
						if (target2.size() != 1 || ex_scenario2.getSize() != 1)
						{
							//error
							throw LACoreInvalidData("parallel risk. target name size and extra target1 size are not 1", __FILE__, __LINE__);
						}
					}
					ex_targets2.push_back(target2);
					ex_scenarios2.push_back(&ex_scenario2);
				}
			}
		}
		
		DoubleMatrix pv_1, pv_2;
		// base scenario
		dh = &infos.get(i).getData(PRICING_DATA_BASESCENARIO, NOCHECK);
		LAStringVector baseTargets,b_ex_targets;
		if (dh->isDefined() && !dh->isNull())
		{			
			const LAStringVector& b_scenario = dynamic_cast<const LADataStrings&>(dh->get()).get();

			// get base target name
			dh = &(infos.get(i).getData(PRICING_DATA_BASETARGETNAMES, ISNOTNULL));
			baseTargets = dynamic_cast<const LADataStrings&>(dh->get()).get();

			// baseoperator
			dh = &(infos.get(i).getData(PRICING_DATA_BASEOPERATOR, ISNOTNULL));
			LAFunctionBase& b_func = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

			// coefficient
			dh = &(infos.get(i).getData(PRICING_DATA_BASECOEFFICIENT, ISNOTNULL));
			const DoubleArray& b_coeff = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			b_func.setParam(b_coeff);

			// output attr name
			dh = &infos.get(i).getData(PRICING_DATA_BASEOUTPUTNAME, ISNOTNULL);
			const LAString& b_outputname = dynamic_cast<const LADataString&>(dh->get()).get();
			for (unsigned int j = 0; j < unders.getSize(); j++)
				unders.get(j).remove(b_outputname);

			// extra scenario
			LADataMultiReference b_ex_scenarios;

			dh = &infos.get(i).getData(PRICING_DATA_BASEEXTRATARGETNAMES, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				b_ex_targets = dynamic_cast<const LADataStrings&>(dh->get()).get();
				dh = &infos.get(i).getData(PRICING_DATA_BASEEXTRASCENARIO, ISNOTNULL);
				b_ex_scenarios = dynamic_cast<LADataMultiReference&>(dh->get());

				if (b_ex_scenarios.getSize() != b_ex_targets.size())
				{
					//error
					throw LACoreInvalidData("grid risk. Extra scenario size and base extra target size are not same", __FILE__, __LINE__);
				}
			}

			DoubleArray pv_b_sce(trades.size());
			if (isgrid)
			{
				//setScenario(targets[0], objPool.getObject(b_scenario[0], ENCHKTYPE_ISDEFINED).get(), rpool);
				setScenario(baseTargets[0], objPool.getObject(b_scenario[0], ENCHKTYPE_ISDEFINED).get(), rpool);
				// set extra target
				for (unsigned int j = 0; j < b_ex_targets.size(); j++)
					setScenario(b_ex_targets[j], b_ex_scenarios.get(j).get(), rpool);

				for (unsigned int j = 0; j < trades.size(); j++)
				{
					if (pv_base[j] != FAIL_VALUE)
					{
						try
						{
cout << "LAPricePortfolioValue calc trade value called.." << endl;
cout << "[Scenario = Risk:Grid(BasePV), TradeNum = " << j << "]" << endl;
clock_t cstart = clock();
							pv_b_sce[j] = trades[j]->value(basedate);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LAPricePortfolioValue calc trade value end.." << endl;
cout << "-> time = " << time << endl;
						}
						catch(...)
						{
							pv_b_sce[j] = FAIL_VALUE;
						}
					}
					else
					{
						pv_b_sce[j] = FAIL_VALUE;
					}
				}
				////backToBase(targets[0], objPool, rpool);
				//backToBase(baseTargets[0], objPool, rpool);
				//for (unsigned int j = 0; j < b_ex_targets.size(); j++)
				//	backToBase(b_ex_targets[j], objPool, rpool);

			}
			else
			{
				if (b_scenario.size() != 1 && targets.size() != b_scenario.size())
				{
					//error
					throw LACoreInvalidData("target name size and baseScenario size are not same", __FILE__, __LINE__);
				}

				//for (unsigned int j = 0; j < targets.size(); j++)
				/*for (unsigned int j = 0; j < b_scenario.size(); j++)
					setScenario(targets[j], objPool.getObject(b_scenario[j], ENCHKTYPE_ISDEFINED).get(), rpool);*/
				for (unsigned int j = 0; j < b_scenario.size(); j++)
					setScenario(baseTargets[j], objPool.getObject(b_scenario[j], ENCHKTYPE_ISDEFINED).get(), rpool);
				// set extra target
				for (unsigned int j = 0; j < b_ex_targets.size(); j++)
					setScenario(b_ex_targets[j], b_ex_scenarios.get(j).get(), rpool);

				for (unsigned int j = 0; j < trades.size(); j++)
				{
					if (pv_base[j] != FAIL_VALUE)
					{
						try
						{
cout << "LAPricePortfolioValue calc trade value called.." << endl;
cout << "[Scenario = Risk:Parallel(BasePV), TradeNum = " << j << "]" << endl;
clock_t cstart = clock();
							pv_b_sce[j] = trades[j]->value(basedate);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LAPricePortfolioValue calc trade value end.." << endl;
cout << "-> time = " << time << endl;
						}
						catch(...)
						{
							pv_b_sce[j] = FAIL_VALUE;
						}
					}
					else
					{
						pv_b_sce[j] = FAIL_VALUE;
					}
				}

				//for (unsigned int j = 0; j < targets.size(); j++)
				/*for (unsigned int j = 0; j < b_scenario.size(); j++)
					backToBase(targets[j], objPool, rpool);*/
				/*for (unsigned int j = 0; j < b_scenario.size(); j++)
					backToBase(baseTargets[j], objPool, rpool);
				for (unsigned int j = 0; j < b_ex_targets.size(); j++)
					backToBase(b_ex_targets[j], objPool, rpool);*/

			}
			// set outputattribute
			double out = 0.0;
			double value_PL = 0.0;
			DoubleArray var(2);			
			for (unsigned int j = 0; j < trades.size(); j++)
			{
				if (pv_base[j] != FAIL_VALUE && pv_b_sce[j] != FAIL_VALUE)
				{
					var[0] = pv_base[j];
					var[1] = pv_b_sce[j];
					out = b_func(var);
					value_PL = out - pv_base[j];
				}
				else
				{
					out = FAIL_VALUE;
					value_PL = FAIL_VALUE;
				}
				trades[j]->LAObject::remove(b_outputname);
				trades[j]->LAObject::remove(b_outputname + "_PL");
				trades[j]->LAObject::add(b_outputname, new LADataDouble(out));
				trades[j]->LAObject::add(b_outputname + "_PL", new LADataDouble(value_PL));
			}

			// set base pv scenario 
			pv_base = pv_b_sce;
		}

		// calc PV
		if (ex_targets1.empty())
		{
			calcPV(basedate, trades, targets, scenario1, isgrid, objPool, rpool, pv_1);	
		}
		else
		{
			calcPV(basedate, trades, targets, scenario1, isgrid, objPool, rpool, pv_1, &ex_targets1, &ex_scenarios1);	
		}
		if (pscenario2)
		{
			if (ex_targets2.empty())
			{
				calcPV(basedate, trades, targets, *pscenario2, isgrid, objPool, rpool, pv_2);
			}
			else
			{
				calcPV(basedate, trades, targets, *pscenario2, isgrid, objPool, rpool, pv_2, &ex_targets2, &ex_scenarios2);
			}
		}
		
		// return to original enrity in base scenario
		for (unsigned int j = 0; j < baseTargets.size(); j++)
			backToBase(baseTargets[j], objPool, rpool);
		for (unsigned int j = 0; j < b_ex_targets.size(); j++)
			backToBase(b_ex_targets[j], objPool, rpool);
		
		// wave ?
		bool iswave = false;
		dh = &infos.get(i).getData(PRICING_DATA_ISWAVE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
			iswave = dynamic_cast<const LADataBool&>(dh->get()).get();

		// IMM fwd risk conversion
		dh = &(infos.get(i).getData(PRICING_DATA_IMMRISKYIELDCURVENAME, NOCHECK));
		std::shared_ptr<LAPriceIMMFwdRiskConversionMatrix> conv;
		const IntVector* immFwdRateTerm = 0;
		if (dh->isDefined())
		{
			// Market rate bump risk -> IMM forward risk
			conv.reset(new LAPriceIMMFwdRiskConversionMatrix(calcIMMFwdRiskConversionMatrix(infos.get(i).get(), scenario1)));
		}
		else
		{
			dh = &(infos.get(i).getData(PRICING_DATA_IMMFWDRATETERM, NOCHECK));
			if (dh->isDefined())
			{
				// Zero rate bump -> IMM forward risk
				immFwdRateTerm = &dynamic_cast<const LADataInts&>(dh->get()).get();
			}
		}

		// Vega conversion
		dh = &infos.get(i).getData(PRICING_DATA_VEGACONVERTEDSHIFTVAL);
		bool convertsVega;
		if (dh->isDefined() && !dh->isNull())
		{
			convertsVega = true;
		}
		else
		{
			dh = &infos.get(i).getData(PRICING_DATA_VEGACONVERTEDUNDERLYING);
			if (dh->isDefined() && !dh->isNull())
			{
				convertsVega = true;
			}
			else
			{
				dh = &infos.get(i).getData(PRICING_DATA_VEGACONVERTEDCAPLETTENOR);
				convertsVega = dh->isDefined() && !dh->isNull();
			}
		}

		if (!iswave || !isgrid)
		{		
			// set risk value1
			setRiskValue(unders, scenario1, pscenario2, method, isgrid, outputname, pv_base, pv_1, pv_2, infos.get(i).get());

			// IMM fwd risk conversion
			if (conv.get())
			{
				// Market rate bump risk -> IMM forward risk
				putIMMFwdRisksOnMarketRateBump(infos.get(i).get(), unders, outputname, *conv);
			}
			else if (immFwdRateTerm)
			{
				// Zero rate bump -> IMM forward risk
				putIMMFwdRisksOnZeroRateBump(infos.get(i).get(), unders, outputname);
			}

			// Vega conversion
			if (convertsVega)
			{
				putConvertedVega(infos.get(i).get(), unders, outputname);
			}

			// operator2
			dh = &(infos.get(i).getData(PRICING_DATA_OPERATOR2, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LAFunctionBase& func2 = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

				// coefficient
				dh = &(infos.get(i).getData(PRICING_DATA_COEFFICIENT2, ISNOTNULL));
				const DoubleArray& coeff2 = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				func2.setParam(coeff2);		
			
				// output attr name
				dh = &infos.get(i).getData(PRICING_DATA_OUTPUTNAME2, ISNOTNULL);
				const LAString& outputname2 = dynamic_cast<const LADataString&>(dh->get()).get();
				for (unsigned int j = 0; j < unders.getSize(); j++)
					unders.get(j).remove(outputname2);

				// set risk value2
				setRiskValue(unders, scenario1, pscenario2, func2, isgrid, outputname2, pv_base, pv_1, pv_2, infos.get(i).get());
			
				if (conv.get())
				{
					// Market rate bump risk -> IMM forward risk
					putIMMFwdRisksOnMarketRateBump(infos.get(i).get(), unders, outputname2, *conv);
				}
				else if (immFwdRateTerm)
				{
					// Zero rate bump -> IMM forward risk
					putIMMFwdRisksOnZeroRateBump(infos.get(i).get(), unders, outputname2);
				}
			}
		}
		else
		{
			// wave operator
			dh = &(infos.get(i).getData(PRICING_DATA_WAVEOPERATOR, ISNOTNULL));
			LAFunctionBase& funcW = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

			// wave coefficient
			dh = &(infos.get(i).getData(PRICING_DATA_WAVECOEFFICIENT, ISNOTNULL));
			const DoubleMatrix& coeffW = dynamic_cast<const LADataDoubleMatrix&>(dh->get()).get();

			// size check
			if (scenario1.getSize() != coeffW.size())
			{
				throw LACoreInvalidData("wave coefficient size and scenario size is not same.", __FILE__, __LINE__);
			}
			// set risk value1
			setWaveRiskValue(unders, scenario1, pscenario2, coeffW, funcW, method, outputname, pv_base, pv_1, pv_2);

			if (conv.get())
			{
				// Market rate bump risk -> IMM forward risk
				putIMMFwdRisksOnMarketRateBump(infos.get(i).get(), unders, outputname, *conv);
			}
			else if (immFwdRateTerm)
			{
				// Zero rate bump -> IMM forward risk
				putIMMFwdRisksOnZeroRateBump(infos.get(i).get(), unders, outputname);
			}

			// operator2
			dh = &(infos.get(i).getData(PRICING_DATA_OPERATOR2, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LAFunctionBase& func2 = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

				// coefficient
				dh = &(infos.get(i).getData(PRICING_DATA_COEFFICIENT2, ISNOTNULL));
				const DoubleArray& coeff2 = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				func2.setParam(coeff2);		
			
				// output attr name
				dh = &infos.get(i).getData(PRICING_DATA_OUTPUTNAME2, ISNOTNULL);
				const LAString& outputname2 = dynamic_cast<const LADataString&>(dh->get()).get();
				for (unsigned int j = 0; j < unders.getSize(); j++)
					unders.get(j).remove(outputname2);

				// set risk value2
				setWaveRiskValue(unders, scenario1, pscenario2, coeffW, funcW, func2, outputname2, pv_base, pv_1, pv_2);

				if (conv.get())
				{
					// Market rate bump risk -> IMM forward risk
					putIMMFwdRisksOnMarketRateBump(infos.get(i).get(), unders, outputname2, *conv);
				}
				else if (immFwdRateTerm)
				{
					// Zero rate bump -> IMM forward risk
					putIMMFwdRisksOnZeroRateBump(infos.get(i).get(), unders, outputname2);
				}
			}
		}
	}

	for (unsigned int i = 0; i < unders.getSize(); i++)
		unders.get(i).remove(PRICING_DATA_ISCALCRISK);
	return ret;

}

/*!
	@brief calculate pv of scenario cases

	@param[in] basedate evaluate day
	@param[in] trades trades
	@param[in] targets target object names of scenario
	@param[in] scenario scenarios
	@param[in] isgrid target risk index is grid sensitivity?
	@param[in] objPool object pool
	@param[in] rpool reference object pool
	@param[out] output output variable
	
*/
void
LAPricePortfolioValue::calcPV(const LADate& basedate,
						   std::vector<LAMathObjectValue*> trades,
										const LAStringVector& targets,
										LADataMultiReference& scenario, 
										bool isgrid,
										LAObjectPool& objPool, 
										LACoreReferencePool& rpool,
										DoubleMatrix& output,
										vector<LAStringVector>* ex_targets,
										vector<LADataMultiReference*>* ex_scenarios) const
{
	if (isgrid)
	{
		output.resize(scenario.getSize());
		for (unsigned int i = 0; i < scenario.getSize(); i++)
		{
			output[i].resize(trades.size());
			setScenario(targets[i], scenario.get(i).get(), rpool);
			// set extra target
			if (ex_targets)
			{
				for (unsigned int j = 0; j < ex_targets->size(); j++)
				{
					setScenario((*ex_targets)[j][i], (*ex_scenarios)[j]->get(i).get(), rpool);
				}
			}

			for (unsigned int j = 0; j < trades.size(); j++)
			{
				LADataStrings &errorStatus = dynamic_cast<LADataStrings &>(trades[j]->getData(PRICING_DATA_ERRORSTATUS, ISDEFINED).get());
				if (errorStatus.get().end() == std::find(errorStatus.begin(), errorStatus.end(), PV_ERROR))
				{
					try
					{
cout << "LAPricePortfolioValue calc trade value called.." << endl;
cout << "[Scenario = Risk:Grid#" << i << ", TradeNum = " << j << "]" <<  endl;
clock_t cstart = clock();
						output[i][j] = trades[j]->value(basedate);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LAPricePortfolioValue calc trade value end.." << endl;
cout << "-> time = " << time << endl;
					}
					catch(...)
					{
						output[i][j] = FAIL_VALUE;
					}
				}
				else
				{
					output[i][j] = FAIL_VALUE;
				}			
			}
			backToBase(targets[i], objPool, rpool);
			// back extra target
			if (ex_targets)
			{
				for (unsigned int j = 0; j < ex_targets->size(); j++)
				{
					backToBase((*ex_targets)[j][i], objPool, rpool);
				}
			}
		}
	}
	else
	{
		output.resize(1);
		output[0].resize(trades.size());
		for (unsigned int i = 0; i < targets.size(); i++)
			setScenario(targets[i], scenario.get(i).get(), rpool);
		// set extra target
		if (ex_targets)
		{
			for (unsigned int i = 0; i < ex_targets->size(); i++)
			{
				setScenario((*ex_targets)[i][0], (*ex_scenarios)[i]->get(0).get(), rpool);
			}
		}

		for (unsigned int i = 0; i < trades.size(); i++)
		{
			LADataStrings &errorStatus = dynamic_cast<LADataStrings &>(trades[i]->getData(PRICING_DATA_ERRORSTATUS, ISDEFINED).get());
			if (errorStatus.end() == std::find(errorStatus.begin(), errorStatus.end(), PV_ERROR))
			{
				try
				{
cout << "LAPricePortfolioValue calc trade value called.." << endl;
cout << "[Scenario = Risk:Parallel, TradeNum = " << i << "]" << endl;
clock_t cstart = clock();
					output[0][i] = trades[i]->value(basedate);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LAPricePortfolioValue calc trade value end.." << endl;
cout << "-> time = " << time << endl;
				}
				catch(...)
				{
					output[0][i] = FAIL_VALUE;
				}
			}
			else
			{
				output[0][i] = FAIL_VALUE;
			}
		}
		// back extra target
		if (ex_targets)
		{
			for (unsigned int i = 0; i < ex_targets->size(); i++)
			{
				backToBase((*ex_targets)[i][0], objPool, rpool);
			}
		}

		for (unsigned int i = 0; i < targets.size(); i++)
			backToBase(targets[i], objPool, rpool);		
	}
}


/*!
	@brief set risk value

	@param[in,out] unders underlying trade
	@param[in]     scenario1 scenario1
	@param[in]     pscenario2 pointer to scenario2
	@param[in]     method  function to calc risk
	@param[in]     isgrid target risk index is grid sensitivity?
	@param[in]     outputname output risk name
	@param[in]     pv_base 
	@param[in]     pv_1  pv for scenario1
	@param[in]     pv_2  pv for scenario2
	
*/
void
LAPricePortfolioValue::setRiskValue(const LADataMultiReference &unders,
										const LADataMultiReference &scenario1, 
												const LADataMultiReference *pscenario2,
												const LAFunctionBase &method,
												bool isgrid,
												const LAString &outputname,
												const DoubleArray &pv_base, 
												const DoubleMatrix &pv_1,
												const DoubleMatrix &pv_2,
												const LAObject& riskEntity) const
{
	DoubleArray var;
	if (!pscenario2) var.resize(2);
	else var.resize(3);

	for (unsigned int j = 0; j < unders.getSize(); j++)
	{
		LAObjectHolder &under = unders.get(j);
		LADataStrings &errorStatus = dynamic_cast<LADataStrings &>
				(under.getData(PRICING_DATA_ERRORSTATUS, ISDEFINED).get());

		var[0] = pv_base[j];
		if (var[0] != FAIL_VALUE)
		{
			// no pv error case
			if (isgrid)
			{
				DoubleArray out(scenario1.getSize());
				for (unsigned int k = 0; k < scenario1.getSize(); k++)
				{
					var[1] = pv_1[k][j];
					if (pscenario2) var[2] = pv_2[k][j];
					// check
				if (var.size() == 2 && var[1] != FAIL_VALUE || 
					var.size() == 3 && var[1] != FAIL_VALUE && var[2] != FAIL_VALUE)
					{
						out[k] = method(var);
					}
					else
					{
						out[k] = FAIL_VALUE;
						if (errorStatus.end() == std::find(errorStatus.begin(), errorStatus.end(), outputname + ERROR))
						{
							errorStatus.push_back(outputname + ERROR);
						}
					}	
				}

				bool issemianalytic = false;
				const LADataHolder* dh = &(riskEntity.getData(PRICING_DATA_ANALYTICCALCTYPE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					LAString analytictype = dynamic_cast<const LADataString &>(dh->get());
					issemianalytic = (analytictype.toUpper() == SEMIANALYTIC);
				}
				if (issemianalytic)
				{
					throw LACoreInvalidData("Analytic risk is not supported now!",__FILE__,__LINE__);
					//// we stopped to calculate the analytic risk on 2012/11/26 because of bad performance of the linepricer
					////#define AP_CALIBRATION_DATA_RISK_GRID   "RiskGrid"
					////#define AP_CALIBRATION_DATA_ISPARALLELSHIFT   "isParallelShift"
					//unsigned int mdfySize = out.size();
					//
					//const LADataHolder* dh = &(riskEntity.getData(PRICING_DATA_RISKCURVETYPENAME, ISNOTNULL));
					//LAString keycurve = dynamic_cast<const LADataString &>(dh->get());

					//dh = &(riskEntity.getData(PRICING_DATA_RISKCURVETYPECURRENCY, ISNOTNULL));
					//LAString keyccy = dynamic_cast<const LADataString &>(dh->get());

					////get asof
					//dh = &(under.getData("PathEntity", NOCHECK));
					//if (!dh->isDefined() || dh->isNull())
					//{
					//	dh = &(under.getData("MarketParam", ISNOTNULL));
					//}
					//const LADataReference& pathref = dynamic_cast<const LADataReference &>(dh->get());
					//if (!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
					//{
					//	//const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(pathref.get().get());
					//	throw LACoreInvalidData("SemiAnalyticError",__FILE__,__LINE__);
					//}
					//
					//const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get());
					//const LAMathYieldCurvePro& yldPro = path.getIRCurvePro(keyccy);

					//LAString tmpRiskName = outputname;

					//const LAStringVector &gridArray = dynamic_cast<const LADataStrings &>(under.getData(tmpRiskName.toUpper() + "RiskGrid", ISNOTNULL).get()).get();
					//const DoubleArray &gridArrayVals = dynamic_cast<const  LADataDoubles &>(under.getData(tmpRiskName.toUpper() + "RiskGridActVals", ISNOTNULL).get()).get();
					//const bool isParallel = dynamic_cast<const LADataBool &>(under.getData(tmpRiskName.toUpper() + "isParallelShift", ISNOTNULL).get()).get();

					//if (isParallel)
					//	out.erase(out.begin());

					//LAStringVector marketgrids;
					//DoubleVector marketvals;
					//
					//out.resize(gridArrayVals.size());
					//yldPro.changeZeroRiskIntoMarketRisk(keycurve, gridArrayVals, out, marketgrids, marketvals);


					//under.remove(tmpRiskName.toUpper() + "RiskGrid");
					//under.add(tmpRiskName.toUpper() + "RiskGrid", new LADataStrings(marketgrids));
					//
					////now alyways parallel case
					//if (isParallel)
					//{
					//	double gridsum = std::accumulate(marketvals.begin(), marketvals.end(), 0.0);
					//	marketvals.insert(marketvals.begin(), gridsum);
					//}
					//under.add(outputname, new LADataDoubles(marketvals));
				
				}
				else
				{
					under.add(outputname, new LADataDoubles(out));
				}
			}
			else
			{
				var[1] = pv_1[0][j];
				if (pscenario2) var[2] = pv_2[0][j];
				double out = 0.0;
				// check
				if (var.size() == 2 && var[1] != FAIL_VALUE || 
					var.size() == 3 && var[1] != FAIL_VALUE && var[2] != FAIL_VALUE)
				{
					out = method(var);
				}
				else
				{
					out = FAIL_VALUE;
					errorStatus.push_back(outputname + ERROR);
				}	
				under.add(outputname, new LADataDouble(out));
			}
		}
		else
		{
			// pv error case
			if (isgrid)
			{
				DoubleArray out(scenario1.getSize(), FAIL_VALUE);
				under.add(outputname, new LADataDoubles(out));
			}
			else
			{
				under.add(outputname, new LADataDouble(FAIL_VALUE));
			}
			errorStatus.push_back(outputname + ERROR);
		}
	}
}

/*!
	@brief set risk value

	@param[in,out] unders underlying trade
	@param[in]     scenario1 scenario1
	@param[in]     pscenario2 pointer to scenario2
	@param[in]     coeffW coefficient to funcW
	@param[in]     funcW  function to calc risk
	@param[in]     method  function to calc risk
	@param[in]     outputname output risk name
	@param[in]     pv_base 
	@param[in]     pv_1  pv for scenario1
	@param[in]     pv_2  pv for scenario2
	
*/
void
LAPricePortfolioValue::setWaveRiskValue(const LADataMultiReference &unders,
										const LADataMultiReference &scenario1, 
												const LADataMultiReference *pscenario2,
												const DoubleMatrix &coeffW,
												LAFunctionBase &funcW,
												const LAFunctionBase &method,
												const LAString &outputname,
												const DoubleArray &pv_base, 
												const DoubleMatrix &pv_1,
												const DoubleMatrix &pv_2) const
{
	DoubleArray var;
	if (!pscenario2) var.resize(2);
	else var.resize(3);

	for (unsigned int j = 0; j < unders.getSize(); j++)
	{
		LAObjectHolder &under = unders.get(j);
		LADataStrings &errorStatus = dynamic_cast<LADataStrings &>
				(under.getData(PRICING_DATA_ERRORSTATUS, ISDEFINED).get());

		var[0] = pv_base[j];
		if (var[0] != FAIL_VALUE)
		{
			double val_before = 0.0;
			DoubleArray out(scenario1.getSize());
			for (unsigned int k = 0; k < scenario1.getSize(); k++)
			{
				DoubleArray var_w(2);
				var_w[0] = val_before;
				var[1] = pv_1[k][j];
				if (pscenario2) var[2] = pv_2[k][j];
				
				// check
				if (val_before != FAIL_VALUE && 
						var[1] != FAIL_VALUE &&
							(var.size() <= 2 || var[2] != FAIL_VALUE))
				{
					var_w[1] = method(var);
					funcW.setParam(coeffW[k]);
					out[k] = funcW(var_w);
				}
				else
				{
					out[k] = FAIL_VALUE;
					if (errorStatus.end() == std::find(errorStatus.begin(), errorStatus.end(), outputname + ERROR))
					{
						errorStatus.push_back(outputname + ERROR);
					}
					continue;
				}
				val_before = var_w[1];
			}
			under.add(outputname, new LADataDoubles(out));
		}
		else
		{
			// pv error case
			DoubleArray out(scenario1.getSize(), FAIL_VALUE);
			under.add(outputname, new LADataDoubles(out));
		}

	}
}

/*!
	@brief set scenario

	@param[in] name target object name to exchange
	@param[in] scenario scenario
	@param[in] rpool reference object pool
	
*/
void
LAPricePortfolioValue::setScenario(const LAString& name, 
								LAObject& scenario, 
								LACoreReferencePool& rpool) const
{
	LAObjectHolder& h = rpool.getReference(name);
	if (!h.isDefined()) return;
	scenario.update();
	h.setEntity(&scenario, false);
}

/*!
	@brief back to base market

	@param[in] name target object name to back to base 
	@param[in] objPool object pool
	@param[in] rpool reference object pool
	
*/
void
LAPricePortfolioValue::backToBase(const LAString& name, 
								LAObjectPool& objPool, 
								LACoreReferencePool& rpool) const
{
	LAObjectHolder& h = rpool.getReference(name);
	if (!h.isDefined()) return;
	LAObject* pe = &objPool.getObject(name).get();
	pe->update();
	h.setEntity(pe, false);
}
