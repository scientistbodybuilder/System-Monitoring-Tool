Encountered Problems

CPU usage: 

To calculate the total CPU use, I planned to use the recommended /proc/stat file. This file displays multiple rows cpu, cpu0, cpu1, … and each each has 10 columns, which represent the time spent by the CPU in different states. I found that I could find the total CPU usage by performing a calculation using all of the columns by searching online at https://www.youtube.com/shorts/YMiCJl_Uncg.
I created a function called totalCPU, and the key design was to implement a way to parse a string into a substring at a specified character and store all the substrings inside a 2D array. I was able to do this using the strtok() function in the string library, which allowed me to obtain an array of all the column values, which I then used to turn the strings into integers and perform operations on them. 

Command line prompts:

The command line prompts ‘--sample=N’ and ‘--tdelay=T’ can have T and N be any positive integer. Because these commands are one argument but contain two pieces of information, I needed a way to extract the pieces of information separately before moving forward. To help with this, I developed the split() function, which relies on strtok() and is specifically designed to separate a string into two substrings at the ‘=’ character. From there, I can simply check whether the substrings match the format of the two command line prompts, and perform necessary actions.

Screen Refreshing:

This was likely the biggest problem I had, and I spent several hours just thinking about the problem, and trying to understand how things would need to work for it to be possible. What helped me find a solution was researching ESCape codes. After I had a decent understanding of how escape codes work, I implemented a function to clear the screen and reset the cursor, which created the “refreshing” part. Then, to load the samples of the memory usage, I thought of a method involving a fixed number of lines, where some of the lines are statements and the others are ‘\n’, where the ratio between them would dynamically change. I brought this to life through the implementation of the newLine() and refreshMem() functions. Once these three functions were complete, I was able to create a display resembling a refreshing screen. It is not 100% accurate, so this is a problem I am still working on. 

Function Overview

void clearScreen()
This function “resets” the screen, clears anything displayed on the current screen, and moves the cursor back to the top left corner.

Void newLine(int n) 
This function receives an int argument, and iterates for the duration of that argument, printing a new line character each iteration.

refreshMem(int n)
This function receives an int argument, and iterates for the duration of that argument, calling the function MemInfo() each iteration.

Void refreshCPU()
This function calculates the percent difference in cpu usage between a current sample and a base sample, as int (0.01 > 1) and float representation. It then calls cpuGraphic() to create a graphic representation of the change.

Void MemInfo() 
 This is a void function that retrieves the total RAM and the free RAM (physical and virtual) by the <sys/sysinfo.h> library and then calculates and prints the used RAM over the total RAM according to a specified format.
	
void UsersInfo()
 This void function retrieves and prints the username, terminal, and hosting location of any logged-in users on the system by using <utmp.h> library.
	
void OSInfo() 
This void function will retrieve and print system information from the <sys/utname.h> library. and calculates and prints the time since the last reboot using information from the <sys/sysinfo.h> library.

int coresCount() 
This function accesses the ‘/proc/cpuinfo’ file to count the number of cores in the system, before returning them as an int.

float totalCPU()
This function accesses the ‘/proc/stat’ file to calculate the total CPU usage based on the CPU statistics and return the value as a float.

void memoryUsage()
Retrieves and prints the current memory usage using the <sys/resource.h> library.

void generateStat(int sample, int tdelay)
 accepts arguments of sample (int) and tdelay(). It prints formatting statements for the system page that include formatting arguments of sample, tdelay, coresCount(), totalCPU(), and memoryUsage(). It also calls functions clearScreen, refreshMem, newLine, UserInfo, and OSInfo.
	
void generateSystemStat(int sample, int tdelay)
Accepts arguments of sample (int) and tdelay(). It prints formatting statements for the system page that include formatting arguments of sample, tdelay, coresCount(), totalCPU(), and memoryUsage(). It also calls functions clearScreen, refreshMem, newLine, and OSInfo.

void generateUserStat(int sample, int tdelay)
Accepts arguments of sample (int) and tdelay(). It prints formatting statements for the system page that include formatting arguments of sample, tdelay, coresCount(), totalCPU(), and memoryUsage(). It also calls functions UserInfo and OSInfo.

void printMemoryInfo(int it) 
This function prints out only the Memory information by calling the MemInfo() function and displaying the current iteration using the ‘it’ argument.

void printUserInfo(int it)
This function prints out only the User information by calling the UserInfo() function and displaying the current iteration using the ‘it’ argument.

void sequence(int it) 
This function sequentially prints out the memory information (by calling printMemoryInfo(int it)), user information (by calling printUserInfo(int it)), and OS information (by calling OSInfo()) with a 1 second delay between calls. It passes its ‘it’ argument into the printMemoryInfo and printUserInfo.

int countSeparators(char* string, char ch)
 This function receives a string and a character and counts the number of occurrences of that character within the string. It then returns the count.
	
int checkInt(char* string)
A function that receives a string, and checks whether the string is an integer number, returning 1 if all characters are integers and 0 otherwise.

void split(char* command, char** arr)
The function receives a string command, and a pointer to an array of strings. The ultimate purpose is to determine if the command is of the format ‘substring’ ‘=’ ‘substring’, where neither substring contains ‘=’. If that is the case, then the function will insert the two substrings into the array

cpuGraphic(int percent_diff_int, float percent_diff_float)
This function receives the percent difference between the total cpu usage of different samples as an int (whole number, so 0.11 -> 11) and as a float (0.11). It calls printGraphicElement() if the percent is > 0, otherwise it prints the percent with a single ‘|’ graphic representation.

printGraphicElement(int num_times, float percent_diff_float)
This function receives an integer, representing the percent increase as a whole number, and a float representing the percentage difference in CPU usage. It prints out a representation of the percent as ‘|’ characters.

generateGraphic(int sample, int tdelay)
This function receives two integer arguments and creates a display similar to generateStat() by taking sample samples of the memory and cpu usage, with tdelay time in between. It also generates a graphic for the CPU usage between different samples. 

Int main(int argc, char **argv)
The main function of this program is to direct what to display on the terminal based on the command line arguments it receives. If there is only one argument (argc==1), it will display the default information page. If not, it checks whether there are two arguments, and if so either displays only system information, or user information, sequentially displays information, updates the sample size, or updates the tdelay, depending on whether the argument is ‘--system’, ‘--user’, ‘--sequential’, ‘--sample=N”, or ‘--tdelay=T” respectively. N and T represent integers.

Use Instructions

To view system utilization metrics with a default of 10 samples and a 1 second tdelay, compile and execute the file without any command line arguments.

To view the system utilization metrics with custom sample size and tdelay, use the following command line arguments
‘--sample=N’ will change the sample size, where N is any positive integer
‘--tdelay=T’ will change the delay value, where T is any positive integer
‘N T’ where N and T are both positive integers

To view system information only, use the command line argument
‘--system’ after compilation

To view user information only, use the command line argument
‘--user’ after compilation

To view the system utilization information sequentially instead of as refreshing, use the command line argument ‘--sequential’ after compilation

To view a graphical representation of the cpu usage between samples, use the command line argument ‘--graphics’ or ‘-g’
