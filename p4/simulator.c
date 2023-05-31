#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "StringUtils.h"
#include "simtimer.h"
#include <stdbool.h>

#include <assert.h>


//char *memory_alloca = NULL;
//process_t *root_process_list, *process_list;
//int running_process_identifier = 0;




 

  



process_t* schedule(ready_queue_t* ready_q, ConfigDataType* configPtr){
  if(rq_size(ready_q)==0){
    return NULL;
  }
  process_t* selected = rq_pop_front(ready_q);
  selected->next=NULL;
  return selected;
}

void enqueue(ready_queue_t* ready_q,process_t* proc, ConfigDataType* configPtr,Boolean init){
  assert(proc->cur_exe<proc->exe_size);
  if(configPtr->cpuSchedCode == CPU_SCHED_FCFS_N_CODE)
  {
    if(init){
      rq_push_back(ready_q,proc);
    }
    else{
      rq_push_front(ready_q,proc);
    }
  }
  else if(configPtr->cpuSchedCode==CPU_SCHED_SJF_N_CODE){
    if(init){
      rq_insert_cmp_by_pid(ready_q,proc);
    }
    else{
      rq_push_front(ready_q,proc);
    }
  }
  else if(configPtr->cpuSchedCode==CPU_SCHED_FCFS_P_CODE){
    rq_insert_cmp_by_pid(ready_q,proc);
  }
  else if(configPtr->cpuSchedCode==CPU_SCHED_SRTF_P_CODE){
    rq_insert_cmp_by_remaining_time(ready_q,proc);
  }
  else if(configPtr->cpuSchedCode==CPU_SCHED_RR_P_CODE){
    rq_push_back(ready_q,proc);
  }
}

void process_end(process_t* proc,ConfigDataType* config)
{
  assert(proc->execution_flow);
  char value[BUFSIZ];
  sprintf(value, "OS: Process %d ended",proc->pid);
  output_with_time(value,config);
  sprintf(value,"OS: Process %d set to EXIT",proc->pid);
  output_with_time(value,config);

  free(proc->execution_flow);
  free(proc);
}



Boolean has_interrupt(sim_t* sim)
{
  pthread_mutex_lock(&sim->mutex);
  Boolean ret = rq_size(sim->interrupt_q)>0;
  pthread_mutex_unlock(&sim->mutex);
  return ret;
}



void print_interrupt(sim_t* sim, ConfigDataType* config)
{
  char value[BUFSIZ];
  process_t* current_process;
  pthread_mutex_lock(&sim->mutex);

  if(rq_size(sim->interrupt_q)>0){
    current_process= sim->interrupt_q->tail;
    sprintf(value,"OS: Interrupted by process %d, %s %sput operation",current_process->pid,current_process->execution_flow[current_process->cur_exe].origin->strArg1,current_process->execution_flow[current_process->cur_exe].origin->inOutArg);
    output_with_time(value,config);
  }
  pthread_mutex_unlock(&sim->mutex);
}

void handle_interrupt(sim_t* sim, ConfigDataType* config){
  char value[BUFSIZ];
  process_t* current_process;
  pthread_mutex_lock(&sim->mutex);

  while(rq_size(sim->interrupt_q)>0){
    current_process= rq_pop_front(sim->interrupt_q);
    current_process->cur_exe++;

    if(current_process->cur_exe<current_process->exe_size){
      current_process->state=PROCESS_STATE_READY;
      sprintf(value,"OS: Process %d set from BLOCKED to READY",current_process->pid);
      output_with_time(value,config);
      enqueue(sim->ready_q, current_process, config,false);
      sim->interrupt = true;
    }
    else{
      sim->num_ended++;
      sim->cur_proc=NULL;
      process_end(current_process,config);
    }
  }
sim->interrupt_q=false;
pthread_mutex_unlock(&sim->mutex);
}


void* block_for_io(void* args){
  char value[BUFSIZ];
  block_for_io_args_t* arg= args;
  ConfigDataType* config= arg->config;
  process_t* current_process= arg->proc;
  executable_t* exec= &arg->proc->execution_flow[arg->proc->cur_exe];
  timer(exec->time);
  current_process->total_time -= exec->time;
  if(exec->command == DEVIN){
    sprintf(value, "Process: %d, %s input operation end",current_process->pid,exec->origin->strArg1);
    output_with_time(value,config);
  }
  else{
    sprintf(value, "Process: %d, %s output operation end",current_process->pid,exec->origin->strArg1);
    output_with_time(value,config);
  }
  pthread_mutex_lock(&arg->sim->mutex);
  arg->sim->interrupt= true;
  rq_push_back(arg->sim->interrupt_q, current_process);
  pthread_mutex_unlock(&arg->sim->mutex);
  free(args);
  return NULL;
}


