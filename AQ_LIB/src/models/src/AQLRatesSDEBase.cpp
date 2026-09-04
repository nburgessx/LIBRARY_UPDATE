/*! @file
    @brief Declaration of base class of sde class



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesSDEBase.h"
#include "AQLRatesSDEIntegralBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLAlgorithm.h"

using namespace std;
//================ AQLRatesSDEBase ===================================
/*!
	@brief default constructor
	@param[in] type sde type (dX or DIVIDEdXbyX)
*/
AQLRatesSDEBase::AQLRatesSDEBase(SDE_TYPE type)
: mType(type), mID(0), mpNumeraire(0),
   mpBM(0), mpInter(0), mpIntegral(0), mpInitial(0), mpTemplate(0), mPos(0)
/*, mpPathElement(0)*//*, mIsCalcAll(false)*/
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesSDEBase::AQLRatesSDEBase(const AQLRatesSDEBase& v) 
: AQLCoreFunctionBase(v), mType(v.mType), mID(v.mID), mpNumeraire(0), mpBM(0),
   mpInter(0), mTimeGrid(v.mTimeGrid), mpIntegral(0), mpInitial(0), mpTemplate(0), mPos(v.mPos)
/*, mpPathElement(0)*//*, mIsCalcAll(v.mIsCalcAll)*/
{
	if (v.mpNumeraire != 0)
		mpNumeraire = dynamic_cast<AQLRatesNumeraireBase*>(v.mpNumeraire->clone());
	if (v.mpIntegral != 0)
		mpIntegral = dynamic_cast<AQLRatesSDEIntegralBase*>(v.mpIntegral->clone());
	if (v.mpInitial != 0)
		mpInitial = dynamic_cast<AQLRatesPathElementBase*>(v.mpInitial->clone());
	if (v.mpTemplate != 0)
		mpTemplate = dynamic_cast<AQLRatesPathElementBase*>(v.mpTemplate->clone());
	if (v.mpInter != 0)
		mpInter = dynamic_cast<AQLRatesPEInterpolationBase*>(v.mpInter->clone());
//	if (v.mpPathElement != 0)
//		mpPathElement = dynamic_cast<AQLRatesPathElementBase*>(v.mpPathElement->clone());


	mPath.resize(v.mPath.size());
	for (unsigned int i = 0; i < mPath.size(); i++)
		mPath[i] = v.mPath[i]->clone();
	
	mDrift.resize(v.mDrift.size());
	for (unsigned int i = 0; i < mDrift.size(); i++)
		mDrift[i] = dynamic_cast<AQLFunctionBase*>(v.mDrift[i]->clone());

	mVolatility.resize(v.mVolatility.size());
	for (unsigned int i = 0; i < mVolatility.size(); i++)
	{
		mVolatility[i].resize(v.mVolatility[i].size());
		for (unsigned int j = 0; j < mVolatility[i].size(); j++)
			mVolatility[i][j] = dynamic_cast<AQLFunctionBase*>(v.mVolatility[i][j]->clone());
	}

    try 
	{
		if (v.mpBM != 0) mpBM = v.mpBM->clone();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }

}

