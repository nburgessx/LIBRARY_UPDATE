
#include <numeric>
#include "LAModelUtilities.h"
#include "LAEigenSystems.h"

//
DoubleMatrix LAEigenSystems::factorloading2corre( const DoubleMatrix& factor_loading )
{
    size_t corr_s = factor_loading.size();
    DoubleMatrix corr_mat(corr_s, DoubleVector(corr_s, 1));
    for( size_t i = 0; i < corr_s; i++ )
    {
        for( size_t j = i + 1; j < corr_s; j++ )
        {
            corr_mat[i][j] = std::inner_product(factor_loading[i].begin(), factor_loading[i].end(), factor_loading[j].begin(), 0.0);
            corr_mat[j][i] = corr_mat[i][j];
        }
    }
    return corr_mat;
}

//
DoubleMatrix LAEigenSystems::corre2factorloading( const DoubleMatrix& corr, size_t no_factors )
{
    size_t corr_s = corr.size();
    no_factors = ((no_factors < corr_s)&&(0 < no_factors)) ? no_factors : corr_s;

    DoubleMatrix factor_loading( corr_s, DoubleVector(no_factors) );
    DoubleMatrix e_vecs( corr_s, DoubleVector(corr_s) );
    DoubleVector e_vals( corr_s );

    eigensystem(corr, e_vals, e_vecs);
    eigsrt(e_vals, e_vecs);

    double tmp;// normalized the factor loading
    for (size_t i = 0; i < corr_s; i++)
    {
        tmp = 0.0;
        for (size_t j = 0; j < no_factors; j++)
        {
            factor_loading[i][j] = e_vecs[i][j] * sqrt(e_vals[j]);
            tmp += factor_loading[i][j] * factor_loading[i][j];
        }
    }

    return LAEigenSystems::normalizeMat(factor_loading);
}

DoubleMatrix LAEigenSystems::normalizeMat( DoubleMatrix& Mat )
{
	size_t n = Mat.size();
	size_t m = Mat[0].size();

	if( n == 0 || m == 0 ) throw LACoreInvalidData("Mat.size() and 1 Mat[0].size() must bigger than 1!", __FILE__, __LINE__);
	
	double tmp;
	for(size_t i = 0; i < n; ++i)
	{
		tmp = inner_product( Mat[i].begin(), Mat[i].end(), Mat[i].begin(), 0.0 );
		for(size_t j = 0; j < m; ++j)
		{
			Mat[i][j] /= sqrt(tmp);
		}
	}
	return Mat;
}

//Functions for solving eigenproblems

//unsorted eigenvalues and eigenvectors
void LAEigenSystems::eigensystem(const DoubleMatrix&	orig_mat,
						  DoubleVector&			e_vals,
						  DoubleMatrix&			e_vecs
		)
{
    size_t mat_rank = orig_mat.size();

    //check for symmetry and rank
    if (orig_mat.front().size() != mat_rank)
    {
        throw LACoreInvalidData("The matrix is not square : eigensystem", __FILE__, __LINE__);
    }

    for (size_t i = 0; i < mat_rank; i++)
    {
        for (size_t j = i + 1; j < mat_rank; j++)
        {
            if ( !LAModelUtilities::eq(orig_mat[i][j],orig_mat[j][i]) )
            {
                throw LACoreInvalidData("The matrix is not symmetric : eigensystem", __FILE__, __LINE__);
            }
        }
    }

    e_vals.clear();
    e_vals.resize(mat_rank, 1);
    e_vecs.clear();
    e_vecs = orig_mat;

    DoubleVector sub_diag(mat_rank, 1);
    //get tridiagonal matrix
    tred2(e_vecs, e_vals, sub_diag);
    // get eigendata
    tqli( e_vals, sub_diag, e_vecs);
}


//
void LAEigenSystems::tred2(DoubleMatrix &a, DoubleVector &d, DoubleVector &e)
{
    size_t l, k, j, i;
    double scale, hh, h, g, f;
    size_t n = d.size();

    for (i = n - 1; i > 0; i--)
    {
        l = i - 1;
        h = scale = 0.;
        if (l > 0)
        {
            for (k = 0; k < l + 1; k++)
                scale += fabs(a[i][k]);
//          if (scale == 0.)
            if (LAModelUtilities::eq(scale,0.))
            {
                e[i] = a[i][l];
            }
            else
            {
                for (k = 0; k < l + 1; k++)
                {
                    a[i][k] /= scale;
                    h += a[i][k] * a[i][k];
                }
                f = a[i][l];
                g = ( (f>0. || LAModelUtilities::eq(f,0.)) ? -sqrt(h) : sqrt(h) );
                e[i] = scale * g;
                h -= f * g;
                a[i][l] = f - g;
                f = 0.;
                for ( j = 0; j < l + 1; j++)
                {
                // Next statement can be omitted if eigenvectors not wanted
                    a[j][i] = a[i][j] / h;
                    g = 0.;
                    for ( k = 0; k < j + 1; k++)
                    {
                        g += a[j][k] * a[i][k];
                    }
                    for (k = j + 1; k < l + 1; k++)
                    {
                        g += a[k][j] * a[i][k];
                    }
                    e[j] = g / h;
                    f += e[j] * a[i][j];
                }
                hh = f / ( h + h );
                for ( j = 0; j < l + 1; j++)
                {
                    f = a[i][j];
                    e[j] = g = e[j] - hh * f;
                    for ( k = 0; k < j+1; k++)
                    {
                        a[j][k] -= (f * e[k] + g * a[i][k]);
                    }
                }
            }
        }
        else
        {
            e[i] = a[i][l];
        }
        d[i] = h;
    }
    // Next statement can be omitted if eigenvectors not wanted
    d[0] = 0.;
    e[0] = 0.;
    // Contents of this loop can be omitted if eigenvectors not
    //  wanted except for statement d[i]=a[i][i];
    for (i = 0; i < n; i++)
    {
        l = i;
        if ( !LAModelUtilities::eq(d[i],0.) )
        {
            for ( j = 0; j < l; j++)
            {
                g = 0.;
                for ( k = 0; k < l; k++)
                {
                    g += a[i][k] * a[k][j];
                }
                for (k = 0; k < l; k++)
                {
                    a[k][j] -= g * a[k][i];
                }
            }
        }
        d[i] = a[i][i];
        a[i][i] = 1.;
        for (j = 0; j < l; j++)
        {
            a[j][i] = a[i][j] = 0.;
        }
    }
}

