#include <string>
#include <vector>

using namespace std;

class Daemon {
private:
  const string PID_FILENAME = "pid_info.txt";
  const string PROC_DIR = "/proc";

  int UPDATE_FREQUENCY;
  string CONFIG_FILE;
  string WORK_DIR;
  string SRC_DIR;
  string DST_DIR;
  bool need_work = true;
  int WAIT_OF_KILLING_TIME = 1;

  static Daemon instance;

  void SafeChdir (const string& absPath);
  void CreateDirectory (const string& absPath, const string& name);
  friend int UnlinkCB (const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf);
  void ClearDirectory (const string& absPath);
  vector<string> GetContentList (const string& absPath);
  void CopyFile (const string& absPathSrc, const string& absPathDst);
  bool IsPngFormat (const string& filename);
  void DoDaemonWork (void);
  bool IsExistDir (const string& absPath);
  bool SubmitNewParams (const string& newSrc, const string& newDst, int newFreq);
  bool LoadConfig (void);
  void InitPidFile (void);
  string ReadPidFile (void);
  void KillIfOpened (void);
  friend void OnSignalRecieve (int sig);
  Daemon() = default;
  
public:
  Daemon(const Daemon&) = delete;
  Daemon& operator=(const Daemon&) = delete;
  static Daemon& getInstance();
  void WorkProc();
  void InitDaemon (const std::string& path_to_config);
};