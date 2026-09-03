// AQLConstrainedSplineInterpolation.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLConstrainedSplineInterpolation.h"
#include "AQLBasic.h"
using namespace std;

#define D_ZERO		0.00000001
#define D_UPPER		10000000000.0

//================ AQLInterpolationBase ===================================
/*!
    @brief constructor
*/
AQLConstrainedSplineInterpolation::AQLConstrainedSplineInterpolation()
: AQLInterpolationBase()
{
	mpDataProvider=NULL;

}
/*!
    @brief destructor
*/

AQLConstrainedSplineInterpolation::~AQLConstrainedSplineInterpolation()
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}
/*!
    @brief copy constructor
*/
AQLConstrainedSplineInterpolation::AQLConstrainedSplineInterpolation(const AQLConstrainedSplineInterpolation& v)
: AQLInterpolationBase(v), mpDataProvider(NULL)
{
    try 
	{	
		mpDataProvider = new AQLConstrainedSplineInterpolationDataProvider(*v.mpDataProvider);
		
		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index, mpDataProvider->value );
	}
    catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief clone 
*/
AQLCoreFunctionBase*	
AQLConstrainedSplineInterpolation::clone() const
{
    try 
	{
		AQLConstrainedSplineInterpolation* pTmp;
		pTmp =new AQLConstrainedSplineInterpolation;

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new AQLConstrainedSplineInterpolationDataProvider(this->mpDataProvider->index ,this->mpDataProvider->value, this->mpDataProvider->a,
                this->mpDataProvider->b, this->mpDataProvider->c, this->mpDataProvider->d, this->mpDataProvider->size, this->mpDataProvider->slope0, this->mpDataProvider->slope1);
			
			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index, pTmp->mpDataProvider->value);
		}

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
//===================	AQLConstrainedSplineInterpolation.cpp20051014(1)  end  =======================//
}

/*!
    @brief check whether the class is a class of ID 
	@param[in] id the type of class(function_t type)
	@return True or false
*/
bool
AQLConstrainedSplineInterpolation::isTypeOf(function_t id) const
{
	return (id == FN_CONSTRAINEDSPLINEINTERPOLATION ? true : AQLInterpolationBase::isTypeOf(id));
}

/*!
    @brief return the type of class
	@return the type of class
*/
function_t
AQLConstrainedSplineInterpolation::getType() const
{
	return FN_CONSTRAINEDSPLINEINTERPOLATION;
}

/*!
    @brief return a one-dimensional interpolated value
	@param[in] x1  point to interpolate
	@return a interpolated value(one-dimension)
*/
double
AQLConstrainedSplineInterpolation::value(const double x1) const
{
	if (mpDataProvider == NULL)
	{
		throw AQLCoreInvalidData("#Error: Constrained Spline interpolation data has not been set", __FILE__, __LINE__);
	}

	if ( mpDataProvider->size == 1) return mpDataProvider->value[0];

    double ret;
	size_t Nmax, Num;

    //Number of lines = points - 1
    Nmax = mpDataProvider->index.size() - 1;

    //(1a) Find LineNumber or segment. Linear extrapolate if outside range.
    Num = 0;

    if( (x1 < mpDataProvider->index[0]) || (x1 > mpDataProvider->index[Nmax]) )
    {
        //X outisde range. Linear interpolate
        //Below min or max?
        if (x1 < mpDataProvider->index[0]) 
        {
            ret = mpDataProvider->value[0] + (x1 - mpDataProvider->index[0]) * mpDataProvider->slope0;

        }
        else 
        {
            ret = mpDataProvider->value[Nmax] + (x1 - mpDataProvider->index[Nmax]) * mpDataProvider->slope1;

        }

        return ret;

        /* This is the original constrained splines algorithm to do expolation.  We use a slightly modified algorithm to do the expolation 
        with thte a, b, c, d values that we already obtained.
        stdflt B = (y[Num] - y[Num - 1]) / calc_dxx(mpDataProvider->x[Num], mpDataProvider->x[Num - 1]);
        stdflt A = y[Num] - B * mpDataProvider->x[Num];
        o_Y = A + B * x1 ;


        return 0;
        */
    }
    else
    {
        //(1b) Find LineNumber or segment. 
        //this->mpDataProvider->x.search(x1, Num);
        
        for (size_t i = 1; i <= Nmax; i++)
        {
            if (x1 <= mpDataProvider->index[i])
            {
                Num = i;
                break;
            }
        }
        
    }
    
    ret = mpDataProvider->a[Num] + x1 * (mpDataProvider->b[Num]  +  x1  * (mpDataProvider->c[Num] + mpDataProvider->d[Num] * x1));

    return ret;
} 

	/*!
    @brief set one-dimensional information
	@param[in] index  x-axis value to be interpolated
	@param[in] value  y-axis value to be interpolated
	*/
