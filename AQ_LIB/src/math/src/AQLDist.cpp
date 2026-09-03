/*! @file
    @brief Implementation to provide distribution functions.
            Only static functions are implemented on this class.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
//// SWITCH For Pricing
// CHINO #define NORMSDIST_CRIMAX // ON WITH CRIMAX
//#define NORMSDIST_TUCHIY // ON WITH TUCHIYA-SAN FUNCTION
//#define NORMSDIST_ERRFNC // ON WITH USE ERRFUNC :: DEFAULT

//#define NORMSDIST2_CRIMAX // ON WITH CRIMAX
// CHINO#define NORMSDIST2_MINE   

// CHINO#define INVNORM_TUCHY


#include "AQLDist.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <climits>

#ifdef _MSC_VER
#include <FLOAT.H>
#endif



/*!
    @briefcumulative density function of two-dimensional standard normal distribution \n
    (See OPTIONS, FUTURES, & OTHER DERIVATIVES)
    @param[in] x the probability that the first variable is less than x
    @param[in] y the probability that the second variable is less than y
    @param[in] rho the correlation coefficient between two variables
    @return cumulative density
*/
double          
AQLDist::normsdist2(double x, double y, double rho)
{

    double wk;
    AQLMath::epsValue(wk);

    if (AQLMath::abs(1-rho) < wk){
        //rho = 1, normsdist2(x,y,rho)=normdist(min(x,y))
        return normsdist(AQLMath::min(x,y));

    }else if (AQLMath::abs(1+rho) < wk){
        //rho=-1, x=<-y then normsdist2(x,y,rho)=0,
        //              x=>-y then normsdist2(x,y,rho)=normdist(x)-normdist(-y)
        return (x+y>0 ? normsdist(x)-normsdist(-y):0);

    }else if (AQLMath::abs(rho) < wk){
        //rho=0,normsdist2(x,y,0)=normsdist(x)*normsdist(y)
        return (normsdist(x)*normsdist(y));

    }else{
        double A[4]={0.3253030, 0.4211071, 0.1334425, 0.006374323};
        double B[4]={0.1337764, 0.6243247, 1.3425378, 2.2626645};
        double xx, yy;
        double sum =0;
        if (x <=0 && y<=0 && rho<0){
            xx=x/(AQLMath::sqrt(2*(1-rho*rho)));
            yy=y/(AQLMath::sqrt(2*(1-rho*rho)));

            for (unsigned int i = 0 ; i<4;i++){
                for (unsigned int j=0;j<4;j++){
                    sum +=A[i]*A[j]
                    *(AQLMath::exp(xx*(2*B[i]-xx)+yy*(2*B[j]-yy)+2*rho*(B[i]-xx)*(B[j]-yy)));
                }
            }
            
            return AQLMath::sqrt(1-rho*rho)/AQLMath::pi()*sum;

        }else if (x <=0 && y>=0 && rho>0){
            return AQLDist::normsdist(x)-AQLDist::normsdist2(x,-y,-rho);

        }else if (x >=0 && y<=0 && rho>0){
            return AQLDist::normsdist(y)-AQLDist::normsdist2(-x,y,-rho);
            
        }else if (x >=0 && y>=0 && rho<0){
            return AQLDist::normsdist(x) +AQLDist::normsdist(y)-1+AQLDist::normsdist2(-x,-y,rho);
        
        }else{
            double rho1, rho2, delta;
            rho1=(rho*x-y)*AQLMath::sign(1,x)/(AQLMath::sqrt(x*x-2*rho*x*y+y*y));
            rho2=(rho*y-x)*AQLMath::sign(1,y)/(AQLMath::sqrt(x*x-2*rho*x*y+y*y));
            delta = ( 1 - AQLMath::sign(1,x)*AQLMath::sign(1,y) ) * 0.25;
            return AQLDist::normsdist2(x,0,rho1) + AQLDist::normsdist2(y,0,rho2) -delta;
        }
    }   
}

/*!
    @brief cumulative binomial probability function, and return the probability tha
		   success is greater than or equal to the number of k of n trials
    @param[in] k number of success (integer)
    @param[in] n number of trials (integer)
    @param[in] p ratio of success double)
    @return cumulative density (double)
*/
double          
AQLDist::binomdist(unsigned int k, unsigned int n, double p)
{
    return AQLMath::betai((double)k, (double)(n-k+1), p);
}

/*!
    @brief cumulative binomial probability function, and return the probability that
		   number of occurrences is from 0 to k-1 [0,k-1) in case the expectation is x

	@param[in] x expectation
    @param[in] k number of occurrences
    @return cumulative density (double)
*/
double          
AQLDist::poissondist(double x, unsigned int k)
{
    return (double)1.0 - AQLMath::gammp((double)k, x);
}

/*!
    @brief chi-square probability function, and return the probability that
		   the chi-square value is less than x under n degrees of freedom

	@param[in] x realized value (double)
    @param[in] n degrees of freedom (integer)
    @return cumulative density (double)
*/
double          
AQLDist::chidist(double x, unsigned int n)
{
    return AQLMath::gammp((double)n /(double)2.0, x/(double)2.0);
}

/*!
    @brief t-distribution function, and return the probability that
		   the realized value is less than t (both sides) under n degrees of freedom.
           Beware that this is not the CDF, it is |CDF(x) - CDF(-x)|.
    @param[in] t realized value (double)
    @param[in] n degrees of freedom (integer)
    @return cumulative density (double)
*/
double
AQLDist::tdist(double t, unsigned int n)
{
    double nn = double(n);
    return (double)1.0 - AQLMath::betai(nn /(double)2.0, (double)0.5,
                                        nn / (nn + AQLMath::sqr(t)));
}

/*!
    @brief F distribution function, and return the probability that
		   the realized value is less than t under n1 and n2 degrees of freedom
	
	@param[in] n1 degrees of freedom (integer)
    @param[in] n2 degrees of freedom (integer)
    @param[in] t realized value (double)
    @return cumulative density (double)
*/
double          
AQLDist::fdist(double t, unsigned int n1, unsigned int n2)
{
    double nn1 = (double)n1;
    double nn2 = (double)n2;

    return AQLMath::betai(nn2 /(double)2.0, nn1 /(double)2.0,
                                        nn2 / (nn2 + nn1*t));
}

