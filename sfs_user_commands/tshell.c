#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../sfs.h"    // SFS structures provided by ar sir
#define MAX_NUMBER_OF_PARAMS 4

void parseCmd(char*, char**);

int main(){
    char cmd[MAX_COMMAND_LENGTH + 1];
    char* params[MAX_NUMBER_OF_PARAMS];
    
    char *path=calloc(MAX_COMMAND_LENGTH,sizeof(int));
    strncpy(path,"/",sizeof(char)+1);

    while(1) {
        printf("tshell### [%s]$ ",path);
        //char prefix[MAX_COMMAND_LENGTH] = "./";       
        
        // Read command from standard input, exit on Ctrl+D
        if(fgets(cmd, sizeof(cmd), stdin) == NULL){
            break;
        }        
        
        //strncat(prefix, cmd, sizeof(prefix) - strlen(prefix) - 1); // add ./ infront of the command

        // Remove trailing newline character, if any
        if(cmd[strlen(cmd)-1] == '\n') {
            cmd[strlen(cmd)-1] = '\0';
        }

        parseCmd(cmd, params);
        
        //printf("params[0]: %s\n",params[0]);
        //printf("params[1]: %s\n",params[1]);        
        
        if(strncmp(params[0],"ls_t",strlen("ls_t")) == 0){
            char *ls_t=malloc(MAX_COMMAND_LENGTH);
            strncpy(ls_t,"./ls_t ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){               
                strncat(ls_t, params[1], strlen(params[1]));
            }else{
                strncat(ls_t, path, strlen(path));
            }
            //printf("%s\n",ls_t);
            system(ls_t);
        }
        
        else if(strncmp(params[0],"cd_t",strlen("cd_t")) == 0){
            //char *cd_t=malloc(MAX_COMMAND_LENGTH);
            //strncpy(cd_t,"./cd_t ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){                               
                strncpy(path,params[1],strlen(params[1]));
                path[strlen(params[1])]='\0';
            } 
            //system(cd_t);
        }
        
        else if(strncmp(params[0],"mkdir_t",strlen("mkdir_t")) == 0){
            char *mkdir_t=malloc(MAX_COMMAND_LENGTH);
            strncpy(mkdir_t,"./mkdir_t ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){                               
                strncat(mkdir_t,params[1],strlen(params[1]));                
            } 
            //printf("%s\n",mkdir_t);
            system(mkdir_t);
        }
        
        else if(strncmp(params[0],"external_cp",strlen("external_cp")) == 0){
            char *external_cp=malloc(MAX_COMMAND_LENGTH);
            strncpy(external_cp,"./external_cp ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){                               
                strncat(external_cp,params[1],strlen(params[1]));                
            } 
            if(params[2]!=NULL){  
                strncat(external_cp," ",sizeof(char));  
                strncat(external_cp,params[2],strlen(params[2]));                
            } 
            //printf("%s\n",external_cp);
            system(external_cp);
        }
        
        else if(strncmp(params[0],"cp_t",strlen("cp_t")) == 0){
            char *cp_t=malloc(MAX_COMMAND_LENGTH);
            strncpy(cp_t,"./cp_t ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){                               
                strncat(cp_t,params[1],strlen(params[1]));                
            } 
            if(params[2]!=NULL){  
                strncat(cp_t," ",sizeof(char));  
                strncat(cp_t,params[2],strlen(params[2]));                
            } 
            //printf("%s\n",cp_t);
            system(cp_t);
        }
        
        else if(strncmp(params[0],"cat_t",strlen("cat_t")) == 0){
            char *cat_t=malloc(MAX_COMMAND_LENGTH);
            strncpy(cat_t,"./cat_t ",MAX_COMMAND_LENGTH);
            if(params[1]!=NULL){                               
                strncat(cat_t,params[1],strlen(params[1]));                
            } 
            //printf("%s\n",cat_t);
            system(cat_t);
        }
        
    }

    return 0;
}

// Split cmd into array of parameters
void parseCmd(char* cmd, char** params)
{       
    for(int i = 0; i < MAX_NUMBER_OF_PARAMS; i++) {
        params[i] = strsep(&cmd, " ");
        if(params[i] == NULL) break;
    }
}

