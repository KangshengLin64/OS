// header files
#include "StringUtils.h"


const char NULL_CHAR='\0';
const char SPACE=' ';
const char COLON=':';
const char COMMA=',';
const char SEMICOLON=';';
const char PERIOD='.';
const int STD_STR_LEN=64;
const int MAX_STR_LEN=128;
const int STR_EQ=0;
const int SUBSTRING_NOT_FOUND=-1001;
const Boolean IGNORE_LEADING_WS=true;
const char ACCEPT_LEADONG_WS=False;



int compareString( const char *oneStr, const char *otherStr )
  {
   // initialize function/variables
   int diff, index = 0;

   // loop to end of shortest string
   //  with overrun protection
   while( oneStr[ index ] != NULL_CHAR
             && otherStr[ index ] != NULL_CHAR
                && index < MAX_STR_LEN)
      {
       // get difference in characters
       diff= oneStr[ index ] - otherStr[ index ];

       // check for  difference between characters
       if( diff != 0 )
          {
           // return  difference
           return diff;
          }

       // increment index
       index++;
      }
      //end loop

      //return difference in lengths, if any
      // function: getStringLength
   return getStringLength( oneStr ) - getStringLength( otherStr );
  }

/*
 Name: concatenateString
 Process: appends one string onto another
 Function Iutput/Parameters: c-style source strings (char *)
 Function Output/Parameters: c-style destination string (char *)
 Function Output/Returned: none
 Device Input/Keyboard: none
 Device Output/Monitor: none
 Dependencies: getStringLength
 */
void concatenateString(  char *destStr, const char *sourceStr )
  {
   // initialize function/variables

      // set destination index
         // function: getStringLength
      int destIndex = getStringLength( destStr );
      
      // set source string index
         // function: getStringLength
      int sourceStrLen = getStringLength( sourceStr);
      
      //create temporary string pointer
      char *tempStr;

      // set other variables
      int sourceIndex = 0;
      
      //copy source string in case of aliasing
        //function: malloc copyString
      tempStr = (char *)malloc( sizeof( sourceStrLen + 1));
      copyString( tempStr, sourceStr);

   // loop to end of source string
   while( tempStr[ sourceIndex ] != NULL_CHAR && destIndex < MAX_STR_LEN )
      {
       // assign characters to end of destination string
       destStr[ destIndex ] = tempStr[ sourceIndex ];

       // update indices
        destIndex++; sourceIndex++;
        
       // set temporary end of destination string
       destStr[ destIndex ] = NULL_CHAR;
      }
      //end loop
      
      //release memort used for temp string
        //function: free
      free( tempStr);
  }


/*
 Name: copyString
 Process: copies one string into another,
          overwriting data in the destination string
 Function Iutput/Parameters: c-style source string (char *)
 Function Output/Parameters: c-style destination sring (char *)
 Function Output/Returned: none
 Device Input/Keyboard: none
 Device Output/Monitor: none
 Dependencies: getStringLength
 */
void copyString( char *destStr, const char *sourceStr )
  {
   // initialize function/variables
   int index = 0;
      
  //check for source/destination not the same(aliasing)
      if (destStr != sourceStr)
      {
          //loop to end of source string
          while( sourceStr[ index ] != NULL_CHAR && index < MAX_STR_LEN   )
             {
              // assign character to end of destination string
              destStr[ index ] = sourceStr[ index ];

              // update index
              index++;

              // set temporart end of destination string
              destStr[ index ] = NULL_CHAR;
             }
          // end loop
      }
   
  }




/*
 Name: getStringLength
 Process: find the length of a string
          by counting characters up to the NULL_CHAR character
 Function Iutput/Parameters: c-style string (char *)
 Function Output/Parameters: none
 Function Output/Returned: length of string
 Device Input/Keyboard: none
 Device Output/Monitor: none
 Dependencies: none
 */
int getStringLength( const char *teststr )
  {
   // initialize function/variables
   int index = 0;

   // loop to end of string, protect from overflow
   while( index < STD_STR_LEN && teststr[ index ] != NULL_CHAR )
      {
       // update index
       index++;
      }
      // end loop

   // return index/length
   return index;
  }

int findSubString(const char *testStr, const char *searchSubStr)
{
  int testStrLen=getStringLength(testStr);

int masterIndex=0;

int searchIndex, internalIndex;


while(masterIndex<testStrLen)
{
 internalIndex=masterIndex;
 searchIndex=0;

while(internalIndex<=testStrLen&&testStr[internalIndex]==searchSubStr[searchIndex])
{
 internalIndex++;searchIndex++;
 if(searchSubStr[searchIndex]==NULL_CHAR)
  {
   return masterIndex;
  }

}
masterIndex++;

}

return SUBSTRING_NOT_FOUND;

}