Boolean runProcess(process_t* current_process, sim_t* sim, ConfigDataType* config){
  char value[BUFSIZ];
  assert(current_process->cur_exe<current_process->exe_size);
  executable_t* exec = &current_process->execution_flow[current_process->cur_exe];
  if(current_process->state != PROCESS_STATE_RUN){
    sprintf(value,"OS: Process %d set from READY to RUNNING\n",current_process->pid);
    output_with_time(value, config);
    current_process->state= PROCESS_STATE_RUN;
  }
  if(exec->command==CPU){
    sprintf(value,"Process: %d, cpu process operation start",current_process->pid);
    output_with_time(value,config);
    int quantum = config->quantumCycles;
    Boolean interrupted = false;
    while(exec->time>0&&quantum>0){
      timer(config->procCycleRate);
      exec->time-=config->procCycleRate;
      current_process->total_time-=config->procCycleRate;

      pthread_mutex_lock(&sim->mutex);

      if(sim->interrupt){
        interrupted=true;
        pthread_mutex_unlock(&sim->mutex);
        break;
      }
      pthread_mutex_unlock(&sim->mutex);
      quantum--;
    }
    if(exec->time==0){
      current_process->cur_exe++;
    }
    if(!interrupted && quantum == 0 && exec->time>0){
      output("",config);
      sprintf(value,"OS: Process %d quantum time out, cpu process operation end\n",current_process->pid);
      output_with_time(value, config);
    }
    else{
      sprintf(value,"Process: %d, cpu process opration end\n",current_process->pid);
      output_with_time(value,config);
    }
    if(interrupted){
      print_interrupt(sim,config);
    }
  }
  else if(exec-> command==DEVIN||exec->command==DEVOUT){
    if(exec->command==DEVIN){
      sprintf(value,"Process: %d, %s input operation start\n",current_process->pid,exec->origin->strArg1);
      output_with_time(value,config);
      sprintf(value,"OS: Process %d blocked for input operation",current_process->pid);
      output_with_time(value,config);
    }
    else{
      sprintf(value,"Process: %d,%s output operation start\n",current_process->pid, exec->origin->strArg1);
      output_with_time(value,config);
      sprintf(value,"OS: Process %d blocked for output operation",current_process->pid);
      output_with_time(value,config);
    }
    sprintf(value,"OS: Process %d set from RUNNING to BLOCKED",current_process->pid);
    output_with_time(value,config);
    current_process->state=PROCESS_STATE_BLOCKED;
    pthread_t pid;
    block_for_io_args_t* args= (block_for_io_args_t*)malloc(sizeof(block_for_io_args_t));
    assert(args);
    args->sim=sim;
    args->proc=current_process;
    args->config=config;
    if(config->cpuSchedCode==CPU_SCHED_FCFS_N_CODE||config->cpuSchedCode==CPU_SCHED_SJF_N_CODE){
      block_for_io((void*)args);
    }
    else{
      pthread_create(&pid,NULL,block_for_io,(void*)args);
      pthread_detach(pid);
    }
    return false;
  }
  else if(exec->command==MEM){
    if(exec->strArg1==ALLOCATE){
      sprintf(value,"Process: %d, attempting mem allocate request",current_process->pid);
      output_with_time(value,config);
    }
    else if(exec->strArg1==ACCESS){
      sprintf(value,"Process: %d, attempting mem access request",current_process->pid);
      output_with_time(value,config);
    }
    current_process->cur_exe++;
  }
  else{
    current_process->cur_exe++;
  }
  pthread_mutex_lock(&sim->mutex);
  sim->cur_proc=current_process;
  pthread_mutex_unlock(&sim->mutex);
  return true;
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
  char value[BUFSIZ];
  accessTimer(ZERO_TIMER,value);
  output("OS: Simulation Start", configPtr);
  
  process_t* proc;
  OpCodeType *ptr=metaDataMsterPtr;
  ready_queue_t* ready_q=rq_new();
  OpCodeType *app_start, *app_end;
  int range = 0;
  int process_cnt =0;
  int command_index;
  struct OpCodeType* app_ptr;
  executable_t* exec;

  while(ptr != NULL){

  

   if(compareString(ptr->command,"app")==0){
  
   if(compareString(ptr->strArg1,"start")==0){
    range=0;
    app_start=ptr;
    }

   else if(compareString(ptr->strArg1,"end")==0){

  
  app_end=ptr;

  proc=(process_t*)malloc(sizeof(process_t));
  
  memset_usr((char*)proc,sizeof(process_t),0);
  proc->execution_flow=(executable_t*)malloc(sizeof(executable_t)*(range));
  memset_usr((char*)proc->execution_flow,sizeof(executable_t)*(range),0);

  proc->exe_size=range;
  proc->cur_exe=0;
  proc->total_time=0;
  proc->pid=process_cnt;
  proc->state=PROCESS_STATE_NEW;

  command_index=0;

  for(app_ptr=app_start;app_ptr!=app_end;app_ptr=app_ptr->nextNode){
    exec=&proc->execution_flow[command_index];
    exec->intArg2=app_ptr->intArg2;
    exec->intArg3=app_ptr->intArg3;
    exec->origin=app_ptr;
   

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
   proc->total_time-=exec->time;
   command_index++;
  }

  enqueue(ready_q,proc,configPtr,true);
  process_cnt++;
  }
   }
   else if(compareString(ptr->command,"sys")!=0)
   {
    range++;
   }
   ptr=ptr->nextNode;
   }

   initial_process(process_cnt,configPtr);
   sim_t sim;
   sim_init(&sim,ready_q);

   Boolean result;
   process_t* selected_proc;
   Boolean is_idle=false;

   while(1){
    pthread_mutex_lock(&sim.mutex);
    sim.interrupt=false;
    if(sim.num_ended==process_cnt){
      pthread_mutex_unlock(&sim.mutex);
      break;
    }
    selected_proc= schedule(ready_q,configPtr);
    if(selected_proc){
      if(sim.cur_proc!=selected_proc){
        sprintf(value,"OS: Process %d selected with %d ms remaining",selected_proc->pid,selected_proc->total_time);
        output_with_time(value,configPtr);
      }
      pthread_mutex_unlock(&sim.mutex);
      result= runProcess(selected_proc,&sim,configPtr);
      if(result==true){
        if(selected_proc->cur_exe<selected_proc->exe_size){
          pthread_mutex_lock(&sim.mutex);
          enqueue(sim.ready_q,selected_proc,configPtr,false);
          pthread_mutex_unlock(&sim.mutex);
        }
        else{
          sim.cur_proc=NULL;
          sim.num_ended++;
          process_end(selected_proc,configPtr);
        }
      }
      is_idle=false;
    }
    else{
      if(!is_idle){
        output_with_time("OS: CPU idle , all active processes blocked",configPtr);
        timer(1);
        is_idle =true;
      }
      pthread_mutex_unlock(&sim.mutex);
    }
    if(is_idle && has_interrupt(&sim)){
      output_with_time("OS: CPU interrupt, end idle",configPtr);
      print_interrupt(&sim,configPtr);
    }
    handle_interrupt(&sim,configPtr);
   }
   assert(sim.num_ended==process_cnt);
   output_with_time("OS: System stop",configPtr);
   output_with_time("OS: Simulation end",configPtr);
   if(configPtr->logToCode==LOGTO_BOTH_CODE||configPtr->logToCode==LOGTO_FILE_CODE){
    FILE* fp=fopen(configPtr->logToFileName,"a+");
    fprintf(fp,"\nEnd Simulation - Complete\n");
    fprintf(fp,"=========================\n\n");
    fclose(fp);
   }
   sim_destory(&sim);
  


}
  
  
void sim_init(sim_t* sim, ready_queue_t* ready_q){
  sim->num_ended=0;
  sim->cur_proc=NULL;
  sim->ready_q=ready_q;
  sim->interrupt_q=rq_new();
  sim->interrupt=false;
  pthread_mutex_init(&sim->mutex,NULL);
}


