#pragma once

#include "AQLString.h"
#include "AQLDate.h"
#include "AQLCoreTemplateType.h"
#include "AQLCoreAppError.h"
#include "LabelValueBlock.h"
#include "LabelValueBlockValidation.h"
#include "TypeName.h"
#include "ContainerUtilities.h"

#include <map>
#include <vector>
#include <fstream>

namespace etrading
{
    //
    // CLASS
    //    CreateDataFile
    //
    // PURPOSE
    //    Dump C++ objects in CSV format for use in test recording and debugging.
    //
    // SYNPOSIS
    //
    //    //Example
    //    //-------
    //
    //    #include "CreateDataFile.h"
    //    using etrading::CreateDataFile;
    //    // ..
    //    if (CreateDataFile::recordEnabled()) {
    //      CreateDataFile p("C:\\temp\\mycapture");
    //      AQLString myval; // ...
    //      AQLStringVector myvec; // ...
    //      AQLStringMatrix mymat; // ...
    //      double myval; // any streamable type
    //      bool myflag;
    //      std::vector<double> mynums; // ...
    //      std::vector<AQLDate> mydates; // ...
    //      p.write(myval);
    //      p.write(myvec);
    //      p.write(mymat);
    //      p.write(myval);
    //      p.write(myflag);
    //      p.write(mynums);
    //      p.write(mydates);
    //    }
    //


    /*
    	this is a namespace that holds an implementation for the write in CreateDataFile.
    	The reason for its existence is to be able to write containers to a file
    	(because the arguments to the tryXXXX methods can be containers)
    	Because C++ does not yet allow default template arguments on template member functions
    	it is not possible to bring this inside the CreateDataFile class without redesigning the class to take template
    	arguments (which it should not and would alter the interface for every client)
    */
    namespace implementation
    {
        
        template<typename T, bool = false>
        struct WriteForCreateDataFile
        {
            static void write( const AQLString& name, const T& value, std::ofstream& file )
            {
                file << name << ',' << value << "\n\n";
            };
        };

        template<>
        struct WriteForCreateDataFile<const char*, false>
        {
            static void write( const AQLString& name, const char* value, std::ofstream& file )
            {
                file << name << ',' << value << "\n\n";
            };
        };

        template<typename T>
        struct WriteForCreateDataFile<T, true>
        {
            static void write( const AQLString& name, const T& value, std::ofstream& file )
            {
                file << name << ',' << etrading::containerAsString( value, ":" ) << "\n\n";
            };
        };

        template<>
        struct WriteForCreateDataFile<AQLString, false>
        {
            static void write( const AQLString& comment, std::ofstream& file )
            {
                file << comment << "\n\n";
            };
        };

    };


    class CreateDataFile
    {
    public:
        
        // generic exception class
        // -----------------------
        class Exception : public AQLCoreAppError
        {
        public:
            Exception( const AQLString& fileName );
            /* virtual */ const char* what() const;
        private:
            AQLString msg;
        };

        // file output exception class
        // ---------------------------
        class OutputError : public Exception
        {
        public:
            OutputError( const AQLString& fileName );
        private:
            static AQLString makeMessage( const AQLString& fileName );
        };

        
        /*! @brief Main Constructor
		@param[in]  basename	    The filename
		@param[in]  ext		        The file extension, defaults to ".csv"
		@param[in]  useAppendMode	Set to TRUE to append results to the same file or FALSE to clear file contents and overwrite.
	    */
        CreateDataFile( const AQLString& basename, const AQLString& ext = ".csv", const bool useAppendMode = false );

        // Destructor
        // Compiler-generated destructor is fine

        // return filename / full path name
        AQLString filename() const { return filename_; }
        AQLString fullpath() const { return fullpath_; }

        //
        // explicit printing support for double to prevent rounding / truncation errors
        //
        void write( const AQLString& name, const double& value, int precision = 15 );

        // Printing Comments for Logging and Debug Support
        void write( const AQLString& comment );

        //
        // printing AQLString-based types
        //
        void write( const AQLString& name, const AQLString& value );
        void write( const AQLString& name, const AQLStringVector& value );
		void write( const AQLString& name, const std::vector<std::string>& value);
		void write( const AQLString& name, const AQLStringMatrix& value );
        void write( const AQLString& name, const DoubleMatrix& value );
        void write( const AQLString& name, const LabelValueBlock& value );
        void write( const AQLString& name, const AnyTypeMatrix& value );  // Boost::Any
		void write( const AQLString& name, const VariantMatrix& value);	 // In-house AlgoQuantLib Variant
        //
        // printing native types and arrays of native types
        //

        // special handling for booleans: true -> "TRUE", false -> "FALSE"
        void write( const AQLString& name, const bool value );

