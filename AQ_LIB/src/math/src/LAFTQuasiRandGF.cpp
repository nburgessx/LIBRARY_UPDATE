/*! @file
    @brief Generalized Faure数列を作るクラスの定義
*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAFTQuasiRandGF.cpp
//
//  SYNOPSIS    :       LAFTQuasiRandGF
//  DESCRIPTION :
//
//
//
//  SEE ALSO    :       
//  TYPE        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "LAFTQuasiRandGF.h"

using namespace std;

//データ保持クラスの関数を定義---------------------------------------------------
/*!
    @brief データ保持クラスのdefault constructor。
    (1×1)の領域だけを確保する。
*/
LAFTQuasiRandGF::LAMatrixData::LAMatrixData(void)
:mpData(NULL),mRow(1),mCol(1)
{
     try {
         mpData = new int*[1];
         mpData[0]=new int[1*1];
     }
     catch(...)
     {
         LACoreSystemError e(__FILE__, __LINE__);
         if (mpData != NULL) 
         {   
             delete[] mpData;
             mpData = NULL;
         }
         throw e;
     }
}
/*! 
    @brief データ保持クラスのdestructor
*/
LAFTQuasiRandGF::LAMatrixData::~LAMatrixData()
{
    if (mpData != NULL) {
        delete[] mpData[0];
        delete[] mpData;
    }
}
/*! 
    @brief コピー・constructor
    @param[in] m マトリックスクラスm
*/
LAFTQuasiRandGF::LAMatrixData::LAMatrixData(const LAMatrixData& m) 
:mpData(NULL),mRow(0),mCol(0)
{
    if (m.row() == 0 || m.col() == 0)
    {
        if (mpData != NULL) 
        {
            delete[] mpData[0];
            delete[] mpData;
        }
        mpData = NULL;
        mRow = mCol = 0;
        return;
    }
    
    resize(m.row(),m.col());

    for (unsigned int i=0;i<m.row();i++)
    {
        for (unsigned int j=0;j<m.col();j++)
        {
            setValue(i,j,m.getValue(i,j));
        }
    }
}

/*! 
    @brief データ保持クラスのサイズを変更する為の関数（変更後の行数・列数指定、private）\n
    変更前のデータが失われ全て0になる。
    @param[in] row 変更後の行数row
    @param[in] col 変更後の列数coL
*/
void
LAFTQuasiRandGF::LAMatrixData::resize(unsigned int row, unsigned int col)
{
    // 0のケースも考えよう。
    if (row == 0 || col == 0)
    {
        if (mpData != NULL) {
            delete[] mpData[0];
            delete[] mpData;
        }
        mpData = NULL;
        mRow = mCol = 0;
        return;
    }
    // データサイズが大きくなる場合
    unsigned int size = col * row;
    int** newData;
// unused.    int*  oldData = (mpData == NULL) ? NULL : mpData[0];
    if (size > mCol*mRow)
    { // 新しいデータ領域が必要
        // Rowサイズでの比較
        if (row > mRow)
            newData = new int*[row];
        else
            newData = mpData;
        // データ領域の割り当て
        newData[0] = new int[size];
    } 
    else
    { // 既存の領域でOK。
        // Rowサイズでの比較
        if (row > mRow)
            newData = new int*[row];
        else
            newData = mpData;
        // データ領域の割り当て
        newData[0] = mpData[0];
    }

    // Indexの再構成
    unsigned int i;
    for (i=1;i<row;++i) 
    {
        newData[i]=newData[i - 1]+col;
    }
    // Resizeすると、データの中身は0になる。
    unsigned int j;
    for (i=0;i<row;++i) 
    {
// unused.        unsigned int colPos = i * mCol;
        for (j = 0; j < col; ++j)
        {
            newData[i][j]= 0;
        }
    }
    // 古いデータ領域の解放
    if (mpData != NULL)
    {
        if (newData[0] != mpData[0]) delete mpData[0];
        if (newData != mpData) delete mpData;
    }
    // 設定しなおし
    mpData = newData;
    mRow = row;
    mCol = col;
}

/*! 
    @brief データを取得する為の関数
    @param[in] i 変更後の行数row（-1した場所を指定）
    @param[in] j 変更後の列数coL（-1した場所を指定）
    @return 取得値
*/