/*!
	@brief destructor
*/
AQLRatesSDEBase::~AQLRatesSDEBase() 
{
	delete mpNumeraire;	
	delete 	mpBM;	
	mTimeGrid.clear();	

	for (unsigned int i = 0; i < mPath.size(); i++)
		delete mPath[i];
	mPath.clear();

	for (unsigned int i = 0; i < mDrift.size(); i++)
		delete mDrift[i];
	mDrift.clear();		

	for (unsigned int i = 0; i < mVolatility.size(); i++)
	{
		for (unsigned int j = 0; j < mVolatility[i].size(); j++)
			delete mVolatility[i][j];									
		mVolatility[i].clear();
	}

	mVolatility.clear();	
	delete mpIntegral; 
	delete mpInitial;	
	delete mpTemplate;
	delete mpInter;
//	delete mpPathElement;
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesSDEBase::isTypeOf(function_t id) const
{
	return (id==FN_SDEBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesSDEBase::getType() const
{
	return FN_SDEBASE;
}

/*!
    @brief get path
    @return path
*/
const vector<AQLRatesPathElementBase*>&
AQLRatesSDEBase::getPath(void)
{
/*	if (mpBM->getCurrentID() != mID || !mIsCalcAll)
	{
		if ((mPos == 0 || mPos == mpBM->getTimeGrid().size() - 1)//
			&& mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
		}
		//calculate path
		calcPath(mpBM->getTimeGrid().size() - 1);
		mID = mpBM->getCurrentID();
		mPos = mpBM->getTimeGrid().size() - 1;
		mpPathElement->set(mPath[mTimeGrid.size() - 1]);
		mIsCalcAll = true;
	}*/
	getPathElement(mpBM->getTimeGrid().size() - 1);
	return mPath;
}

/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
/*AQLRatesPathElementBase*
AQLRatesSDEBase::getPathElement(unsigned int pos)
{
	const DoubleArray& timegrid = mpBM->getTimeGrid();
	if (pos > timegrid.size() - 1)
	{
		throw AQLCoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	if (mpBM->getCurrentID() == mID && pos == mPos)
		return mpPathElement;
	
	if (mpBM->getCurrentID() != mID)
	{
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
		}
		mPos = 0;
	}
	
	mID = mpBM->getCurrentID();

    //calculate path
	calcPath(pos);
	
	mPos = pos;
		
	return mpPathElement;
}*/
/*!
	@brief set sde integral function
	@param[in] pintegral integral function
	@note this class is pointer owner of the integral function 
*/	
void
AQLRatesSDEBase::setIntegralFunction(AQLRatesSDEIntegralBase* pintegral)
{
	delete mpIntegral; 
	mpIntegral = pintegral;
	mpIntegral->setSDEType(mType);
}
/*!
	@brief set numeraire
	@param[in] pnumeraire	pointer to numeraire
	@note this class becomes pointer owner of the input numeraire 
*/
void
AQLRatesSDEBase::setNumeraire(AQLRatesNumeraireBase* pnumeraire)
{
	delete mpNumeraire;
	mpNumeraire = pnumeraire;
}
/*!
	@brief set drift
	@param[in] drift	drift
	@note this class becomes pointer owner of the input drift function 
*/								
void
AQLRatesSDEBase::setDrift(std::vector<AQLFunctionBase*>& drift) 
{
	for (unsigned int i = 0; i < mDrift.size(); i++)
		delete mDrift[i];
	mDrift.clear();		
	mDrift = drift;
}
/*!
	@brief set volatility
	@param[in] volatility	volatility
	@note this class becomes pointer owner of the volatility function 
*/												
void
AQLRatesSDEBase::setVolatility(std::vector<std::vector<AQLFunctionBase*> >& volatility)
{	
	for (unsigned int i = 0; i < mVolatility.size(); i++)
	{
		for (unsigned int j = 0; j < mVolatility[i].size(); j++)
			delete mVolatility[i][j];									
		mVolatility[i].clear();
	}
	mVolatility.clear();
	mVolatility = volatility;
}


/*!
	@brief set templete of output path element	
	@param[in] pelement templete of output path element
	@note this class becomes pointer owner of input path element	
*/	
void
AQLRatesSDEBase::setOutputTemplate(const AQLRatesPathElementBase* pelement)
{
	delete mpTemplate;
	mpTemplate = pelement;
}
/*!
	@brief set initial value
	@param[in] initial initial value
*/	
void
AQLRatesSDEBase::setInitialValue (const AQLRatesPathElementBase& initial)
{
	delete mpInitial;
	mpInitial = initial.clone();
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
AQLRatesSDEBase::check(void) const
{
	const DoubleArray& grid = mpBM->getTimeGrid();
	if (mTimeGrid.size() == 0) return false;
	
	unsigned int j = 0;
	for (unsigned int i = 0; i < grid.size(); i++)
	{
		if (mTimeGrid[j] == grid[i])
			j++;
    }
	if (j != mTimeGrid.size()) return false;
	return true;
}
/*!
    @brief set up this class for path calculation

*/
void
AQLRatesSDEBase::setUp()
{
	if (!check())
	{
		//error
		throw AQLCoreInvalidData("SDE check status is false", __FILE__, __LINE__);
	}
	mID = 0;
	mPos = 0;
	for (unsigned int i = 0; i < mPath.size(); i++)
		delete mPath[i];
	mPath.clear();
	mPath.resize(mTimeGrid.size());
	for (unsigned int i = 0; i < mPath.size(); i++)
	{
		if (i == 0)
		{
			//delete mPath[0];
			mPath[0] = mpTemplate->clone();
			if (mpTemplate->isTypeOf(PE_CURVE))
				dynamic_cast<AQLRatesPathElementCurve*>(mPath[i])->set_t(mTimeGrid[i]);

			mPath[0]->set(*mpInitial);
//			mpPathElement = mPath[0]->clone(); 
		}
		else 
		{
			//delete mPath[i];
			mPath[i] = mPath[0]->clone();
			if (mpTemplate->isTypeOf(PE_CURVE))
				dynamic_cast<AQLRatesPathElementCurve*>(mPath[i])->set_t(mTimeGrid[i]);
		}
	}
	
	if (mpInter != 0) mpInter->init();
	if (mpTemplate->isTypeOf(PE_CURVE) && mpNumeraire != 0)
	{
		mpNumeraire->setInterpolationMethod(mpInter);
		mpNumeraire->setSDE(this);
	}

	if (mpIntegral->getIntegralType() == LOG_INTEGRAL && mVolatility.at(0).size() > 1)
	{
		//error
		throw AQLCoreInvalidData("Multi volatility is not support when SDE IntegralType is LOG", __FILE__, __LINE__);
	}
		
}
/*!
    @brief return string representaion
	@return  string representaion
*/
/*AQLString
AQLRatesSDEBase::convertToString(void) const
{
	return "";
}*/

/*!
    @brief transform from string representaion
	@param[in]	string representaion
*/
/*void
AQLRatesSDEBase::convertFromString(const AQLString& str)
{

}*/
