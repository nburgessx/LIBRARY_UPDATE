#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

//===================== LACholeskyDecompSC =============================================
/*! 
    @brief Class to provide algorithms
*/
class LACholeskyDecompSC 
{
public:

								//======================================
								// CholeskyDecomposition by Mizuho SC Algorithm
	static	LAMatrix			choleskyDecompositionSC(const LAMatrix& mat);

private:
	static bool					cholesky(const LAMatrix& in, LAMatrix& out);
	static bool					check_off_diagonal(const LAMatrix& C);
	static bool					check_eVal(const LAMatrix& eVal);
	static void					spectral_decomp(const LAMatrix& eVec, LAMatrix& eVal, LAMatrix& L);
	static bool					cholesky_decomp(const LAMatrix& C, LAMatrix& L);
	static void					reduce_matrix(const LAMatrix& C, size_t &size2,
												LAMatrix& C2, IntArray& rec_info);
	static void					recover_matrix(const LAMatrix& C, const IntArray& rec_info, LAMatrix& C2);
	static void					construct_matrix(const LAMatrix& L, LAMatrix& C);
	static void					shift_diag(LAMatrix& C);

};

