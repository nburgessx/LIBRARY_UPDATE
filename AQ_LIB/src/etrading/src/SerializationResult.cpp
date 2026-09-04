//
// @Description: This is a test program



#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "SerializationResult.h"
#include "ETradingException.h"
#include "SerializationResultWrapper.h"
#include "SerializeContainedData.h"

namespace etrading
{

    SerializationResult::SerializationResult( serialize::SerializationMethodEnum method,
                                               serialize::SerializationTargetEnum target,
                                               const SerializationResultWrapper& result,
                                               const std::string& info )
        : HasConstInstance<serialize::SerializationMethodEnum>( method ),
          HasConstInstance<serialize::SerializationTargetEnum>( target ),
          result_( std::make_shared<SerializationResultWrapper>(result) ),
          info_( info )
    {
        // TODO: delegate out to different Serialization method handler classes when we get more methods
        if( method == serialize::JSON )
        {
			if(result_ == nullptr)
			{
                throw ETradingException( "SerializationResultWrapper was a nullptr" );
			}

            if( result_->jsonDocument == nullptr )
            {
                throw ETradingException( "No object of type rapidjson::Document available to convert to JSON" );
            }

            std::string jsonString =  createStringFromJSON( *( result_->jsonDocument.get() ) );
            if( target == serialize::FILE )
            {
                if( info_ == "" )
                {
                    throw ETradingException( "Requested JSON serialiazation to file but no valid file name was set" );
                }
                std::ofstream outputFileStream( info_ );
                outputFileStream << jsonString;
                if ( !outputFileStream.good() )
                {
                    throw ETradingException( ( boost::format( "Cannot write the JSON string to the file %s" ) % info_.c_str() ).str() );
                }
                outputFileStream.close();  // is this really necessary?
            }
            if( target == serialize::STRING )
            {
                info_ = jsonString;
            }
        }
    };

    SerializationResult::operator std::string() const
    {
        return info_;
    };



}



/*
FileStream f(stdout);
PrettyWriter<FileStream> writer(f);
document.Accept(writer);
*/
/*
FILE* fp = fopen("output.json", "wb"); // non-Windows use "w"
char writeBuffer[65536];
FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
Writer<FileWriteStream> writer(os);
d.Accept(writer);
fclose(fp);
*/
/*
StringBuffer buffer;
Writer<StringBuffer> writer(buffer);
d.Accept(writer);

// Output {"project":"rapidjson","stars":11}
std::cout << buffer.GetString() << std::endl;
std::string s(buffer.GetString(), buffer.GetSize());

std::string json (buf.GetString(), buf.GetSize());

std::ofstream of ("/tmp/example.json");
of << json;
if (!of.good()) throw std::runtime_error ("Can't write the JSON string to the file!");

*/

/*
StringBuffer sb;
PrettyWriter<StringBuffer> writer(sb);
document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
puts(sb.GetString());
*/
