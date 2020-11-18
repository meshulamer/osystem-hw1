#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
    std::string cmd_line;
 public:
  Command(const char* cmd_line): cmd_line(cmd_line) {};
  virtual ~Command() {};
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line): Command(cmd_line) {};
  virtual ~BuiltInCommand() {};
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory();
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
        Command* cmd;
        int job_id;
        bool is_finished;
        bool is_stopped;
    public:
        friend class JobsList;
    };
    // TODO: Add your data members
private:
    int current_max_job_id;
    std::vector<JobEntry> jobs_list;
public:
    JobsList();
    ~JobsList();
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout ?

class SmallShell {
 private:
    std::string prompt = "smash> ";
    char old_dir[128];
    bool old_dir_exist = false;
    JobsList job_list = JobsList();
  SmallShell();
 public:
  void chprompt(std::string new_prompt);
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  char* cdret() {
      if (old_dir_exist) {
          return old_dir;
      }
      return nullptr;
  }
  void update_old_dir(char* updated_old_dir){
      strcpy(old_dir, updated_old_dir);
      this->old_dir_exist = true;
  }
  std::string promptDisplay() const;
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};

class ChpromptCommand : public BuiltInCommand{
private:
    std::string prompt;
    SmallShell* shell;
public:
    ChpromptCommand(const char* cmd_line, char** cmd_arg, int size, SmallShell* shell);
    ~ChpromptCommand() override = default;
    void execute() override;
};


class LsCommand : public BuiltInCommand {
private:
    SmallShell* shell;
    std::vector<std::string> files_vector;
public:
    LsCommand(const char* cmd_line, SmallShell* shell);
    ~LsCommand() override  = default;
    void execute() override;
};

class PwdCommand : public BuiltInCommand{
private:
    SmallShell* shell;
    std::string path;
public:
    PwdCommand(const char* cmd_line, char** cmd_arg, int size, SmallShell* shell);
    ~PwdCommand() override = default;
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
private:
    std::string to_print;
    typedef enum{Ready, SmashError} Action;
    Action action = Ready;
    SmallShell* shell;
    char new_dir[128];
public:
    ChangeDirCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell);
    virtual ~ChangeDirCommand() {};
    void execute() override;
};

#endif //SMASH_COMMAND_H_
