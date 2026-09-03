/*! @file
    @brief Generalized Faure
*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLFTQuasiRandGF.cpp
//
//  SYNOPSIS    :       AQLFTQuasiRandGF
//  DESCRIPTION :
//
//
//
//  SEE ALSO    :       
//  TYPE        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "AQLFTQuasiRandGF.h"

using namespace std;

//---------------------------------------------------
/*!
    @brief default constructor
    (11)
*/
AQLFTQuasiRandGF::AQLMatrixData::AQLMatrixData(void)
:mpData(NULL),mRow(1),mCol(1)
{
     try {
         mpData = new int*[1];
         mpData[0]=new int[1*1];
     }
     catch(...)
     {
         AQLCoreSystemError e(__FILE__, __LINE__);
         if (mpData != NULL) 
         {   
             delete[] mpData;
             mpData = NULL;
         }
         throw e;
     }
}
/*! 
    @brief destructor
*/
AQLFTQuasiRandGF::AQLMatrixData::~AQLMatrixData()
{
    if (mpData != NULL) {
        delete[] mpData[0];
        delete[] mpData;
    }
}
/*! 
    @brief constructor
    @param[in] m m
*/
AQLFTQuasiRandGF::AQLMatrixData::AQLMatrixData(const AQLMatrixData& m) 
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
    @brief (private)\n
    0
    @param[in] row row
    @param[in] col coL
*/
void
AQLFTQuasiRandGF::AQLMatrixData::resize(unsigned int row, unsigned int col)
{
    // 0
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
    // 
    unsigned int size = col * row;
    int** newData;
// unused.    int*  oldData = (mpData == NULL) ? NULL : mpData[0];
    if (size > mCol*mRow)
    { // 
        // Row
        if (row > mRow)
            newData = new int*[row];
        else
            newData = mpData;
        // 
        newData[0] = new int[size];
    } 
    else
    { // OK
        // Row
        if (row > mRow)
            newData = new int*[row];
        else
            newData = mpData;
        // 
        newData[0] = mpData[0];
    }

    // Index
    unsigned int i;
    for (i=1;i<row;++i) 
    {
        newData[i]=newData[i - 1]+col;
    }
    // Resize0
    unsigned int j;
    for (i=0;i<row;++i) 
    {
// unused.        unsigned int colPos = i * mCol;
        for (j = 0; j < col; ++j)
        {
            newData[i][j]= 0;
        }
    }
    // 
    if (mpData != NULL)
    {
        if (newData[0] != mpData[0]) delete mpData[0];
        if (newData != mpData) delete mpData;
    }
    // 
    mpData = newData;
    mRow = row;
    mCol = col;
}

/*! 
    @brief 
    @param[in] i row(-1)
    @param[in] j coL(-1)
    @return 
*/

int 
AQLFTQuasiRandGF::AQLMatrixData::getValue(unsigned int i, unsigned int j) const
{
    if (i>=mRow || j>=mCol) 
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    return mpData[i][j]; 
}

/*! 
    @brief (void)
    @param[in] other 
*/
void
AQLFTQuasiRandGF::AQLMatrixData::operator =(const AQLMatrixData& other)
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
    @brief (base)
    @param[in] other 
    @param[in] base 
