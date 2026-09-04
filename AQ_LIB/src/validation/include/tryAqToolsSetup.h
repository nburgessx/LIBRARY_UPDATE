#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
 	 /* @brief	Set-up AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string trySetupMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath, const std::string& centralBankCalendarFullFilePath = "" );

	/* @brief	Tear-down AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string tryTearDownMLIB();

    /* @brief	Clear the object pool
     *  @return	A notification string
     */
    AQLString tryAqToolsClearEntityPool();

    /* @brief	Clear the object pool and the AQObj object cache
     *  @return	A notification string
     */
    AQLString tryAqObjClearCache();


    /* @brief			validation interface for the aqToolsLoadCalendarFile function
    *  @param [in]		filepath		The full name of the calendar file
    *  @return	A notification string
    */
    AQLString tryAqToolsLoadCalendarFile( const AQLString& filepath );

    /* @brief			validation interface for the aqToolsLoadStaticData function
    *  @param [in]		filepath The full name of the static data file
    *  @return			A notification string
    */
    AQLString tryAqToolsLoadStaticData( const AQLString& filepath );
	
    /* @brief			validation interface for loading the optional AQObj configuration files
    *  @return			A notification string
    */
	AQLString tryAqToolsLoadConfigurationFiles();

    /* @brief			validation interface for the aqToolsVersion function
    *  @param [in]		expiryMonth			An integer representing license expiry month
    *  @param [in]		expiryYear			An integer representing license expiry year
    *  @param [in]		showLicenceExpiry	Boolean to show licence expiry date. Defaults to false.
    *  @return	A string showing current version
    */
    std::string tryAqToolsVersion( int& expiryMonth, int& expiryYear, bool showLicenceExpiry = false );

    // Method to enable OMP Parallelization Mode and OMP Threaded Methods
    std::string tryAqToolsParallelModeEnable( const bool enable );

    // Method to get the OMP Parallelization Mode Status
    std::string tryAqToolsParallelModeStatus();

}