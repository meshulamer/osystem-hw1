#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <dirent.h>
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
    prompt = new_prompt;
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
        //return new GetCurrDirCommand(cmd_line);
    } else if (cmd_s.find("chprompt") == 0) {
        return new ChpromptCommand(arg, arg_size, this);
    } else if (cmd_s.find("ls") == 0) {
        return new LsCommand(arg[1], this);
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
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

ChpromptCommand::ChpromptCommand(char** cmd_arg , int arg_vec_size, SmallShell* shell): BuiltInCommand(cmd_arg[0]), shell(shell){
    if(arg_vec_size == 1){
        prompt = "smash> ";
    }
    else prompt = cmd_arg[1];
}


void ChpromptCommand::execute() {
    shell->chprompt(prompt);
}

LsCommand::LsCommand(const char* cmd_line, SmallShell* shell): BuiltInCommand(cmd_line), shell(shell){
    FILE *proc = popen("/bin/ls -al","r");
    char buf[1024];
    while ( !feof(proc) && fgets(buf,sizeof(buf),proc) )
    {
        files_vector.emplace_back(buf);
    }
}


void LsCommand::execute() {
    for(int i = 0 ; i < files_vector.size() ; i++){
        cout << files_vector[i] << endl;
    }
}
