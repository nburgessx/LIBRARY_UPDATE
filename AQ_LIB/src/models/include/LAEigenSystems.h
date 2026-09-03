#pragma once

#include "AQLCoreTemplateType.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

class LAEigenSystems
{
	public:
		// Get integer part of double.
		const static double IntOfDouble(const double n);

		// Get decimal part of double.
		const static double DecOfDouble(const double n);

		static DoubleMatrix factorloading2corre( const DoubleMatrix& factor_loading );

		static DoubleMatrix corre2factorloading( const DoubleMatrix& corr, size_t no_factors );

		static DoubleMatrix normalizeMat( DoubleMatrix& Mat );

		// from LinAlgebra.h
		//+++++++++++++++++++++++++++++
		//++++++++ EIGENSYSTEMS +++++++
		//+++++++++++++++++++++++++++++
		
		// wrapped function for solving eigenproblems (includes all functions listed below)
		static void eigensystem(const DoubleMatrix&	orig_mat, 
								DoubleVector&		e_vals, 
								DoubleMatrix&		e_vecs
				);
		
		//Reduction of a real symmetric matrix to tridiagonal form
		static void tred2(DoubleMatrix& a, DoubleVector& d, DoubleVector& e);
		
		// Finding the eigenvalues and eigenvectors of initial matrix "a"
		static void tqli(DoubleVector& d, DoubleVector& e, DoubleMatrix& z);
		
		// sort eigenvalues and eigenvectors in descending order
		static void eigsrt(DoubleVector& d, DoubleMatrix &v);

		//=================================
		static double sigma0(double t,
							 double T,
							 const DoubleVector& paramV,
							 const DoubleVector& paramF,
							 double G
				);

		//=================================
		static double volV( double tT, const DoubleVector& paramV );

		//=================================
		static double volF( double t, const DoubleVector& paramF );

};
