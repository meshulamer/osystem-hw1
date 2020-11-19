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

#define BASH_PATH "C:/cygwin64/bin/bash.exe"
using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

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
    char* arg[20];
    int arg_size =_parseCommandLine(cmd_line, arg);
    string cmd_s = string(cmd_line);
    cmd_s = _trim(cmd_s);
    Command* rtnCmd = nullptr;
    if (cmd_s.find("pwd") == 0) {
        rtnCmd = new PwdCommand(cmd_line,arg, arg_size, this);
    } else if (cmd_s.find("chprompt") == 0) {
        rtnCmd = new ChpromptCommand(cmd_line,arg, arg_size, this);
    } else if (cmd_s.find("ls") == 0) {
        rtnCmd = new LsCommand(arg[0], this);
    }
    else if (cmd_s.find("showpid") == 0) {
        rtnCmd = new ShowPidCommand(cmd_line);
    }
    else if (cmd_s.find("cd") == 0){
        rtnCmd = new ChangeDirCommand(cmd_line,arg, arg_size, this);
    }
    else if (cmd_s.find("jobs") == 0){
        rtnCmd = new JobsCommand(cmd_line,arg, arg_size, this);
    }
    else if (cmd_s.find("kill") == 0){
        rtnCmd = new KillCommand(cmd_line,arg, arg_size, this);
    }
    else if (cmd_s.find("bg") == 0){
        rtnCmd = new BackgroundCommand(cmd_line,arg, arg_size, this);
    }
    else{
        rtnCmd = new ExternalCommand(cmd_line,arg, arg_size, this);
    }
    for(int i=0; i<arg_size; i++) {
        assert(arg[i] != NULL);
        free(arg[i]);
    }
    return rtnCmd;
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

void SmallShell::addJob(int pid, time_t startime, char* cmd_line) {
    job_list.addJob(pid, startime, cmd_line);
}

void SmallShell::cleanup() {
    job_list.removeFinishedJobs();
}

ChpromptCommand::ChpromptCommand(const char* cmd_line, char** cmd_arg , int arg_vec_size, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell){
    if(arg_vec_size == 1){
        prompt = "smash> ";
    }
    else prompt = cmd_arg[1];
}


void ChpromptCommand::execute() {
    shell->chprompt(prompt);
}

LsCommand::LsCommand(const char* cmd_line, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell) {}

