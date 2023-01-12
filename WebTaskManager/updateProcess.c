#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// variable
FILE *processID, *processInfo, *processName, *processLoad, *psrFile,
    *allowedCPUsFile;
char pid[100], commandName[100], loadOfCpu[100], psr[100], allowedCPUs[100];
char command1[120], command2[120], command3[120], command4[120];

// Functions
void readTxt(FILE *file, char target[120], char save[100]) {
  char fileData[100];

  file = fopen(target, "r");
  if (file == NULL) {
    processInfo = fopen("./data_files/updatedinfo.json", "w");
    fprintf(processInfo, "[\"%s\", \"error\"]", pid); // This will be used for creating log.
    exit(EXIT_FAILURE);
  } else {
    fgets(fileData, 100, file);
    fclose(file);
  }

  strcpy(save, fileData);
}

char *removeSpacesFromStr(char *string) {
  int nonSpaceCount = 0;

  for (int i = 0; string[i] != '\0'; i++) {
    if (string[i] != ' ') {
      string[nonSpaceCount] = string[i];
      nonSpaceCount++;
    }
  }

  string[nonSpaceCount] = '\0';
  return string;
}

void removeLastChar(char string[]) {
  int size = strlen(string);  // Total size of string
  string[size - 1] = '\0';
}

void getAC() {
  sprintf(command1, "ps -o psr -p %d > ./data_files/psr.txt", atoi(pid));
  sprintf(command2, "taskset -pc %d > ./data_files/allowedcpus.txt", atoi(pid));

  system(command1);
  system(command2);

  psrFile = fopen("./data_files/psr.txt", "r");
  fgets(psr, 100, psrFile);
  fgets(psr, 100, psrFile);
  fclose(psrFile);

  allowedCPUsFile = fopen("./data_files/allowedcpus.txt", "r");
  fgets(allowedCPUs, 100, allowedCPUsFile);
  fclose(allowedCPUsFile);

  removeSpacesFromStr(psr);
  removeLastChar(psr);
  removeLastChar(allowedCPUs);
}

void getLoad() {
  sprintf(command3,
          "top -b -n 2 -d 1 -p %d | tail -1 | awk '{print $9}' > "
          "./data_files/load.txt",
          atoi(pid));
  system(command3);

  readTxt(processLoad, "./data_files/load.txt", loadOfCpu);
  removeLastChar(loadOfCpu);
  getAC();
}

void getName() {
  sprintf(command2, "/proc/%s/comm", pid);
  readTxt(processName, command2, commandName);
  removeLastChar(commandName);
  getLoad();
}

void getPID() {
  readTxt(processID, "./data_files/pinfo.txt", pid);
  removeLastChar(pid);
  getName();
}

int main(int argc, char const *argv[]) {
  if (argv[1]) {
    strcpy(pid, argv[1]);
    getName();

  } else {
    getPID();
  }

  if (0) {
    
  }

  // create JSON
  processInfo = fopen("./data_files/updatedinfo.json", "w");
  fprintf(processInfo,
          "[{\"pid\" : \"%s\", \"command\" : \"%s\", \"load\" : \"%s\", "
          "\"currentcpu\" : \"%s\", \"allowedcpu\" : \"%s\"}]",
          pid, commandName, loadOfCpu, psr, allowedCPUs);

  return 0;
}