//
void LAEigenSystems::tqli(DoubleVector &d, DoubleVector &e, DoubleMatrix &z)
{
    int m, l, iter, i, k;
    double s, r, p, g, f, dd, c, b;
    int n = static_cast<int>(d.size());

    // convenient to adjust the size of "e"
    for (i = 1; i < n; i++)
    {
        e[i-1] = e[i];
    }
    e[n-1] = 0.;
    //e.erase(e.begin()); e.push_back(0);

    for (l = 0; l < n; l++)
    {
        iter = 0;
        do
        {
            for ( m = l; m < n - 1; m++)
            {
                // look for a single small subdiagonal element to split the matrix
                dd = fabs(d[m]) + fabs(d[m+1]);
//              if (fabs(e[m]) + dd == dd)
                if ( LAModelUtilities::eq(fabs(e[m]) + dd, dd) )
                {
                    break;
                }
            }
            if (m != l)
            {
                if (iter++ == 30)
                {
                    throw LACoreInvalidData("Too many iterations. No convergence in 30 iterations : tqli ", __FILE__, __LINE__);
                }
                //form shift
                g = (d[l+1] - d[l]) / (2. * e[l]);
                r = LAModelUtilities::pythag(g,1.);
                g = d[m] - d[l] + e[l] / (g + LAModelUtilities::sign(r,g));// this is d_m - k_s
                s = c = 1.;
                p = 0.;
                for (i = m - 1; i >= l; i--)
                {
                    // A plane rotation as in original QL, followed by Givens relations to restore tridiagonal form,
                    f = s * e[i];
                    b = c * e[i];
                    e[i+1] = (r = LAModelUtilities::pythag(f,g));
//                  if (r == 0.)
                    if ( LAModelUtilities::eq(r,0.) )
                    {
                        //recover from underflow
                        d[i+1] -= p;
                        e[m] = 0.;
                        break;
                    }
                    s = f / r;
                    c = g / r;
                    g = d[i+1] - p;
                    r = (d[i] - g) * s + 2. * c * b;
                    d[i+1] = g + (p = s * r);
                    g = c * r - b;
                    // Next loop can be omitted if eigenvectors not wanted
                    for (k = 0; k < n; k++)
                    {
                        // form eigenvectors
                        f = z[k][i+1];
                        z[k][i+1] = s * z[k][i] + c * f;
                        z[k][i] = c * z[k][i] - s * f;
                    }
                }
//              if (r == 0. && i >= l)
                if ( LAModelUtilities::eq(r,0.) && i >= l)
                {
                    continue;
                }
                d[l] -= p;
                e[l] = g;
                e[m] = 0.;
            }
        } while (m != l);
    }
}

//
void LAEigenSystems::eigsrt(DoubleVector &d, DoubleMatrix &v)
{
    size_t i,j,k;
    double p;
    size_t n = d.size();

    for (i = 0; i < n - 1; i++)
    {
        p = d[k = i];
        for (j = i; j < n; j++)
        {
            if (d[j]>p || LAModelUtilities::eq(d[j],p) )
            {
                p = d[k = j];
            }
        }
        if (k != i)
        {
            d[k] = d[i];
            d[i] = p;
            for (j = 0; j < n; j++)
            {
                swap(v[j][i],v[j][k]);
                //p = v[j][i]; v[j][i] = v[j][k];   v[j][k] = p;
            }
        }
    }
}

double LAEigenSystems::volV( double tT, const DoubleVector& paramV )
{
    if (tT < 0 ) return 0;
    double ret = ( paramV[0] + paramV[1] * tT ) * exp( -paramV[2] * tT ) + paramV[3];
    return 0 < ret ? ret : 0;
}

//
double LAEigenSystems::volF( double t, const DoubleVector& paramF )
{
    if (t < 0.0 || LAModelUtilities::eq(t,0.0) ) return 0.0;

//    const double PI = 3.14159265358979;
    const double PI = M_PI;
    double theta = PI / paramF[8];
    double f = 0.0;

    for (size_t i = 0; i < 3; i++ )
    {
        f += paramF[2 * i + 1] * sin((i + 1) * theta * t + paramF[2 * (i + 1)]);
    }

    // decay, shift and return
    double ret = exp( -paramF[0] * t) * f + paramF[7];
    return 0 < ret ? ret : 0;
}

//
double LAEigenSystems::sigma0(double t,
					   double T,
					   const DoubleVector& paramV,
					   const DoubleVector& paramF,
					   double G
		)
{
    if ( T < t ) return 0.0;
    double ret = LAEigenSystems::volV(T - t, paramV) * LAEigenSystems::volF(t, paramF) * G;
    return 0.0 < ret ? ret : 0.0;
}

