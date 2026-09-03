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
								// CholeskyDecomposition by AlgoQuantHub SC Algorithm
	static	AQLMatrix			choleskyDecompositionSC(const AQLMatrix& mat);

private:
	static bool					cholesky(const AQLMatrix& in, AQLMatrix& out);
	static bool					check_off_diagonal(const AQLMatrix& C);
	static bool					check_eVal(const AQLMatrix& eVal);
	static void					spectral_decomp(const AQLMatrix& eVec, AQLMatrix& eVal, AQLMatrix& L);
	static bool					cholesky_decomp(const AQLMatrix& C, AQLMatrix& L);
	static void					reduce_matrix(const AQLMatrix& C, size_t &size2,
												AQLMatrix& C2, IntArray& rec_info);
	static void					recover_matrix(const AQLMatrix& C, const IntArray& rec_info, AQLMatrix& C2);
	static void					construct_matrix(const AQLMatrix& L, AQLMatrix& C);
	static void					shift_diag(AQLMatrix& C);

};

