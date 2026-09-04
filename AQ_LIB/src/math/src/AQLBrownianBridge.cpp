/*! @file
    @brief 
*/



#include "AQLBrownianBridge.h"

using namespace std;

/*!
    @brief constructor
    @param[in] v MMLARandBase\n
                 (v.getType()=FN_RAND_QUASIGF Or v.getType()=FN_RAND_QUASIGS)
*/
AQLBrownianBridge::AQLBrownianBridge(const AQLRandBase& v): 
    mpRand(NULL)
{
    if (((v.getType())!=FN_RAND_QUASIGF)&&((v.getType())!=FN_RAND_QUASIGS))
    {
        AQLString msg =  "Quasi-rand-type Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mpRand=dynamic_cast<AQLRandBase*>(v.clone());
}

/*!
    @brief destructor
*/
AQLBrownianBridge::~AQLBrownianBridge()
{
    delete mpRand;
}
/*!
    @brief copy constructor
    @param[in] v AQLBrownianBridge
*/
AQLBrownianBridge::AQLBrownianBridge(const AQLBrownianBridge& v):
AQLRandBase(v),mMTRand1(v.mMTRand1),mMTRand2(v.mMTRand2)
{
    mpRand = dynamic_cast<AQLRandBase*>( v.mpRand->clone() );
//===================   AQLBrownianBridge.cpp20051017(2) start =======================//
//
//  :
//
    mMinUpper=v.mMinUpper;
    mMaxUnder=v.mMaxUnder;
    mBBVariates=v.mBBVariates;
    mLdsVariates=v.mLdsVariates;
    mMtVariates=v.mMtVariates;
//===================   AQLBrownianBridge.cpp20051017(2)  end  =======================//
}
/*!
    @brief deep copy of this object
    @return the copied object
*/
AQLCoreFunctionBase*
AQLBrownianBridge::clone() const
{
    try
    {
        return new AQLBrownianBridge(*this);
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id ID to check
    @return True or False
*/
bool
AQLBrownianBridge::isTypeOf(function_t id) const
{
    return (id == FN_RAND_BB ? true : AQLRandBase::isTypeOf(id));
}

/*!
    @brief get type of the function
    @return function type
*/
function_t
AQLBrownianBridge::getType() const
{
    return FN_RAND_BB;  
}

/*!
    @brief 
    @param[in] variates 
*/
void
AQLBrownianBridge::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    this->getGaussians(variates);

    for (i=0;i<this->getDim()[0];i++)
    {
        variates[i]=AQLDist::normsdist(variates[i]);
    }
}

/*!
    @brief 
    @param[in] variates 
*/
void
AQLBrownianBridge::getGaussians(DoubleArray& variates)
{
    //AQLRandBase::mDim
    //      mDim[0]                     
    //      mDim[1]                     LDS1
    //      mDim[2]                     MT1
    //      mDim[3]                     LDS2
    //      mDim[4]                     MT2
    //      mDim[5+i]                   [i](i=0,...,this->getDim()[0]-1)

    if (this->getDim()[0]!=variates.size())
    {
        AQLString msg =  "Initial output's size is not equal to Total dimension";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    //  
    unsigned int i,minL;                //
    DoubleArray tmpVariates;

    //

    //LDS
    (this->mpRand)->getGaussians(mLdsVariates);

    //MT1
    if (this->getDim()[2]!=0){
        tmpVariates.resize((this->getDim()[2]));
        (this->mMTRand1).getGaussians(tmpVariates);
        
        for (i=0;i<getDim()[2];i++){
            mMtVariates[i]=tmpVariates[i];
        }
    }

    //MT2
    if (this->getDim()[4]!=0){
        tmpVariates.resize(this->getDim()[4]);
        (this->mMTRand2).getGaussians(tmpVariates);
//      mMtVariates=mMtVariates.shift(-(this->getDim()[2]));

        for (i=0;i<this->getDim()[4];i++){
            mMtVariates[i+this->getDim()[2]]=tmpVariates[i];
        }

//      mMtVariates=mMtVariates.shift(/*!mMtVariates.size()*/  (this->getDim()[2]));
    }


    //BrownianBridge

    //  0
    mBBVariates[0]=0;

    //LDS
    if (this-> getDim()[1] < this->getDim()[3])
    {
        minL = this->getDim()[1];
    }else{
        minL=this->getDim()[3];
    }

#ifdef __MUTEST_DEBUG__
    FILE *fp_nrmRN;
        fp_nrmRN = fopen("nrmRN.csv","a");

        for (unsigned int j=0;j<minL;j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mLdsVariates[2*j]);
        }
        for (unsigned int j=0;j<this->getDim()[1]-minL;j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mLdsVariates[j+minL]);
        }
        for (unsigned int j=0;j<this->getDim()[2];j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mMtVariates[j]);
        }
        for (unsigned int j=0;j<minL;j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mLdsVariates[2*j+1]);
        }
        for (unsigned int j=0;j<this->getDim()[3]-minL;j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mLdsVariates[j+minL]);
        }
        for (unsigned int j=this->getDim()[2];j<this->getDim()[2]+this->getDim()[4];j++)
        {
            fprintf(fp_nrmRN,"%22.20f\t",mMtVariates[j]);
        }
        fprintf(fp_nrmRN,"\n");
        fclose(fp_nrmRN);