void LsCommand::execute() {
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
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void ShowPidCommand::execute() {
    cout << getpid() << endl;
}

void JobsList::addJob(int pid, time_t startime, char* com) {
    JobEntry new_job = JobEntry();
    new_job.start_time = startime;
    new_job.pid = pid;
    current_max_job_id++;
    new_job.job_id = current_max_job_id;
    new_job.is_finished = false;
    new_job.is_stopped = false;
    strcpy(new_job.cmd_line, com);
    jobs_list.push_back(new_job);
}

void JobsList::removeFinishedJobs() {
    int retval =0;
    int new_max =0;
    for (auto it = jobs_list.begin(); it != jobs_list.end();){
        /*if(it->is_stopped){          ///Might not be necesarry and is casuing problems
            ++it;
            continue;
        }*/
        retval = waitpid(it->pid,NULL,WNOHANG);
        if(retval > 0){
            try{
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

}

JobsList::JobEntry JobsList::getJobById(int jobId) {
    for(auto& job : jobs_list){
        if(job.job_id == jobId){
            return job;
        }
    }
    throw "Doesnt Exist";
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


void PwdCommand::execute() {
    cout << path << endl;
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

void ChangeDirCommand::execute() {
    if(action == SmashError){
        cout << to_print << endl;
        return;
    }
    else if (strcmp(new_dir, "-") == 0) {
        char *old_dir = shell->cdret();
        if (old_dir == nullptr) {
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        else{
            char buf[128];
            char* current_dir = getcwd(buf,128);
            if (chdir(old_dir) == 0){
                shell->update_old_dir(current_dir);
            }
            else{
                perror("shell error: cd: not able to change directory");
                return;
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
            return;
        }
    }
}

ExternalCommand::ExternalCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell): Command(cmd_line), shell(shell) {
    std::string last_arg = cmd_arg[arg_vec_size-1];
    arg_size = arg_vec_size;
    if(last_arg[last_arg.size()-1] == '&'){
        bg_cmd = true;
    }
}

void ExternalCommand::execute() {
    char exec_arg[COMMAND_ARGS_MAX_LENGTH];
    strcpy(exec_arg,cmd_string);
    if(exec_arg[0] != '\000'){
        _removeBackgroundSign(exec_arg);
    }
    char *execv_args[] = {BASH_PATH, "-c", exec_arg, nullptr};
    time_t startime = time(NULL);
    int pid = fork();
    if (pid == 0) {
        if (execv(execv_args[0], execv_args) == -1) {
            perror("smash error: execv() failed");
            exit(1);
        }
    }
    else {
        if (!bg_cmd) {
            waitpid(pid, nullptr, 0);
        }
        else{
            shell->addJob(pid, startime, cmd_string);
        }
    }
}

JobsCommand::JobsCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell):BuiltInCommand(cmd_line),shell(shell) {

}
void JobsCommand::execute() {
    shell->printJobs();
}

void SmallShell::printJobs() {
    std::sort(job_list.jobs_list.begin(),job_list.jobs_list.end(),JobNuGreaterThen);
    for(int i=0; i< job_list.jobs_list.size(); i++){
        cout<< "[" << job_list.jobs_list[i].job_id << "] "<< job_list.jobs_list[i].cmd_line << " : " << difftime(time(
                nullptr),job_list.jobs_list[i].start_time) <<" secs ";
        if (job_list.jobs_list[i].is_stopped) cout << "(stopped)";
        cout<< endl;
    }
}

JobsList::JobEntry SmallShell::getJob(int job_id) {
    return job_list.getJobById(job_id);
}

void SmallShell::JobHalted(int jobId) {
    auto it = job_list.jobs_list.begin();
    while(it != job_list.jobs_list.end()){
        if( it->job_id != jobId) ++it;
        else{
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

KillCommand::KillCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell):BuiltInCommand(cmd_line),shell(shell) {
    std::string killnum, cmdjob;
    int signum;
    if(cmd_arg[1] != nullptr){
        killnum = cmd_arg[1];
        if(killnum.find_first_of('-') != 0){
            status = SyntaxError;
        }
        killnum = killnum.substr(1);
        try{
            signum = std::stoi(killnum);
            signal = signum;
        }
        catch(...){
            status = SyntaxError;
        }
        if(cmd_arg[2] != nullptr){
            try{
                cmdjob = cmd_arg[2];
                job_id = std::stoi(cmdjob);
            }
            catch(...){
                status = SyntaxError;
            }
        }

    }


}

void KillCommand::execute(){
    JobsList::JobEntry job;
    if(status == SyntaxError){
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    try{
        job = shell->getJob(job_id);
    }
    catch(...){
        cout << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }
    pid_t job_pid = job.getPid();
    if(kill(job_pid,signal)==-1){
        perror("smash error: failed to send signal");
        return;
    }
    if(signal == SIGSTOP || signal ==  SIGTSTP){
        try{
            shell->JobHalted(job_id);
        }
        catch(...){
            assert(false);/// Cannot happen. we just checked that it exists
            return;
        }
    }
    else if(signal == SIGCONT){
        try{
            shell->JobContinued(job_id);
        }
        catch(...){
            assert(false);/// Cannot happen. we just checked that it exists
            return;
        }
    }
    cout << "signal number " << signal << " was sent to pid " << job.getPid() << endl;

}


ForegroundCommand::ForegroundCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell) : BuiltInCommand(cmd_line), shell(shell) {
    job_id = shell->getCurrMaxJobId();
    if (arg_vec_size > 1) {
        try {
            job_id = stoi(cmd_arg[1]);
        }
        catch (...){
            syntax_error = true;
        }
    }
    if(arg_vec_size > 2) syntax_error = true;
}


void ForegroundCommand::execute() {
    if(syntax_error){
        cout << "smash error: fg: invalid arguments" << endl;
    }
    else if(shell->getJobsListSize() == 0){
        cout << "smash error: fg: jobs list is empty" << endl;
    }
    //else if(//the job doest not exists do you shit)

}

BackgroundCommand::BackgroundCommand(const char *cmd_line, char **cmd_arg, int arg_vec_size, SmallShell *shell) : BuiltInCommand(cmd_line),shell(shell) {
    if(cmd_arg[1] == nullptr){
        job_id = -1;
    }
    else{
        try{
            job_id = stoi(cmd_arg[1]);
        }
        catch(...) {
            syntax_error = true;
        }
    }

}
void BackgroundCommand::execute() {
    if (syntax_error) {
        cout << "smash error: bg: invalid arguments" << endl;
        return;
    }
    shell-> returnFromBackground(job_id);
}

void SmallShell::returnFromBackground(int jobId) {
    JobsList::JobEntry job{};
    if(jobId != -1){  ///Return last stopped job
        try{
            job = getJob(jobId);
        }
        catch(...){
            cout << "smash error: bg: job-id " << jobId << " does not exist" << endl;
            return;
        }
        if(job.is_stopped){
            kill(SIGCONT,job.pid);
            JobContinued(jobId);
        }
        else{ ///Job exists but not stopped
            cout << "smash error: bg: job-id " << job.job_id <<" is already running in the background" << endl;
        }
        return;
    }
    else{
        if(job_list.stopped_jobs.empty()){
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
        else{
            jobId = job_list.stopped_jobs.front();
            try{
                job = getJob(jobId);
            }
            catch(...){
                assert(false);/// Cannot happen. Exists in stop and therefore exists in jobs
            }
            kill(SIGCONT,job.pid);
            JobContinued(jobId);

        }
    }
}