*/
AQLFTQuasiRandGF::AQLMatrixData 
AQLFTQuasiRandGF::AQLMatrixData::matMult(const AQLMatrixData& other,int base) const
{
    if (col() != other.row()) 
    {
        throw 
        AQLCoreNumericalError("Can not multiply", __FILE__, __LINE__);
    }
    AQLMatrixData ret;
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
AQLFTQuasiRandGF::AQLMatrixData::setValue(unsigned int i, unsigned int j, int value)
{
    if (i>mRow || j>mCol) 
        throw AQLCoreInvalidData("Boundary Error", __FILE__, __LINE__);
    (mpData)[i][j] = value;
} 
void
AQLFTQuasiRandGF::AQLMatrixData::print(void) 
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

//---------------------------------------------------
/*!
    @brief FTQuasiRandGFdefault constructor
    Rand()

*/

AQLFTQuasiRandGF::AQLFTQuasiRandGF() 
: AQLRandBase(),mSeedSet(false),mDimSet(false),mBase(0),mMax(0)
{
    this->ftholdrand = 1L;//Rand()'s Default Seed
}

AQLFTQuasiRandGF::~AQLFTQuasiRandGF()
{
}

/*!
    @brief deep copy of this object

*/
AQLCoreFunctionBase*
AQLFTQuasiRandGF::clone() const
{
    try 
    {
        return new AQLFTQuasiRandGF(*this);
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function type to be checked
    @return True or false
*/
bool
AQLFTQuasiRandGF::isTypeOf(function_t id) const
{
    return (id == FN_RAND_QUASIGF ? true : AQLRandBase::isTypeOf(id));
}

/*!
    @brief get type of the function
    @return function type
*/
function_t
AQLFTQuasiRandGF::getType() const
{
    return FN_RAND_QUASIGF;
}
/*!
    @brief 
    @param[in] variates ValarrayVector
*/
void
AQLFTQuasiRandGF::getUniforms(DoubleArray& variates)
{
    if (!(mSeedSet && mDimSet)){
        AQLString msg =  "SeedSet or DimSet Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        return;
    }

    if (getDim()[1] ==0){
        AQLString msg =  "Quasi-Rand-Dim=0 Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        return;
    }

    //
    unsigned int i,j,k;
    double xx(0);

    //Size
    unsigned int size=(this->mGeneMat.col());
    
    //Digit Expansion
    unsigned long statetmp = (this->mState);
    for (i=0;i<size;i++){
        mDigitVector.setValue(i,0,statetmp % mBase);
        statetmp=(unsigned long)(statetmp/mBase);
    }
//  mDigitVector.print();//test 
//  mGeneMat.print();//test
    //Dim=1
    //
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
        xx +=mDigitVectortmp.getValue(i,0)/AQLMath::pow(mBase,(i+1));
    }
    variates[0]=xx;
    xx=0;

//Dim=2
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
            xx +=mDigitVectortmp.getValue(i,0)/AQLMath::pow(mBase,(i+1));
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

    //
    this->mState = this->mState + 1;
    
    // counter increments it by one, if exceeds the Max, back to the first
    if (this->mState==this->mMax){
        this->mState=getSeed()[0];
    }

    return;
}

/*!
    @brief Seed
    @param[in] seed SeedValarrayVector\n
        seed[0]=,\n
            seed[1]=Sequence,\n
            seed[2]=Seed For Generator Matrix,\n
            seed[3]=Seed For Mersennne Twister
*/
void
AQLFTQuasiRandGF::setSeed(const UlongArray& seed)
{
    if (seed.size()!=4){
        AQLString msg =  "Number-of-Seed Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        //error. wrong Seed setting
    }
    AQLRandBase::setSeed(seed);
    mMax=seed[0]+seed[1];
    mState=seed[0];
    mSeedSet=true;

    //Mersennne Twisterseed
    UlongArray seedtmp(1);

    seedtmp[0]=seed[3];
    (this->mInner).setSeed(seedtmp);

    if (mDimSet && mSeedSet) paraCalc();
    return;
}
/*!
    @brief  \n
            ()

    @param[in] dimValue ValarrayVector\n
        dimValue[0]=,\n
            dimValue[1]=
*/
void
AQLFTQuasiRandGF::setDim(const UintArray& dimValue)
{
    if (dimValue.size()!=2){
        AQLString msg =  "Number-of-Dim Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        //error. wrong Dim setting
    }
    AQLRandBase::setDim(dimValue);
    mDimSet=true;
    
    //Mersennne TwisterDim
    UintArray dimtmp(1);
    if ((dimValue[0]-dimValue[1])<0){
        AQLString msg =  "TotalDim<QuasiRandDim Error";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        // 
    }
    dimtmp[0]=dimValue[0]-dimValue[1];

	if (dimtmp[0] != 0) (this->mInner).setDim(dimtmp);

    if (mDimSet && mSeedSet) paraCalc();
    return;
}

/*!
    @brief  Seed
*/
void
AQLFTQuasiRandGF::paraCalc()
{

//mBase
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
                            for (i=3;i <= (int)(AQLMath::sqrt(mBase))+1;i++)
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

    //Generator((n))
    unsigned long ntmp=mMax;
    unsigned int size = 1;
    if (getSeed()[1]!=0){
        while (ntmp>=mBase)
        {
            ntmp /= mBase;
            size += 1;
        }
    }

    //Size
    mGeneMat.resize(size,size);
    mRandMat.resize(size,size);
    mDigitVector.resize(size,1);
    mDigitVectortmp.resize(size,1);
    mqvec.resize(getDim()[1],1);

    //Generalization Matrix1
    if ((this->getDim()[0])!=0){
        getmqvec();
    }

    //Faure
    for (unsigned i=0;i<size;i++)
    {
        for (unsigned j=i;j<size;j++)
        {
            mGeneMat.setValue(i,j, (((int)(AQLMath::bico(j,i)+0.000000001)) % mBase));
        }
    }

    //MersenneTwister
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
AQLFTQuasiRandGF::ftsrand (unsigned long seed)
{
    this->ftholdrand  = (long)seed;
}
/*!
    @brief rand function
    @return random numbers
*/
int
AQLFTQuasiRandGF::ftrand (void)
{
    return(((ftholdrand = ftholdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}
/*!
    @brief  parameter calc2
*/
void 
AQLFTQuasiRandGF::getmqvec(void)
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
