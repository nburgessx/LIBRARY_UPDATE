/*
 * @brief			validation interface for the tryMeUtility setup related functions
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
 	 /* @brief	Set-up MLIBQ - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string trySetupMLIB( const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath, const std::string& centralBankCalendarFullFilePath = "" );

	/* @brief	Tear-down MLIBQ - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string tryTearDownMLIB();

    /* @brief	Clear the object pool
     *  @return	A notification string
     */
    LAString tryMeUtilityClearEntityPool();

    /* @brief	Clear the object pool and the LWO Cache
     *  @return	A notification string
     */
    LAString tryMeUtilityClearLWOCache();


    /* @brief			validation interface for the meUtilityLoadCalendarFile function
    *  @param [in]		filepath		The full name of the calendar file
    *  @return	A notification string
    */
    LAString tryMeUtilityLoadCalendarFile( const LAString& filepath );

    /* @brief			validation interface for the meUtilityLoadStaticData function
    *  @param [in]		filepath The full name of the static data file
    *  @return			A notification string
    */
    LAString tryMeUtilityLoadStaticData( const LAString& filepath );
	
    /* @brief			validation interface for loading the optional LWO configuration files
    *  @return			A notification string
    */
	LAString tryMeUtilityLoadConfigurationFiles();

    /* @brief			validation interface for the meUtilityVersion function
    *  @param [in]		expiryMonth			An integer representing license expiry month
    *  @param [in]		expiryYear			An integer representing license expiry year
    *  @param [in]		showLicenceExpiry	Boolean to show licence expiry date. Defaults to false.
    *  @return	A string showing current version
    */
    std::string tryMeUtilityVersion( int& expiryMonth, int& expiryYear, bool showLicenceExpiry = false );

    // Method to enable OMP Parallelization Mode and OMP Threaded Methods
    std::string tryMeUtilityParallelModeEnable( const bool enable );

    // Method to get the OMP Parallelization Mode Status
    std::string tryMeUtilityParallelModeStatus();

}