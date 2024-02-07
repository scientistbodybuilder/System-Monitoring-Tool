#include <fcntl.h>
#include <unistd.h>
#include <utmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <ctype.h>

void MemInfo(){
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        perror("sysinfo");
        printf("Error2");
    } else{
        float totalram = ((float)(sys_info.totalram))/1000000000.00;  //convert the bytes into gigabytes
        float freeram = ((float)(sys_info.freeram))/1000000000.00;
        float usedram = totalram - freeram;
        float totalVram = ((float)sys_info.totalswap)/1000000000.00 + totalram;
        float freeVram = ((float)sys_info.freeswap)/1000000000.00 + freeram;
        float usedVram = totalVram - freeVram;

        printf("%.2f GB / %.2f GB -- : %.2f GB / %.2f GB\n", usedram, totalram, usedVram, totalVram);
    }
}

int UsersInfo(){
    struct utmp *user;
    int count = 0;
    setutent();
    while((user = getutent()) != NULL){
        if(user->ut_type == USER_PROCESS){
            printf(" %s\t%s (%s)\n", user->ut_user, user->ut_line, user->ut_host);
            count++;
        }
    }
    endutent();
    return count;
}

void OSInfo(){
    struct utsname system_info;
    struct sysinfo sys_info;

    if (uname(&system_info) == -1) {
        perror("uname");
        printf("Error1");
    }
    if (sysinfo(&sys_info) == -1) {
        perror("sysinfo");
        printf("Error2");
    }

    unsigned long uptime_seconds = sys_info.uptime;             //perform calculation to convert seconds to different time measures
    unsigned int days = uptime_seconds / (60 * 60 * 24);
    unsigned int hours = (uptime_seconds / (60 * 60)) % 24;
    unsigned int minutes = (uptime_seconds / 60) % 60;
    unsigned int seconds = uptime_seconds % 60;
    int total_hours = days*24 + hours;

    printf("### System Information ###\n");
    printf(" System name: %s\n", system_info.sysname);
    printf(" Machine name: %s\n", system_info.nodename);
    printf(" Version: %s\n", system_info.version);
    printf(" Release: %s\n", system_info.release);
    printf(" Architecture: %s\n", system_info.machine);
    printf(" System running since last reboot: %u days, %u:%u:%u (%d:%u:%u)\n", days, hours, minutes, seconds, total_hours, minutes,seconds );
    printf("---------------------------------------\n");

}

int coresCount(){ //calculate the number of cores on the system
    int core = 0;
    char line[100];
    FILE *core_file = fopen("/proc/cpuinfo", "r");

    if (core_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return -1;
    } else {
        while (fgets(line, sizeof(line), core_file) != NULL) { //read each line of the proc/cpuinfo file,
            if (strstr(line, "processor") != NULL) {         // and count the number of lines that contain 'processor'
                core++;
            }
        }
        fclose(core_file);
        return core;
    }
}

float totalCPU(){ //calculate the total cpu usage using information from proc/stat
    FILE *statfile = fopen("/proc/stat", "r");
    if (statfile == NULL) {
        perror("Error opening file");
        return 1;
    }
    char line[1024];
    int sum = 0;

    while(fgets(line, sizeof(line), statfile) != NULL) {
        //First check if the line starts with 'cpu'

        if(strncmp(line,"cpu ",4)==0){ //check that the first substring is 'cpu'
            char* array[11];
            int i = 0;
            char* sub = strtok(line, " "); //populate the array with substrings
            while(sub != NULL && i<11){
                array[i] = sub;
                sub = strtok(NULL, " ");
                i++;
            } //upon completion of the while loop, I should have a 2D array containing the substrings of that line
            for(int j = 1; j<11;j++){               //this loop will sum the integer substrings within the array.
                int num = atoi(array[j]);      //Each of those integers represents time spent by the cpu in different states
                sum += num;
            }
            int num1 = atoi(array[3])*100;
            float cpu_use = (float)num1 / (float)sum;  //calculate cpu usage by diving by time spent by processes
            fclose(statfile);                   //in kernel by the sum of time spent by the cpu in all states
            return cpu_use;
        }
        fclose(statfile);
        return 0.0;
    }
    fclose(statfile);
    return 0.0;
}

int memoryUsage(){
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss;
    } else {
        perror("getrusage");
        return 0;
    }
}
void clearScreen() { // Clear the screen
    printf("\033[2J"); // Clears the screen and moves the cursor to the top left corner to "refresh"
    printf("\033[H");
}
void newLine(int n){
    for( int i=0;i<n;i++){
        printf("\n");
    }
}
void refreshMem(int n){ //generates memory information each iteration
    for(int i=0;i<n;i++){
        MemInfo();
    }
}
void printGraphicElement1(int num_times, float percent_diff_float){
    char ch = '|';
    printf("         ");
    for(int i =0;i<num_times;i++){
        printf("%c", ch);
    }
    printf(" %.2f\n", percent_diff_float);
}

