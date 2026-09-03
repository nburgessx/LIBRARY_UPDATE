/*! @file
    @brief Source code of of path element class that represents curve object for HJM


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
#define GAUSSLEGENDREPOINTNUM 20


#include "LAModelDynamicsHJMCurve.h"
#include "LAAlgorithm.h"
#include "LA1DDataSet.h"
#include "LAGaussLegendre.h"
#include "LABasic.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ LARatesPathElementHJMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
LARatesPathElementHJMCurve::LARatesPathElementHJMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: LARatesPathElementCurve(t), mpRefCount(0), mpTenor(0), mpDeltaTenor(0),
   mpInitialData_L(0), mpInitialCurve(0)
{
	if (tenor.size() <= 1 || (tenor[0] == 0.0 && tenor.size() <= 2))
	{
		//error
		throw LACoreInvalidData("tenor size is one", __FILE__, __LINE__);
	}
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw LACoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}

	mpRefCount = new int(1);
	mpTenor = new DoubleArray(tenor);	
	mpDeltaTenor = new DoubleArray(delta_tenor);
	if ((*mpTenor)[0] == 0.0)//if first element is 0, erase it
	{
		mpTenor->erase(mpTenor->begin());
		mpDeltaTenor->erase(mpDeltaTenor->begin());
	}

	mpInitialData_L = new SCALARARRAY(mpTenor->size() - 1);
	set_t();
}

/*!
	@brief destructor
*/
LARatesPathElementHJMCurve::~LARatesPathElementHJMCurve() 
{
	clear();
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementHJMCurve::LARatesPathElementHJMCurve(const LARatesPathElementHJMCurve& v)
: LARatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
   mpTenor(v.mpTenor), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve)
{	
	(*mpRefCount)++;
}
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementHJMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_HJMCURVE ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementHJMCurve::getType() const
{
	return PE_HJMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementHJMCurve::clone() const
{
    try 
	{
		return new LARatesPathElementHJMCurve(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
LARatesPathElementHJMCurve&
LARatesPathElementHJMCurve::operator = (const LARatesPathElementHJMCurve& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

/*	if (!a.isTypeOf(PE_HJMCURVE)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesPathElementHJMCurve : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}*/

	// コピー。
	clear();
	
	mValue = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mValue;
	mpInitialData_L = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mpInitialData_L;
	mpInitialCurve = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mpInitialCurve;
	mpTenor = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mpTenor;
	mpDeltaTenor = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mpDeltaTenor;
	mpRefCount = dynamic_cast<const LARatesPathElementHJMCurve&>(a).mpRefCount;

	(*mpRefCount)++;

	LARatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesPathElementHJMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
	}
	else if (m_t >= T) return 1.0;
	else if (m_t == 0.0) return mpInitialCurve->getP(T);
	

	unsigned int pos  = 0;
	LAGaussLegendre GL(GAUSSLEGENDREPOINTNUM);// integral method

	LAAlgorithm::locate<DoubleArray,double>(*mpTenor, m_t, mpTenor->size(), pos);
	
	DoubleArray tenor = *mpTenor;
	tenor.erase(tenor.begin(), tenor.begin() + pos + 1);

	LA1DDataSet method;
	//method.setParam(mValue);
	method.set(tenor, mValue);
	method.setInterpolation(*mpInter);
	
	double ret = method.integral(m_t, T, &GL);
	ret = LAMath::exp(-ret * (T - m_t));

	return ret;	//method.integral(m_t, T, &GL);

}

/*!
    @brief set initial time of this curve
	@param[in] t initial time
*/
void
LARatesPathElementHJMCurve::set_t (double t)
{
	if (m_t == t) return;
	LARatesPathElementCurve::set_t(t);
	mValue.clear();
	set_t();
}

/*!
    @brief set initial time of this curve
	
*/
void
LARatesPathElementHJMCurve::set_t(void)
{
	if (m_t == 0.0)
	{
		mValue.resize(mpTenor->size() - 1);
		return;
	}	
	unsigned int pos;
	LAAlgorithm::locate<DoubleArray, double>(*mpTenor, m_t, mpTenor->size(), pos);
	if (pos == mpTenor->size())
	{
		throw LACoreInvalidData("curve start time is after last tenor", __FILE__, __LINE__);
	}
	else if ((*mpTenor)[pos] == m_t)
	{
		mValue.resize(mpTenor->size() - pos - 1);
	}
	else
	{
		//文字列変換等で値が少しずれるかもしれないので厳密にテナー上にのっていないくても可とする。
		double diff;
		if ((*mpTenor)[pos] - m_t > m_t - (*mpTenor)[pos - 1])
		{
			diff = m_t - (*mpTenor)[pos - 1];
			mValue.resize(mpTenor->size() - pos);
		}
		else
		{
			diff = (*mpTenor)[pos] - m_t;
			mValue.resize(mpTenor->size() - pos - 1);
		}
		
		if (diff > INFINITESIMAL)
		{
			//error
			throw LACoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
	
}


/*!
    @brief set value
	@param[in] curve curve object
*/	
void
LARatesPathElementHJMCurve::set(const LARatesPathElementBase& a)
{
	if (a.isTypeOf(PE_HJMCURVE))
	{
		*this = dynamic_cast<const LARatesPathElementHJMCurve&>(a);
		return;
	}
	
	LARatesPathElementCurve::set(a);
	const LARatesPathElementCurve& curve = dynamic_cast<const LARatesPathElementCurve&>(a);
	
	

	if (curve.get_t() == 0.0)//aが0スタートのカーブのときは初期カーブとみなす
	{
		DoubleArray* p = mpTenor;
		mpTenor = new DoubleArray(*p);
		DoubleArray* p2 = mpDeltaTenor;
		mpDeltaTenor = new DoubleArray(*p2);
		if ((*mpRefCount)-- == 1)
		{
			mpInitialData_L->clear();
			p->clear();
			p2->clear();
			delete mpInitialData_L;
			delete p;
			delete p2;
			delete mpInitialCurve;
			delete mpRefCount;
		}
		
		mpRefCount = new int(1);
		mpInitialCurve = dynamic_cast<LARatesPathElementCurve*>(curve.clone());
		mpInitialData_L = new SCALARARRAY(mpDeltaTenor->size());	
	
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; j >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			//double te = (*mpTenor)[j + 1];
			(*mpInitialData_L)[j] = curve.getF(ts);
			if (i >= 0) mValue[i] = (*mpInitialData_L)[j];
		}
	}
	else //それ以外は単にLデータをセット
	{
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; i >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			//double te = (*mpTenor)[j + 1];
			mValue[i] = curve.getF(ts);
		}	
	}

}
void
LARatesPathElementHJMCurve::setInterpolationMethod(LAInterpolationBase* pinter) 
{
	mpInter = pinter; 
}

/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
LARatesPathElementHJMCurve::clear()
{
	if ((*mpRefCount)-- == 1)
	{
		mpInitialData_L->clear();
		mpTenor->clear();
		mpDeltaTenor->clear();
		delete mpInitialData_L;
		delete mpTenor;
		delete mpDeltaTenor;
		delete mpInitialCurve;
		delete mpRefCount;
		mpInitialData_L = 0;
		mpTenor = 0;
		mpDeltaTenor = 0;
		mpInitialCurve = 0;
		mpRefCount = 0;
	}
	mValue.clear();
}
