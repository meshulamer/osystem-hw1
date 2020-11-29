#include <unistd.h>
#include <string.h>
#include <iostream>
#include <utility>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <dirent.h>
#include <assert.h>
#include <complex>
#include "Commands.h"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BASH_PATH "C:/cygwin64/bin/bash.exe"
//TODO: All syscalls fail need to print the correct thing and exit. find out the correct system call using strace
// TODO: add foreground job vector
//TODO:  add timeuot integration to pipeline
//TODO: background integration to redirection
//TODO: cp commands
//TODO: signals
//TODO: tests
//TODO: get tests from eilon
using namespace std;
void isSpecial(int* redir, int* pipe, char** arg, int arg_size);
const std::string WHITESPACE = " \n\r\t\f\v";
pid_t executePiped(Command* cmd,int* filedes,int channel1, int pipeuse);

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg)

  int JobNuGreaterThen(JobsList::JobEntry &a, JobsList::JobEntry &b){
      return (b.pid - a.pid) > 0;
  }
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}
void SmallShell::chprompt(std::string new_prompt) {
    prompt = std::move(new_prompt);
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    int pipecommand = -1;
    int redirection_command = -1;
    char* arg[COMMAND_MAX_ARGS];
    int arg_size =_parseCommandLine(cmd_line, arg);
    string cmd_s = string(cmd_line);
    cmd_s = _trim(cmd_s);
    isSpecial(&redirection_command, &pipecommand, arg, arg_size);
    Command* rtnCmd = nullptr;
    if(redirection_command != -1){
        rtnCmd = new RedirectionCommand(cmd_line, redirection_command ,this);
    }
    else if(pipecommand != -1){
        rtnCmd = new PipeCommand(cmd_line, pipecommand, this);
    }
    else if(arg_size == 0 || cmd_s.size() == 0){
        rtnCmd = new ExternalCommand(cmd_s.c_str(), arg, 0, this);
    }
    else if (strcmp(arg[0], "pwd") == 0) {
        rtnCmd = new PwdCommand(cmd_line, arg, arg_size, this);
    } else if (strcmp(arg[0], "chprompt") == 0) {
        rtnCmd = new ChpromptCommand(cmd_line, arg, arg_size, this);
    } else if (strcmp(arg[0], "ls") == 0) {
        rtnCmd = new LsCommand(arg[0], this);
    }
    else if (strcmp(arg[0], "showpid") == 0) {
        rtnCmd = new ShowPidCommand(cmd_line);
    }
    else if (strcmp(arg[0], "cd") == 0){
        rtnCmd = new ChangeDirCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "jobs") == 0){
        rtnCmd = new JobsCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "kill") == 0){
        rtnCmd = new KillCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "bg") == 0){
        rtnCmd = new BackgroundCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "fg") == 0) {
        rtnCmd = new ForegroundCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "quit") == 0) {
        rtnCmd = new QuitCommand(cmd_line, arg, arg_size, this);
    }
    else if (strcmp(arg[0], "timeout") == 0){
        rtnCmd = new TimeoutCommand(cmd_line, arg, arg_size, this);
    }
    else{
        rtnCmd = new ExternalCommand(cmd_line, arg, arg_size, this);
    }
    for(int i=0; i<arg_size; i++) {
        assert(arg[i] != NULL);
        free(arg[i]);
    }
    return rtnCmd;
}

void isSpecial(int* redir, int* pipe, char** arg, int arg_size){
    for(int i =0; i< arg_size; i++){
        if(strcmp(arg[i],"|")==0 || strcmp(arg[i],"|&")==0) *pipe = strcmp(arg[i],"|&")==0;
        if(strcmp(arg[i],">")==0 ||strcmp(arg[i],">>")==0) *redir = strcmp(arg[i],">>")==0;
    }
}

std::string SmallShell::promptDisplay() const {
    return prompt;
}

