#include "tryMirSetupPCA.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace
{
    static std::map<std::string, size_t> num_call_pca;
}

namespace validation
{

    /* @brief			validation interface for the mirSetupPCA method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		id				ID used to identify a PCA analysis
    *  @param [in]		Data			Data under analysis
    *  @param [in]		isScale			True means using correlation matrix. False means using covariance matrix. Default to false
    *  @param [in]		factorNum		The number of PCA factors to show results for
    *  @return			A string that identifies the current PCA analysis
    */
    const AQLString tryMirSetupPCA( AQLDataInstance* dataInstance,
                                   const AQLString& id,
                                   const DoubleMatrix& data,
                                   bool isScale,
                                   int factorNum )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirSetupPCA_inputs" );
            file.write( "generatorFunction", "tryMirSetupPCA" );
            file.write( "id", id );
            file.write( "data", data );
            file.write( "isScale", isScale );
            file.write( "factorNum", factorNum );
        }

        DoubleArray mean;
        DoubleMatrix coVar;
        etrading::LAUpdateStaticDataManager::CalcMeanAndCovariance( data, mean, coVar );

        DoubleMatrix corr( coVar.size() );
        if ( isScale )
        {
            // Convert to correlation matrix
            for ( size_t i = 0; i < coVar.size(); i++ )
            {
                corr[i].resize( coVar[i].size() );
                for( size_t j = i; j < coVar[i].size(); j++ )
                {
                    corr[i][j] = coVar[i][j] / sqrt( coVar[i][i] * coVar[j][j] );
                }
            }

            for ( size_t i = 0; i < coVar.size(); i++ )
            {
                for( size_t j = i; j < coVar[i].size(); j++ )
                {
                    corr[j][i] = corr[i][j];
                }
            }
        }
        else
        {
            // Keep the covariance matrix
            corr = coVar;
        }

        // Carry out PCA
        etrading::LAUpdateStaticDataManager::SetUpPCA( dataInstance, corr, factorNum, id );

        ++num_call_pca[id.getCString()];
        AQLString msg = id + ":" + AQLString( static_cast<int > ( num_call_pca[id.getCString()] ) );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirSetupPCA_outputs" );
            file.write( "output", msg );
        }

        return msg;

        VALID_EXCEPTION_END
    }

}
