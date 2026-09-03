#pragma once

#include <vector>

#include "AQLModelDynamicsCurve.h"
#include "AQLFunction.h"
#include "AQLCoreTemplateType.h"
#include "AQLModelUtilities.h"

using namespace std;

//
class AQLMathVolatilityLMM
{

public:

    virtual ~AQLMathVolatilityLMM() {}

    virtual AQLMathVolatilityLMM* clone() const = 0;

    virtual double get( double t, size_t i ) = 0;

    virtual double get( double t, size_t i, double L ) { (void)L; return get(t, i); }

    virtual double get( double t, size_t i, AQLRatesPathElementCurve& curve ) { (void)curve; return get(t, i); }

    virtual double integrate0( double t, size_t i ) = 0;

    virtual double integrate0( double t, size_t i, double L ) { (void)L; return integrate0(t, i);}

    virtual double integrate0( double t, size_t i, AQLRatesPathElementCurve& curve ) { (void)curve; return integrate0(t, i);}

    virtual double integrateSQ0( double t, size_t i ) = 0;

    virtual double integrateSQ0( double t, size_t i, double L ) { (void)L; return integrateSQ0(t, i);}

    virtual double integrateSQ0( double t, size_t i, AQLRatesPathElementCurve& curve ) { (void)curve; return integrateSQ0(t, i);}

    virtual double integrate( double t_s, double t_e, size_t i ) { return integrate0(t_e, i) - integrate0(t_s, i); }

    virtual double integrate( double t_s, double t_e, size_t i, double L ) { return integrate0(t_e, i, L) - integrate0(t_s, i, L); }

    virtual double integrate( double t_s, double t_e, size_t i, AQLRatesPathElementCurve& curve ) { return integrate0(t_e, i, curve) - integrate0(t_s, i, curve); }

    virtual double integrateSQ( double t_s, double t_e, size_t i ) { return integrateSQ0(t_e, i) - integrateSQ0(t_s, i); }

    virtual double integrateSQ( double t_s, double t_e, size_t i, double L ) { return integrateSQ0(t_e, i, L) - integrateSQ0(t_s, i, L); }

    virtual double integrateSQ( double t_s, double t_e, size_t i, AQLRatesPathElementCurve& curve ) { return integrateSQ0(t_e, i, curve) - integrateSQ0(t_s, i, curve); }

    virtual DoubleVector getTGrid() const = 0;

    virtual bool isDiscrete() const = 0;
};

//
class AQLMathVolatilityLMMCont : public AQLMathVolatilityLMM
{

public:

    AQLMathVolatilityLMMCont( DoubleVector T_fix_, size_t n_ );

    AQLMathVolatilityLMMCont( const AQLMathVolatilityLMMCont& rhs );

    virtual ~AQLMathVolatilityLMMCont() {}

    AQLMathVolatilityLMMCont& operator =(const AQLMathVolatilityLMMCont& rhs);

    virtual AQLMathVolatilityLMMCont* clone() const = 0;

    virtual double get(double t, size_t i) = 0;

    virtual double integrate0( double t, size_t i );

    virtual double integrateSQ0( double t, size_t i );

    void setNumAbscissas(size_t n_) { mn = n_; }

    size_t getNumAbscissas() const { return mn; }

    DoubleVector getTGrid() const { return mT_fix; }

    bool isDiscrete() const { return false; }

protected:
    
    DoubleVector mT_fix;

private:

// Followings are integration related members.
    AQLMathFunction<AQLMathVolatilityLMMCont> mIntegrate;
    AQLMathFunction<AQLMathVolatilityLMMCont> mIntegrateSQ;
    double get(double t);
    double getSQ(double t);
    size_t mn;
    size_t mi_temp;
};

//
class AQLMathVolatilityLMMContFlat : public AQLMathVolatilityLMMCont
{

public:

    AQLMathVolatilityLMMContFlat( double vol_, DoubleVector T_fix ) : AQLMathVolatilityLMMCont( T_fix, 1 ), mVol( vol_ ) {}

    AQLMathVolatilityLMMContFlat* clone() const { return new AQLMathVolatilityLMMContFlat(*this); }

    virtual double get(double t, size_t i) { return (0 < t && t <= mT_fix[i]) ? mVol : 0; }

    virtual double integrate0( double t, size_t i ) { return get(t,i) * t;}

