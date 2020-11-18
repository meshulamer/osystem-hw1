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

#define EXEC(path, arg) \
  execvp((path), (arg));

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

    if (cmd_s.find("pwd") == 0) {
        return new PwdCommand(cmd_line,arg, arg_size, this);
    } else if (cmd_s.find("chprompt") == 0) {
        return new ChpromptCommand(cmd_line,arg, arg_size, this);
    } else if (cmd_s.find("ls") == 0) {
        return new LsCommand(arg[0], this);
    }
    else if (cmd_s.find("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    if (cmd_s.find("cd") == 0){
        return new ChangeDirCommand(cmd_line,arg, arg_size, this);
    }
    for(int i=0; i<arg_size; i++) {
        assert(arg[i] != NULL);
        free(arg[i]);
    }
    return nullptr;
}

std::string SmallShell::promptDisplay() const {
    return prompt;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  Command* cmd = CreateCommand(cmd_line);
  cmd->execute();
  delete cmd;
  // Please note that you must fork smash process for some commands (e.g., external commands....)
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

JobsList::JobsList() : current_max_job_id(0), jobs_list() {}

void JobsList::addJob(Command* cmd, bool isStopped) {
    JobEntry new_job;
    new_job.cmd = cmd;
    new_job.job_id = ++current_max_job_id;
    new_job.is_finished = false;
    new_job.is_stopped = isStopped;
    jobs_list.push_back(new_job);
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