void sim_destory(sim_t* sim){
  rq_delete(sim->ready_q);
  rq_delete(sim->interrupt_q);
  pthread_mutex_destroy(&sim->mutex);
}


void* msleep(void* args){

  double millisecond =*((double*) args);
  char dummy[100];
  double start_time = accessTimer(LAP_TIMER, dummy);
  while(1000*(accessTimer(LAP_TIMER,dummy)-start_time)<= millisecond);
  return NULL;

}

void timer(double millisecond){

  if(millisecond>SMALL_QUANTUM) {
    pthread_t pid;
    double* ptr=(double*)malloc(sizeof(double));
    *ptr=millisecond;
    pthread_create(&pid,NULL,msleep,(void*)ptr);
    pthread_join(pid,NULL);
  }
}

void initial_process(int non,ConfigDataType* configPtr){
  char value[BUFSIZ];
  int index;
  for(index=0;index<non;++index){
    sprintf(value,"OS: Process %d set to READY state from NEW state",index);
    output_with_time(value,configPtr);
  }
}

void memset_usr(char* ptr, int count, char value){
  int index;
  for(index=0;index<count;++index){
    ptr[index]=value;
  }
}


int process_cmp_by_remaining_time(process_t* pro1,process_t* pro2){
  return pro1->total_time-pro2->total_time;
}