void cpuGraphic(int percent_diff_int, float percent_diff_float){ //
    if(percent_diff_int<=0){
        printf("         | %.2f\n", percent_diff_float);
    } else {
        for(int i =0;i<percent_diff_int;i++){
            printGraphicElement1(percent_diff_int, percent_diff_float);
        }
    }
}

void refreshCPU(int n, float base){
    float current_cpu = totalCPU();
    float percent_diff_float = current_cpu - base;
    int percent_diff_int = (int)(10*(percent_diff_float))+1;

    for(int i = 0;i<n;i++){
        cpuGraphic(percent_diff_int, percent_diff_float);
    }
}

int generateOutline(int sample, int tdelay){
    clearScreen();
    printf("Nbr of samples: %d -- every %d secs\n"
           " Memory usage: %d kilobytes\n"
           "---------------------------------------\n"
           "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n", sample, tdelay, memoryUsage());

    printf("\033[K"); // Clear from cursor to end of line
    printf("\033[%d;1H---------------------------------------\n"
           "### Sessions/users ###\n", sample+5);
    //USER INFO
    int line = UsersInfo();
    printf("---------------------------------------\n"
           "Number of cores: %d\n"
           " total cpu use = %.2f%%\n"
           "---------------------------------------\n", coresCount(), totalCPU());
    return line;

}

void generateStat( int sample, int tdelay) {  //Create the default display
    int line = generateOutline(sample, tdelay);
    for (int i = 1; i <= sample; i++) {
        //clearScreen();
        //printf("\033[K"); // Clear from cursor to end of line
        printf("\033[%d;1H",i+4);
        MemInfo();
        sleep(tdelay);

        printf("\033[%d;1H",sample+line+9);
        printf(" total cpu use = %.2f%%\n"
               "---------------------------------------\n",totalCPU());
        sleep(tdelay);
    }

    printf("\033[K"); // Clear from cursor to end of line
    printf("\033[%d;1H",sample+line+11);
    OSInfo();
}
void generateGraphic( int sample, int tdelay) {  //Create the default display
    float basecpu = totalCPU();
    for (int i = 0; i < sample; i++) {
        clearScreen();
        printf("Nbr of samples: %d -- every %d secs\n"
               " Memory usage: %d kilobytes\n"
               "---------------------------------------\n"
               "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n", sample, tdelay, memoryUsage());
        refreshMem(i + 1);
        newLine(sample - 1 - i);
        printf("---------------------------------------\n"
               "### Sessions/users ###\n");
        //USER INFO
        UsersInfo();
        printf("---------------------------------------\n"
               "Number of cores: %d\n"
               " total cpu use = %.2f%%\n", coresCount(), totalCPU());
        refreshCPU(i+1, basecpu);
        newLine(sample- 1 - i);
        sleep(tdelay);
    }
    OSInfo();
}

void generateSystemStat( int sample, int tdelay){ //generate the memory and system information
    for(int i =0;i<sample;i++){
        clearScreen();
        printf("Nbr of samples: %d -- every %d secs\n"
               " Memory usage: %d kilobytes\n"
               "---------------------------------------\n"
               "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n", sample, tdelay, memoryUsage());
        refreshMem(i + 1);
        newLine(sample - 1 - i);
        printf("Number of cores: %d\n"
               " total cpu use = %.2f%%\n", coresCount(), totalCPU());
        sleep(tdelay);
    }
    printf("---------------------------------------\n");
    OSInfo();

}
void generateUserStat( int sample, int tdelay){ //generate the user and system information
    for(int i=0;i<sample;i++){
        clearScreen();
        printf("Nbr of samples: %d -- every %d secs\n"
               " Memory usage: %d kilobytes\n", sample, tdelay, memoryUsage());
        printf("---------------------------------------\n"
               "### Sessions/users ###\n");
        UsersInfo();
        printf("---------------------------------------\n");
        sleep(tdelay);
    }
    printf("---------------------------------------\n");
    //OS INFORMATION
    OSInfo();
}