void SmallShell::executeCommand(const char *cmd_line) {
  cleanup();
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  delete cmd;
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::addJob(int pid, time_t startime, char* cmd_line, bool is_timed) {
    job_list.addJob(pid, startime, cmd_line, is_timed);
}

void SmallShell::cleanup() {
    std::vector<int>* removedVec = job_list.removeFinishedJobs();
    for(int & it : *removedVec) {
        removeTimedJob(it);
    }
    delete removedVec;
}

ChpromptCommand::ChpromptCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell){
    if(arg_vec_size == 1){
        prompt = "smash> ";
    }
    else prompt = cmd_arg[1];
}


pid_t ChpromptCommand::execute() {
    shell->chprompt(prompt);
    return 0;
}

LsCommand::LsCommand(const char* cmd_line, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell) {}

pid_t LsCommand::execute() {
    struct dirent **namelist;
    int n;
    n = scandir(".", &namelist, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }
    int i = 2;
    while (i < n) {
        printf("%s\n", namelist[i]->d_name);
        free(namelist[i]);
        i++;
    }
    free(namelist);
    return 0;
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

pid_t ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
    return 0;
}


JobsList::JobEntry::JobEntry(int pid, time_t startime, char *com, bool is_timed) : start_time(startime), pid(pid), is_timed(is_timed),
    is_stopped(false) {
    strcpy(cmd_line, com);
}


void JobsList::addJob(int pid, time_t startime, char* com, bool is_timed) {
    JobEntry new_job = JobEntry(pid, startime, com, is_timed);
    current_max_job_id++;
    new_job.job_id = current_max_job_id;
    new_job.is_finished = false;
    jobs_list.push_back(new_job);
}

std::vector<int>* JobsList::removeFinishedJobs() {
    int retval =0;
    int new_max =0;
    auto* removedVec = new std::vector<int>;
    for (auto it = jobs_list.begin(); it != jobs_list.end();){
        /*if(it->is_stopped){          ///Might not be necesarry and is casuing problems
            ++it;
            continue;
        }*/
        retval = waitpid(it->pid,NULL,WNOHANG);
        if(retval > 0){
            try{
                if(it->is_timed){
                    removedVec->push_back(it->job_id);
                }
                stopped_jobs.remove(it -> pid);
            }
            catch(...){
                ///Element not in the list. Continue
            }
            it = jobs_list.erase(it);
        }
        else if(retval == -1){
            perror("smash error: waitpid failed during status check");
        }
        else{
            if (it->job_id > new_max) new_max = it -> job_id;
            ++it;
        }
        current_max_job_id = new_max;
    }
    if(jobs_list.size()==0){
        current_max_job_id =0;
    }
    else {
        for (auto &job : jobs_list) {
            int max = 0;
            if (job.job_id > max) {
                max = job.job_id;
            }
            current_max_job_id = max;
        }
    }
    return removedVec;
}

JobsList::JobEntry JobsList::getJobById(int jobId) {
    for(auto& job : jobs_list){
        if(job.job_id == jobId){
            return job;
        }
    }
    throw "Doesnt Exist";
}


void JobsList::removeJobById(int job_id) {
    for(auto it = jobs_list.begin(); it != jobs_list.end(); it++){
        if(it->job_id == job_id){
            if(it->is_stopped){
                stopped_jobs.remove(it->job_id);
            }
            jobs_list.erase(it);
            return;
        }
    }
    //throw "Doesnt Exist";
}



int SmallShell::getCurrMaxJobId() {
    return job_list.current_max_job_id;
}


void SmallShell::setCurrMaxJobIdBy(int add_val) {
    job_list.current_max_job_id += add_val;
}


int SmallShell::getJobsListSize(){
    return job_list.jobs_list.size();
}

bool JobsList::JobEntry::IsStopped() {
    return is_stopped;
}

bool JobsList::JobEntry::IsTimed() {
    return is_timed;
}