int process_cmp_by_pid(process_t* pro1, process_t* pro2){
  return pro1->pid-pro2->pid;
}


ready_queue_t* rq_new(){
  ready_queue_t* ready_q=(ready_queue_t*)malloc(sizeof(ready_queue_t));
  ready_q->head= ready_q->tail=NULL;
  ready_q->size=0;
  return ready_q;
}

void rq_delete(ready_queue_t* ready_q){
  process_t* proc=ready_q->head;
  while(proc){
    ready_q->head=proc->next;
    free(proc->execution_flow);
    free(proc);
    proc=ready_q->head;
  }
  free(ready_q);
}

void rq_push_back(ready_queue_t* ready_q,process_t* proc){
  assert(proc);
  proc->next=NULL;
  if(ready_q->head){
    ready_q->tail->next=proc;
    ready_q->tail=proc;
  }
  else{
    ready_q->head=ready_q->tail=proc;
  }
  ++ready_q->size;
}


void rq_push_front(ready_queue_t* ready_q, process_t* proc){
  assert(proc);
  proc->next=ready_q->head;
  if(ready_q->head){
    
    ready_q->head=proc;
  }
  else{
    ready_q->head=ready_q->tail=proc;
  }
  ++ready_q->size;
}


void rq_insert_cmp_by_remaining_time(ready_queue_t* ready_q,process_t* proc){
  assert(proc);
  if( ready_q->head == NULL || process_cmp_by_remaining_time(proc,ready_q->head)<0){
    rq_push_front(ready_q,proc);
  }
  else if(process_cmp_by_remaining_time(ready_q->tail,proc)<=0){
    rq_push_back(ready_q,proc);
  }
  else{
    process_t* proc_node =ready_q->head;
    while(proc_node->next){
      if(process_cmp_by_remaining_time(proc,proc_node->next)<0){
        proc->next= proc_node->next;
        proc_node->next=proc;
        break;
      }
      proc_node =proc_node->next;
    }
    ++ready_q->size;
  }
}


void rq_insert_cmp_by_pid(ready_queue_t* ready_q, process_t* proc){
  assert(proc);
  if(ready_q->head==NULL || process_cmp_by_pid(proc,ready_q->head)<0){
    rq_push_front(ready_q,proc);
  }
  else if(process_cmp_by_pid(ready_q->tail,proc)<=0){
    rq_push_back(ready_q,proc);
  }
  else{
    process_t* proc_node= ready_q->head;
    while(proc_node->next){
      if(process_cmp_by_pid(proc,proc_node->next)<0){
        proc->next=proc_node->next;
        proc_node->next=proc;
        break;
      }
      proc_node= proc_node->next;
    }
    ++ready_q->size;
  }
}

process_t* rq_pop_front(ready_queue_t* ready_q){
  assert(ready_q->head);
  process_t* head = ready_q->head;
  ready_q->head=head->next;
  if(ready_q->head==NULL){
    ready_q->tail=NULL;
  }
  --ready_q->size;
  return head;
}


int rq_size(ready_queue_t* ready_q){
  return ready_q->size;
}