#endif

    //  LDS1

    //  i=0,...,minL-1
    for (i=0;i<minL;i++)
    {
        /*!  LDS */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*i];

        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*i];
        }
    }
    //  i=minL,...,this->getDim()[1]
    for (i=minL;i<this->getDim()[1];i++)
    {
        /*!  LDS */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*minL+(i-minL)];
        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*minL+(i-minL)];
        }
    }


    //  MT1
    for (i=this->getDim()[1];i<this->getDim()[1]+this->getDim()[2];i++)
    {
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mMtVariates[i-this->getDim()[1]];
        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mMtVariates[i-this->getDim()[1]];
        }
    }


    //  LDS2

    /*!  i=this->getDim()[1]+this->getDim()[2],...,this->getDim()[1]+this->getDim()[2]+minL-1    */
    for (i=this->getDim()[1]+this->getDim()[2];i<this->getDim()[1]+this->getDim()[2]+minL;i++)
    {
        /*!  LDS */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*(i-this->getDim()[1]-this->getDim()[2])+1];
        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*(i-this->getDim()[1]-this->getDim()[2])+1];
        }
    }

    /*!  i=this->getDim()[1]+this->getDim()[2]+minL,...,this->getDim()[1]+this->getDim()[2]+this->getDim()[3]    */
    for (i=this->getDim()[1]+this->getDim()[2]+minL;i<this->getDim()[1]+this->getDim()[2]+this->getDim()[3];i++)
    {
        /*!  LDS */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*minL+(i-this->getDim()[1]-this->getDim()[2]-minL)];
        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*minL+(i-this->getDim()[1]-this->getDim()[2]-minL)];
        }
    }

    //  MT2
    for (i=this->getDim()[1]+this->getDim()[2]+this->getDim()[3];i<this->getDim()[1]+this->getDim()[2]+this->getDim()[3]+this->getDim()[4];i++)
    {
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[]=(-)/(-)*bb[]
                            +(-)/(-)*bb[]
                            +sqrt((-)*(-)/(-))* */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)AQLMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mMtVariates[i-this->getDim()[1]-this->getDim()[3]];
        }else{
            /*!  bb[]=bb[]+sqrt(-)*      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)AQLMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mMtVariates[i-this->getDim()[1]-this->getDim()[3]];
        }
    }

#ifdef __MUTEST_DEBUG__
    FILE *fp_BBpath;
        fp_BBpath = fopen("BBpath.csv","a");
        for (unsigned int j=0;j<this->getDim()[0];j++)
        {
            fprintf(fp_BBpath,"%22.20f\t",mBBVariates[j+1]);
        }
        fprintf(fp_BBpath,"\n");
        fclose(fp_BBpath);
#endif

//===================   AQLBrownianBridge.cpp20051111(X) start =======================//
//
//  :
//

	tmpVariates.resize(getDim()[0]);
	unsigned int minD,flgD;
	if (getDim()[1]+getDim()[2]<getDim()[3]+getDim()[4])
	{
		minD=getDim()[1]+getDim()[2];
		flgD=1;
	}else{
		minD=getDim()[3]+getDim()[4];
		flgD=2;
	}

    /*!  ()  */
    for (unsigned int j=0;j<minD;j++)
    {
		//()
		tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
		//
		variates[2*j]=tmpVariates[j];
		//()
		tmpVariates[j+getDim()[1]+getDim()[2]]=mBBVariates[j+getDim()[1]+getDim()[2]+1]-mBBVariates[j+getDim()[1]+getDim()[2]];
		//
		variates[2*j+1]=tmpVariates[j+getDim()[1]+getDim()[2]];
	}

	if (flgD==1)
	{
		for (unsigned int j=2*minD;j<getDim()[0];j++)
		{
			//()
			tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
			//
			variates[j]=tmpVariates[j];
		}
	}else{
		for (unsigned int j=minD;j<getDim()[1]+getDim()[2];j++)
		{
			//()
			tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
			//
			variates[minD+j]=tmpVariates[j];
		}
	}
//    for (unsigned int j=0;j<this->getDim()[0];j++)
//    {
//        variates[j]=mBBVariates[j+1]-mBBVariates[j];//yohda@20051111
//    }
//===================   AQLBrownianBridge.cpp20051111(X)  end  =======================//

#ifdef __MUTEST_DEBUG__

    FILE *fp_BBnorm;
        fp_BBnorm = fopen("BBnorm.csv","a");
        for (unsigned int j=0;j<this->getDim()[0];j++)
        {
            fprintf(fp_BBnorm,"%22.20f\t",variates[j]);
        }
        fprintf(fp_BBnorm,"\n");
        fclose(fp_BBnorm);
#endif

    return;
}

