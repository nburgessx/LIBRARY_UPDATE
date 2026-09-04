/*! @file
    @brief Source code of drift function of fx sde
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftFX.h"

#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLMathAttrSDE.h"
#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLPriceDriftFX ===================================
/*!
	@brief default constructor
	@param[in] s displaced diffusion parameter
*/
AQLPriceDriftFX::AQLPriceDriftFX(const double s, SDE_TYPE type)
: mpNumeraireD(NULL), mpNumeraireF(NULL), mS(s), mType(type)
{

}
/*!
	@brief constructor
	@param[in] sdeAttrNameD data name of domestic ir model
	@param[in] sdeAttrNameD data name of foreign ir model
	@param[in] s displaced diffusion parameter
*/
AQLPriceDriftFX::AQLPriceDriftFX(const AQLString& sdeAttrNameD, const AQLString& sdeAttrNameF, const double s, SDE_TYPE type)
: mpNumeraireD(NULL), mpNumeraireF(NULL), mSDEAttrNameD(sdeAttrNameD), mSDEAttrNameF(sdeAttrNameF), mS(s), mType(type) 
{

}


/*!
	@brief copy constructor
*/
/*AQLPriceDriftFX::AQLPriceDriftFX(const AQLPriceDriftFX& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLPriceDriftFX::~AQLPriceDriftFX() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftFX::clone() const	
{
    try 
	{
		return new AQLPriceDriftFX(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceDriftFX::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFX ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftFX::getType() const
{
	return FN_DRIFTFX;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] must be time
    @return drift value

*/
double
AQLPriceDriftFX::operator()(const DoubleArray& x) const
{
	const AQLRatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const AQLRatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

	double te = x[2]; double ts = x[0];
    double numeRatioD = mpNumeraireD->operator ()(te) / mpNumeraireD->operator ()(ts);
    double numeRatioF = mpNumeraireF->operator ()(te) / mpNumeraireF->operator ()(ts);

	double ret = AQLMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}

//furuya
double
AQLPriceDriftFX::getDriftValue(const DoubleArray& x) const
{

	const AQLRatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const AQLRatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

	double te = x[2]; double ts = x[0];
    //double numeRatioD = mpNumeraireD->operator ()(te) / mpNumeraireD->operator ()(ts);
    //double numeRatioF = mpNumeraireF->operator ()(te) / mpNumeraireF->operator ()(ts);
	double numeRatioD = pInitialCurveD->getP(ts) / pInitialCurveD->getP(te);
	double numeRatioF = pInitialCurveF->getP(ts) / pInitialCurveF->getP(te);

	//double a = mpNumeraireD->operator ()(te);
	//double b = mpNumeraireD->operator ()(ts);
	//double c = mpNumeraireF->operator ()(te);
	//double d = mpNumeraireF->operator ()(ts);
	//double aa = pInitialCurveD->getP(te);
	//double bb = pInitialCurveD->getP(ts);
	//double cc = pInitialCurveF->getP(te);
	//double dd = pInitialCurveF->getP(ts);

	double ret = AQLMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}

/*!
    @brief return string representaion
    @return string representaion (domestic sde attr name ":" foregin sde attr name)
*/
AQLString
AQLPriceDriftFX::convertToString(void) const
{
	AQLString ret(mSDEAttrNameD);
	ret += ":";
	ret += mSDEAttrNameF;
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion (domestic sde attr name ":" foregin sde attr name)
*/
void
AQLPriceDriftFX::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() != 2)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}
	mSDEAttrNameD = tmp.get()[0];
	mSDEAttrNameF = tmp.get()[1];
}


/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftFX::setUp(AQLMathPathEntity& path)
{
	AQLDataHolder* dh = &path.getData(mSDEAttrNameD, ISNOTNULL);
	AQLMathAttrSDE* pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().AQLRatesSDEBase::getNumeraire();
	const AQLRatesPathElementCurve* pInitialCurveD = dynamic_cast<const AQLRatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());

	dh = &path.getData(mSDEAttrNameF, ISNOTNULL);
	pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpNumeraireF = pattrsde->getSDE().AQLRatesSDEBase::getNumeraire();
	const AQLRatesPathElementCurve* pInitialCurveF = dynamic_cast<const AQLRatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());
	
	cache_pos.clear();
	cache.clear();
	const DoubleArray& time = pattrsde->getSDE().getBM()->getTimeGrid();
	unsigned int size = time.size();
	for (unsigned int i = 1; i < size; i++)
	{
		cache_pos[time[i - 1]] = i;
		double adj = AQLMath::log(pInitialCurveD->getP(time[i - 1]) / pInitialCurveD->getP(time[i])) / (time[i] - time[i - 1])
					- 0.5 * (pInitialCurveD->getF(time[i - 1]) + pInitialCurveD->getF(time[i]))
					- AQLMath::log(pInitialCurveF->getP(time[i - 1]) / pInitialCurveF->getP(time[i])) / (time[i] - time[i - 1])
					+ 0.5 * (pInitialCurveF->getF(time[i - 1]) + pInitialCurveF->getF(time[i]));
		cache.push_back(adj);
	}	
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQLPriceDriftFX::integral(const vector<pair<double,double> >& x) const
{
	//if (mType == dX)
	//	return  (0.5 * (x[1].first + x[1].second) - mS) * 
	//		(-AQLMath::log(mpNumeraireD->getCurve(x[0].first).getP(x[0].second))
	//			+ AQLMath::log(mpNumeraireF->getCurve(x[0].first).getP(x[0].second)));
	//else
	//	return  0.5 * ((1.0 - mS / x[1].first) + (1.0 - mS / x[1].second)) * 
	//		(-AQLMath::log(mpNumeraireD->getCurve(x[0].first).getP(x[0].second))
	//			+ AQLMath::log(mpNumeraireF->getCurve(x[0].first).getP(x[0].second)));

	/*return 0.5 * (mpNumeraireD->getCurve(x[0].first).getF(x[0].first) 
				- mpNumeraireF->getCurve(x[0].first).getF(x[0].first)
				+ mpNumeraireD->getCurve(x[0].second).getF(x[0].second) 
				- mpNumeraireF->getCurve(x[0].second).getF(x[0].second))
				* (x[0].second - x[0].first);*/
	double a1 = mpNumeraireD->getCurve(x[0].first).getF(x[0].first);
	double a2 = mpNumeraireF->getCurve(x[0].first).getF(x[0].first);
	double a3 = mpNumeraireD->getCurve(x[0].second).getF(x[0].second);
	double a4 = mpNumeraireF->getCurve(x[0].second).getF(x[0].second);


	double adj;
	unsigned int pos;
	if ((pos = cache_pos[x[0].first]) != 0) 
		adj = cache[pos - 1];
	else
	{
		const AQLRatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();

		const AQLRatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

		adj = AQLMath::log(pInitialCurveD->getP(x[0].first) / pInitialCurveD->getP(x[0].second)) / (x[0].second - x[0].first)
					- 0.5 * (pInitialCurveD->getF(x[0].first) + pInitialCurveD->getF(x[0].second))
					- AQLMath::log(pInitialCurveF->getP(x[0].first) / pInitialCurveF->getP(x[0].second)) / (x[0].second - x[0].first)
					+ 0.5 * (pInitialCurveF->getF(x[0].first) + pInitialCurveF->getF(x[0].second));

	}

	return (0.5*(a1-a2+a3-a4)+adj)*(x[0].second - x[0].first);

}