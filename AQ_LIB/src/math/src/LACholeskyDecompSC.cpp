/*! @file
    @brief 
            Only static functions are implemented on this class.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "LAMatrix.h"
#include "LACholeskyDecompSC.h"
#include <limits>

using namespace std;

/*! 
    @brief  \n 
	@
    
*/
LAMatrix
LACholeskyDecompSC::choleskyDecompositionSC(const LAMatrix& mat)
{
    if (! mat.isSymmetric()) 
    {
        throw LACoreNumericalError("matrix is Not symmetric one", 
                                        __FILE__, __LINE__);
    }

    size_t size = mat.row();

	IntArray rec_info(size);
	
	LAMatrix red_C(mat);

	// Working matrix in this routine
	size_t red_size = size;
    
	bool off_diagonal = check_off_diagonal(mat);

	if ( off_diagonal )
    {
		// This happens only if the references have strong dependency in capital structure
		reduce_matrix(mat, red_size, red_C, rec_info);
	//	cout << "Matrix reduced." << size << "->" << red_size << endl;
	}

		
	LAMatrix eVal, eVec;	// eigen values, eigen vectors
	LAMatrix L(red_size, red_size);		// temporary matrix
	
	red_C.eigenMatrix(eVec, eVal);
	
	bool pos_def = check_eVal(eVal);
	if ( !pos_def )
    {								// forcibly modify to positive definite matrix
		spectral_decomp(eVec, eVal, L);
		construct_matrix(L, red_C);		// modified matrix
//		cout<<"Matrix modified by spectral decomposition."<<endl;
	}

	while(!cholesky_decomp(red_C, L)) shift_diag(red_C);	// to deal with precision error

	// Create output M
	LAMatrix M(mat.row(), mat.row());
	if (off_diagonal) recover_matrix(L, rec_info, M);
	else M = L;
	return M;
}


bool
LACholeskyDecompSC::cholesky(const LAMatrix& in,LAMatrix& out)
{
 	unsigned int size = in.row();
	double x;
	for (unsigned int i = 0; i < size; i++)
    {
		for (unsigned int  j = i; j < size; j++)
        {
			x = in[i][j];
            unsigned int k;
			for (k = 0; k < i; k++) x -= out[i][k] * out[j][k];
			if (j == k)
            {
				if (x <= 0) return false;	// if unable to decompose, return 0 (false)
				else out.setValue(i, i, LAMath::sqrt(x));
			}
			else
            {
				out.setValue(j, i, x / out[i][i]);
            }
		}
    }
	return true;
}
bool
LACholeskyDecompSC::check_off_diagonal(const LAMatrix& C)
{
    size_t size = C.row();
	for (size_t i = 0; i < size; i++)
    {
		for (size_t j = 0; j < i; j++)
        {
			if ( C[i][j] == 1 )
            {
//				cout << "off_diagonal==1 found at (" << i << "," << j << ")" << endl;
				return true;
			}
        }
    }
	return false;
}

bool
LACholeskyDecompSC::check_eVal(const LAMatrix& eVal)
{
    size_t size = eVal.column();
	for (size_t i = 0; i < size; i++) if (eVal[0][i] < 0) return false;
	return true;
}

void
LACholeskyDecompSC::spectral_decomp(const LAMatrix& eVec,
                     LAMatrix& eVal,
                     LAMatrix& L)
{
    size_t size = eVal.column();
	size_t i, j = 0;
	for (i = 0; i < size; i++)
    {
		if (eVal[0][i] < 0) eVal.setValue(0, i, 0);
		else eVal.setValue(0, i, LAMath::sqrt(eVal[0][i]));
	}

	DoubleArray t(size);
	for (i = 0; i < size; i++)
    {
		t[i] = 0;
		for (j = 0; j < size; j++)
        {
			L.setValue(i, j, eVec[i][j] * eVal[0][j]);
			t[i] += L[i][j] * L[i][j];
		}
	}
	// normalize row vectors of the matrix
	for (i = 0; i < size; i++) 
		for (j = 0; j < size; j++)
			L.setValue(i, j, L[i][j] / LAMath::sqrt(t[i]));
}

bool
LACholeskyDecompSC::cholesky_decomp(const LAMatrix& C,LAMatrix& L)
{
    size_t size = C.row();
	size_t i, j = 0;
	L.setValue(0);
	if ( cholesky(C, L) )
    {
		if (L[0][0] != 1.0) 
			for (i = 0; j < size; i++) 
				for (j = 0; j < size; j++) 
					L.setValue(i, j, L[i][j] / L[0][0]);
		return true;
	}
	return false;
}

void
LACholeskyDecompSC::reduce_matrix(const LAMatrix& C,
                   size_t &size2,
                   LAMatrix& C2,
                   IntArray& rec_info)
{
    size_t size = C.row();
	size_t i, j, row, col;

	// Initialize recovery info.
	for (i = 0; i < size; i++) rec_info[i] = -1;
	for (i = 0; i < size; i++) for (j = i + 1; j < size; j++) if (C[i][j] == 1 && rec_info[j] == -1) rec_info[j] = (int)i;

	// Calculate the reduced matrix size.
	for (size2 = 0, i = 0; i < size; i++) if ( rec_info[i] == -1) size2++;

	if (size != size2) C2.resize(size2, size2);
	for (row = 0, i = 0; i < size; i++)
    {
		if (rec_info[i] != -1) continue;
		for (col = 0, j = 0; j < size; j++)
        {
			if (rec_info[j] != -1) continue;
			C2.setValue(row, col, C[i][j]);
			col++;
		}
		row++;
	}
}

void
LACholeskyDecompSC::recover_matrix(const LAMatrix& C,
                    const IntArray& rec_info,
                    LAMatrix& C2)
{
    size_t size = rec_info.size();

	size_t i, j, row, col;
	for (row=0, i = 0; i < size; i++)
	{
		if (rec_info[i] != -1) for (j = 0; j < size; j++) C2.setValue(i, j, C2[rec_info[i]][j]);
		else
		{
			for (col = 0, j = 0; j < size; j++)
			{
				if (rec_info[j] != -1) C2.setValue(i, j, 0);
				else
				{
					C2.setValue(i, j, C[row][col]);
					col++;
				}
			}
			row++;
		}
	}
}

void
LACholeskyDecompSC::construct_matrix(const LAMatrix& L,LAMatrix& C)
{
    size_t size = L.row();
	size_t i, j, k;
	double tmp;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			for (tmp = 0.0, k = 0; k < size; k++) tmp += L[i][k] * L[j][k]; //C[i][j] += L[i][k] * Transposed_L[k][j];
			C.setValue(i, j, tmp);
		}
	}
}

void
LACholeskyDecompSC::shift_diag(LAMatrix& C)
{
    static const double SMALL_NUMBER = numeric_limits<double>::epsilon();
    size_t size = C.row();
	size_t i;
	for (i = 0; i < size; i++) C.setValue(i, i, C[i][i] + SMALL_NUMBER);
}