/*!
    @brief  \n
    RandBasesetDim\n
    LDSLDSDIM\n
    
    @param[in] dimValue \n
    dimValue[0]                 
    dimValue[1]                 LDS1
    dimValue[2]                 MT1
    dimValue[3]                 LDS2 
    dimValue[4]                 MT2
    dimValue[5dimValue[0]+5]  //() ex.(5,2,4,1,3) 
*/
void
AQLBrownianBridge::setDim(const UintArray& dimValue)
{

    if (dimValue.size()>5)
    {
        if (dimValue.size()!=dimValue[0]+5)
        {
            AQLString msg =  "Number-of-DimInputs Error";
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        //setDim
        AQLRandBase::setDim(dimValue);

    }else{
        //
        if (dimValue.size()!=5)
        {
            AQLString msg =  "Number-of-DimInputs Error";
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        unsigned int f_Dim =  dimValue[1]+dimValue[2];
        unsigned int s_Dim =  dimValue[3]+dimValue[4];
        
        //Vector
        UintArray dimInfo;
        dimInfo.resize((f_Dim+s_Dim),0);

        //
        UintArray record_dim(1);
        record_dim[0]=f_Dim;

        //(2)
        UintArray record_dim_tmp(1);

        //
        unsigned int j;
        j=1;

        unsigned int d;

        //
        while(j<=f_Dim)
        {
            //
            for (unsigned int i=0;i<record_dim.size();i++)
            {
                if (dimInfo[record_dim[record_dim.size()-i-1]-1]==0)
                {
                    dimInfo[record_dim[record_dim.size()-i-1]-1]=j;
                    j+=1;
                }
            }

            //set a milestone
            if (j!=(f_Dim+1))
            {
                //tmp
                record_dim_tmp.resize(record_dim.size()*2);
                d=0;
                unsigned int tmp=(unsigned int)((record_dim[0])/2);
                if (tmp!=0)
                {
                    record_dim_tmp[d]=tmp;
                    d+=1;
                    record_dim_tmp[d]=record_dim[0];
                    d+=1;
                }

                for (unsigned int i=1;i<record_dim.size();i++)
                {
                    tmp=(unsigned int)((record_dim[i-1]+record_dim[i])/2);
                    if ( tmp != record_dim[i-1] )
                    {
                        record_dim_tmp[d]=tmp;
                        d+=1;
                        record_dim_tmp[d]=record_dim[i];
                        d+=1;
                    }
                }
                //record_dimtmp
                record_dim.resize(d);
                for (unsigned int i = 0;i<d;i++)
                {
                    record_dim[i]=record_dim_tmp[i];
                }
            }
        }
        //
        j=1;
        record_dim.resize (1);
        record_dim[0]=s_Dim;

        while(j<=s_Dim)
        {
            //
            for (unsigned int i=0;i<record_dim.size();i++)
            {
                if (dimInfo[record_dim[record_dim.size()-i-1]-1+f_Dim]==0)
                {
                    dimInfo[record_dim[record_dim.size()-i-1]-1+f_Dim]=j+f_Dim;
                    j+=1;
                }
            }

            //set a milestone
            if (j!=(s_Dim+1)){
                //tmp
                record_dim_tmp.resize(record_dim.size()*2);
                d=0;
                unsigned int tmp=(unsigned int)((record_dim[0])/2);
                if (tmp!=0)
                {
                    record_dim_tmp[d]=tmp;
                    d+=1;
                    record_dim_tmp[d]=record_dim[0];
                    d+=1;
                }

                for (unsigned int i=1;i<record_dim.size();i++)
                {
                    tmp=(unsigned int)((record_dim[i-1]+record_dim[i])/2);
                    if ( tmp != record_dim[i-1] )
                    {
                        record_dim_tmp[d]=tmp;
                        d+=1;
                        record_dim_tmp[d]=record_dim[i];
                        d+=1;
                    }
                }
                //record_dimtmp
                record_dim.resize(d);
                for (unsigned int i = 0;i<d;i++)
                {
                    record_dim[i]=record_dim_tmp[i];
                }
            }
        }
        UintArray dimValuetmp(5+dimInfo.size());
        for (unsigned int i=0;i<5;i++)
        {
            dimValuetmp[i]=dimValue[i];
        }
        for (unsigned int i=0;i<dimInfo.size();i++)
        {
            dimValuetmp[dimInfo[i]-1+5]=i+1;
        }
        
        //setDim
        AQLRandBase::setDim(dimValuetmp);
    }

    UintArray dimtmp(1);

    //MT
    if (dimValue[2]!=0)
    {
        dimtmp[0]=dimValue[2];
        mMTRand1.setDim(dimtmp);
    }

    //MT
    if (dimValue[4]!=0)
    {
        dimtmp[0]=dimValue[4];
        mMTRand2.setDim(dimtmp);
    }

    dimtmp.resize(2);
    dimtmp[0]=dimValue[1]+dimValue[3];
    dimtmp[1]=dimtmp[0];
    mpRand->setDim(dimtmp);

	mBBVariates.resize(dimValue[0]+1);
    mMinUpper.resize(dimValue[0]);  //
    mMaxUnder.resize(dimValue[0]);  //

    /*!    */
    unsigned int i,j;   //

    /*!      */
    for (i=0;i<dimValue[0];i++)
    {
        mMaxUnder[i]=0;             //()
        mMinUpper[i]=dimValue[0]+1; //()
        for (j=0;j<i;j++)
        {
            if ((this->getDim()[5+j])<(this->getDim()[5+i]))
            {   
                //dimValue[5+j] < dimValue[5+i]
                if (mMaxUnder[i]<(this->getDim()[5+j]))mMaxUnder[i]=(this->getDim()[5+j]);
                    //
            }else{                      
                //dimValue[5+j] > dimValue[5+i]
                if (mMinUpper[i]>(this->getDim()[5+j]))mMinUpper[i]=(this->getDim()[5+j]);
                    //
            }
        }
    }

#ifdef __MUTEST_DEBUG__
    FILE *fp_culcNUM;
        fp_culcNUM = fopen("culcNUM.csv","w");

        for (unsigned int i=0;i<dimValue[0];i++)
        {
            fprintf(fp_culcNUM,"%d\t%d\t%d\n",this->getDim()[5+i],mMinUpper[i],mMaxUnder[i]);
        }
        fclose(fp_culcNUM);
#endif


    //  update size of array
    mLdsVariates.resize(dimValue[1]+dimValue[3]);   //update size of LDS array
    mMtVariates.resize(dimValue[2]+dimValue[4]);    //update size of MT array

    return;
}

/*!
    @brief function to set Seed
    @param[in] seedValue value of Seed \n
    seed[0]     Seed of quasi-random numbers. Set the number of start of Seed
    seed[1]     Seed of quasi-random numbers. Set the number of necessary Seequence 
    seed[2]     Seed of quasi-random numbers. Set the number of a quasi-random number generator matrix
    seed[3]     Seed of the first MT. Set the dummy even in not necessary case
    seed[4]     Seed of the second MT. Set the dummy even in not necessary case
*/
//===================   AQLBrownianBridge.cpp20051017(8) start =======================//
//
//  1/3:
//  2/3:()
//
void
AQLBrownianBridge::setSeed(const UlongArray& seedValue)
{
    if (seedValue.size()!=5)
    {
        AQLString msg =  "Number-of-SeedInput Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }else{
        AQLRandBase::setSeed(seedValue);
        UlongArray seedtmp(1);
        
        seedtmp[0]=seedValue[3];
        mMTRand1.setSeed(seedtmp);

        seedtmp[0]=seedValue[4];
        mMTRand2.setSeed(seedtmp);

        seedtmp.resize(seedValue.size()-1);
        for ( unsigned int i = 0;i<(seedValue.size()-1);i++)
        {
            seedtmp[i]=seedValue[i];
        }
        mpRand->setSeed(seedtmp);
    }
}
//
//  3/3:setInnerGenerator
//
/*!
    @brief BrownianBridgev
*/
//void
//AQLBrownianBridge::setInnerGenerator(const AQLRandBase& v)
//{
//  mpRand=dynamic_cast<AQLRandBase*>(v.clone());
//}
//===================   AQLBrownianBridge.cpp20051017(8)  end  =======================//
