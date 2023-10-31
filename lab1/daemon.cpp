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


#include <csignal>

#include <sys/stat.h>


using namespace std;
namespace fs = std::filesystem;

Daemon Daemon::instance;
Daemon& Daemon::getInstance() {
  return instance;
}


void Daemon::SafeChdir (const string& absPath) {
   if (chdir(absPath.c_str()) < 0) {
      syslog(LOG_USER, "Error during chdir: code %i, arg %s", errno, absPath.c_str());
      exit(EXIT_FAILURE);
   }
}


void Daemon::CreateDirectory (const string& absPath, const string& name) {
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


void Daemon::ClearDirectory (const string& absPath) {
   nftw(absPath.c_str(), UnlinkCB, 64, FTW_DEPTH | FTW_PHYS); 
}


vector<string> Daemon::GetContentList (const string& absPath) {
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


void Daemon::CopyFile (const string& absPathSrc, const string& absPathDst) {
   ifstream src(absPathSrc.c_str(), ios::binary);
   if (!src.good()) {
      syslog(LOG_USER, "Copy file goes bad");
      return;
   }   
   ofstream dst(absPathDst.c_str(), ios::binary);
   dst << src.rdbuf();
}


bool Daemon::IsPngFormat (const string& filename) {
   const string extension = ".png";
   if (filename.length() < extension.length()) {
        return false;
   }
   return std::equal(extension.rbegin(), extension.rend(), filename.rbegin());
}


void Daemon::DoDaemonWork (void) {
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


bool Daemon::IsExistDir (const string& absPath) {
   fs::path directory(absPath);
   return (fs::exists(directory) && fs::is_directory(directory));
}


bool Daemon::SubmitNewParams (const string& newSrc, const string& newDst, int newFreq) {
   if (newFreq <= 0 || !IsExistDir(newSrc) || !IsExistDir(newDst)) {
      return false;
   }

   SRC_DIR = newSrc;
   DST_DIR = newDst;
   UPDATE_FREQUENCY = newFreq;

   return true;
}


bool Daemon::LoadConfig (void) {
   string line;
   string newSrc;
   string newDst;
   int newFreq;
  
   fstream in(CONFIG_FILE.c_str());
   if (in.is_open()) {
      getline(in, newSrc);
      getline(in, newDst);
      getline(in, line);
      newFreq = atoi(line.c_str());
      return SubmitNewParams(newSrc, newDst, newFreq);
   } 
   return false;
}


void Daemon::InitPidFile (void) {
   string pidFilePath = WORK_DIR + "/" + PID_FILENAME;
   ofstream f(pidFilePath.c_str());
   syslog(LOG_USER, "pid file created: %s, pid is %i", pidFilePath.c_str(), getpid());  
   
   f << getpid() << endl; 
}


string Daemon::ReadPidFile (void) {
   string pidFilePath = WORK_DIR + "/" + PID_FILENAME;
   fstream f(pidFilePath.c_str());
   if (f.good()) {
      string line;
      getline(f, line);
      return line;
   }   

   return string(""); 
}


void Daemon::KillIfOpened (void) {
   DIR* dir;
   string pid = ReadPidFile();
   string pidFolder = (PROC_DIR + "/" + pid);
   
   if (pid.empty()) {
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


void Daemon::WorkProc () {
   while (true) {
      if (need_work) {
         DoDaemonWork();
      }
      sleep(UPDATE_FREQUENCY);
   }
}


void OnSignalRecieve (int sig) {
   switch (sig) {
      case SIGHUP:
      { 
         Daemon::getInstance().need_work = false;
         syslog(LOG_USER, "Reload daemon's config by signal");

         if (!Daemon::getInstance().LoadConfig()) {
            syslog(LOG_USER, "Bad config file!");         
         }

         Daemon::getInstance().need_work = true;
         break;
      }
      case SIGTERM:
      {
         Daemon::getInstance().need_work = false;
         string pidFilePath = Daemon::getInstance().WORK_DIR + "/" + Daemon::getInstance().PID_FILENAME;
         syslog(LOG_USER, "Terminate daemon by signal");
         unlink(pidFilePath.c_str());
         closelog();
         exit(SIGTERM);
         break;
      }
   }
}


void Daemon::InitDaemon (const std::string& path_to_config) {    
   string configFileName(path_to_config); 
   WORK_DIR = string(get_current_dir_name());
   CONFIG_FILE = WORK_DIR + "/" + configFileName;
   
   SafeChdir("/"); 
   if (!LoadConfig()) {
      cout << "Bad config file" << endl;
      return;
   }

   openlog("daemon_lab", 0, LOG_USER);
 
   umask(0);
   pid_t sid = setsid();
   if (sid < 0) {
      syslog(LOG_USER, "Error while making new session");
      exit(EXIT_FAILURE);
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

}
