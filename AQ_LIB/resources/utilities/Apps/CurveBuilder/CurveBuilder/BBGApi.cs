using System;
using System.Collections.Generic;

using Element = Bloomberglp.Blpapi.Element;
using Event = Bloomberglp.Blpapi.Event;
using InvalidRequestException = Bloomberglp.Blpapi.InvalidRequestException;
using Message = Bloomberglp.Blpapi.Message;
using Name = Bloomberglp.Blpapi.Name;
using Request = Bloomberglp.Blpapi.Request;
using Service = Bloomberglp.Blpapi.Service;
using Session = Bloomberglp.Blpapi.Session;
using SessionOptions = Bloomberglp.Blpapi.SessionOptions;

namespace CurveBuilder
{
    class BBGResponse
    {
        public BBGResponse( string inputTickerName )
        {
            tickerName = inputTickerName;
            fieldData = new SortedDictionary<string, string>();
        }

        public string tickerName { get; set; }

        // mapping from field name to value  e.g LAST_PRICE  0.02
        public SortedDictionary<string, string> fieldData { get; set; }        
    };


    class BBGApi
    {
        private List<string> mSecurities = new List<string>();
        private List<string> mFields = new List<string>();

        private static readonly String APIREFDATA_SVC = "//blp/refdata";
        private static readonly Name SECURITY_DATA = Name.GetName("securityData");
        private static readonly Name SECURITY = Name.GetName("security");
        private static readonly Name FIELD_DATA = Name.GetName("fieldData");
        private static readonly Name RESPONSE_ERROR = Name.GetName("responseError");

        private static readonly Name SECURITY_ERROR = Name.GetName("securityError");
        private static readonly Name FIELD_EXCEPTIONS = Name.GetName("fieldExceptions");
        private static readonly Name FIELD_ID = Name.GetName("fieldId");
        private static readonly Name ERROR_INFO = Name.GetName("errorInfo");
        private static readonly Name CATEGORY = Name.GetName("category");
        private static readonly Name MESSAGE = Name.GetName("message");

        public void sendRefDataRequest(Session session)
        {
            Console.WriteLine("Hello Bloomberg!");

            if (!session.OpenService(APIREFDATA_SVC))
            {
                Console.Error.WriteLine("Failed to open service: " + APIREFDATA_SVC);
                return;
            }
            Service refDataService = session.GetService(APIREFDATA_SVC);

            Request request = refDataService.CreateRequest("ReferenceDataRequest");

            Element securities = request.GetElement("securities");

            for (int i = 0; i < mSecurities.Count; ++i)
            {
                securities.AppendValue((string)mSecurities[i]);
            }

            Element fields = request.GetElement("fields");

            for (int i = 0; i < mFields.Count; ++i)
            {
                fields.AppendValue((string)mFields[i]);
            }

            //Console.WriteLine("Sending Request: " + request);
            session.SendRequest(request, null);
        }

        public SortedDictionary<string, BBGResponse> processResponseEvent(Event eventObj)
        {
            SortedDictionary<string, BBGResponse> results = new SortedDictionary<string, BBGResponse>();

            foreach (Message msg in eventObj.GetMessages())
            {
                if (msg.HasElement(RESPONSE_ERROR))
                {
                    Console.WriteLine("REQUEST FAILED: " + msg.GetElement(RESPONSE_ERROR));
                    continue;
                }

                Element securities = msg.GetElement(SECURITY_DATA);
                int numSecurities = securities.NumValues;
                Console.WriteLine("Processing " + numSecurities + " securities:");

                for (int i = 0; i < numSecurities; ++i)
                {
                    Element security = securities.GetValueAsElement(i);
                    string ticker = security.GetElementAsString(SECURITY);
                    Console.WriteLine("   " + ticker);

                    BBGResponse bbgResponse = new BBGResponse(ticker);

                    if (security.HasElement("securityError"))
                    {
                        System.Console.WriteLine("\tSECURITY FAILED: " + security.GetElement(SECURITY_ERROR));
                        continue;
                    }

                    Element fields = security.GetElement(FIELD_DATA);

                    if (fields.NumElements > 0)
                    {
                        //Console.WriteLine("FIELD\t\tVALUE");
                        //Console.WriteLine("-----\t\t-----");
                        int numElements = fields.NumElements;

                        for (int j = 0; j < numElements; ++j)
                        {
                            Element field = fields.GetElement(j);
                            string fieldName = field.Name.ToString();
                            string fieldValue = field.GetValueAsString();
                            bbgResponse.fieldData[fieldName] = fieldValue;

                            //Console.WriteLine(field.Name + "\t\t" + field.GetValueAsString());
                        }
                        results[ticker] = bbgResponse;
                    }
                    else
                    {
                        Console.WriteLine("No fields");
                    }

                }
            }
            return results;
        }

        public SortedDictionary<string, BBGResponse> eventLoop(Session session)
        {
            bool done = false;

            SortedDictionary<string, BBGResponse> allResults = new SortedDictionary<string, BBGResponse>();
            while (!done)
            {
                Event eventObj = session.NextEvent();
                if (eventObj.Type == Event.EventType.PARTIAL_RESPONSE)
                {
                    Console.WriteLine("Processing Partial Response");
                    SortedDictionary<string, BBGResponse> results = processResponseEvent(eventObj);
                    foreach (var keyValuePair in results )
                    {
                        //Console.Out.WriteLine("Adding: " + keyValuePair.Key + " : " + keyValuePair.Value);
                        allResults.Add(keyValuePair.Key, keyValuePair.Value);
                    }
                }
                else if (eventObj.Type == Event.EventType.RESPONSE)
                {
                    Console.WriteLine("Processing Response");
                    SortedDictionary<string, BBGResponse> results = processResponseEvent(eventObj);
                    foreach (var keyValuePair in results)
                    {
                        //Console.Out.WriteLine("Adding: " + keyValuePair.Key + " : " + keyValuePair.Value);
                        allResults.Add(keyValuePair.Key, keyValuePair.Value);
                    }
                    done = true;
                }
                else
                {
                    foreach (Message msg in eventObj.GetMessages())
                    {
                        //System.Console.WriteLine(msg.AsElement);
                        if (eventObj.Type == Event.EventType.SESSION_STATUS)
                        {
                            if (msg.MessageType.Equals("SessionTerminated"))
                            {
                                done = true;
                            }
                        }
                    }
                }
            }
            return allResults;
        }

        public SortedDictionary<string, BBGResponse> sendRequest(List<string> securities, List<string> fields)
        {
            mSecurities = securities;
            mFields = fields;

            string serverHost = "localhost";
            int serverPort = 8194;

            SessionOptions sessionOptions = new SessionOptions();
            sessionOptions.ServerHost = serverHost;
            sessionOptions.ServerPort = serverPort;

            //System.Console.WriteLine("Connecting to " + serverHost + ":" + serverPort);

            Session session = new Session(sessionOptions);
            bool sessionStarted = session.Start();

            if (!sessionStarted)
            {
                System.Console.Error.WriteLine("Failed to start session.");
                return new SortedDictionary<string, BBGResponse>(); // empty results
            }

            try
            {
                sendRefDataRequest(session);
            }

            catch (InvalidRequestException e)
            {
                Console.WriteLine(e.ToString());
            }

            // wait for events from session.
            SortedDictionary<string, BBGResponse> allResults = eventLoop(session);
            session.Stop();

            return allResults;
        }
    }
}