int 
LAFTQuasiRandGF::LAMatrixData::getValue(unsigned int i, unsigned int j) const
{
    if (i>=mRow || j>=mCol) 
        throw LACoreNumericalError("Boundary Error", __FILE__, __LINE__);
    return mpData[i][j]; 
}

/*! 
    @brief 代入（void）
    @param[in] other 右オペランド
*/
void
LAFTQuasiRandGF::LAMatrixData::operator =(const LAMatrixData& other)
{

    this->resize(other.row(),other.col());
    for (unsigned int i=0;i<other.col();i++){
        for (unsigned int j=0;i<other.row();i++){
            this->setValue(i,j,other.getValue(i,j));
        }
    }
    return;
}

/*! 
    @brief 行列の積（baseで余りをとる）
    @param[in] other 右オペランド
    @param[in] base 余りを取る基底
*/
LAFTQuasiRandGF::LAMatrixData 
LAFTQuasiRandGF::LAMatrixData::matMult(const LAMatrixData& other,int base) const
{
    if (col() != other.row()) 
    {
        throw 
        LACoreNumericalError("Can not multiply", __FILE__, __LINE__);
    }
    LAMatrixData ret;
    ret.resize(row(), other.col());

    unsigned int i, j, k;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < other.col(); j++) {
            (ret.mpData[i][j]) = 0;
            for (k = 0; k < col(); k++) {
                (ret.mpData[i][j]) += 
                    (((mpData[i][k]) * (other.mpData[k][j]))) % base;
            }
            ret.mpData[i][j]=(ret.mpData[i][j]) % base;
        }
    }
    return ret;
}

