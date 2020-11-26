#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <utility>
#include <vector>
#include <string.h>
#include <time.h>
#include <list>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MAX_BUFFER_LENGTH (128)
class SmallShell;
class Command {
public:
    typedef enum{NOTUSED,CMDSTDOUT,CMDSTDIN,CMDSTDERRIN,CMDSTDERROUT} PipeUse;
    int pipe_args[2] = {0,0};
    PipeUse pipeuse = NOTUSED;
private:
    bool is_timeout_command = false;
    unsigned int duration = 0;
    std::string original_command;
    bool will_start_in_background = false;
public:
    char cmd_string[COMMAND_ARGS_MAX_LENGTH];
    Command(const char *cmd_line){
        strcpy(cmd_string,cmd_line);
    }
    virtual ~Command() {};
    virtual pid_t execute() = 0;
    bool isTimed(){
        return is_timeout_command;
    }
    void setIsTimed(unsigned int set_duration, std::string original_cmd_line){
        is_timeout_command = true;
        duration = set_duration;
        original_command = original_cmd_line;
    }
    unsigned int getDuration(){
        return duration;
    }
    std::string originalString(){
        return original_command;
    }
    void Piped(PipeUse use, const int* filedes){
        pipeuse = use;
        pipe_args[0] = filedes[0];
        pipe_args[1] = filedes[1];
    }
    bool virtual inBackground(){
        return false;
    }
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
    pid_t execute() override;
    bool virtual inBackground() override{
        return bg_cmd;
    }
};

class PipeCommand : public Command {
public:
    typedef enum {STANDARD,ERROR} Type;
private:
    Type type = STANDARD;
    Command* cmd1 = nullptr;
    Command* cmd2 = nullptr;
    SmallShell* shell;
public:
    PipeCommand(const char *cmd_line, int index_of_pipe_sign, SmallShell *shell);
    virtual ~PipeCommand() {
        if(cmd1) {
            delete cmd1;
        }
        if(cmd2) {
            delete cmd2;
        }
    }
    pid_t execute() override;
};

class RedirectionCommand : public Command {
    bool append = false;
    SmallShell *shell;
    Command* cmd;
    std::string output_path;
 public:
  explicit RedirectionCommand(const char* cmd_line, int index_of_redir_sign, SmallShell* shell);
  virtual ~RedirectionCommand() {
      if(cmd != nullptr){
          delete cmd;
      }
  }
  pid_t execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  pid_t execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
private:
    SmallShell* shell;
    bool kill_flag;
public:
  QuitCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
  virtual ~QuitCommand() {}
  pid_t execute() override;
};

class TimedJob{
public:
    bool runs_in_background;
    time_t startime;
    pid_t pid;
    int jobid;
    unsigned int duration;
    std::string cmd_line;
public:
    TimedJob(bool background, time_t startime, pid_t pid, int jobid, unsigned int time_to_stop, std::string cmd_line) : runs_in_background(background),
    startime(startime), pid(pid), jobid(jobid), duration(time_to_stop), cmd_line(std::move(cmd_line)) {};
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
        bool is_timed = false;
    public:
        friend class JobsList;
        friend class SmallShell;
        friend class ExternalCommand;
        friend void MyctrlCHandler(int signal);
        friend int JobNuGreaterThen(JobsList::JobEntry &a, JobsList::JobEntry &b);
        int getPid(){return pid;};
        bool IsStopped();
        JobEntry(int pid, time_t startime, char* com, bool is_timed);
        JobEntry() = default;
        JobEntry& operator=(const JobEntry& other) = default;


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
    void addJob(int pid, time_t startime, char* com, bool is_timed);
    void printJobsList();
    void killAllJobs();
    std::vector<int>* removeFinishedJobs();
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
  pid_t execute() override;
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
  pid_t execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 SmallShell* shell;
 int job_id;
 bool syntax_error = false;
 public:
  ForegroundCommand(const char* cmd_line, char** cmd_arg, int arg_vec_size, SmallShell* shell);
  virtual ~ForegroundCommand() {}
  pid_t execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    SmallShell* shell;
    int job_id;
    bool syntax_error = false;
 public:
  BackgroundCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
  virtual ~BackgroundCommand() {}
  pid_t execute() override;
};

class TimeoutCommand : public BuiltInCommand {
    SmallShell* shell;
    bool syntax_error = false;
    bool no_command = false;
    Command* cmd = nullptr;
    unsigned int duration;
public:
    TimeoutCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell);
    virtual ~TimeoutCommand() {
        if(cmd != nullptr){
            delete cmd;
        }
    }
    pid_t execute() override;
};
// maybe ls, timeout ?

class SmallShell {
private:
    std::string prompt = "smash> ";
    char old_dir[128];
    bool old_dir_exist = false;
    JobsList job_list = JobsList();
    std::vector<TimedJob> TimedJobsList;
    JobsList::JobEntry* job_in_fg = nullptr;
    SmallShell();

public:
    friend class ExternalCommand;
    friend void MyctrlCHandler(int signal);
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
    void addJob(int pid, time_t startime, char* cmd_line, bool is_timed);
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
    void AlarmTriggered(time_t time);
    void addTimed(bool runs_in_backround, time_t timestamp,pid_t pid,int job_id, unsigned int duration, std::string cmd_line);
    void removeTimedJob(int job_pid);
    time_t getClosestAlarmTime();
};

class ChpromptCommand : public BuiltInCommand{
private:
    std::string prompt;
    SmallShell* shell;
public:
    ChpromptCommand(const char* cmd_line, char** cmd_arg, int size, SmallShell* shell);
    ~ChpromptCommand() override = default;
    pid_t execute() override;
};


class LsCommand : public BuiltInCommand {
private:
    SmallShell* shell;
    std::vector<std::string> files_vector;
public:
    LsCommand(const char* cmd_line, SmallShell* shell);
    ~LsCommand() override  = default;
    pid_t execute() override;
};

class PwdCommand : public BuiltInCommand{
private:
    SmallShell* shell;
    std::string path;
public:
    PwdCommand(const char* cmd_line, char** cmd_arg, int size, SmallShell* shell);
    ~PwdCommand() override = default;
    pid_t execute() override;
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
    pid_t execute() override;
};

#endif //SMASH_COMMAND_H_