PwdCommand::PwdCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell){
    char buf[1024];
    try {
        path = getcwd(buf, 1024);
    }
    catch(std::exception &err){
        cout << "ERROR caught something in PwdCommand:"<< err.what();
        exit(1);
    }
}


pid_t PwdCommand::execute() {
    cout << path << endl;
    return 0;
}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell) {
    if(arg_vec_size > 2 ) {
        to_print = "smash error: cd: too many arguments";
        action = SmashError;
    }
    if(arg_vec_size !=2){
        cmd_arg[1]= nullptr;
    }
    if(cmd_arg[1] != nullptr) {
        strcpy(new_dir, cmd_arg[1]);
    }
}

pid_t ChangeDirCommand::execute() {
    if(action == SmashError){
        cout << to_print << endl;
        return 0;
    }
    else if (strcmp(new_dir, "-") == 0) {
        char *old_dir = shell->cdret();
        if (old_dir == nullptr) {
            cout << "smash error: cd: OLDPWD not set" << endl;
            return 0;
        }
        else{
            char buf[128];
            char* current_dir = getcwd(buf,128);
            if (chdir(old_dir) == 0){
                shell->update_old_dir(current_dir);
            }
            else{
                perror("shell error: cd: not able to change directory");
                return 0;
            }
        }
    }
    else {
        char buf[128];
        char* current_dir = getcwd(buf,128);
        if (chdir(new_dir) == 0){
            shell->update_old_dir(current_dir);
        }
        else{
            perror("shell error: cd: not able to change directory");
            return 0;
        }
    }
    return 0;
}

ExternalCommand::ExternalCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell): Command(cmd_line), shell(shell) {
    std::string last_arg = cmd_arg[arg_vec_size-1];
    arg_size = arg_vec_size;
    if(last_arg[last_arg.size()-1] == '&'){
        bg_cmd = true;
    }
}

