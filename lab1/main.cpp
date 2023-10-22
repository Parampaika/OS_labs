#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include <dirent.h>
#include <ftw.h>
#include <errno.h>

#include <sys/stat.h>

using namespace std;


const string PID_FILENAME = "pid_info.txt"; 
const string PROC_DIR = "/proc";

int UPDATE_FREQUENCY;

string CONFIG_FILE;
string WORK_DIR;

string SRC_DIR;
string DST_DIR;

bool NEED_WORK = true;
int WAIT_OF_KILLING_TIME = 1;


void SafeChdir (const string& absPath) {
   if (chdir(absPath.c_str()) < 0) {
      syslog(LOG_USER, "Error during chdir: code %i, arg %s", errno, absPath.c_str());
      exit(EXIT_FAILURE);
   }
}


void CreateDirectory (const string& absPath, const string& name) {
   string currentPath(get_current_dir_name()); 

   SafeChdir(absPath);

   int mkdirErr = mkdir(name.c_str(), S_IRWXU);
   if (mkdirErr && errno != EEXIST) {
      syslog(LOG_USER, "Something goes wrong while creating directory (CreatingDirectory), stop executing");
      exit(EXIT_FAILURE);
   }   
   
   SafeChdir("/");
   SafeChdir(currentPath.c_str());
}


int UnlinkCB (const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
   int rv;

   if (ftwbuf->level == 0) {
      return 0;
   }

   rv = remove(fpath);
   if (rv) {
      perror(fpath);
      syslog(LOG_USER, "Something goes wrong while deleting files (UnlinkCB), stop executing");
      exit(EXIT_FAILURE);
   }  

   return rv;
}


void ClearDirectory (const string& absPath) {
   nftw(absPath.c_str(), UnlinkCB, 64, FTW_DEPTH | FTW_PHYS); 
}


vector<string> GetContentList (const string& absPath) {
   DIR* dir;
   struct dirent* ent;
   vector<string> res;

   if ((dir = opendir(absPath.c_str())) != NULL) {
      while ((ent = readdir(dir)) != NULL) {
         if (ent->d_type == DT_DIR) {
            continue;
         }
         res.push_back(ent->d_name);
      }
      closedir(dir);
   } else {
      return vector<string>();
   }
   return res;
}


void CopyFile (const string& absPathSrc, const string& absPathDst) {
   ifstream src(absPathSrc.c_str(), ios::binary);

   if (!src.good()) {
      syslog(LOG_USER, "Copy file goes bad");
      return;
   }   

   ofstream dst(absPathDst.c_str(), ios::binary);
   dst << src.rdbuf();
}


bool IsPngFormat (const string& filename) {
   int last = filename.size() - 1;
   return (filename.size() > 4 && filename[last] == 'g' && filename[last - 1] == 'n' && filename[last - 2] == 'p' && filename[last - 3] == '.');
}


void DoDaemonWork (void) {
   ClearDirectory(DST_DIR + "/IMG");
   ClearDirectory(DST_DIR + "/OTHERS");
   
   vector<string> srcContent = GetContentList(SRC_DIR);
   for (unsigned i = 0; i < srcContent.size(); i++) {
      string filename = srcContent[i];
      string srcFilePath = SRC_DIR + "/" + filename;
      string dstFilePath;
      
      if (IsPngFormat(filename)) {
         dstFilePath = DST_DIR + "/IMG/" + filename;
      } else {
         dstFilePath = DST_DIR + "/OTHERS/" + filename;
      }

      CopyFile(srcFilePath, dstFilePath);
   }
}


bool IsExistDir (const string& absPath) {
   DIR* dir = opendir(absPath.c_str());
   if (dir) {
      closedir(dir);
      return true;
   }
   return false;
}


bool SubmitNewParams (const string& newSrc, const string& newDst, int newFreq) {
   if (newFreq <= 0 || !IsExistDir(newSrc) || !IsExistDir(newDst)) {
      return false;
   }

   SRC_DIR = newSrc;
   DST_DIR = newDst;
   UPDATE_FREQUENCY = newFreq;

   return true;
}