bool getStringConstrained(FILE *inStream,bool clearLeadingNonPrintable,bool clearLeadingSpace, bool stopAtNonPrintable,char delimiter,char *capturedString)
{
int intChar=EOF,index=0;
capturedString[index]=NULL_CHAR;

intChar = fgetc(inStream);

while((intChar != EOF)&&((clearLeadingNonPrintable && intChar==(int)SPACE))||(clearLeadingSpace && intChar==(int)SPACE))

{
 intChar =fgetc(inStream);
}

if(intChar ==EOF)
{
 return false;
}

while(  (intChar != EOF && index<MAX_STR_LEN -1) && ( (stopAtNonPrintable && intChar>=(int)SPACE) || (!stopAtNonPrintable) ) && (intChar !=(char)delimiter) )

{
capturedString[index]=(char)intChar;

index++;
capturedString [index] =NULL_CHAR;

intChar= fgetc(inStream);
}

return true;


}

int gerStringLength(const char *teststr)
{
  int index=0;
  while(index< STD_STR_LEN && teststr[index] !=NULL_CHAR)

{
   index++;
}


return index;



}

bool getStringToDelimiter(FILE *inStream,char delimiter, char *capturedString)

{
  return getStringConstrained(inStream,Ture,Ture,Ture,delimiter,capturedString);
}


void getSubString (char *destStr, const char *sourceStr, int startIndex, int endIndex)

{
  int sourceStrLen= getStringLength(sourceStr);

int destIndex=0;
int sourceIndex=startIndex;
char *tempStr;

if(startIndex >= 0 && startIndex <= endIndex && endIndex < sourceStrLen)

{
 tempStr =(char *) malloc(sourceStrLen +1);
 copyString(tempStr,sourceStr);

 while(sourceIndex <= endIndex)
{
 destStr[destIndex]= tempStr[sourceIndex];
 destIndex++; sourceIndex++;

destStr[destIndex] =NULL_CHAR;
   }

}

free(tempStr);

}



void setStrToLowerCase(char *destStr, const char *sourceStr)
{
int sourceStrLen= getStringLength(sourceStr);
char *tempStr;
int index=0;

tempStr=(char *) malloc(sizeof(sourceStrLen +1));
copyString(tempStr,sourceStr);

while(tempStr[index] != NULL_CHAR && index < MAX_STR_LEN)

{
 destStr[index]= toLowerCase(tempStr[index]);
index++;
destStr[index] =NULL_CHAR;

}

free(tempStr);

}


char toLowerCase(char testChar)
{
 if(testChar >='A' && testChar<='Z')
 {
   testChar= testChar -'A'+'a';

 }
 return testChar;
} 



int getLineTo(FILE *filePtr, int bufferSize, char stopChar, char *buffer, Boolean omitLeadingWhiteSpace, Boolean stopAtNonPrintable)
{
 int charAsInt, charIndex=0;
 int statusReturn = NO_ERR;
 Boolean bufferSizeAvailable= true;
 Boolean nonPrintableFound = False;


charAsInt =fgetc(filePtr);

while(omitLeadingWhiteSpace == true && charAsInt !=(int)stopChar && charIndex < bufferSize && charAsInt <=(int)SPACE)

{
 charAsInt = fgetc(filePtr);
}

if(stopAtNonPrintable == true && charAsInt<(int)SPACE)
{
 nonPrintableFound = true;
}

while (charAsInt != (int)stopChar && nonPrintableFound == False && bufferSizeAvailable == true)
{
if(isEndOfFile(filePtr)==true)
{
 return INCOMPLETE_FILE_ERR;

}

if(charAsInt >=(int)SPACE)
{
 buffer[ charIndex]= (char) charAsInt;
 charIndex++;
}

buffer[charIndex]= NULL_CHAR;

if(charIndex < bufferSize -1)
{
 charAsInt = fgetc(filePtr);
 if(stopAtNonPrintable == true && charAsInt < (int)SPACE)
 {
 nonPrintableFound = true;

 }





}

else
{
 bufferSizeAvailable = False;
 statusReturn = INPUT_BUFFER_OVERRUN_ERR;

}

}

return statusReturn;

}


Boolean isEndOfFile( FILE *filePtr)
{
if(feof(filePtr)!=0)
{return true;}
return false;

}





























