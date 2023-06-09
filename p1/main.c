//header files
#include "stdio.h"
#include "metadataops.h"
#include "configops.h"
#include "runSim.h"

extern const int STR_EQ;

void showProgramFormat();

int main( int argc, char **argv )
{
   // Initialize program
     // initialize variables
	Boolean programRunFlag = False;

	Boolean configDisplayFlag = False;

	Boolean configUploadFlag = False;

	Boolean mdDisplayFlag = False;

	Boolean runSimFlag = False;

	Boolean inforFlag = False;

	int argIndex = 1;

	int lastFourLetters = 4;

	int fileStrLen, fileStrSubLoc;

	char fileName[ STD_STR_LEN ];

	char errorMessage[ MAX_STR_LEN ];

	ConfigDataType *configDataPtr = NULL;

	OpCodeType *metaDataPtr = NULL;

        // show title
          // function: printf
	printf("\nSimulator Program\n");
	printf("=================\n\n");

        // check for only one arg
	if (argc < 2 )
	{
                // show error message, end program
                   // function: showProgramFormat
         
		showProgramFormat();

                // unset program run flag
		programRunFlag = False;

                // set info flag
		inforFlag = true;
	}

        // initialize file name to empty
	fileName[ 0 ] = NULL_CHAR;

        // loop across args (starting at 1) and program runflag
	while( programRunFlag == False && argIndex < argc )
	{
             // find length to vertify file name
                // function: getStringLength, findSubString
		fileStrLen = getStringLength( argv[ argIndex ] );
		fileStrSubLoc = findSubString( argv[ argIndex ], ".cnf" );

             // check for -dc (display config flag)
                // function: compareString
		if( compareString( argv[ argIndex ], "-dc" ) == STR_EQ )
		{
                        // set config upload flag
			configUploadFlag = true;

                        // set config display flag
			configDisplayFlag = true;
		}

                // otherwise, check for -dm (display metadata flag)
                   // function: compareString
		else if( compareString( argv[ argIndex ], "-dm") == STR_EQ )
		{
                        // set config upload flag
			configUploadFlag = true;

                        // set meta data display flag
			mdDisplayFlag = true;
		}

                // otherwise, check for -rs(run simulator)
                   // function:compareString
		else if( compareString( argv[ argIndex ], "-rs" ) == STR_EQ )
		{
                        // set config upload flag
			configUploadFlag = true;
  
                        // set run simulator flag
			runSimFlag = true;
		}

                // otherwise, check for file name, ending in .cnf
                // must be last flag
		else if( fileStrSubLoc != SUBSTRING_NOT_FOUND && fileStrSubLoc == fileStrLen - lastFourLetters )
		{
                 // set file name to variable
                    // function: copyString
			copyString( fileName, argv[ argIndex ] );

                        // set program run flag
			programRunFlag = true;
		}
               // update arg index
		argIndex++;

	}
        // end loop
       
        // check for command line failure
	if( programRunFlag == False && inforFlag == False )
	{
                // show error message, end program
                  // function printf
		printf("Incorrect argument line format, program aborted\n\n" );

                // function: showProgram format
		showProgramFormat();
	}

        // check for program run and config upload flag
	if( programRunFlag == true && configUploadFlag == true )
	{
                // upload config file, check for success
                  // function: getStringLength, getConfigData
		if( getStringLength( fileName ) > 0 && getConfigData( fileName, &configDataPtr, errorMessage) == true )
		{
                        // check config display flag
			if( configDisplayFlag == true )
			{
                                // display config data
                                   // function: displayConfigData 
				displayConfigData( configDataPtr );
			}
		}

                // otherwise, assume config file upload failure
		else
		{
                        // show error message, end program
                          // function: printf
			printf("\nConfig Upload Error: %s, program aborted\n\n", errorMessage );

                        // unset program run flag
			programRunFlag = False;
		}
	}

        // check for program run flag
	if( programRunFlag == true && ( mdDisplayFlag == true || runSimFlag == true ) )
	{
                // upload meta data file, check for success
                  // function: getMetaData
		if( getMetaData( configDataPtr->metaDataFileName, &metaDataPtr, errorMessage ) == true )
		{
                        // check meta data display flag
			if( mdDisplayFlag == true )
			{
                                // display meta data
                                  // function: displayMetaData
				displayMetaData( metaDataPtr );
			}

                        // check run simulator flag
			if(runSimFlag == true )
			{
                                // run simulator
                                  // function: runSim
				runSim( configDataPtr,metaDataPtr );
			}
		}

                //otherwise, assume meta data upload file
		else
		{
                        // show errormessage
                          // function: printf
			printf("\nMetadata Upload Error: %s, program aborted\n", errorMessage );
		}
	}
   
        // clean up config data as needed
          // function: clearConfigDataList
	configDataPtr = clearConfigData( configDataPtr );

        // clean up meta data as needed
          // function: clearMetaDataList
	metaDataPtr = clearMetaDataList( metaDataPtr );

        // show program end
          // function: printf
	printf( "\nSimulator Program End.\n\n" );

        // return success
	return 0;
}

/*
name: showProgramFormat
process: displays command line argument requirements for this program
method input/parameters: none
method output/parameters: none
method output/returned: none
device input/keyboard: none
device output/monitor: instructions provided as specified
dependencies: none
*/
	void showProgramFormat()
	{
                // print out command line argument instructions
                  // function: printf
		printf("program Format:\n");
		printf("     sim_0x [-dc] [-dm] [-rs] <config file name>\n");
		printf("     -dc [optional] displays configuration data\n");
		printf("     -dm [optional] display meta data\n");
		printf("     -rs [optional] runs simulator\n");
		printf("     config file name is required\n");
	}

	





