/*! @file
    @brief ブラウ二アンブリッジ法クラスの定義
*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABrownianBridge.cpp
//
//  SYNOPSIS    :       LABrownianBridge.h
//  DESCRIPTION :       
//                      
//                     
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "LABrownianBridge.h"

using namespace std;

/*!
    @brief constructor
    @param[in] v MMLARandBaseクラスのオブジェクト\n
                 (v.getType()=FN_RAND_QUASIGF Or v.getType()=FN_RAND_QUASIGS)
*/
LABrownianBridge::LABrownianBridge(const LARandBase& v): 
    mpRand(NULL)
{
    if (((v.getType())!=FN_RAND_QUASIGF)&&((v.getType())!=FN_RAND_QUASIGS))
    {
        LAString msg =  "Quasi-rand-type Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mpRand=dynamic_cast<LARandBase*>(v.clone());
}

/*!
    @brief destructor
*/
LABrownianBridge::~LABrownianBridge()
{
    delete mpRand;
}
/*!
    @brief copy constructor
    @param[in] v LABrownianBridgeクラスのオブジェクト
*/
LABrownianBridge::LABrownianBridge(const LABrownianBridge& v):
LARandBase(v),mMTRand1(v.mMTRand1),mMTRand2(v.mMTRand2)
{
    mpRand = dynamic_cast<LARandBase*>( v.mpRand->clone() );
//===================   LABrownianBridge.cpp20051017(2) start =======================//
//
//  修正内容:メンバー変数を追加
//
    mMinUpper=v.mMinUpper;
    mMaxUnder=v.mMaxUnder;
    mBBVariates=v.mBBVariates;
    mLdsVariates=v.mLdsVariates;
    mMtVariates=v.mMtVariates;
//===================   LABrownianBridge.cpp20051017(2)  end  =======================//
}
/*!
    @brief deep copy of this object
    @return the copied object
*/
LACoreFunctionBase*
LABrownianBridge::clone() const
{
    try
    {
        return new LABrownianBridge(*this);
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief check whether this class derives from base class with type id
    @param[in] id ID to check
    @return True or False
*/
bool
LABrownianBridge::isTypeOf(function_t id) const
{
    return (id == FN_RAND_BB ? true : LARandBase::isTypeOf(id));
}

/*!
    @brief get type of the function
    @return function type
*/
function_t
LABrownianBridge::getType() const
{
    return FN_RAND_BB;  
}

/*!
    @brief ブラウン橋法を用いて正規乱数を算出し、逆正規変換により一様乱数を算出する関数
    @param[in] variates 算出結果を返す変数
*/
void
LABrownianBridge::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    this->getGaussians(variates);

    for (i=0;i<this->getDim()[0];i++)
    {
        variates[i]=LADist::normsdist(variates[i]);
    }
}

/*!
    @brief ブラウン橋法を用いて正規乱数を算出する関数
    @param[in] variates 算出結果を返す変数
*/
void
LABrownianBridge::getGaussians(DoubleArray& variates)
{
    //LARandBase::mDimの内容
    //      mDim[0]                     総次元
    //      mDim[1]                     LDS1次元
    //      mDim[2]                     MT1次元
    //      mDim[3]                     LDS2次元
    //      mDim[4]                     MT2次元
    //      mDim[5+i]                   計算時点配列[i](i=0,...,this->getDim()[0]-1)

    if (this->getDim()[0]!=variates.size())
    {
        LAString msg =  "Initial output's size is not equal to Total dimension";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    //  変数定義
    unsigned int i,minL;                //変数
    DoubleArray tmpVariates;

    //乱数の取得

    //LDS乱数の取得
    (this->mpRand)->getGaussians(mLdsVariates);

    //MT乱数1の取得
    if (this->getDim()[2]!=0){
        tmpVariates.resize((this->getDim()[2]));
        (this->mMTRand1).getGaussians(tmpVariates);
        
        for (i=0;i<getDim()[2];i++){
            mMtVariates[i]=tmpVariates[i];
        }
    }

    //MT乱数2の取得
    if (this->getDim()[4]!=0){
        tmpVariates.resize(this->getDim()[4]);
        (this->mMTRand2).getGaussians(tmpVariates);
//      mMtVariates=mMtVariates.shift(-(this->getDim()[2]));

        for (i=0;i<this->getDim()[4];i++){
            mMtVariates[i+this->getDim()[2]]=tmpVariates[i];
        }

//      mMtVariates=mMtVariates.shift(/*!mMtVariates.size()*/  (this->getDim()[2]));
    }


    //取得した乱数に、BrownianBridge手法を適用

    //  端点は常に0
    mBBVariates[0]=0;

    //LDS次元のうちより小さい方を認識
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

    //  LDS1に関する計算

    //  i=0,...,minL-1
    for (i=0;i<minL;i++)
    {
        /*!  偶数番号のLDSを利用 */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*i];

        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*i];
        }
    }
    //  i=minL,...,this->getDim()[1]
    for (i=minL;i<this->getDim()[1];i++)
    {
        /*!  残りのLDSを利用 */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*minL+(i-minL)];
        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*minL+(i-minL)];
        }
    }


    //  MT1に関する計算
    for (i=this->getDim()[1];i<this->getDim()[1]+this->getDim()[2];i++)
    {
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mMtVariates[i-this->getDim()[1]];
        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mMtVariates[i-this->getDim()[1]];
        }
    }


    //  LDS2に関する計算

    /*!  i=this->getDim()[1]+this->getDim()[2],...,this->getDim()[1]+this->getDim()[2]+minL-1    */
    for (i=this->getDim()[1]+this->getDim()[2];i<this->getDim()[1]+this->getDim()[2]+minL;i++)
    {
        /*!  奇数番号のLDSを利用 */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*(i-this->getDim()[1]-this->getDim()[2])+1];
        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*(i-this->getDim()[1]-this->getDim()[2])+1];
        }
    }

    /*!  i=this->getDim()[1]+this->getDim()[2]+minL,...,this->getDim()[1]+this->getDim()[2]+this->getDim()[3]    */
    for (i=this->getDim()[1]+this->getDim()[2]+minL;i<this->getDim()[1]+this->getDim()[2]+this->getDim()[3];i++)
    {
        /*!  残りのLDSを利用 */
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mLdsVariates[2*minL+(i-this->getDim()[1]-this->getDim()[2]-minL)];
        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
                        *mLdsVariates[2*minL+(i-this->getDim()[1]-this->getDim()[2]-minL)];
        }
    }

    //  MT2に関する計算
    for (i=this->getDim()[1]+this->getDim()[2]+this->getDim()[3];i<this->getDim()[1]+this->getDim()[2]+this->getDim()[3]+this->getDim()[4];i++)
    {
        if (mMinUpper[i]!=(this->getDim()[0]+1)){
            /*!  bb[計算時点]=(上限-計算時点)/(上限-下限)*bb[下限]
                            +(計算時点-下限)/(上限-下限)*bb[上限]
                            +sqrt((上限-計算時点)*(計算時点-下限)/(上限-下限))*正規乱数 */          
            mBBVariates[this->getDim()[5+i]]
                =(double)(mMinUpper[i]-this->getDim()[5+i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMaxUnder[i]]
                +(double)(this->getDim()[5+i]-mMaxUnder[i])/(mMinUpper[i]-mMaxUnder[i])*mBBVariates[mMinUpper[i]]
                +(double)LAMath::sqrt((double)(mMinUpper[i]-this->getDim()[5+i])*(this->getDim()[5+i]-mMaxUnder[i])
                    /(mMinUpper[i]-mMaxUnder[i]))*mMtVariates[i-this->getDim()[1]-this->getDim()[3]];
        }else{
            /*!  bb[計算時点]=bb[下限]+sqrt(計算時点-下限)*正規乱数      */
            mBBVariates[this->getDim()[5+i]]
                =(double)mBBVariates[mMaxUnder[i]]
                    +(double)LAMath::sqrt((double)(this->getDim()[5+i]-mMaxUnder[i]))
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

//===================   LABrownianBridge.cpp20051111(X) start =======================//
//
//  修正内容:出力変数の順番を変更
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

    /*!  ブラウン・パスの差分（正規乱数）を計算  */
    for (unsigned int j=0;j<minD;j++)
    {
		//ブラウン・パスの差分（正規乱数）を計算
		tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
		//交互に出力されるように並び替え
		variates[2*j]=tmpVariates[j];
		//ブラウン・パスの差分（正規乱数）を計算
		tmpVariates[j+getDim()[1]+getDim()[2]]=mBBVariates[j+getDim()[1]+getDim()[2]+1]-mBBVariates[j+getDim()[1]+getDim()[2]];
		//交互に出力されるように並び替え
		variates[2*j+1]=tmpVariates[j+getDim()[1]+getDim()[2]];
	}

	if (flgD==1)
	{
		for (unsigned int j=2*minD;j<getDim()[0];j++)
		{
			//ブラウン・パスの差分（正規乱数）を計算
			tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
			//交互に出力されるように並び替え
			variates[j]=tmpVariates[j];
		}
	}else{
		for (unsigned int j=minD;j<getDim()[1]+getDim()[2];j++)
		{
			//ブラウン・パスの差分（正規乱数）を計算
			tmpVariates[j]=mBBVariates[j+1]-mBBVariates[j];
			//交互に出力されるように並び替え
			variates[minD+j]=tmpVariates[j];
		}
	}
//    for (unsigned int j=0;j<this->getDim()[0];j++)
//    {
//        variates[j]=mBBVariates[j+1]-mBBVariates[j];//yohda@20051111
//    }
//===================   LABrownianBridge.cpp20051111(X)  end  =======================//

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
    @brief 次元情報を設定する関数 \n
    設定された次元情報を元に、パラメータの算出を行う為、RandBaseのsetDimをオーバーライドする。\n
    この関数による次元情報は、LDS関数までは届かない為、LDS関数のDIMを変更する為には、\n
    再度関数自身を設定しなおす必要あり。
    @param[in] dimValue 以下の通り内容を設定\n
    dimValue[0]                 総次元
    dimValue[1]                 LDS1次元
    dimValue[2]                 MT1次元
    dimValue[3]                 LDS2次元 
    dimValue[4]                 MT2次元
    dimValue[5～dimValue[0]+5]  //計算時点配列(総次元) ex.(5,2,4,1,3)　
*/
void
LABrownianBridge::setDim(const UintArray& dimValue)
{

    if (dimValue.size()>5)
    {
        if (dimValue.size()!=dimValue[0]+5)
        {
            LAString msg =  "Number-of-DimInputs Error";
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        //基底クラスのsetDim実行
        LARandBase::setDim(dimValue);

    }else{
        //ここでは、次元の設定を簡易的に実行するロジックを定義
        if (dimValue.size()!=5)
        {
            LAString msg =  "Number-of-DimInputs Error";
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        unsigned int f_Dim =  dimValue[1]+dimValue[2];
        unsigned int s_Dim =  dimValue[3]+dimValue[4];
        
        //次元情報を設定するVectorの用意
        UintArray dimInfo;
        dimInfo.resize((f_Dim+s_Dim),0);

        //節目を記録
        UintArray record_dim(1);
        record_dim[0]=f_Dim;

        //節目を記録（その2）
        UintArray record_dim_tmp(1);

        //順序のコントロール
        unsigned int j;
        j=1;

        unsigned int d;

        //まず前半部分のロジック
        while(j<=f_Dim)
        {
            //セットされている節目に従って順序を記録する
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
                //tmpのレコードを用意
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
                //record_dimにtmpの内容をコピー
                record_dim.resize(d);
                for (unsigned int i = 0;i<d;i++)
                {
                    record_dim[i]=record_dim_tmp[i];
                }
            }
        }
        //次、後半部分
        j=1;
        record_dim.resize (1);
        record_dim[0]=s_Dim;

        while(j<=s_Dim)
        {
            //セットされている節目に従って順序を記録する
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
                //tmpのレコードを用意
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
                //record_dimにtmpの内容をコピー
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
        
        //基底クラスのsetDim実行
        LARandBase::setDim(dimValuetmp);
    }

    UintArray dimtmp(1);

    //二つのうち、一方のMTに次元を設定
    if (dimValue[2]!=0)
    {
        dimtmp[0]=dimValue[2];
        mMTRand1.setDim(dimtmp);
    }

    //二つのうち、もう一方のMTに次元を設定
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
    mMinUpper.resize(dimValue[0]);  //既計算時点下限配列
    mMaxUnder.resize(dimValue[0]);  //既計算時点上限配列

    /*!  変数の定義  */
    unsigned int i,j;   //変数

    /*!  既計算時点の上限・下限配列の設定    */
    for (i=0;i<dimValue[0];i++)
    {
        mMaxUnder[i]=0;             //既計算時点下限配列（計算前）
        mMinUpper[i]=dimValue[0]+1; //既計算時点上限配列（計算前)
        for (j=0;j<i;j++)
        {
            if ((this->getDim()[5+j])<(this->getDim()[5+i]))
            {   
                //既計算時点dimValue[5+j] < 計算時点dimValue[5+i]
                if (mMaxUnder[i]<(this->getDim()[5+j]))mMaxUnder[i]=(this->getDim()[5+j]);
                    //既計算時点下限よりも大きい場合に、既計算時点下限配列を更新
            }else{                      
                //既計算時点dimValue[5+j] > 計算時点dimValue[5+i]
                if (mMinUpper[i]>(this->getDim()[5+j]))mMinUpper[i]=(this->getDim()[5+j]);
                    //既計算時点上限よりも小さい場合に、既計算時点上限配列を更新
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
//===================   LABrownianBridge.cpp20051017(8) start =======================//
//
//  修正内容1/3：エラー出力を追加
//  修正内容2/3：入力シードを変更（不必要なシード入力をなくし、準乱数と整合的に変更）
//
void
LABrownianBridge::setSeed(const UlongArray& seedValue)
{
    if (seedValue.size()!=5)
    {
        LAString msg =  "Number-of-SeedInput Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }else{
        LARandBase::setSeed(seedValue);
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
//  修正内容3/3：setInnerGeneratorを削除
//
/*!
    @brief BrownianBridge法を適用する準乱数生成器vをセットする関数
*/
//void
//LABrownianBridge::setInnerGenerator(const LARandBase& v)
//{
//  mpRand=dynamic_cast<LARandBase*>(v.clone());
//}
//===================   LABrownianBridge.cpp20051017(8)  end  =======================//
