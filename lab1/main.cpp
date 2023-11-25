#include "daemon.h"

#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <filesystem>
#include <algorithm>

#include <dirent.h>
#include <ftw.h>
#include <errno.h>

#include <sys/stat.h>

using namespace std;



int main (int argc, char** argv) {
   if (argc != 2) {
      cout << "Bad argument to launch daemon: needed config file name" << endl;
      return -1;
   }     

   auto& daemon = Daemon::getInstance();
   int pid = fork();
   if (pid == -1) {
    cout << "error: start daemon failed" << endl;
    return -1;
   }
   else if(!pid) {
    daemon.InitDaemon(argv[1]);
    daemon.WorkProc();
   } 
   
   return 0;
}