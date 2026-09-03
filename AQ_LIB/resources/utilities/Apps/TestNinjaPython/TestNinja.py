#!/usr/bin/python

import time
import argparse
import datetime
import subprocess
import multiprocessing

from ThreadPool import *

class MyCalledProcessError(Exception):
    def __init__( self, returncode, outError ):
        self.output = outError
        self.returncode = returncode


def check_output(cmd):
    """ https://docs.python.org/2/library/subprocess.html#subprocess.Popen
        Implementation subprocess.check_output() for Python 2.6
     """
    cmd_list = cmd
    myProcess = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
 
    output,err = myProcess.communicate()

    if myProcess.returncode > 0:
       # GOOGLE_TEST.exe actually writes the error message to stdout.
       # So store the output and ignore the err
       raise MyCalledProcessError( myProcess.returncode, output )

    return output



class CommandLineRunner:
    def __init__(self, nThreads, workspace, filter):
        self.nThreads = nThreads
        self.workspace = workspace
        self.filter = filter
        self.testFailures = []
        self.testWarnings = []

        self.google_test = "GOOGLE_TEST.exe"
        self.googleTestPath = workspace + '/' + self.google_test
        print "Using", nThreads, "threads and workspace: ", workspace

        if filter is not None:
            print "Using test filter: ", filter

        
    def commandLine(self, command, param="" ):
        arg = command + param
        print self.googleTestPath + ' ' + arg
        
        additionalArgs = arg.split()
        
        allArgs = [ self.googleTestPath ]
        allArgs += additionalArgs
        

        result = ""
        try:
            result = check_output( allArgs )
        except MyCalledProcessError as error:   
            print "error code", error.returncode, error.output
            self.testFailures.append( param )
        except OSError:
            print "Error: Unable to invoke", self.google_test, "from path:", self.googleTestPath
            sys.exit(1)

        if "#Warning" in result:
            print result
            self.testWarnings.append( param )
        return result

    
    def getAllTests(self, filter):
        listCommand = "--gtest_list_tests"

        if filter is not None:
            listCommand += " --gtest_filter="+filter
        
        result = self.commandLine( listCommand )
        output = result.split('\n')

        # Parse the output of google_test: The format is:
        # TestFixture1.
        #   TestName1
        #   TestName2
        # TestFixture2.
        #   ...

        allTests = []
        testFixture=""
        for line in output:
            trimmed = line.rstrip()
            if len(trimmed) == 0:
                pass
            elif trimmed.endswith('.'):
                testFixture = trimmed.strip()
            elif trimmed.startswith('  '):  # test name begins with 2 spaces
                testName = testFixture + trimmed.strip()
                allTests.append(testName)

        nTests = len( allTests )
        if nTests == 0:
            print "Error: No tests found! Please check workspace setting."
            sys.exit(1)
        else:
            print "Found", nTests, "tests.\n"
        return allTests

    
    def runTest(self, testName):
        filterCommand = "--gtest_filter="
        result = self.commandLine( filterCommand, testName )

        
    def run(self):
        # Find all of the available tests
        allTests = self.getAllTests(filter)

        startTime = time.time()
        # Now run all tests in parallel
        self.testFailures = []
        self.testWarnings = []
        # Instantiate a thread pool with 5 worker threads
        pool = ThreadPool( nThreads )

        # Add the jobs in bulk to the thread pool. Alternatively you could use
        # `pool.add_task` to add single jobs. The code will block here, which
        # makes it possible to cancel the thread pool with an exception when
        # the currently running batch of workers is finished.
        pool.map( self.runTest, allTests )
        pool.wait_completion()

        elapsedTime = time.time() - startTime
        print "\nTime taken: ", round( elapsedTime, 1), "sec"

        nWarnings = len( self.testWarnings )
        print "Number of test warnings:", nWarnings
        if nWarnings > 0:
            print "\nTests which had warning:"
            for warning in self.testWarnings:
                print "  ", warning
        
        nFailures = len( self.testFailures )
        print "Number of test failures:", nFailures
        if nFailures > 0:
            print "\nTests which failed:"
            for failure in self.testFailures:
                print "  ", failure
        return nFailures
        
if __name__ == "__main__":
    print "TestNinja is starting in command line mode!"

    # Parse the command line. The Windows TestNinja supports the following:
    #   --arch  win32 x64        # Not applicable here
    #   --build debug / release  # Not applicable here
    #   --vstudio 2010 2015      # Not applicable here
    #   --threads 8              # Default 8 ( or however many cores the machine has)
    #   --workspace              # Location of the GOOGLE_TEST.exe
    #   --filter                 # Specify a test filter to run a subset of tests

    parser = argparse.ArgumentParser(description='TestNinja Parallel GOOGLE_TEST runner')
    parser.add_argument('--threads',
                        help='specify the number of CPU threads to use.',
                        type=int)
    parser.add_argument('--workspace',
                        help='specify the workspace to use, containing GOOGLE_TEST.exe.')
    parser.add_argument('--filter',
                        help='specify a test filter (prefix with ^ to exclude).')
    
    args = parser.parse_args()

    nCores = multiprocessing.cpu_count();
    nThreads = args.threads
    if nThreads == None:
        nThreads = nCores

    workspace = args.workspace
    if workspace == None:
        workspace = "."

    filter = args.filter
    
    # replace filter prefix ^ with -
    if filter is not None:
        if filter.startswith('^'):
            filter = '-' + args.filter[1:]

    # Invoke the test runner
    testRunner = CommandLineRunner( nThreads, workspace, filter )
    nFailures = testRunner.run()

    now = datetime.datetime.now()
    print "\nTestNinja completed at "+ now.strftime("%Y-%m-%d %H:%M:%S")

    # Report whether tests have failed back to the caller.
    if nFailures > 0:
        sys.exit( 1 )

        
