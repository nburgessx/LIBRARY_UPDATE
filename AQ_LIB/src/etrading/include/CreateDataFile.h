#pragma once

#include "LAString.h"
#include "LADate.h"
#include "LACoreTemplateType.h"
#include "LACoreAppError.h"
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
    //      LAString myval; // ...
    //      LAStringVector myvec; // ...
    //      LAStringMatrix mymat; // ...
    //      double myval; // any streamable type
    //      bool myflag;
    //      std::vector<double> mynums; // ...
    //      std::vector<LADate> mydates; // ...
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
            static void write( const LAString& name, const T& value, std::ofstream& file )
            {
                file << name << ',' << value << "\n\n";
            };
        };

        template<>
        struct WriteForCreateDataFile<const char*, false>
        {
            static void write( const LAString& name, const char* value, std::ofstream& file )
            {
                file << name << ',' << value << "\n\n";
            };
        };

        template<typename T>
        struct WriteForCreateDataFile<T, true>
        {
            static void write( const LAString& name, const T& value, std::ofstream& file )
            {
                file << name << ',' << etrading::containerAsString( value, ":" ) << "\n\n";
            };
        };

        template<>
        struct WriteForCreateDataFile<LAString, false>
        {
            static void write( const LAString& comment, std::ofstream& file )
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
        class Exception : public LACoreAppError
        {
        public:
            Exception( const LAString& fileName );
            /* virtual */ const char* what() const;
        private:
            LAString msg;
        };

        // file output exception class
        // ---------------------------
        class OutputError : public Exception
        {
        public:
            OutputError( const LAString& fileName );
        private:
            static LAString makeMessage( const LAString& fileName );
        };

        
        /*! @brief Main Constructor
		@param[in]  basename	    The filename
		@param[in]  ext		        The file extension, defaults to ".csv"
		@param[in]  useAppendMode	Set to TRUE to append results to the same file or FALSE to clear file contents and overwrite.
	    */
        CreateDataFile( const LAString& basename, const LAString& ext = ".csv", const bool useAppendMode = false );

        // Destructor
        // Compiler-generated destructor is fine

        // return filename / full path name
        LAString filename() const { return filename_; }
        LAString fullpath() const { return fullpath_; }

        //
        // explicit printing support for double to prevent rounding / truncation errors
        //
        void write( const LAString& name, const double& value, int precision = 15 );

        // Printing Comments for Logging and Debug Support
        void write( const LAString& comment );

        //
        // printing LAString-based types
        //
        void write( const LAString& name, const LAString& value );
        void write( const LAString& name, const LAStringVector& value );
		void write( const LAString& name, const std::vector<std::string>& value);
		void write( const LAString& name, const LAStringMatrix& value );
        void write( const LAString& name, const DoubleMatrix& value );
        void write( const LAString& name, const LabelValueBlock& value );
        void write( const LAString& name, const AnyTypeMatrix& value );  // Boost::Any
		void write( const LAString& name, const VariantMatrix& value);	 // In-house AlgoQuantLib Variant
        //
        // printing native types and arrays of native types
        //

        // special handling for booleans: true -> "TRUE", false -> "FALSE"
        void write( const LAString& name, const bool value );

        // handles any streamable scalar (except LAString, bool)
        template<typename T>
        void write( const LAString& name, const T& value );

        // friend WriteForCreateDataFile<T, etrading::is_etrading_container<T>::value>;

        void write( const LAString& name, const std::vector<double>& value, int precision = 12 ); // set the precission to 12 by default
		void write( const LAString& name, const std::vector<int>& value );
		void write( const LAString& name, const std::vector<bool>& value );
		void write( const LAString& name, const std::vector<LADate>& value );
		void write( const LAString& name, const std::vector< boost::gregorian::date >& value );

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
        static LAString outputFolder();

        // Start generating output files with indexed file names,
        // increasing the index each time a file with a fixed basename is created.
        // Note: not thread-safe.
        static int beginTestCount( int index = 0 );

        // Revert to un-indexed file names
        // Note: not thread-safe.
        static int endTestCount();

        // return indexed file name
        static LAString makeFilename( const LAString& basename, int index, const LAString& fileExtension = ".csv" );

        // return un-indexed file name
        static LAString makeFilename( const LAString& basename, const LAString& fileExtension = ".csv" )
        {
            return makeFilename( basename, -1, fileExtension );
        }

        // return indexed file name with a suffix
        static LAString makeFilename( const LAString& basename, const LAString& suffix, int index, const LAString& fileExtension = ".csv" );

        // return un-indexed file name with a suffix
        static LAString makeFilename( const LAString& basename, const LAString& suffix, const LAString& fileExtension = ".csv" )
        {
            return makeFilename( basename, suffix, -1, fileExtension );
        }

        // return indexed file name with a prefix and suffix
        static LAString makeFilename( const LAString& basename, const LAString& prefix, const LAString& suffix, int index, const LAString& fileExtension = ".csv" );

        // return un-indexed file name with a prefix and suffix
        static LAString makeFilename( const LAString& basename, const LAString& prefix, const LAString& suffix, const LAString& fileExtension = ".csv" )
        {
            return makeFilename( basename, prefix, suffix, -1, fileExtension );
        }

        static LAString setOutputFolder( const LAString& p, bool fullPathGiven = true );

        static int setMaxIndex( const int& i );

        // Returns the current time
        // Use as follows: char buff[20]; currentTime(buff);
        static void currentTime(char* buff);
        
    private:

        CreateDataFile( const CreateDataFile& );
        CreateDataFile& operator=( const CreateDataFile& );


        LAString filename_;
        LAString fullpath_;
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
        static LAString outputFolder_;

        // initialise max index value by reading environment variable
        static void initMaxIndex();

        // max index value; safety feature;
        static int maxIndex_;

        //
        // support for vectorised filenames
        //

        // maintain current array index for each file given by basename
        static std::map<LAString, int> usedIndices_;

        // get index for file, and increase it if not negative
        static int useTestIndex( const LAString& basename );

        // first index to be used; if negative, generate ordinary (un-indexed) filenames
        static int baseIndex_;
};

    // add market key information to filename
    LAString decorateCurvename( const LAString& curvename, const LAString& curveIDPrefix1 = "", const LAString& marketNamePrefix2 = "" );
	
    // decorate file name with a prefix and suffix
    LAString decorateFilename( const LAString& filename, const LAString& prefix = "" );
    LAString decorateFilename( const LAString& filename, const LAString& prefix, const LAString& suffix );
}

template<typename T>
inline void etrading::CreateDataFile::write( const LAString& name, const T& value )
{
    implementation::WriteForCreateDataFile<T, etrading::is_etrading_container<T>::value>::write( name, value, file_ );
};

inline void etrading::CreateDataFile::write( const LAString& comment )
{
    implementation::WriteForCreateDataFile<LAString,false>::write( comment, file_ );
};

