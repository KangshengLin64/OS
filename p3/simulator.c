#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "StringUtils.h"
#include "simtimer.h"
#include <stdbool.h>

#include <pthread.h>


//char *memory_alloca = NULL;
//process_t *root_process_list, *process_list;
//int running_process_identifier = 0;




 

  



void *msleep(void* args){

  double millisecond =*((double*) args);
  char dummy[100];
  double start_time = accessTimer(LAP_TIMER, dummy);
  while(1000*(accessTimer(LAP_TIMER,dummy)-start_time)<= millisecond);
  return NULL;

}



void timer(double millisecond){

  if(millisecond<=1e-3) return;
  pthread_t pid;
  double *ptr = (double*) malloc(sizeof(double));
  *ptr=millisecond;
  pthread_create(&pid,NULL,msleep,(void*) ptr);
  pthread_join(pid,NULL);
  free(ptr);



}


Boolean mem_alloca_func(memory_t **mem_list,ConfigDataType *config, int base, int offset,int pid){
  if(base +offset>config->memAvailable){
    return false;
  }
  memory_t *mem=*mem_list;
  while(mem){
    if(!((mem->base<base &&mem->base +mem->offset<base +offset)||(base<mem->base&&base+offset<mem->base+mem->offset))){
     return false;
    }
    mem=mem->next;
  }
  mem=(memory_t *)malloc(sizeof(memory_t));
  mem->base=base;
  mem->offset=offset;
  mem->pid=pid;
  mem->next=*mem_list;
  *mem_list=mem;
  return true;
}


Boolean mem_access_func(memory_t *mem_list, int base, int offset,int pid){
  memory_t *mem=mem_list;
  while(mem){
    if(mem->base<=base&&base+offset<=mem->base+mem->offset){
      return pid==mem->pid;
    }
    mem=mem->next;
  }
  return false;
}