void printMemoryInfo(int it){ //print only the memory information, for the sequential command
    printf(">>>> iteration %d\n"
           "Memory usage: %d kilobytes\n"
           "---------------------------------------\n"
           "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n\n\n\n", it, memoryUsage());
    MemInfo();
}
void printUserInfo(){ //print only the user information for the sequential command
    printf("\n\n\n---------------------------------------\n"
           "### Sessions/users ###\n");
    //USER INFO
    UsersInfo();

    printf("---------------------------------------\n"
           "Number of cores: %d\n"
           "total cpu use = %.2f%%\n"
           "---------------------------------------\n", coresCount(), totalCPU());
}

void sequence(int it){
    printMemoryInfo(it);
    sleep(1);
    printUserInfo();
    sleep(1);
    OSInfo();
}
int countSeparators(char* string,char ch){
    int count = 0;
    int len = strlen(string);
    for(int i=0;i<len;i++){
        if(string[i]== ch) {
            count++;
        }
    }
    return count;
}

int checkInt(char* string){ //check whether the passed string is a string integer
    int len = strlen(string);
    for(int i = 0;i<len;i++){
        if(!isdigit(string[i])){
            return 0;
        }
    }
    return 1;
}

void split(char* command, char** arr){  //designed to split a string into two substring if it is of format
    // 'substring''=''substring', and insert them into a 2D array
    int count = countSeparators(command, '=');
    if(count==1){
        char* substring1 = strtok(command, "=");  //use the strtoken function to 'parse' the command into substrings split at the '=' operator
        char* substring2 = strtok(NULL,"=");
        if(substring2 != NULL && substring1 != NULL){ //check whether the '=' was preceded and followed by proper substrings
            arr[0] = (char *)malloc(strlen(substring1) + 1); //allocate memory for the substrings
            arr[1] = (char *)malloc(strlen(substring2) + 1);

            strcpy(arr[0], substring1);
            strcpy(arr[1], substring2);
            //return array;
        } else {
            fprintf(stderr, "Invalid command format\n");
            arr = NULL;
        }
    } else{  //command format is not valid
        fprintf(stderr, "Command does not contain exactly one '='\n");
        arr = NULL;
    }
}

int main(int argc, char **argv){
    int sample = 10; //set default sample and tdelay values
    int tdelay = 1;

    if(argc==2){
        if(strcmp(argv[1], "--system")==0){
            clearScreen();
            generateSystemStat(sample, tdelay);
        }
        if(strcmp(argv[1], "--user")==0){
            clearScreen();
            generateUserStat(sample, tdelay);
        }
        if(strcmp(argv[1], "--sequential")==0){
            for(int i =0; i<sample; i++){ //'i' represents the current iteration
                sequence(i);
                sleep(1);
            }
        }
        if(strcmp(argv[1], "--graphics")==0 || strcmp(argv[1],"-g")==0){
            clearScreen();
            generateGraphic(sample, tdelay);
        }
        //
        if(strchr(argv[1], '=') != NULL){
            char **array = malloc(2 * sizeof(char*)); //initialize and allocate memory for a pointer to an array of strings
            split(argv[1],array); //pass the array pointer to split(), which will populate it with substrings
            if(array!=NULL) { //array now points to an array of two strings
                char* sub1 = array[0];
                char* sub2 = array[1];
                if(strcmp(sub1,"--sample")==0){ //che
                    //check if the sub2 is an int
                    if(checkInt(sub2)==1 && atoi(sub2)!=0){ //check whether the second sub string is a positive integer
                        int n = atoi(sub2);
                        sample = n;
                        clearScreen();
                        generateStat(sample, tdelay);
                        free(array); //deallocate to avoid memory leak
                    } else {
                        printf("Invalid command\n");
                        free(array);
                    }
                }
                else if(strcmp(sub1,"--tdelay")==0 && atoi(sub2)!=0){
                    //check if the sub2 is an int
                    if(checkInt(sub2)==1){ //check whether the second sub string is a positive integer
                        int n = atoi(sub2);
                        tdelay = n;
                        clearScreen();
                        generateStat(sample, tdelay);
                        free(array); //deallocate to avoid memory leak
                    } else {
                        printf("Invalid command\n");
                        free(array);
                    }
                } else {
                    printf("Invalid command\n");
                    free(array);
                }

            } else {
                printf("Invalid command\n");
            }
        }
    } else if(argc==3){
        char* str1 = argv[1];
        char* str2 = argv[2];
        if(checkInt(str1) && checkInt(str2) && atoi(str1)!=0 && atoi(str2)!=0){   //when we receive to command arguments, we check
            sample = atoi(str1);                                                                    //whether they are both positive integers.
            tdelay = atoi(str2);                                                                    //If yes, update sample and tdelay and generate a display
            generateStat(sample,tdelay);
        } else{
            printf("Invalid command\n");
        }
    } else {
        generateStat(sample, tdelay); //This will be executed first and show
        // the default display
    }
}
