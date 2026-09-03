#include "tryMirGetPCAResults.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace validation
{

    /* @brief			validation interface for the mirGetPCAResults method
    *  @param [in]		id			ID used to identify a PCA analysis
    *  @param [in]		update		A number that tracks the nth calculation of the same PCA analysis
    *  @return			PCA results
    */
    LAStringMatrix tryMirGetPCAResults( LADataInstance* dataInstance,
                                      const LAString& id,
                                      int update )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetPCAResults_inputs" );
            file.write( "generatorFunction", "tryMirGetPCAResults" );
            file.write( "id", id );
            file.write( "update", update );
        }

        // Get Eigen vector results
        DoubleMatrix tmp;
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "EIGEN_VECTORS" ), id );
        size_t num_factor = tmp.size();
        size_t dim_data = tmp[0].size();

        // Initialise output matrix
        LAStringMatrix ret;
        ret.resize( dim_data + 3 );
        for( size_t i = 0; i < dim_data + 3; ++i )
        {
            ret[i].resize( num_factor + 1 );
        }

        // Headers
        ret[0][0] = "";
        for( size_t i = 0; i < num_factor; i++ )
        {
            LAString tmp_str = LAString( "PC" ) + LAString( static_cast<double>( i + 1 ), 0 );
            ret[0][i + 1] = tmp_str.getCString();
        }

        // Data
        for ( size_t j = 0; j < dim_data; j++ )
        {
            LAString tmp_str = LAString( "Series" ) + LAString( static_cast<double>( j + 1 ), 0 );
            ret[j + 1][0] = tmp_str.getCString();

            for( unsigned int i = 0; i < num_factor; i++ )
            {
                ret[j + 1][i + 1] = LAString( tmp[i][j] );
            }
        }

        // Get POV results
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "POV" ), id );

        ret[dim_data + 1][0] = "POV";
        for( unsigned int i = 0; i < num_factor; i++ )
        {
            ret[dim_data + 1][i + 1] = LAString( tmp[0][i] );
        }

        // Get Eigen value results
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "EIGEN_VALUES" ), id );
        ret[dim_data + 2][0] = "Eigen Values";
        for( unsigned int i = 0; i < num_factor; i++ )
        {
            ret[dim_data + 2][i + 1] = LAString( tmp[0][i] );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirGetPCAResults_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}


