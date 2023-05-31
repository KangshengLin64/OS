#include "stdio.h"
#include "OS_SimDriver.h"
#include "metadataops.h"
#include "configops.h"
#include "simulator.h"
#include "simtimer.h"
#include <stdbool.h>

extern const int STR_EQ;

void showProgramFormat();

int main(int argc,char **argv){


  Boolean programRunFlag = false;
  Boolean configDisplayFlag = false;
  Boolean configUploadFlag = false;
  Boolean mdDisplayFlag = false;
  Boolean runSimFlag = false;
  Boolean inforFlag = false;

  int argIndex =1;
  int lastFourLetters =4;
  int fileStrLen,fileStrSubLoc;
  char fileName[STD_STR_LEN];
  char errorMessage[MAX_STR_LEN];
  ConfigDataType *configDataPtr = NULL;
  OpCodeType *metaDataPtr = NULL;

  printf("\nSimulator Program\n");
  printf("=================\n\n");

  if(argc < 2){
  
  showProgramFormat();

  programRunFlag = false;

  inforFlag = true;


   }

  fileName[0] = NULL_CHAR;

  while (programRunFlag == false && argIndex < argc){

  fileStrLen = getStringLength(argv[argIndex]);
  fileStrSubLoc = findSubString(argv[argIndex],".cnf");

  if(compareString(argv[argIndex],"-dc")== STR_EQ){
    
  configUploadFlag = true;

  configDisplayFlag = true;

  }

  else if(compareString(argv[argIndex],"-dm")== STR_EQ){

  configUploadFlag = true;

  mdDisplayFlag = true;

  }

  else if(compareString(argv[argIndex],"-rs")== STR_EQ){

  configUploadFlag = true;

  runSimFlag = true;


  }

  else if (fileStrSubLoc != SUBSTRING_NOT_FOUND && fileStrSubLoc == fileStrLen - lastFourLetters){

  copyString(fileName,argv[argIndex]);

  programRunFlag = true;

  }

  argIndex++;

  }


  if(programRunFlag == false && inforFlag == false){

  printf("Incorrect argument line format, program aborted\n\n");

  showProgramFormat();


  }


  if(programRunFlag == true && configUploadFlag == true){

  if(getStringLength(fileName)>0 && getConfigData(fileName, &configDataPtr, errorMessage)== true){

  if(configDisplayFlag == true){

  displayConfigData(configDataPtr);
  }
  }


  else{

  printf("\nConfig Upload Error: %s, program aborted\n\n",errorMessage);

  programRunFlag = false;

  }


}


if (programRunFlag == true && (mdDisplayFlag == true || runSimFlag == true)){

  if(getMetaData(configDataPtr->metaDataFileName, &metaDataPtr, errorMessage)== true){


  if(mdDisplayFlag == true){

  displayMetaData(metaDataPtr);

  }

  if(runSimFlag == true){

  runSim(configDataPtr, metaDataPtr);
  }
  }
  else{
   
  printf("\nMetadata Upload Error: %s, program aborted\n",errorMessage);


  }

  }

  configDataPtr = clearConfigData(configDataPtr);
  
  metaDataPtr  = clearMetaDataList(metaDataPtr);

  printf("\nSimulator Program End.\n\n");

  return 0;
  }



  void showProgramFormat(){

  printf("program Format:\n");
  printf("     sim_0x [-dc] [-dm] [-rs] < config file name>\n");
  printf("     -dc [optional] displays configuration data\n");
  printf("     -dm [optional] display meta data\n");
  printf("     -rs [optional] runs simulator\n");
  printf("     config file name is required\n");
  }
