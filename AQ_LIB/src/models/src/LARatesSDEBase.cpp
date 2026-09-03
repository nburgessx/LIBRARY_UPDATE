/*! @file
    @brief Declaration of base class of sde class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesSDEBase.cpp
//
//  SYNOPSIS    :       LARatesSDEBase
//  DESCRIPTION :       Declaration of base class of sde class
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


#include "LARatesSDEBase.h"
#include "LARatesSDEIntegralBase.h"
#include "LAModelDynamicsCurve.h"
#include "LAAlgorithm.h"

using namespace std;
//================ LARatesSDEBase ===================================
/*!
	@brief default constructor
	@param[in] type sde type (dX or DIVIDEdXbyX)
*/
LARatesSDEBase::LARatesSDEBase(SDE_TYPE type)
: mType(type), mID(0), mpNumeraire(0),
   mpBM(0), mpInter(0), mpIntegral(0), mpInitial(0), mpTemplate(0), mPos(0)
/*, mpPathElement(0)*//*, mIsCalcAll(false)*/
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesSDEBase::LARatesSDEBase(const LARatesSDEBase& v) 
: LACoreFunctionBase(v), mType(v.mType), mID(v.mID), mpNumeraire(0), mpBM(0),
   mpInter(0), mTimeGrid(v.mTimeGrid), mpIntegral(0), mpInitial(0), mpTemplate(0), mPos(v.mPos)
/*, mpPathElement(0)*//*, mIsCalcAll(v.mIsCalcAll)*/
{
	if (v.mpNumeraire != 0)
		mpNumeraire = dynamic_cast<LARatesNumeraireBase*>(v.mpNumeraire->clone());
	if (v.mpIntegral != 0)
		mpIntegral = dynamic_cast<LARatesSDEIntegralBase*>(v.mpIntegral->clone());
	if (v.mpInitial != 0)
		mpInitial = dynamic_cast<LARatesPathElementBase*>(v.mpInitial->clone());
	if (v.mpTemplate != 0)
		mpTemplate = dynamic_cast<LARatesPathElementBase*>(v.mpTemplate->clone());
	if (v.mpInter != 0)
		mpInter = dynamic_cast<LARatesPEInterpolationBase*>(v.mpInter->clone());
//	if (v.mpPathElement != 0)
//		mpPathElement = dynamic_cast<LARatesPathElementBase*>(v.mpPathElement->clone());


	mPath.resize(v.mPath.size());
	for (unsigned int i = 0; i < mPath.size(); i++)
		mPath[i] = v.mPath[i]->clone();
	
	mDrift.resize(v.mDrift.size());
	for (unsigned int i = 0; i < mDrift.size(); i++)
		mDrift[i] = dynamic_cast<LAFunctionBase*>(v.mDrift[i]->clone());

	mVolatility.resize(v.mVolatility.size());
	for (unsigned int i = 0; i < mVolatility.size(); i++)
	{
		mVolatility[i].resize(v.mVolatility[i].size());
		for (unsigned int j = 0; j < mVolatility[i].size(); j++)
			mVolatility[i][j] = dynamic_cast<LAFunctionBase*>(v.mVolatility[i][j]->clone());
	}

    try 
	{
		if (v.mpBM != 0) mpBM = v.mpBM->clone();
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }

}

/*!
	@brief destructor
*/
LARatesSDEBase::~LARatesSDEBase() 
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
LARatesSDEBase::isTypeOf(function_t id) const
{
	return (id==FN_SDEBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesSDEBase::getType() const
{
	return FN_SDEBASE;
}

/*!
    @brief get path
    @return path
*/
const vector<LARatesPathElementBase*>&
LARatesSDEBase::getPath(void)
{
/*	if (mpBM->getCurrentID() != mID || !mIsCalcAll)
	{
		if ((mPos == 0 || mPos == mpBM->getTimeGrid().size() - 1)//
			&& mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const LARatesPathElementCurve*>(mPath[0]));
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
/*LARatesPathElementBase*
LARatesSDEBase::getPathElement(unsigned int pos)
{
	const DoubleArray& timegrid = mpBM->getTimeGrid();
	if (pos > timegrid.size() - 1)
	{
		throw LACoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	if (mpBM->getCurrentID() == mID && pos == mPos)
		return mpPathElement;
	
	if (mpBM->getCurrentID() != mID)
	{
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const LARatesPathElementCurve*>(mPath[0]));
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
LARatesSDEBase::setIntegralFunction(LARatesSDEIntegralBase* pintegral)
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
LARatesSDEBase::setNumeraire(LARatesNumeraireBase* pnumeraire)
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
LARatesSDEBase::setDrift(std::vector<LAFunctionBase*>& drift) 
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
LARatesSDEBase::setVolatility(std::vector<std::vector<LAFunctionBase*> >& volatility)
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
LARatesSDEBase::setOutputTemplate(const LARatesPathElementBase* pelement)
{
	delete mpTemplate;
	mpTemplate = pelement;
}
/*!
	@brief set initial value
	@param[in] initial initial value
*/	
void
LARatesSDEBase::setInitialValue (const LARatesPathElementBase& initial)
{
	delete mpInitial;
	mpInitial = initial.clone();
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
LARatesSDEBase::check(void) const
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
LARatesSDEBase::setUp()
{
	if (!check())
	{
		//error
		throw LACoreInvalidData("SDE check status is false", __FILE__, __LINE__);
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
				dynamic_cast<LARatesPathElementCurve*>(mPath[i])->set_t(mTimeGrid[i]);

			mPath[0]->set(*mpInitial);
//			mpPathElement = mPath[0]->clone(); 
		}
		else 
		{
			//delete mPath[i];
			mPath[i] = mPath[0]->clone();
			if (mpTemplate->isTypeOf(PE_CURVE))
				dynamic_cast<LARatesPathElementCurve*>(mPath[i])->set_t(mTimeGrid[i]);
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
		throw LACoreInvalidData("Multi volatility is not support when SDE IntegralType is LOG", __FILE__, __LINE__);
	}
		
}
/*!
    @brief return string representaion
	@return  string representaion
*/
/*LAString
LARatesSDEBase::convertToString(void) const
{
	return "";
}*/

/*!
    @brief transform from string representaion
	@param[in]	string representaion
*/
/*void
LARatesSDEBase::convertFromString(const LAString& str)
{

}*/
