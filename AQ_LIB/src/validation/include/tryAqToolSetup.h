#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
 	 /* @brief	Set-up AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string trySetupAQL( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath, const std::string& centralBankCalendarFullFilePath = "" );

	/* @brief	Tear-down AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string tryTearDownAQL();

    /* @brief	Clear the object pool
     *  @return	A notification string
     */
    AQLString tryAqToolClearEntityPool();

    /* @brief	Clear the object pool and the AQObj object cache
     *  @return	A notification string
     */
    AQLString tryAqObjectClearCache();


    /* @brief			validation interface for the aqToolLoadCalendarFile function
    *  @param [in]		filepath		The full name of the calendar file
    *  @return	A notification string
    */
    AQLString tryAqToolLoadCalendarFile( const AQLString& filepath );

    /* @brief			validation interface for the aqToolLoadStaticData function
    *  @param [in]		filepath The full name of the static data file
    *  @return			A notification string
    */
    AQLString tryAqToolLoadStaticData( const AQLString& filepath );
	
    /* @brief			validation interface for loading the optional AQObj configuration files
    *  @return			A notification string
    */
	AQLString tryAqToolLoadConfigurationFiles();

    /* @brief			validation interface for the aqToolVersion function
    *  @param [in]		expiryMonth			An integer representing license expiry month
    *  @param [in]		expiryYear			An integer representing license expiry year
    *  @param [in]		showLicenceExpiry	Boolean to show licence expiry date. Defaults to false.
    *  @return	A string showing current version
    */
    std::string tryAqToolVersion( int& expiryMonth, int& expiryYear, bool showLicenceExpiry = false );

    // Method to enable OMP Parallelization Mode and OMP Threaded Methods
    std::string tryAqToolParallelModeEnable( const bool enable );

    // Method to get the OMP Parallelization Mode Status
    std::string tryAqToolParallelModeStatus();

}