bool LoadConfig (void) {
   string line;
   string newSrc;
   string newDst;
   int newFreq;
  
   fstream in(CONFIG_FILE.c_str());
   if (in.is_open()) {
      getline(in, line);
      newSrc = line;
      getline(in, line);
      newDst = line;
      getline(in, line);
      newFreq = atoi(line.c_str());
      
      if (!SubmitNewParams(newSrc, newDst, newFreq)) {
         return false;
      }
      return true;
   } 
   return false;
}


void InitPidFile (void) {
   string pidFilePath = WORK_DIR + "/" + PID_FILENAME;
   ofstream f(pidFilePath.c_str());
   syslog(LOG_USER, "pid file created: %s, pid is %i", pidFilePath.c_str(), getpid());  
   
   f << getpid() << endl; 
}


string ReadPidFile (void) {
   string pidFilePath = WORK_DIR + "/" + PID_FILENAME;
   fstream f(pidFilePath.c_str());
   if (f.good()) {
      string line;
      getline(f, line);
      return line;
   }   

   return string(""); 
}


void KillIfOpened (void) {
   DIR* dir;
   string pid = ReadPidFile();
   string pidFolder = (PROC_DIR + "/" + pid);
   
   if (pid == "") {
      return;
   }

   syslog(LOG_USER, "Deamon already works - kill him");
   if ((dir = opendir(pidFolder.c_str())) != NULL) {
      kill(atoi(pid.c_str()), SIGTERM);  
   }

   while ((dir = opendir(pidFolder.c_str())) != NULL) {
      sleep(WAIT_OF_KILLING_TIME);
   }
}


void WorkProc (void) {
   while (true) {
      if (NEED_WORK) {
         DoDaemonWork();
      }
      sleep(UPDATE_FREQUENCY);
   }
}


void OnSignalRecieve (int sig) {
   switch (sig) {
      case SIGHUP:
      { 
         NEED_WORK = false;
         syslog(LOG_USER, "Reload daemon's config by signal");

         if (!LoadConfig()) {
            syslog(LOG_USER, "Bad config file!");         
         }

         NEED_WORK = true;
         break;
      }
      case SIGTERM:
      {
         NEED_WORK = false;
         string pidFilePath = WORK_DIR + "/" + PID_FILENAME;
         syslog(LOG_USER, "Terminate daemon by signal");
         unlink(pidFilePath.c_str());
         closelog();
         exit(SIGTERM);
         break;
      }
   }
}


void InitDaemon (void) {    
   openlog("daemon_lab", 0, LOG_USER);
 
   umask(0);
   pid_t sid = setsid();
   if (sid < 0) {
      syslog(LOG_USER, "Error while making new session");
      exit(EXIT_FAILURE);
   }

   int pid = fork();
   if (pid == -1) {
      syslog(LOG_USER, "Error while initing daemon");
      exit(EXIT_FAILURE);
   } else if (pid) {
      return;
   }

   KillIfOpened();
   InitPidFile();

   CreateDirectory(DST_DIR, "IMG");
   CreateDirectory(DST_DIR, "OTHERS");
   ClearDirectory(DST_DIR + "/IMG");
   ClearDirectory(DST_DIR + "/OTHERS");

   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);
  
   signal(SIGHUP, OnSignalRecieve);   
   signal(SIGTERM, OnSignalRecieve);    

   WorkProc(); 
}


int main (int argc, char** argv) {
   if (argc != 2) {
      cout << "Bad argument to launch daemon: needed config file name" << endl;
      return -1;
   }     

   string configFileName(argv[1]); 
   WORK_DIR = string(get_current_dir_name());
   CONFIG_FILE = WORK_DIR + "/" + configFileName;
   
   SafeChdir("/"); 
   if (!LoadConfig()) {
      cout << "Bad config file" << endl;
      return -1;
   }

   int pid = fork();
   if (pid == -1) {
      cout << "Error: start daemon failed" << endl;
      return -1;
   } else if (!pid) {
      InitDaemon();
   }
   return 0;
}
