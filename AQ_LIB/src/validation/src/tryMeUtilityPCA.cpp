#include "tryMeUtilityPCA.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace
{
    static std::map<std::string, size_t> num_call_pca;
}

namespace validation
{


    /* @brief			Retrieve the PCA results based on the PCA key
    *  @param [in]		key			Key used to identify a PCA analysis
    *  @return			A LAStringMatrix for the PCA results
    */
    LAStringMatrix getPCAResults( const LAString& key )
    {

        LADataInstance* dataInstance = etrading::getDataInstance();

        // Get Eigen vector results
        DoubleMatrix tmp;
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "EIGEN_VECTORS" ), key );
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
        ret[0][0] = LAString();
        for( size_t i = 0; i < num_factor; i++ )
        {
            ret[0][i + 1] = LAString( "PC" ) + LAString( static_cast<double>( i + 1 ), 0 );
        }

        // Data
        for ( size_t j = 0; j < dim_data; j++ )
        {
            ret[j + 1][0] = LAString( "Series" ) + LAString( static_cast<double>( j + 1 ), 0 );

            for( unsigned int i = 0; i < num_factor; i++ )
            {

                std::stringstream s;
                s << tmp[i][j];
                ret[j + 1][i + 1] = s.str().c_str();
            }
        }

        // Get POV results
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "POV" ), key );

        ret[dim_data + 1][0] = "Proportion of Variance";
        for( unsigned int i = 0; i < num_factor; i++ )
        {
            std::stringstream s;
            s << tmp[0][i];
            ret[dim_data + 1][i + 1] = s.str().c_str();
        }

        // Get Eigen value results
        tmp = etrading::LAUpdateStaticDataManager::GetPCAResult( dataInstance, LAString( "EIGEN_VALUES" ), key );
        ret[dim_data + 2][0] = "Eigen Values";
        for( unsigned int i = 0; i < num_factor; i++ )
        {
            std::stringstream s;
            s << tmp[0][i];
            ret[dim_data + 2][i + 1] = s.str().c_str();
        }

        return ret;
    }

    /* @brief			validation interface for the meUtilityPCA method
    *  @param [in]		key						Key used to identify a PCA analysis
    *  @param [in]		data					Data under analysis
    *  @param [in]		useCorrelationMatrix	True to use correlation matrix. False to use covariance matrix. Default to False
    *  @param [in]		nFactors				The number of PCA factors
    *  @return			A LAStringMatrix for the PCA results
    */
    LAStringMatrix tryMeUtilityPCA( const LAString& key, const DoubleMatrix& data, bool useCorrelationMatrix, int nFactors )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeUtilityPCA_inputs" );
            file.write( "generatorFunction", "tryMeUtilityPCA" );
            file.write( "key", key );
            file.write( "data", data );
            file.write( "useCorrelationMatrix", useCorrelationMatrix );
            file.write( "nFactors", nFactors );
        }

        DoubleArray mean;
        DoubleMatrix coVar;
        etrading::LAUpdateStaticDataManager::CalcMeanAndCovariance( data, mean, coVar );

        DoubleMatrix corr( coVar.size() );
        if ( useCorrelationMatrix )
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
        etrading::LAUpdateStaticDataManager::SetUpPCA( etrading::getDataInstance(), corr, nFactors, key );

        //++num_call_pca[key.getCString()];
        //LAString msg = key + ":" + LAString( static_cast<int > (num_call_pca[key.getCString()]) );

        LAStringMatrix ret = getPCAResults( key );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeUtilityPCA_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