void
LAFTQuasiRandGF::LAMatrixData::setValue(unsigned int i, unsigned int j, int value)
{
    if (i>mRow || j>mCol) 
        throw LACoreInvalidData("Boundary Error", __FILE__, __LINE__);
    (mpData)[i][j] = value;
} 
void
LAFTQuasiRandGF::LAMatrixData::print(void) 
{
    FILE *fp;
    fp = fopen("Out.csv","a");
    for (unsigned int i=0;i<row();i++)
    {
        for (unsigned int j = 0; j<col();j++){
            fprintf(fp,"%d,",this->getValue(i,j));
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
} 

//乱数発生クラスの関数を定義---------------------------------------------------
/*!
    @brief FTQuasiRandGFのdefault constructor
    パラメータのセット情報と、Rand()関数のデフォルトシード値を設定する

*/

LAFTQuasiRandGF::LAFTQuasiRandGF() 
: LARandBase(),mSeedSet(false),mDimSet(false),mBase(0),mMax(0)
{
    this->ftholdrand = 1L;//Rand()'s Default Seed
}

LAFTQuasiRandGF::~LAFTQuasiRandGF()
{
}

/*!
    @brief deep copy of this object

*/
LACoreFunctionBase*
LAFTQuasiRandGF::clone() const
{
    try 
    {
        return new LAFTQuasiRandGF(*this);
    }
    catch (bad_alloc & e)
    {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function type to be checked
    @return True or false
*/
bool
LAFTQuasiRandGF::isTypeOf(function_t id) const
{
    return (id == FN_RAND_QUASIGF ? true : LARandBase::isTypeOf(id));
}

/*!
    @brief get type of the function
    @return function type
*/
function_t
LAFTQuasiRandGF::getType() const
{
    return FN_RAND_QUASIGF;
}
/*!
    @brief 一様乱数を発生する関数
    @param[in] variates 発生した一様乱数を返すValarray型のVector
*/
void
LAFTQuasiRandGF::getUniforms(DoubleArray& variates)
{
    if (!(mSeedSet && mDimSet)){
        LAString msg =  "SeedSet or DimSet Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        return;
    }

    if (getDim()[1] ==0){
        LAString msg =  "Quasi-Rand-Dim=0 Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        return;
    }

    //計算用
    unsigned int i,j,k;
    double xx(0);

    //Sizeを取得
    unsigned int size=(this->mGeneMat.col());
    
    //Digit　Expansion
    unsigned long statetmp = (this->mState);
    for (i=0;i<size;i++){
        mDigitVector.setValue(i,0,statetmp % mBase);
        statetmp=(unsigned long)(statetmp/mBase);
    }
//  mDigitVector.print();//test 
//  mGeneMat.print();//test
    //Dim=1の分
    //乱数行列を作成
    this->ftsrand(this->mMSeed);
    for (i=0;i<size;i++)
    {
        for (j=0;j<=i;j++)
        {
            int tmpCheck = (this->ftrand())%(mBase);
            while(tmpCheck==0){
                tmpCheck=(this->ftrand())%(mBase);
            }
            mRandMat.setValue(i,j, tmpCheck);
        }
    }
    mRandMat.setValue(0,0,mqvec[0]);
//  mRandMat.print(); //test
    mDigitVectortmp=mRandMat.matMult(mDigitVector,mBase);
//  mDigitVectortmp.print();//test
    for (i=0;i<size;i++){
        xx +=mDigitVectortmp.getValue(i,0)/LAMath::pow(mBase,(i+1));
    }
    variates[0]=xx;
    xx=0;

//Dim=2以降
    for (k=1; k<(this->getDim()[1]); k++)
    {
        for (i=0;i<size;i++)
        {
            for (j=0;j<=i;j++)
            {
                int tmpCheck = (this->ftrand())%(mBase);
                while(tmpCheck==0){
                    tmpCheck=(this->ftrand())%(mBase);
                }
                mRandMat.setValue(i,j, tmpCheck);
            }
        }
        mRandMat.setValue(0,0,mqvec[k]);
//      mRandMat.print();//test
        mDigitVector=mGeneMat.matMult(mDigitVector,mBase);
//      mGeneMat.print();//test
//      mDigitVector.print();//test
        mDigitVectortmp=mRandMat.matMult(mDigitVector,mBase);
//      mRandMat.print();//test
//      mDigitVectortmp.print(); //test
        for (i=0;i<size;i++){
            xx +=mDigitVectortmp.getValue(i,0)/LAMath::pow(mBase,(i+1));
        }
        
        variates[k]=xx;
    
        xx=0;
    }
    
    if ((getDim()[0]-getDim()[1])!=0){
        DoubleArray variatesTmp(getDim()[0]-getDim()[1]);
        this->mInner.getUniforms(variatesTmp);

        for (i=0;i<(getDim()[0]-getDim()[1]);i++){
            variates[ getDim()[1] + i ] = variatesTmp[i];
        }
    }

    //カウンタを一つ増やす
    this->mState = this->mState + 1;
    
    // counter increments it by one, if exceeds the Max, back to the first
    if (this->mState==this->mMax){
        this->mState=getSeed()[0];
    }

    return;
}

/*!
    @brief Seedを設定する関数
    @param[in] seed Seedに関する情報を設定する為のValarray型のVector。\n
    ただし、    seed[0]=開始数,\n
            seed[1]=必要Sequence数,\n
            seed[2]=Seed For Generator Matrix,\n
            seed[3]=Seed For Mersennne Twister
*/
void
LAFTQuasiRandGF::setSeed(const UlongArray& seed)
{
    if (seed.size()!=4){
        LAString msg =  "Number-of-Seed Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        //error. wrong Seed setting
    }
    LARandBase::setSeed(seed);
    mMax=seed[0]+seed[1];
    mState=seed[0];
    mSeedSet=true;

    //保有するMersennne Twisterにseedをセット
    UlongArray seedtmp(1);

    seedtmp[0]=seed[3];
    (this->mInner).setSeed(seedtmp);

    if (mDimSet && mSeedSet) paraCalc();
    return;
}
/*!
    @brief  次元を設定する関数\n
            （次元の設定にあわせて一部計算を行う為、基底クラスをオーバーライド）

    @param[in] dimValue 次元に関する情報を設定する為のValarray型のVector。\n
    ただし、    dimValue[0]=全次元,\n
            dimValue[1]=準乱数次元
*/
void
LAFTQuasiRandGF::setDim(const UintArray& dimValue)
{
    if (dimValue.size()!=2){
        LAString msg =  "Number-of-Dim Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        //error. wrong Dim setting
    }
    LARandBase::setDim(dimValue);
    mDimSet=true;
    
    //保有するMersennne TwisterにDimをセット
    UintArray dimtmp(1);
    if ((dimValue[0]-dimValue[1])<0){
        LAString msg =  "TotalDim<QuasiRandDim Error";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        //エラー　全次元と準乱数の次元の関係が変
    }
    dimtmp[0]=dimValue[0]-dimValue[1];

	if (dimtmp[0] != 0) (this->mInner).setDim(dimtmp);

    if (mDimSet && mSeedSet) paraCalc();
    return;
}

/*!
    @brief  設定された次元情報・Seed情報から必要パラメータを算出する関数
*/
void
LAFTQuasiRandGF::paraCalc()
{

//mBaseを求める
    if (this->getDim()[1]!=0)
    {
        if (this->getDim()[1]==600)
        {
            mBase = 607;
        }
        else
        {
            if (this->getDim()[1]==500)
            {
                mBase = 503;
            }
            else{
                    int i;
                    int flag;
                    flag=1;

                    if (this->getDim()[1]==1)
                    {
                        mBase=2;
                        flag=0;
                    }
                    if (this->getDim()[1]==2)
                    {
                        mBase=3;
                        flag=0;
                    }
                    if (this->getDim()[1]==3)
                    {
                        mBase=5;
                        flag=0;
                    }
                    if (flag==1)
                    {
                        mBase=this->getDim()[1]-1;
                        if (mBase % 2 == 0)mBase+=1;
                        while(flag)
                        {
                            mBase+=2;
                            flag=0;
                            for (i=3;i <= (int)(LAMath::sqrt(mBase))+1;i++)
                            {
                                if (mBase%i == 0){
                                    flag=1; 
                                    break;
                                }
                            }
                        }
                    }
                    
            }
        }
    }

    //Generator行列のサイズ算出（最大で(n番目のパスの時)）
    unsigned long ntmp=mMax;
    unsigned int size = 1;
    if (getSeed()[1]!=0){
        while (ntmp>=mBase)
        {
            ntmp /= mBase;
            size += 1;
        }
    }

    //上記で取得したSizeを使って、使用するデータ領域のサイズを調整
    mGeneMat.resize(size,size);
    mRandMat.resize(size,size);
    mDigitVector.resize(size,1);
    mDigitVectortmp.resize(size,1);
    mqvec.resize(getDim()[1],1);

    //Generalization Matrixの素1
    if ((this->getDim()[0])!=0){
        getmqvec();
    }

    //Faure行列を作成
    for (unsigned i=0;i<size;i++)
    {
        for (unsigned j=i;j<size;j++)
        {
            mGeneMat.setValue(i,j, (((int)(LAMath::bico(j,i)+0.000000001)) % mBase));
        }
    }

    //MersenneTwisterに、シード、次元をセット
    UlongArray tmp(1);
    tmp[0]=mSeed[3];
    mInner.setSeed(tmp);
    UintArray tmp1(1);
    if ((getDim()[0]-getDim()[1]) != 0)
    {
        tmp1[0]=getDim()[0]-getDim()[1];
        mInner.setDim(tmp1);
    }

    return;
}

/*!
    @brief set seed of rand function
    @param[in] seed 1-dim long
*/
void
LAFTQuasiRandGF::ftsrand (unsigned long seed)
{
    this->ftholdrand  = (long)seed;
}
/*!
    @brief rand function
    @return random numbers
*/
int
LAFTQuasiRandGF::ftrand (void)
{
    return(((ftholdrand = ftholdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}
/*!
    @brief  parameter calc2
*/
void 
LAFTQuasiRandGF::getmqvec(void)
{
    unsigned int i;
    unsigned int m;

    ftsrand(mSeed[2]);

    if (getDim()[1]==1){
        mqvec[0]=1;
    }

    IntArray vectmp(0,this->mBase);

    for (i=0;i<getDim()[1];i++)
    {
        m=ftrand()%(mBase-2);
        while(vectmp[m])
        {
            m=m+1;
            if (m==(mBase-1))
            {
                m=0;
            }
        }

        mqvec[i]=m+1;

        vectmp[m]=1;
    }
    return;
}