void
AQLConstrainedSplineInterpolation::set(const DoubleArray& index, const DoubleArray& value)
{
	//To be called by yield curve or correlation function.
    //This one is different from the 4 parameters function calc_constrained_splines in that
    //there is no input vector i_vX, i.e., the x-axis values have to be generated with small delta<h>
    //therefore we use the library's setup_abscissas and setup_ordinary to get the values.

    if (index.size() != value.size())
	{
		throw AQLCoreInvalidData("index size and value size are not same", __FILE__, __LINE__);
	}
	if (index.size() == 0)
	{
		throw AQLCoreInvalidData("data size is zero", __FILE__, __LINE__);
	}
	if (index.size() == 1) return;

	// Check for Duplicates
	if ( index.size() > 1 )
	{
		double previousIndex = index[0];
		double thisIndex = index[1];
		for ( size_t i = 1; i < index.size(); ++i )
		{
			thisIndex		= index[i];
			previousIndex	= index[i-1];
			AQ_THROW_IF(thisIndex == previousIndex, "Invalid Interpolation Data: Duplicate data found with time value: " + AQ_TO_STRING_FROM_DOUBLE(thisIndex) + " years" )
		}
	}

    DoubleArray gxx;
    DoubleMatrix ggxx;	// 1st and 2nd derivative for left and right ends of line
    DoubleArray a,b,c,d;
    double slope0, slope1;

    size_t i, size, Nmax, Num;

    size = index.size();
    //Number of lines = points - 1
    Nmax = size - 1;
    Num = 0;

    gxx.resize(size);
    ggxx.resize(size);
    a.resize(size);
    b.resize(size);
    c.resize(size);
    d.resize(size);

    for(i = 0; i < size; i++)
        ggxx[i].resize(2);

    a[0] = b[0] = c[0] = d[0] = 0.0;
    ggxx[0][0] = ggxx[0][1] = 0.0;

    //Calc first derivative (slope) for intermediate points
    for (i = 0; i <=  Nmax; i++)
    {
		if(Nmax == 1 && i == 1)
		{
		      gxx[i] = 0.0;
		}

		else if (i == 0 ||  i == Nmax)
        {
            //Set very large slope at ends
            gxx[i] = D_UPPER;
        }
        else if ( ( AQLMath::abs(value[i + 1] - value[i] ) < D_ZERO) || ( AQLMath::abs(value[i] - value[i - 1]) < D_ZERO) )
        {
            //Only check for 0 dy. dx assumed NEVER equals 0 !
            gxx[i] = 0.0;
        }
        else if ( AQLMath::abs( (index[i + 1] - index[i]) / (value[i + 1] - value[i]) + (index[i] - index[i - 1]) / (value[i] - value[i - 1]) ) < D_ZERO )
        {
            //Pos PLUS neg slope is 0. Prevent div by zero.
            gxx[i] = 0.0;
        }
        else if ( (value[i + 1] - value[i]) * (value[i] - value[i - 1]) < 0 )
        {
            //Pos AND neg slope, assume slope = 0 to prevent overshoot
            gxx[i] = 0;
        }
        else
        {
            //Calculate an average slope for point based on connecting lines
            gxx[i] = 2.0 / (  AQLConstrainedSplineInterpolation::calc_dxx(index[i + 1], index[i]) / (value[i + 1] - value[i]) + AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) / (value[i] - value[i - 1])  );
        }
    }

    //Reset first derivative (slope) at first and last point

    //First point has 0 2nd derivative
    gxx[0] = 3.0 / 2 * (value[1] - value[0]) / AQLConstrainedSplineInterpolation::calc_dxx(index[1], index[0]) - gxx[1] / 2.0 ;
    slope0 = gxx[0];
    //Last point has 0 2nd derivative
    gxx[Nmax] = 3.0 / 2 * (value[Nmax] - value[Nmax-1]) / AQLConstrainedSplineInterpolation::calc_dxx(index[Nmax], index[Nmax-1]) - gxx[Nmax-1] / 2.0 ;
    slope1 = gxx[Nmax];

    //Calc second derivative at points
    for (i = 1; i <=  Nmax  ; i++)
    {
        ggxx[i][0] = -2.0   * (gxx[i] + 2 * gxx[i-1]) / AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) + 6.0 * (value[i] - value[i - 1]) / ((AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) * AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) ));
        ggxx[i][1] = 2.0 * (2 * gxx[i] + gxx[i-1]) / AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) - 6.0 * (value[i] - value[i - 1]) / ((AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]) * AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1])));

        //Calc constants for cubic
        d[i] = 1.0 / 6 * (ggxx[i][1] - ggxx[i][0]) / AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]);
        c[i] = 1.0 / 2 * (index[i] * ggxx[i][0] - index[i - 1] * ggxx[i][1]) / AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]);
        b[i] = (value[i] - value[i - 1] - c[i] * ((index[i] * index[i]) - (index[i - 1] * index[i -1]) ) - d[i] * ((index[i] * index[i] * index[i]) - (index[i - 1]  * index[i - 1]  * index[i - 1]) )) / AQLConstrainedSplineInterpolation::calc_dxx(index[i], index[i - 1]);
        a[i] = value[i - 1] - b[i] * index[i - 1] - c[i] * (index[i - 1] * index[i -1]) - d[i] * (index[i - 1] * index[i - 1] * index[i - 1]);

    }

    if (mpDataProvider != NULL)
	{
		//if value was input in mpDataProvider, delete it
		delete mpDataProvider;
	}
	
	mpDataProvider = new AQLConstrainedSplineInterpolationDataProvider(index, value, a, b, c, d, size, slope0, slope1);
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
}

double AQLConstrainedSplineInterpolation::calc_dxx(const double x1, const double x0)
{
    //Calc Xi - Xi-1 to prevent div by zero
    double dxx = x1 - x0;
    if (AQLMath::abs(dxx) < D_ZERO) 
        dxx = D_UPPER;

    return dxx;
};

std::tuple<std::vector<double>,std::vector<double>> AQLConstrainedSplineInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index, mpDataProvider->value);
};


