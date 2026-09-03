#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LACoreTemplateType.h"


class LARandBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of brownian motion class

*/
class LARatesBM
{
public:
//  LIFECYCLE
	// constructor
	LARatesBM(const DoubleArray& timegrid, const LARandBase& rand, unsigned int factor_num = 1);
	// constructor
	LARatesBM(const DoubleArray& timegrid, const LARandBase& rand, const std::vector<DoubleMatrix>& loading);
	// constructor
	LARatesBM(std::vector<std::pair<LARatesBM*, unsigned int> >& bm, 
			const std::vector<DoubleMatrix>& loading, bool deleteflag = true);
	// copy constructor
	LARatesBM(const LARatesBM& v);
	// clone method
    virtual LARatesBM*     clone() const;
	// Destructor
	virtual ~LARatesBM();
	//  QUERY
	// check generator type
	bool  isGeneratorType(void) const;
	// get time grid
	const DoubleArray& getTimeGrid (void) const;	
	// get current id
	unsigned int getCurrentID() const;
	// get brownian motion value
	const DoubleMatrix&	getBM();
	// calulate brownian motion value
	virtual void		calcBM(bool isIdling = false);
	//  set antithetic or not
	void				setAntithetic(bool flag = true);
	// get seed
    const UlongArray&	getSeed() const;				
	// get dimension
    const UintArray& 	getDim() const;
	// set seed
    void                setSeed(const UlongArray& seedValue);				
	// set dim
//    void				setDim(const UintArray& dimValue);
	// calculate correlation between this BM and other BM
	DoubleArray			calcCorrelation(const LARatesBM& bm, unsigned int pos_other, unsigned int pos_this) const;
	//	get generator BM
//	LARatesBM*				getBaseBM();
	// get all reference BM
	std::set<LARatesBM*>	getReferenceBM();

private:
	
protected:
	int mType;// type of BM
	unsigned int mID;// id
	unsigned long mRefCount;// reference counter
	LARandBase* mpRand;// rand generator
	std::vector<std::pair<LARatesBM*, unsigned int> > mBM;// BM classes 
	std::vector<DoubleMatrix> mLoading;// factor loading
	DoubleMatrix	mData;// BM values
	DoubleArray		mDataInstanceDt;// dataInstance dt
	DoubleArray		mTimeGrid;// time grid
	DoubleArray		mBuff;// buffer
	bool			mIsAntithetic;		// antithetic flag
	bool			mIsOdd;				// odd or even for antithetic flag 
	DoubleMatrix	mAntiData;			// antithetic BM values
	bool			mDeleteFlag;		// delete flag of mBM
};