Boolean runProcess(process_t *current_process,memory_t **mem_list, ConfigDataType *config){

  int i;

  char value[1000];
  sprintf(value,"OS: Process %d set from READY to RUNNING\n",current_process->pid);
  output_with_time(value,config);
  current_process->state=PROCESS_STATE_RUN;
  Boolean mem_fault=false;
  Boolean result;
  executable_t *exec;
  
  

  for(i=0;!mem_fault && i<current_process->exe_size;++i){
   exec=&current_process->execution_flow[i];
   
   switch(exec->command){

   case APP:
    {
    break;
    }


   case DEVIN:{

    sprintf(value,"Process: %d,%s input operation start",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    timer(exec->time);
    sprintf(value,"Process: %d,%s input operation end",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    break;
    }

   case DEVOUT:{

   sprintf(value,"Process: %d,%s input operation start",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    timer(exec->time);
    sprintf(value,"Process: %d,%s input operation end",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    break;
   }


   case CPU:{

    sprintf(value,"Process: %d,%s input operation start",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    timer(exec->time);
    sprintf(value,"Process: %d,%s input operation end",current_process->pid,exec->origin ->strArg1);
    output_with_time(value,config);
    break;
   }

   case MEM:{
     if(exec->strArg1==ALLOCATE){
      sprintf(value,"Process: %d, attempting mem allocate request",current_process->pid);
      output_with_time(value,config);
      result =mem_alloca_func(mem_list, config,exec->intArg2,exec->intArg3,current_process->pid);
      if(result){
        sprintf(value,"Process: %d, successful mem allocate request",current_process->pid);
      }
      else{
        sprintf(value,"Process: %d, failed mem allocate request",current_process->pid);
        mem_fault=true;
      }
      output_with_time(value,config);
      break;
     }
    else if(exec->strArg1==ACCESS){
      sprintf(value,"Process: %d, attempting mem access request",current_process->pid);
      output_with_time(value,config);
      result=mem_access_func(*mem_list,exec->intArg2,exec->intArg3,current_process->pid);
      if(result){
        sprintf(value,"Process: %d, successful mem access request",current_process->pid);
      }
      else{
        sprintf(value,"Process: %d, failed mem access request",current_process->pid);
        mem_fault=true;
      }
      output_with_time(value,config);
      break;

    }
    }
   case SYS: break;

   }
   }

if(mem_fault){
  output("",config);
  sprintf(value,"OS:Segmentation fault, Process %d ended",current_process->pid);
  output_with_time(value,config);
}
else{
  output("",config);
  sprintf(value,"OS: Process %d ended",current_process->pid);
  output_with_time(value,config);
}
sprintf(value,"OS: Process %d set to EXIT",current_process->pid);
output_with_time(value,config);
current_process->state=PROCESS_STATE_END;
return 0;
}



void initial_process(int n, ConfigDataType *configPtr){
  char value[1007];
  int i;
  for(i =0;i<n;++i){
  sprintf(value,"OS: Process %d set to READY state from NEW state.",i);
  output(value,configPtr);
 }

}



void memset_usr(char* ptr, int count,char value){
  int i;
  for(i=0;i<count;++i) ptr[i]=value;

}

process_t *schedule(process_t **process_list, ConfigDataType *configPtr){
  if(*process_list==NULL)return NULL;

  process_t *selected=NULL;
  if(configPtr->cpuSchedCode==CPU_SCHED_FCFS_N_CODE){
    selected= *process_list;
    *process_list =selected->next;
  }
  else if(configPtr->cpuSchedCode== CPU_SCHED_SJF_N_CODE){
    process_t *selected_prev=NULL;
    process_t *prev=NULL;
    process_t *proc= *process_list;
    while(proc){
      if(proc->state==PROCESS_STATE_NEW){
        if(!selected||proc->total_time<selected->total_time){
          selected=proc;
          selected_prev=prev;
        }
      }
      prev=proc;
      proc=proc->next;
    }
    if(selected_prev)
    selected_prev->next=selected->next;
    else
    *process_list=selected->next;
  }
  return selected;
}


void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMsterPtr){
  if(configPtr->logToCode==LOGTO_BOTH_CODE||configPtr->logToCode==LOGTO_FILE_CODE){
    FILE *fp=fopen(configPtr->logToFileName,"w+");
    char displayString[STD_STR_LEN];
    fprintf(fp,"\n=====================================\n");
    fprintf(fp,"Simulator Log File Header\n\n");
    fprintf(fp,"File Name                : %s\n",configPtr->metaDataFileName);
    configCodeToString(configPtr->cpuSchedCode,displayString);
    fprintf(fp,"CPU Scheduling           : %s\n",displayString);
    fprintf(fp,"Quantum Cycles           : %d\n",configPtr->quantumCycles);
    fprintf(fp,"Memory Available(KB)     : %d\n",configPtr->memAvailable);
    fprintf(fp,"Processor Cycle Rate (ms/cycle)     : %d\n",configPtr->procCycleRate);
    fprintf(fp,"I/O Cycle Rate(ms/cycle) : %d\n",configPtr->ioCycleRate);
    fprintf(fp,"\n================\n");
    fprintf(fp,"Begin Simulation\n\n");
    fclose(fp);
  }
  
  printf("Simulator Run\n---------------------\n\n");
  
  
  
  
  runTimer(0);
  char value[100];
  accessTimer(ZERO_TIMER,value);
  output("OS: Simulation Start", configPtr);
  
  OpCodeType *ptr=metaDataMsterPtr;
  process_t *root_process_list =NULL;
  process_t *prevProcess =NULL;
  process_t *process_list;
  OpCodeType *app_start, *app_end;
  int range = 0;
  int process_cnt =0;

  while(ptr != NULL){

  if(compareString(ptr->command,"sys")==0){

  
   }

  else if(compareString(ptr->command,"app")==0){
  
   if(compareString(ptr->strArg1,"start")==0){
    range=1;
    app_start=ptr;
    }

   else if(compareString(ptr->strArg1,"end")==0){

  range++;
  app_end=ptr;
  process_list=(process_t *)malloc(sizeof(process_t));
  memset_usr((char *)process_list,(sizeof(process_t))/sizeof(char),0);
  process_list->execution_flow =(executable_t *)malloc(sizeof(executable_t)*(range));
  memset_usr((char *)process_list->execution_flow,(sizeof(executable_t)*(range))/sizeof(char),0);

  process_list->exe_size =range;
  process_list->total_time=0;
  process_list->pid=process_cnt;
  
  int command_idx=0;
  struct OpCodeType *app_ptr;

  for(app_ptr=app_start;app_ptr!=app_end;app_ptr=app_ptr->nextNode){
    executable_t *exec= &process_list->execution_flow[command_idx];
    exec->intArg2=app_ptr->intArg2;
    exec->intArg3=app_ptr->intArg3;
    exec->origin=app_ptr;
    process_list->state=PROCESS_STATE_NEW;

    if(compareString(app_ptr->command,"app")==0){
      if(compareString(app_ptr->strArg1,"start")==0){
        exec->command=APP;
        exec->strArg1=START;
      
      }
      else if(compareString(app_ptr->command,"app")==0){
        exec->command=APP;
        exec->strArg1=END;
      }
      exec->time = exec->intArg2*configPtr->procCycleRate;
    }
     else if (compareString(app_ptr->command,"cpu")==0){
    if(compareString(app_ptr->strArg1,"process")==0){
      exec->command=CPU;
      exec->strArg1=PROCESS;
    }
    exec->time =exec->intArg2*configPtr->procCycleRate;
   }
   else if(compareString(app_ptr->command,"dev")==0){
    if(compareString(app_ptr->inOutArg,"in")==0){
      exec->command=DEVIN;
    }
    else if(compareString(app_ptr->inOutArg,"out")==0){
      exec->command=DEVOUT;
    }
    if(compareString(app_ptr->strArg1,"monitor")==0)
    exec->strArg1=MONITOR;
    if(compareString(app_ptr->strArg1,"sound signal")==0)
    exec->strArg1= SOUND_SIGNAL;
    if(compareString(app_ptr->strArg1,"ethernet")==0)
    exec->strArg1= ETHERNET;
    if(compareString(app_ptr->strArg1,"hard drive")==0)
    exec->strArg1= HDD;
    if(compareString(app_ptr->strArg1,"keyboard")==0)
    exec->strArg1= KEYBOARD;
    if(compareString(app_ptr->strArg1,"serial")==0)
    exec->strArg1= SERIAL;
    if(compareString(app_ptr->strArg1,"video signal")==0)
    exec->strArg1= VIDEO_SIGNAL;
    if(compareString(app_ptr->strArg1,"usb")==0)
    exec->strArg1= USB;
    exec->time =exec->intArg2*configPtr->ioCycleRate;
   }
   else if(compareString(app_ptr->command,"mem")==0){
    exec->command=MEM;
    if(compareString(app_ptr->strArg1,"access")==0)
    exec->strArg1=ACCESS;
    else if(compareString(app_ptr->strArg1,"allocate")==0)
    exec->strArg1 =ALLOCATE;
    exec->time=0;
   }
   process_list->total_time-=exec->time;
   command_idx+=1;
  }

  if(prevProcess!=NULL)
  prevProcess->next = process_list;
  
  else{
    root_process_list= process_list;
  }
  prevProcess=process_list;
  process_cnt++;

   }
}else{
  range++;
}
    ptr= ptr->nextNode;
    
}

initial_process(process_cnt,configPtr);
process_list =root_process_list;

memory_t *mem_list=NULL;
process_t *selected_proc;
while((selected_proc=schedule(&process_list,configPtr))!=false){
  sprintf(value,"OS: Process %d selected with %d ms remaining",selected_proc->pid,selected_proc->total_time);
  output_with_time(value,configPtr);

  runProcess(selected_proc, &mem_list, configPtr);

  memory_t *mem=mem_list;
  while(mem){
    mem_list=mem->next;
    free(mem);
    mem=mem_list;
  }
  free(selected_proc->execution_flow);
  free(selected_proc);

}
output_with_time("OS: System stop",configPtr);
output_with_time("OS: Simulation end",configPtr);

if(configPtr->logToCode == LOGTO_BOTH_CODE||configPtr->logToCode==LOGTO_FILE_CODE){
  FILE *fp=fopen(configPtr->logToFileName,"a+");
  fprintf(fp,"\nEnd Simulation - Complete\n");
  fprintf(fp,"=========================\n\n");
  fclose(fp);
}

  
}


  