/*! @file
    @brief Source code of of path element class that represents curve object for libor market model


*/
// Copyright (C) 2007, Mizuho International London..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "LAModelDynamicsLMMCurve.h"
#include "LAAlgorithm.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ LARatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
LARatesPathElementLMMCurve::LARatesPathElementLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: LARatesPathElementCurve(t), mpRefCount(0), mpTenor(0), mpDeltaTenor(0),
   mpInitialData_L(0), mpInitialCurve(0), mtP(0.0), mlt(0.0), mlt_(0.0), mPos(10000)
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
	mpInitialCache = new map<double, double>();
	if ((*mpTenor)[0] == 0.0)//if first element is 0, erase it
	{
		mpTenor->erase(mpTenor->begin());
		mpDeltaTenor->erase(mpDeltaTenor->begin());
	}

	mpInitialData_L = new SCALARARRAY(mpTenor->size() - 1);

	if (m_t == 0.0)
	{
		mtP = 1.0;
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

/*		double criteria = 1.0;
		for (unsigned int i = 1; i < LASTRING_DOUBLESIZE; i++)
			criteria *= 0.1;
*/		
		if (diff > INFINITESIMAL)
		{
			//error
			throw LACoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
	
}


/*!
	@brief destructor
*/
LARatesPathElementLMMCurve::~LARatesPathElementLMMCurve() 
{
	clear();
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementLMMCurve::LARatesPathElementLMMCurve(const LARatesPathElementLMMCurve& v)
: LARatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
mpTenor(v.mpTenor), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve), mpInitialCache(v.mpInitialCache), mtP(v.mtP),
mlt(v.mlt), mlt_(v.mlt_), mPos(v.mPos) 
{	
	(*mpRefCount)++;
}
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementLMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_LMMCURVE ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementLMMCurve::getType() const
{
	return PE_LMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementLMMCurve::clone() const
{
    try 
	{
		return new LARatesPathElementLMMCurve(*this);
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
LARatesPathElementLMMCurve&
LARatesPathElementLMMCurve::operator = (const LARatesPathElementLMMCurve& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

/*	if (!a.isTypeOf(PE_LMMCURVE)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesPathElementLMMCurve : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}*/

	// コピー。
	clear();
	
	mValue = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mValue;
	mpInitialData_L = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpInitialData_L;
	mpInitialCurve = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpInitialCurve;
	mpTenor = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpTenor;
	mpDeltaTenor = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpDeltaTenor;
	mpRefCount = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpRefCount;
	mpInitialCache = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mpInitialCache;
	mtP = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mtP;
	mlt = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mlt;
	mlt_ = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mlt_;
	mPos = dynamic_cast<const LARatesPathElementLMMCurve&>(a).mPos;

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
LARatesPathElementLMMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
	}
	else if (m_t + INFINITESIMAL >= T) return 1.0;
	else if (m_t == 0.0) return mpInitialCurve->getP(T);

	double lt = 1.0;
	double lt_ = 1.0;
	double end_t = T;
	bool isOnTenor = false;
	int i, j;
	unsigned int pos  = 0;
	unsigned int tSize = mpTenor->size();
	double _T = T > mpTenor->back() ? mpTenor->back() : T;
	LAAlgorithm::locate<DoubleArray,double>(*mpTenor, _T, tSize, pos);
	if ((*mpTenor)[pos] == _T)
		isOnTenor = true;	


	for (i = pos - 1, j = mValue.size() - (tSize/* - 1*/ - pos); j >= 0; j--, i--)
//    for (i = mpTenor->size() - 1, j = mValue.size(); j >= 0; j--, i--)
	{
/*		if ((*mpTenor)[i] == _T)
		{
			isOnTenor = true;
			continue;
		}*/
		
		if (/*(*mpTenor)[i] < _T && */(*mpTenor)[i] + INFINITESIMAL >= m_t)
		{
			double multiple = (end_t - (*mpTenor)[i]) * (*mpDeltaTenor)[i] / ((*mpTenor)[i + 1] - (*mpTenor)[i]);
			
			lt *= 1.0 + mValue[j] * multiple;
			
			if (!isOnTenor)
				lt_ *= 1.0 + (*mpInitialData_L)[i] * multiple;
			
			end_t = (*mpTenor)[i];
		}
		else/* if ((*mpTenor)[i] < _T && (*mpTenor)[i] < m_t)*/
		{
			break;
		}
	}

	double ret;
	if (isOnTenor) ret = 1.0 / lt;
	else
	{
		//see "Mind the cap" by Peter jackel
		double P = 0.0;
		map<double, double>::iterator it = mpInitialCache->find(_T);
		if (it != mpInitialCache->end())
		{
			P = it->second;
		}
		else
		{
			P = mpInitialCurve->getP(_T);
			mpInitialCache->insert(make_pair(_T, P));
		}
		ret = (lt - 1.0) / (lt_ - 1.0) * (mtP / P - 1.0) + 1.0;		
		//ret = (lt - 1.0) / (lt_ - 1.0) * (mpInitialCurve->getP(m_t) / mpInitialCurve->getP(_T) - 1.0) + 1.0;
		ret = 1.0 / ret;
	}

	return ret;
}

/*!
    @brief set initial time of this curve
	@param[in] t initial time
*/
void
LARatesPathElementLMMCurve::set_t (double t)
{
	if (m_t == t) return;
	LARatesPathElementCurve::set_t(t);
	mValue.clear();
	if (mpInitialCurve)
	{
		mtP = mpInitialCurve->getP(m_t);
	}
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

/*		double criteria = 1.0;
		for (unsigned int i = 1; i < LASTRING_DOUBLESIZE; i++)
			criteria *= 0.1;
*/		
		if (diff > INFINITESIMAL)
		{
			//error
			throw LACoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
}
/*!
    @brief get initial libor rate
	@param[in] ts start time
	@param[in] te end time
	@param[in] delta delta between ts and te of Libor day count
	@param[in] curve curve object

	@return initial libor rate
*/
double
LARatesPathElementLMMCurve::getL (double ts, double te, double delta, const LARatesPathElementCurve& curve) const
{
	return (curve.getP(ts) / curve.getP(te) - 1.0) / delta;
}


/*!
    @brief set value
	@param[in] curve curve object
*/	
void
LARatesPathElementLMMCurve::set(const LARatesPathElementBase& a)
{
	if (a.isTypeOf(PE_LMMCURVE))
	{
		*this = dynamic_cast<const LARatesPathElementLMMCurve&>(a);
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
			delete mpInitialCache;
		}
		
		mpRefCount = new int(1);
		mpInitialCurve = dynamic_cast<LARatesPathElementCurve*>(curve.clone());
		mpInitialData_L = new SCALARARRAY(mpDeltaTenor->size());	
		mpInitialCache = new map<double, double>();
		mtP = mpInitialCurve->getP(m_t);
	
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; j >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			(*mpInitialData_L)[j] = getL(ts, te, (*mpDeltaTenor)[j], curve);
			if (i >= 0) mValue[i] = (*mpInitialData_L)[j];
		}
	}
	else //それ以外は単にLデータをセット
	{
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; i >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			mValue[i] = getL(ts, te, (*mpDeltaTenor)[j], curve);
		}	
	}

}

/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
LARatesPathElementLMMCurve::clear()
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
		delete mpInitialCache;
		mpInitialData_L = 0;
		mpTenor = 0;
		mpDeltaTenor = 0;
		mpInitialCurve = 0;
		mpRefCount = 0;
	}
	mValue.clear();
}
