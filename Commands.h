#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>
#include <time.h>
#include <list>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)
#define MAX_BUFFER_LENGTH (128)
class SmallShell;
class Command {
public:
    char cmd_string[COMMAND_ARGS_MAX_LENGTH];
    Command(const char *cmd_line){
        strcpy(cmd_string,cmd_line);
    }
    virtual ~Command() {};
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup(){};
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line): Command(cmd_line) {};
  virtual ~BuiltInCommand() {};
};

class ExternalCommand : public Command {
private:
    int arg_size;
    bool bg_cmd = false;
    SmallShell* shell;
public:
    ExternalCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
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

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
private:
    SmallShell* shell;
    bool kill_flag;
public:
  QuitCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList {

public:
    class JobEntry {
        time_t start_time;
        int job_id;
        int pid;
        bool is_finished;
        bool is_stopped;
        char cmd_line[COMMAND_ARGS_MAX_LENGTH];
    public:
        friend class JobsList;
        friend class SmallShell;
        friend int JobNuGreaterThen(JobsList::JobEntry &a, JobsList::JobEntry &b);
        int getPid(){return pid;};
        bool IsStopped();



    };
    // TODO: Add your data members
private:
    int current_max_job_id = 0;
    std::vector<JobEntry> jobs_list;
    std::list<int> stopped_jobs;
public:
    friend class SmallShell;
    JobsList(){
        current_max_job_id = 0;
    };
    ~JobsList()= default;
    void addJob(int pid, time_t startime, char* com);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry getJobById(int jobId);
    void removeJobById(int job_id);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    friend int JobNuGreaterThen(JobsList::JobEntry &a, JobsList::JobEntry &b);
};

class JobsCommand : public BuiltInCommand {
    SmallShell* shell;
 public:
  JobsCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    typedef enum{Ready,JobError,SyntaxError} Status;
private:
    SmallShell* shell;
    int signal;
    int job_id = -1;
    Status status = Ready;

 public:
  KillCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 SmallShell* shell;
 int job_id;
 bool syntax_error = false;
 public:
  ForegroundCommand(const char* cmd_line, char** cmd_arg, int arg_vec_size, SmallShell* shell);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    SmallShell* shell;
    int job_id;
    bool syntax_error = false;
 public:
  BackgroundCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
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
    void cleanup();
    void printJobs();
    void chprompt(std::string new_prompt);
    Command *CreateCommand(const char *cmd_line);
    int getCurrMaxJobId();
    void setCurrMaxJobIdBy(int add_val);
    int getJobsListSize();
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void JobHalted(int jobId);
    void KillEveryOne();
    void printBeforeQuit();
    void JobContinued(int jobId);
    void addJob(int pid, time_t startime, char* cmd_line);
    void returnFromBackground(int jobId);
    void moveJobToForeground(int job_id);
    char *cdret() {
        if (old_dir_exist) {
            return old_dir;
        }
        return nullptr;
    }
    void update_old_dir(char *updated_old_dir) {
        strcpy(old_dir, updated_old_dir);
        this->old_dir_exist = true;
    }
    std::string promptDisplay() const;
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    JobsList::JobEntry getJob(int job_id);
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