    virtual double integrateSQ0( double t, size_t i ) { return get(t,i) * get(t,i) * t;}

private:

    double mVol;
};

//
class AQLMathVolatilityLMMDisc : public AQLMathVolatilityLMM
{

public:

    AQLMathVolatilityLMMDisc( DoubleVector T_fix_, size_t num_small_step_ = 1 )
    :
    mT_fix( T_fix_ ),
    mNumSmallStep(num_small_step_),
    mDividedT_fix( vector<double>( mNumSmallStep * mT_fix.size(), 0.0 ) )
    {
        size_t k = 0;
        double intercept;
        double gradient;
        for( size_t i = 0; i < mT_fix.size(); ++i )
        {
            intercept = i == 0 ? 0.0 : mT_fix[i - 1];
            gradient = ( mT_fix[i] - intercept ) / static_cast<double>(mNumSmallStep);
            for(size_t j = 0; j < mNumSmallStep; ++j)
            {
                mDividedT_fix[k++] = gradient * static_cast<double>(j + 1) + intercept;
            }
        }
    }

    AQLMathVolatilityLMMDisc(const AQLMathVolatilityLMMDisc& rhs)
    :
    AQLMathVolatilityLMM(rhs),
    mT_fix(rhs.mT_fix),
    mNumSmallStep(rhs.mNumSmallStep),
    mDividedT_fix(rhs.mDividedT_fix),
    mCacheVolIntMatrix(rhs.mCacheVolIntMatrix),
    mCacheVolSqIntMatrix(rhs.mCacheVolSqIntMatrix)
    {
    }

    virtual ~AQLMathVolatilityLMMDisc() {}

    virtual AQLMathVolatilityLMMDisc* clone() const = 0;

    virtual double get(double t, size_t i) = 0;

    virtual double integrate0( double t, size_t i );

    virtual double integrateSQ0( double t, size_t i );

    DoubleVector getTGrid() const { return mT_fix; }

    DoubleVector getDividedTGrid() const { return mDividedT_fix; }

    bool isDiscrete() const { return true; }

    void createCacheVolIntMatrix();

protected:

    DoubleVector mT_fix;
    size_t mNumSmallStep;
    DoubleVector mDividedT_fix;

    size_t findIndex( double t );

private:

    double integrate0(size_t t, size_t i);
    double integrateSQ0(size_t t, size_t i);

    DoubleMatrix mCacheVolIntMatrix;
    DoubleMatrix mCacheVolSqIntMatrix;
};

//
class AQLMathVolatilityLMMDiscFlat : public AQLMathVolatilityLMMDisc
{

public:

    AQLMathVolatilityLMMDiscFlat( double vol_, DoubleVector T_fix ) : AQLMathVolatilityLMMDisc( T_fix ), mVol(vol_) { createCacheVolIntMatrix(); }

    AQLMathVolatilityLMMDiscFlat* clone() const { return new AQLMathVolatilityLMMDiscFlat(*this); }

    virtual double get(double t, size_t i) { return (0 < t && t <= mT_fix[i]) ? mVol : 0; }

    virtual double integrate0( double t, size_t i ) { return get(t,i) * t;}

    virtual double integrateSQ0( double t, size_t i ) { return get(t,i) * get(t,i) * t;}

private:

    double mVol;
};

//
class AQLMathVolatilityLMMDiscFlatStdDev : public AQLMathVolatilityLMMDiscFlat
{

public:

    AQLMathVolatilityLMMDiscFlatStdDev( double vol, DoubleVector T_fix ) : AQLMathVolatilityLMMDiscFlat( vol, T_fix ) {}

    AQLMathVolatilityLMMDiscFlatStdDev* clone() const { return new AQLMathVolatilityLMMDiscFlatStdDev(*this); }

    virtual double get(double t, size_t i) { return AQLMathVolatilityLMMDiscFlat::get(t,i) / sqrt(getT_fix(i)); }

    virtual double integrate0( double t, size_t i ) { return AQLMathVolatilityLMMDiscFlat::integrate0(t,i) / sqrt(getT_fix(i)); }

    virtual double integrateSQ0( double t, size_t i ) { return AQLMathVolatilityLMMDiscFlat::integrateSQ0(t,i) / getT_fix(i); }

private:

    double getT_fix( size_t i ) const { return mT_fix[i] < eps_1d ? eps_1d : mT_fix[i]; }
};

