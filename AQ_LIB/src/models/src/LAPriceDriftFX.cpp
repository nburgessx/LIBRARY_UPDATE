/*! @file
    @brief Source code of drift function of fx sde



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftFX.cpp
//
//  SYNOPSIS    :       LAPriceDriftFX
//  DESCRIPTION :       Source code of drift function of fx sde
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceDriftFX.h"

#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LABasic.h"

using namespace std;

//================ LAPriceDriftFX ===================================
/*!
	@brief default constructor
	@param[in] s displaced diffusion parameter
*/
LAPriceDriftFX::LAPriceDriftFX(const double s, SDE_TYPE type)
: mpNumeraireD(NULL), mpNumeraireF(NULL), mS(s), mType(type)
{

}
/*!
	@brief constructor
	@param[in] sdeAttrNameD data name of domestic ir model
	@param[in] sdeAttrNameD data name of foreign ir model
	@param[in] s displaced diffusion parameter
*/
LAPriceDriftFX::LAPriceDriftFX(const LAString& sdeAttrNameD, const LAString& sdeAttrNameF, const double s, SDE_TYPE type)
: mpNumeraireD(NULL), mpNumeraireF(NULL), mSDEAttrNameD(sdeAttrNameD), mSDEAttrNameF(sdeAttrNameF), mS(s), mType(type) 
{

}


/*!
	@brief copy constructor
*/
/*LAPriceDriftFX::LAPriceDriftFX(const LAPriceDriftFX& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPriceDriftFX::~LAPriceDriftFX() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftFX::clone() const	
{
    try 
	{
		return new LAPriceDriftFX(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceDriftFX::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFX ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftFX::getType() const
{
	return FN_DRIFTFX;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] must be time
    @return drift value

*/
double
LAPriceDriftFX::operator()(const DoubleArray& x) const
{
	const LARatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const LARatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

	double te = x[2]; double ts = x[0];
    double numeRatioD = mpNumeraireD->operator ()(te) / mpNumeraireD->operator ()(ts);
    double numeRatioF = mpNumeraireF->operator ()(te) / mpNumeraireF->operator ()(ts);

	double ret = LAMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}

//furuya
double
LAPriceDriftFX::getDriftValue(const DoubleArray& x) const
{

	const LARatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const LARatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

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

	double ret = LAMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}

/*!
    @brief return string representaion
    @return string representaion (domestic sde attr name ":" foregin sde attr name)
*/
LAString
LAPriceDriftFX::convertToString(void) const
{
	LAString ret(mSDEAttrNameD);
	ret += ":";
	ret += mSDEAttrNameF;
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion (domestic sde attr name ":" foregin sde attr name)
*/
void
LAPriceDriftFX::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() != 2)
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}
	mSDEAttrNameD = tmp.get()[0];
	mSDEAttrNameF = tmp.get()[1];
}


/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftFX::setUp(LAMathPathEntity& path)
{
	LADataHolder* dh = &path.getData(mSDEAttrNameD, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().LARatesSDEBase::getNumeraire();
	const LARatesPathElementCurve* pInitialCurveD = dynamic_cast<const LARatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());

	dh = &path.getData(mSDEAttrNameF, ISNOTNULL);
	pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpNumeraireF = pattrsde->getSDE().LARatesSDEBase::getNumeraire();
	const LARatesPathElementCurve* pInitialCurveF = dynamic_cast<const LARatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());
	
	cache_pos.clear();
	cache.clear();
	const DoubleArray& time = pattrsde->getSDE().getBM()->getTimeGrid();
	unsigned int size = time.size();
	for (unsigned int i = 1; i < size; i++)
	{
		cache_pos[time[i - 1]] = i;
		double adj = LAMath::log(pInitialCurveD->getP(time[i - 1]) / pInitialCurveD->getP(time[i])) / (time[i] - time[i - 1])
					- 0.5 * (pInitialCurveD->getF(time[i - 1]) + pInitialCurveD->getF(time[i]))
					- LAMath::log(pInitialCurveF->getP(time[i - 1]) / pInitialCurveF->getP(time[i])) / (time[i] - time[i - 1])
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
LAPriceDriftFX::integral(const vector<pair<double,double> >& x) const
{
	//if (mType == dX)
	//	return  (0.5 * (x[1].first + x[1].second) - mS) * 
	//		(-LAMath::log(mpNumeraireD->getCurve(x[0].first).getP(x[0].second))
	//			+ LAMath::log(mpNumeraireF->getCurve(x[0].first).getP(x[0].second)));
	//else
	//	return  0.5 * ((1.0 - mS / x[1].first) + (1.0 - mS / x[1].second)) * 
	//		(-LAMath::log(mpNumeraireD->getCurve(x[0].first).getP(x[0].second))
	//			+ LAMath::log(mpNumeraireF->getCurve(x[0].first).getP(x[0].second)));

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
		const LARatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();

		const LARatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

		adj = LAMath::log(pInitialCurveD->getP(x[0].first) / pInitialCurveD->getP(x[0].second)) / (x[0].second - x[0].first)
					- 0.5 * (pInitialCurveD->getF(x[0].first) + pInitialCurveD->getF(x[0].second))
					- LAMath::log(pInitialCurveF->getP(x[0].first) / pInitialCurveF->getP(x[0].second)) / (x[0].second - x[0].first)
					+ 0.5 * (pInitialCurveF->getF(x[0].first) + pInitialCurveF->getF(x[0].second));

	}

	return (0.5*(a1-a2+a3-a4)+adj)*(x[0].second - x[0].first);

}