pid_t ExternalCommand::execute() {
    char exec_arg[COMMAND_ARGS_MAX_LENGTH];
    strcpy(exec_arg, cmd_string);
    if (exec_arg[0] != '\000') {
        _removeBackgroundSign(exec_arg);
    }
    char *execv_args[] = {BASH_PATH, "-c", exec_arg, nullptr};
    time_t startime = time(NULL);
    int pid = fork();
    if (pid == 0) {
        setpgrp();
        if (pipeuse != NOTUSED) {
            close(pipe_args[0]);
            close(pipe_args[1]);
        }
        if (execv(execv_args[0], execv_args) == -1) {
            perror("smash error: execv() failed");
            exit(1);
        }
    } else {
        int job_id = 0;
        if (bg_cmd) {
            shell->addJob(pid, startime, cmd_string, isTimed());
            job_id = shell->getCurrMaxJobId();
        }
        if (isTimed()) {
            time_t closest_time = shell->getClosestAlarmTime();
            shell->addTimed(bg_cmd, startime, pid, job_id, getDuration(), originalString());
            if (getDuration() < closest_time || closest_time == 0) {
                alarm(getDuration());
            }
        }
        if (!bg_cmd) {
            shell->job_in_fg = new JobsList::JobEntry(pid, startime, cmd_string, isTimed());
            if (!bg_cmd && pipeuse == NOTUSED) {
                waitpid(pid, nullptr, WUNTRACED);
                delete shell->job_in_fg;
                shell->job_in_fg = nullptr;
                if (isTimed()) {
                    shell->removeTimedJob(pid);
                }
            }
        }
        return pid;
    }
}

    JobsCommand::JobsCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell):BuiltInCommand (cmd_line), shell(shell) {

    }
    pid_t JobsCommand::execute() {
        shell->printJobs();
        return 0;
    }

    void SmallShell::printJobs() {
        std::sort(job_list.jobs_list.begin(), job_list.jobs_list.end(), JobNuGreaterThen);
        for (int i = 0; i < job_list.jobs_list.size(); i++) {
            cout << "[" << job_list.jobs_list[i].job_id << "] " << job_list.jobs_list[i].cmd_line << " : "
                 << job_list.jobs_list[i].pid
                 << " " << difftime(time(nullptr), job_list.jobs_list[i].start_time) << " secs ";
            if (job_list.jobs_list[i].is_stopped) cout << "(stopped)";
            cout << endl;
        }
    }


    void SmallShell::printBeforeQuit() {
        std::sort(job_list.jobs_list.begin(), job_list.jobs_list.end(), JobNuGreaterThen);
        cout << "sending SIGKILL signal to " << job_list.jobs_list.size() << " jobs:" << endl;
        for (int i = 0; i < job_list.jobs_list.size(); i++) {
            cout << job_list.jobs_list[i].job_id << ": " << job_list.jobs_list[i].cmd_line << endl;
        }
    }

    JobsList::JobEntry SmallShell::getJob(int job_id) {
        return job_list.getJobById(job_id);
    }


    void SmallShell::KillEveryOne() {
        for (auto it = job_list.jobs_list.begin(); it != job_list.jobs_list.end(); it++) {
            kill(it->getPid(), SIGKILL);
        }
    }


    void SmallShell::JobHalted(int jobId) {
        auto it = job_list.jobs_list.begin();
        while (it != job_list.jobs_list.end()) {
            if (it->job_id != jobId) ++it;
            else {
                assert(!it->is_stopped);
                it->is_stopped = true;
                job_list.stopped_jobs.push_back(jobId);
                return;
            }
        }
        throw "Doesnt Exist";
    }
    void SmallShell::JobContinued(int jobId) {
        auto it = job_list.jobs_list.begin();
        while (it != job_list.jobs_list.end()) {
            if (it->job_id != jobId) ++it;
            else {
                assert(it->is_stopped);
                it->is_stopped = false;
                job_list.stopped_jobs.remove(jobId);
                return;
            }
        }
        throw "Doesnt Exist";
    }

    KillCommand::KillCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell):BuiltInCommand (cmd_line), shell(shell) {
        std::string killnum, cmdjob;
        int signum;
        if (cmd_arg[1] != nullptr) {
            killnum = cmd_arg[1];
            if (killnum.find_first_of('-') != 0) {
                status = SyntaxError;
            }
            killnum = killnum.substr(1);
            try {
                signum = std::stoi(killnum);
                signal = signum;
            }
            catch (...) {
                status = SyntaxError;
            }
            if (cmd_arg[2] != nullptr) {
                try {
                    cmdjob = cmd_arg[2];
                    job_id = std::stoi(cmdjob);
                }
                catch (...) {
                    status = SyntaxError;
                }
            }

        }

    }

    pid_t KillCommand::execute() {
        JobsList::JobEntry job;
        if (status == SyntaxError) {
            cout << "smash error: kill: invalid arguments" << endl;
            return 0;
        }
        try {
            job = shell->getJob(job_id);
        }
        catch (...) {
            cout << "smash error: kill: job-id " << job_id << " does not exist" << endl;
            return 0;
        }
        pid_t job_pid = job.getPid();
        int rrr = 0;
        if ((rrr = kill(job_pid, signal)) == -1) {
            perror("smash error: failed to send signal");
            return 0;
        }
        if (signal == SIGSTOP || signal == SIGTSTP) {
            try {
                shell->JobHalted(job_id);
            }
            catch (...) {
                assert(false);/// Cannot happen. we just checked that it exists
                return 0;
            }
        } else if (signal == SIGCONT) {
            try {
                shell->JobContinued(job_id);
            }
            catch (...) {
                assert(false);/// Cannot happen. we just checked that it exists
                return 0;
            }
        }
        else if ((signal == SIGKILL) && job.IsTimed()) {
            try{
                shell->removeTimedJob(job.getPid());
            }
            catch (...){
                assert(false);
                return 0;
            }
        }
        cout << "signal number " << signal << " was sent to pid " << job.getPid() << endl;
        return 0;

    }


    ForegroundCommand::ForegroundCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell) : BuiltInCommand (cmd_line), shell(shell) {
        job_id = shell->getCurrMaxJobId();
        if (arg_vec_size > 2) {
            syntax_error = true;
        }
        if (arg_vec_size == 2) {
            try {
                job_id = stoi(cmd_arg[1]);
            }
            catch (...) {
                syntax_error = true;
            }
        }
    }


    pid_t ForegroundCommand::execute() {
        if (syntax_error) {
            cout << "smash error: fg: invalid arguments" << endl;
            return 0;
        } else if (shell->getJobsListSize() == 0) {
            cout << "smash error: fg: jobs list is empty" << endl;
            return 0;
        }
        JobsList::JobEntry job;
        try {
            job = shell->getJob(job_id);
        }
        catch (...) {
            cout << "smash error: fg: job-id " << job_id << " does not exists" << endl;
            return 0;
        }
        int i = job.getPid();
        if (job.IsStopped()) {
            shell->JobContinued(job_id);
        }
        shell->moveJobToForeground(job_id);
        return 0;
    }

    void SmallShell::moveJobToForeground(int job_id) {
        int result;
        JobsList::JobEntry job = getJob(job_id);
        cout << job.cmd_line << " : " << job.pid << endl;
        kill(job.getPid(), SIGCONT);
        result = waitpid(job.pid, NULL, WCONTINUED);
        if (result == -1) {
            perror("smash error: waitpid failed during status check");
        }
        job_list.removeJobById(job_id);
    }


    BackgroundCommand::BackgroundCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell) : BuiltInCommand (cmd_line), shell(shell) {
        if (cmd_arg[1] == nullptr) {
            job_id = -1;
        } else {
            try {
                job_id = stoi(cmd_arg[1]);
            }
            catch (...) {
                syntax_error = true;
            }
        }

    }
    pid_t BackgroundCommand::execute() {
        if (syntax_error) {
            cout << "smash error: bg: invalid arguments" << endl;
            return 0;
        }
        shell->returnFromBackground(job_id);
        return 0;
    }

    void SmallShell::returnFromBackground(int jobId) {
        JobsList::JobEntry job;
        if (jobId != -1) {  ///Return last stopped job
            try {
                job = getJob(jobId);
            }
            catch (...) {
                cout << "smash error: bg: job-id " << jobId << " does not exist" << endl;
                return;
            }
            if (job.is_stopped) {
                kill(SIGCONT, job.pid);
                JobContinued(jobId);
            } else { ///Job exists but not stopped
                cout << "smash error: bg: job-id " << job.job_id << " is already running in the background" << endl;
                return;
            }
        } else {
            if (job_list.stopped_jobs.empty()) {
                cout << "smash error: bg: there is no stopped jobs to resume" << endl;
                return;
            } else {
                jobId = job_list.stopped_jobs.front();
                try {
                    job = getJob(jobId);
                }
                catch (...) {
                    assert(false);/// Cannot happen. Exists in stop and therefore exists in jobs
                }
                kill(SIGCONT, job.pid);
                JobContinued(jobId);
            }
        }
        cout << job.cmd_line << " : " << job.pid << endl;
    }

    void SmallShell::AlarmTriggered(time_t time) {
        cleanup();
        for (auto it = TimedJobsList.begin(); it != TimedJobsList.end();) {
            if (difftime(time, it->startime) >= it->duration) {
                pid_t pid = it->pid;
                std::string cmd = it->cmd_line;
                if (it->runs_in_background) {
                    try {
                        job_list.removeJobById(it->jobid);
                    }
                    catch (...) {
                        TimedJobsList.erase(it);
                        return;
                    }
                }
                it = TimedJobsList.erase(it);
                if (pid != getpid()) {
                    kill(pid, SIGKILL);
                }
                _rtrim(cmd);
                cout << "smash: got an alarm" << endl;
                cout << cmd << " timed out!" << endl;
                break;
            }
            ++it;
        }
        time_t temp_closest = getClosestAlarmTime();
        if (temp_closest != 0) {
            alarm(temp_closest);
        }
    }

    QuitCommand::QuitCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell) : BuiltInCommand (cmd_line), shell(shell) {
        kill_flag = false;
        if (arg_vec_size > 1) {
            std::string second_arg = cmd_arg[1];
            if (second_arg == "kill") {
                kill_flag = true;
            }
        }
    }

    pid_t QuitCommand::execute() {
        if (kill_flag) {
            shell->printBeforeQuit();
            shell->KillEveryOne();
        }
        kill(getpid(), SIGKILL);
        return 0;
    }

    RedirectionCommand::RedirectionCommand(
    const char *cmd_line,
    int append, SmallShell
    *shell) : Command (cmd_line), append(append), shell(shell) {
        std::string cmd1 = std::string(cmd_line);
        std::string cmd2 = std::string(cmd_line);
        int last_not_space = cmd2.find_last_not_of(" ");
        if(cmd2[last_not_space] == '&'){
            is_background = true;
            cmd2 = cmd2.substr(0,last_not_space);
        }
        cmd1 = cmd1.substr(0, cmd1.find_first_of(">") - 1);
        output_path = cmd2.substr(cmd2.find_last_of(">") + 1);
        char temp_cmd[COMMAND_ARGS_MAX_LENGTH];
        strcpy(temp_cmd, cmd1.c_str());
        cmd = shell->CreateCommand(temp_cmd);
        for (int i = 0; i < output_path.size(); i++) {
            if (output_path.find_first_of(' ') == 0) {
                output_path = output_path.substr(1);
            } else {
                break;
            }
        }
    }

    pid_t RedirectionCommand::execute() {
        if(is_background){
            cmd->setToBgState();
        }
        int stdout_copy = dup(1);
        close(1);
        int oflags = append ? (O_WRONLY | O_APPEND | O_CREAT) : O_WRONLY | O_CREAT;
        int cflag = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IXOTH;
        int fdt_i = open(output_path.c_str(), oflags, 0666);
        assert(fdt_i == 1);
        if (-1 != fdt_i) {
            cmd->execute();
        } else {
            perror("smash error: opening file failed");
        }
        close(1);
        dup2(stdout_copy, 1);
        close(stdout_copy);
        return 0;
    }

    TimeoutCommand::TimeoutCommand(
    const char *cmd_line,
    char **cmd_arg,
    int arg_vec_size, SmallShell
    *shell): BuiltInCommand (cmd_line),
    shell(shell) {
        if (arg_vec_size < 2) {
            syntax_error = true;
        } else {
            try {
                duration = stoi(cmd_arg[1]);
            }
            catch (...) {
                syntax_error = true;
            }
            std::string tempstr = cmd_line;
            int pos = tempstr.find_first_of(cmd_arg[1]);
            pos += std::string(cmd_arg[1]).size();
            tempstr = tempstr.substr(pos);
            tempstr = _ltrim(tempstr);
            cmd = shell->CreateCommand(tempstr.c_str());
            cmd->setIsTimed(duration, cmd_line);
        }
        if (arg_vec_size == 2) {
            no_command = true;
        }
    }

    pid_t TimeoutCommand::execute() {
        if (syntax_error) {
            cout << "smash error: Timeout command syntax error" << endl;
            return 0;
        }
        if (no_command) {
            cout << "smash error: Timeout command empty command" << endl;
            return 0;
        }
        cmd->execute();
    }

    void SmallShell::addTimed(bool runs_in_backround, time_t timestamp, pid_t pid, int job_id, unsigned int duration,
                              std::string cmd_line) {
        TimedJob temp_timed_job = TimedJob(runs_in_backround, timestamp, pid, job_id, duration, cmd_line);
        TimedJobsList.push_back(temp_timed_job);
    }

    void SmallShell::removeTimedJob(int job_pid) {
        for (auto it = TimedJobsList.begin(); it != TimedJobsList.end();) {
            if (it->pid == job_pid) {
                TimedJobsList.erase(it);
                return;
            }
            it++;
        }
    }

    time_t SmallShell::getClosestAlarmTime() {
        if (TimedJobsList.size() == 0)
            return 0;
        time_t temp_closest_diff_time =
                TimedJobsList.begin()->duration - (difftime(time(NULL), TimedJobsList.begin()->startime));
        for (auto it = TimedJobsList.begin(); it != TimedJobsList.end(); it++) {
            time_t curr_diff = it->duration - difftime(time(NULL), it->startime);
            temp_closest_diff_time = (curr_diff < temp_closest_diff_time) ? curr_diff : temp_closest_diff_time;
        }
        return temp_closest_diff_time;
    }

    PipeCommand::PipeCommand(
    const char *cmd_line,
    int index_of_pipe_sign, SmallShell
    *shell)
    : Command (cmd_line), shell(shell) {
        std::string cmd1 = std::string(cmd_line);
        std::string cmd2 = std::string(cmd_line);
        if (index_of_pipe_sign == 1) {
            type = ERROR;
        }
        int index_of_split = cmd1.find_first_of(" |");
        cmd1 = cmd1.substr(0, index_of_split);
        bool additional_string_sub = type == ERROR;
        cmd2 = cmd2.substr(cmd2.find_last_of("|") + 1 + additional_string_sub);
        _trim(cmd1);
        _trim(cmd2);
        for (int i = 0; i < cmd2.size(); i++) {
            if (cmd2.find_first_of(' ') == 0) {
                cmd2 = cmd2.substr(1);
            } else {
                break;
            }
        }
        this->cmd1 = shell->CreateCommand(cmd1.c_str());
        this->cmd2 = shell->CreateCommand(cmd2.c_str());
    }

    pid_t PipeCommand::execute() {
        int filedes[2] = {0, 0};
        int return_val;
        return_val = pipe(filedes);//TODO: syscall erro pprint to perror and end attempt at pipe
        if (type == STANDARD) {
            cmd1->Piped(PipeUse::CMDSTDOUT, filedes);
            cmd2->Piped(PipeUse::CMDSTDIN, filedes);
        } else {
            cmd1->Piped(PipeUse::CMDSTDOUT, filedes);
            cmd2->Piped(PipeUse::CMDSTDERROUT, filedes);
        }
        int write_channel;
        int read_channel;
        if (type == STANDARD) {
            write_channel = STDOUT_FILENO;
            read_channel = STDIN_FILENO;
        } else {
            write_channel = STDERR_FILENO;
            read_channel = STDIN_FILENO;
        }
        pid_t pid1 = executePiped(cmd1, filedes, write_channel, 1);
        pid_t pid2 = executePiped(cmd2, filedes, read_channel, 0);
        close(filedes[0]);
        close(filedes[1]);
        if (!cmd2->inBackground()) {
            if(pid1 != 0){
                waitpid(pid1, nullptr, WUNTRACED);
            }
            if(pid2 != 0){
                waitpid(pid2, nullptr, WUNTRACED);
            }
        }
        return 0;
    }
    pid_t executePiped(Command *cmd, int *filedes, int channel1, int pipeuse) {
        int copy_channel = dup(channel1);
        close(channel1); /// Prepare write channel and execute first program
        int duped_to = dup(filedes[pipeuse]);
        pid_t pid = cmd->execute();
        close(channel1);/// Fix father FDT
        duped_to = dup(copy_channel);
        close(copy_channel);
        return pid;
    }