        // handles any streamable scalar (except AQLString, bool)
        template<typename T>
        void write( const AQLString& name, const T& value );

        // friend WriteForCreateDataFile<T, etrading::is_etrading_container<T>::value>;

        void write( const AQLString& name, const std::vector<double>& value, int precision = 12 ); // set the precission to 12 by default
		void write( const AQLString& name, const std::vector<int>& value );
		void write( const AQLString& name, const std::vector<bool>& value );
		void write( const AQLString& name, const std::vector<AQLDate>& value );
		void write( const AQLString& name, const std::vector< boost::gregorian::date >& value );

        // Return true iff in DEBUG mode
        // Note: not thread-safe.
        static bool recordEnabled();

        // Enable/disable AlgoQuantLib record flag.
        // Note: not thread-safe.
        static bool setRecordFlag( bool enableRecording );

        // Rebase the test results by overwriting them
        static bool rebaseResultsEnabled();

        // Allow test results to be rebased or not
        static void setRebaseResultsFlag( bool rebaseResults );

        // the folder where files given by relative path name are created
        static AQLString outputFolder();

        // Start generating output files with indexed file names,
        // increasing the index each time a file with a fixed basename is created.
        // Note: not thread-safe.
        static int beginTestCount( int index = 0 );

        // Revert to un-indexed file names
        // Note: not thread-safe.
        static int endTestCount();

        // return indexed file name
        static AQLString makeFilename( const AQLString& basename, int index, const AQLString& fileExtension = ".csv" );

        // return un-indexed file name
        static AQLString makeFilename( const AQLString& basename, const AQLString& fileExtension = ".csv" )
        {
            return makeFilename( basename, -1, fileExtension );
        }

        // return indexed file name with a suffix
        static AQLString makeFilename( const AQLString& basename, const AQLString& suffix, int index, const AQLString& fileExtension = ".csv" );

        // return un-indexed file name with a suffix
        static AQLString makeFilename( const AQLString& basename, const AQLString& suffix, const AQLString& fileExtension = ".csv" )
        {
            return makeFilename( basename, suffix, -1, fileExtension );
        }

        // return indexed file name with a prefix and suffix
        static AQLString makeFilename( const AQLString& basename, const AQLString& prefix, const AQLString& suffix, int index, const AQLString& fileExtension = ".csv" );

        // return un-indexed file name with a prefix and suffix
        static AQLString makeFilename( const AQLString& basename, const AQLString& prefix, const AQLString& suffix, const AQLString& fileExtension = ".csv" )
        {
            return makeFilename( basename, prefix, suffix, -1, fileExtension );
        }

        static AQLString setOutputFolder( const AQLString& p, bool fullPathGiven = true );

        static int setMaxIndex( const int& i );

        // Returns the current time
        // Use as follows: char buff[20]; currentTime(buff);
        static void currentTime(char* buff);
        
    private:

        CreateDataFile( const CreateDataFile& );
        CreateDataFile& operator=( const CreateDataFile& );


        AQLString filename_;
        AQLString fullpath_;
        std::ofstream file_;

        // initialise record flag by reading environment variable
        static bool initRecordEnabled();

        // record flag
        static bool recordEnabled_;

        // rebase results flag
        static bool rebaseResultsEnabled_;

        // initialise output folder by reading environment variable
        static void initOutputFolder();

        // output folder
        static AQLString outputFolder_;

        // initialise max index value by reading environment variable
        static void initMaxIndex();

        // max index value; safety feature;
        static int maxIndex_;

        //
        // support for vectorised filenames
        //

        // maintain current array index for each file given by basename
        static std::map<AQLString, int> usedIndices_;

        // get index for file, and increase it if not negative
        static int useTestIndex( const AQLString& basename );

        // first index to be used; if negative, generate ordinary (un-indexed) filenames
        static int baseIndex_;
};

    // add market key information to filename
    AQLString decorateCurvename( const AQLString& curvename, const AQLString& curveIDPrefix1 = "", const AQLString& marketNamePrefix2 = "" );
	
    // decorate file name with a prefix and suffix
    AQLString decorateFilename( const AQLString& filename, const AQLString& prefix = "" );
    AQLString decorateFilename( const AQLString& filename, const AQLString& prefix, const AQLString& suffix );
}

template<typename T>
inline void etrading::CreateDataFile::write( const AQLString& name, const T& value )
{
    implementation::WriteForCreateDataFile<T, etrading::is_etrading_container<T>::value>::write( name, value, file_ );
};

inline void etrading::CreateDataFile::write( const AQLString& comment )
{
    implementation::WriteForCreateDataFile<AQLString,false>::write( comment, file_ );
};

