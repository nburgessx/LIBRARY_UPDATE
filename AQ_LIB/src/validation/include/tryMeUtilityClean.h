// tryMeUtilityClean.h

/*
 * @brief			Functions to clean data to remove rows or columns with blank or error values
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "DataUtilities.h"

namespace validation
{
    using etrading::VariantMatrix;
    
    /* @brief			Function to clean a matrix of blank or error values
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		cleanByRow                  Boolean: TRUE = Clean by Row (Default), FALSE = Clean by Column
    *  @param [in]		checkRowColNumber           Int: Row / Column number to check, leave blank to check entire inputMatrix
    *  @param [in]		removeBlanks                Boolean: TRUE = Remove blank data (Default), FALSE = Do not remove blank data
    *  @param [in]      removeErrors                Flag to remove AlgoQuantLib errors, when TRUE will remove strings starting with AlgoQuantLib error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix tryMeUtilityClean( const VariantMatrix &      inputMatrix,
                                     const bool &               cleanByRow = true,
                                     const int &                checkRowColNumber = 0,
                                     const bool &               removeBlanks = true,
                                     const bool &               removeErrors = true );

    /* @brief			Function to clean a variant Matrix of blank or error values
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		cleanByRow                  Boolean: TRUE = Clean by Row (Default), FALSE = Clean by Column
    *  @param [in]		checkRowColNumber           Int: Row / Column number to check, leave blank to check entire inputMatrix
    *  @param [in]		removeBlanks                Boolean: TRUE = Remove blank data (Default), FALSE = Do not remove blank data
    *  @param [in]      removeErrors                Flag to remove AlgoQuantLib errors, when TRUE will remove strings starting with AlgoQuantLib error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix cleanVariantMatrix( const VariantMatrix &      inputMatrix,
                                      const bool &               cleanByRow = true,
                                      const int &                checkRowColNumber = 0,
                                      const bool &               removeBlanks = true,
                                      const bool &               removeErrors = true );

    /* @brief			Function to trim a variant Matrix and remove a row or column if empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		trimByRow                  Boolean: TRUE = trim by Row (Default), FALSE = trim by Column
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix trimVariantMatrix( const VariantMatrix &      inputMatrix,
                                     const bool &               trimByRow = true );

	// Function to trim a LA String Vector and remove blanks
	AQLStringVector trimAQLStringVector( const AQLStringVector & inputVector );

	// Function to trim a Standard String Vector and remove blanks
	StandardStringVector trimStandardStringVector( const StandardStringVector & inputVector );
	
    // Function to trim a LA String Matrix and remove blanks
	AQLStringMatrix trimAQLStringMatrix( const AQLStringMatrix & inputMatrix );
	
	// Function to trim a Standard String Matrix and remove blanks
	StandardStringMatrix trimStandardStringMatrix( const StandardStringMatrix & inputMatrix );
}