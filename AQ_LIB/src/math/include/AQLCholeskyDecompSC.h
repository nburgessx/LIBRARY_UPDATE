#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

//===================== AQLCholeskyDecompSC =============================================
/*! 
    @brief Class to provide algorithms
*/
class AQLCholeskyDecompSC 
{
public:

								//======================================
								// Cholesky decomposition algorithm
	static	AQLNumericMatrix			choleskyDecompositionSC(const AQLNumericMatrix& mat);

private:
	static bool					cholesky(const AQLNumericMatrix& in, AQLNumericMatrix& out);
	static bool					check_off_diagonal(const AQLNumericMatrix& C);
	static bool					check_eVal(const AQLNumericMatrix& eVal);
	static void					spectral_decomp(const AQLNumericMatrix& eVec, AQLNumericMatrix& eVal, AQLNumericMatrix& L);
	static bool					cholesky_decomp(const AQLNumericMatrix& C, AQLNumericMatrix& L);
	static void					reduce_matrix(const AQLNumericMatrix& C, size_t &size2,
												AQLNumericMatrix& C2, IntArray& rec_info);
	static void					recover_matrix(const AQLNumericMatrix& C, const IntArray& rec_info, AQLNumericMatrix& C2);
	static void					construct_matrix(const AQLNumericMatrix& L, AQLNumericMatrix& C);
	static void					shift_diag(AQLNumericMatrix& C);

};

