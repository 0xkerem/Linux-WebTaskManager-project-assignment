#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "civetweb.h"

// Variables
FILE *html, *css, *javascript, *load, *countFile;
char htmlContent[10000], cssContent[4000], jsContent[22000], cpuCount[20];
int cpuArray[10], uArray0[16], tArray0[16], uArray1[16], tArray1[16];
double cpuLoadArray[16];
int i = 0;

// Functions
char *readEntireFile(FILE *f, char target[100]) {
  f = fopen(target, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;
  return string;
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

void readTxt(FILE *file, char target[120], char save[]) {
  char fileData[100];

  file = fopen(target, "r");
  fgets(fileData, 100, file);
  fclose(file);

  strcpy(save, fileData);
}

void removeLastChar(char string[]) {
  int size = strlen(string);  // Total size of string
  string[size - 1] = '\0';
}

void createCpuArray(int cpuArray[10], char ld[150]) {
  char *l2Line = ld;
  for (int i = 0; i < 10; i++) {
    char *nextLine = strchr(l2Line, '\n');

    if (nextLine) {
      *nextLine = '\0';
    }

    cpuArray[i] = atoi(l2Line);

    if (nextLine) {
      *nextLine = '\n';
    }

    if (nextLine) {
      l2Line = nextLine + 1;
    } else {
      l2Line = NULL;
    }
  }
}

void terminateProcess(int pid) { kill(pid, SIGSEGV); }

void writeData(FILE *file, double data[], char target[100]) {
  file = fopen(target, "w");

  char json[500];
  char jsonData[1000];

  sprintf(json, "[\"%s\",", cpuCount);
  for (int i = 0; i < atoi(cpuCount); i++) {
    if (i == atoi(cpuCount) - 1) {
      sprintf(jsonData, "%s\"%.2lf\"]", json, data[i]);
      strcpy(json, jsonData);
    } else {
      sprintf(jsonData, "%s\"%.2lf\",", json, data[i]);
      strcpy(json, jsonData);
    }
  }

  fprintf(file, "%s", jsonData);
  fclose(file);
}

int setData(int ua[], int ta[]) {
  char la[150], lb[150];
  i = 0;
  FILE *file = fopen("/proc/stat", "r");
  while (i <= atoi(cpuCount)) {
    if (fgets(la, 150, file) == NULL) {
      break;
    }

    if (i == 0) {
      i++;
      continue;
    }

    strcpy(lb, la);
    memmove(&lb[0], &lb[5], strlen(lb));
    for (char *p = lb; (p = strchr(p, ' ')) != NULL; p++) {
      *p = '\n';
    }
    int array[10], u, t;
    createCpuArray(array, lb);
    u = array[0] + array[2];
    t = array[0] + array[2] + array[3];
    ua[i - 1] = u;
    ta[i - 1] = t;
    i++;
  }
  i = 0;
}

char *loadJSON(char target[], char result[]) {
  FILE *f = fopen(target, "r");
  fgets(result, 150, f);
  fclose(f);
  return result;
}

// Sections

// CPU Load

void cpuLoad() {
  sleep(1);
  setData(uArray0, tArray0);

  sleep(1);

  setData(uArray1, tArray1);

  for (int i = 0; i < atoi(cpuCount); i++) {
    cpuLoadArray[i] = (((double)uArray1[i] - (double)uArray0[i]) * 100) /
                      (double)(tArray1[i] - (double)tArray0[i]);
    if (cpuLoadArray[i] >= 100) {
      cpuLoadArray[i] = 100;
    }
  }

  writeData(load, cpuLoadArray, "./data_files/load.json");
}

void refreshLoadAvg() {
  FILE *loadAvg, *file;
  char la[30];

  loadAvg = fopen("/proc/loadavg", "r");

  fgets(la, 5, loadAvg);
  fclose(loadAvg);

  file = fopen("./data_files/loadavg.json", "w");

  fprintf(file, "[{\"loadavg\" : \"Load average in last 1 minute: %s\"}]", la);

  fclose(file);
}

void refreshUptime() {
  FILE *uptimeFile, *file;
  char uptimeChr[30];
  int uptime = 0;

  if ((uptimeFile = fopen("/proc/uptime", "r")) == NULL) {
    perror("supt"), exit(EXIT_FAILURE);
  }

  fgets(uptimeChr, 12, uptimeFile);
  fclose(uptimeFile);

  uptime = strtol(uptimeChr, NULL, 10);

  int hour = uptime / 3600;
  int minute = (uptime % 3600) / 60;

  file = fopen("./data_files/uptime.json", "w");

  if (hour == 0 && minute == 0) {
    fprintf(file, "[{\"uptime\" : \"System up for %d hour %d minute\"}]", hour,
            minute);
  } else if (hour == 0 && minute != 0) {
    fprintf(file, "[{\"uptime\" : \"System up for %d hour %d minutes\"}]", hour,
            minute);
  } else if (hour != 0 && minute == 0) {
    fprintf(file, "[{\"uptime\" : \"System up for %d hours %d minute\"}]", hour,
            minute);
  } else if (hour != 0 && minute != 0) {
    fprintf(file, "[{\"uptime\" : \"System up for %d hours %d minutes\"}]",
            hour, minute);
  }

  fclose(file);
}

// Server
static const char *html_form = htmlContent;

static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *ri = mg_get_request_info(conn);
  char post_data[1024], input1[sizeof(post_data)], input2[sizeof(post_data)];
  int post_data_len;

  if (!strcmp(ri->uri, "/")) {
    mg_printf(conn,
              "HTTP/1.0 200 OK\r\n"
              "Content-Length: %d\r\n"
              "Content-Type: text/html\r\n\r\n%s",
              (int)strlen(html_form), html_form);
  } else if (!strcmp(ri->uri, "/style.css")) {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/css\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(cssContent), cssContent);
  } else if (!strcmp(ri->uri, "/app.js")) {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/x-javascript\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(jsContent), jsContent);
  } else if (!strcmp(ri->uri, "/refreshcpuload")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    cpuLoad();
  } else if (!strcmp(ri->uri, "/data_files/load0.json")) {
    char loadresult[150];
    loadJSON("./data_files/load0.json", loadresult);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(loadresult), loadresult);
  } else if (!strcmp(ri->uri, "/data_files/load1.json")) {
    char loadresult[150];
    loadJSON("./data_files/load1.json", loadresult);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"  // Always set Content-Length
              "\r\n"
              "%s",
              (int)strlen(loadresult), loadresult);
  } else if (!strcmp(ri->uri, "/refreshloadavg")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    refreshLoadAvg();
  } else if (!strcmp(ri->uri, "/data_files/loadavg.json")) {
    char loadresult[150];
    loadJSON("./data_files/loadavg.json", loadresult);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"  // Always set Content-Length
              "\r\n"
              "%s",
              (int)strlen(loadresult), loadresult);
  } else if (!strcmp(ri->uri, "/refreshuptime")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    refreshUptime();
  } else if (!strcmp(ri->uri, "/data_files/uptime.json")) {
    char loadresult[150];
    loadJSON("./data_files/uptime.json", loadresult);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"  // Always set Content-Length
              "\r\n"
              "%s",
              (int)strlen(loadresult), loadresult);
  } else if (!strcmp(ri->uri, "/reboot")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    if (ri->query_string) {
      char command[100];
      sprintf(command, "shutdown -r +%d", atoi(ri->query_string));
      system(command);
    } else {
      system("reboot");
    }
  } else if (!strcmp(ri->uri, "/shutdown")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    if (ri->query_string) {
      char command[100];
      sprintf(command, "shutdown -h +%d", atoi(ri->query_string));

      system(command);
    } else {
      system("poweroff");
    }
  } else if (!strcmp(ri->uri, "/cancel")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    system("shutdown -c");
  } else if (!strcmp(ri->uri, "/runcommand")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    char command[150];
    char queryDecoded[100];
    mg_url_decode(ri->query_string, strlen(ri->query_string), queryDecoded, 100,
                  1);
    sprintf(command, "%s & echo $! > ./data_files/pinfo.txt", queryDecoded);
    system(command);
    system("./runCommand");

  } else if (!strcmp(ri->uri, "/terminate")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    terminateProcess(atoi(ri->query_string));
  } else if (!strcmp(ri->uri, "/terminatebyname")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    char command[100];
    sprintf(command, "pkill %s", ri->query_string);
    system(command);
  } else if (!strcmp(ri->uri, "/updateprocess")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");
    int pidval;
    if (ri->query_string) {
      char command[150];
      sprintf(command, "./updateProcess %s", ri->query_string);
      system(command);
    } else {
      system("./updateProcess");
    }
  } else if (!strcmp(ri->uri, "/affinity")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");

    char str[30], aff[30], command[100];
    int pid;
    strcpy(str, ri->query_string);
    char delim[] = "&";

    char *ptr = strtok(str, delim);
    int i = 0;
    while (ptr != NULL) {
      if (i == 0) {
        pid = atoi(ptr);
      } else if (i == 1) {
        strcpy(aff, ptr);
      }
      ptr = strtok(NULL, delim);
      i++;
    }

    sprintf(command, "sudo chrt -o -p 0 %d", pid);
    system(command);

    sprintf(command, "taskset -pc %s %d", aff, pid);
    system(command);

  } else if (!strcmp(ri->uri, "/limitcpu")) {
    mg_printf(conn, "HTTP/1.1 200 OK\r\n");

    char str[30], command[100];
    int pid, limit;
    strcpy(str, ri->query_string);
    char delim[] = "&";

    char *ptr = strtok(str, delim);
    int i = 0;
    while (ptr != NULL) {
      if (i == 0) {
        limit = atoi(ptr);
      } else if (i == 1) {
        pid = atoi(ptr);
      }
      ptr = strtok(NULL, delim);
      i++;
    }

    sprintf(command, "sudo -- sh -c './limitCpuLoad %d %d'", limit, pid);
    system(command);

  } else if (!strcmp(ri->uri, "/data_files/processinfo.json")) {
    char loadresult5[150];
    loadJSON("./data_files/processinfo.json", loadresult5);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(loadresult5), loadresult5);
  } else if (!strcmp(ri->uri, "/data_files/updatedinfo.json")) {
    char loadresult[150];
    loadJSON("./data_files/updatedinfo.json", loadresult);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(loadresult), loadresult);
  } else if (!strcmp(ri->uri, "/data_files/load.json")) {
    char loadresult5[150];
    loadJSON("./data_files/load.json", loadresult5);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              (int)strlen(loadresult5), loadresult5);
  }
  return 1;
}

int main(void) {
  strcpy(htmlContent, readEntireFile(html, "./frontend/index.html"));
  strcpy(cssContent, readEntireFile(css, "./frontend/style.css"));
  strcpy(jsContent, readEntireFile(javascript, "./frontend/app.js"));

  // CPU count
  system("nproc > ./data_files/cpucount.txt");
  readTxt(countFile, "./data_files/cpucount.txt", cpuCount);
  removeLastChar(cpuCount);

  struct mg_context *ctx;
  const char *options[] = {"listening_ports", "8080", NULL};
  struct mg_callbacks callbacks;

  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = begin_request_handler;
  ctx = mg_start(&callbacks, NULL, options);
  system("xdg-open http://localhost:8080");
  system("sleep 1");
  system("clear");
  getchar();
  getchar();
  
  mg_stop(ctx);
  return 0;
}