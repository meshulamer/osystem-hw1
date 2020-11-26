#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

void sigalarmhandler(int signal);
void MyctrlCHandler(int signal);
void MyctrlZHandler(int signail);
SmallShell* shell_access = nullptr;

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    SmallShell& smash = SmallShell::getInstance();
    shell_access = &smash;
    struct sigaction sigalarmstruct{{sigalarmhandler},SA_NODEFER,SA_RESTART};
    sigaction(SIGALRM, &sigalarmstruct, NULL);
    struct sigaction ctrlzstruct{{MyctrlZHandler},SA_RESTART};
    sigaction(SIGTSTP, &ctrlzstruct, NULL);
    struct sigaction ctrlcstruct{{MyctrlCHandler},SA_RESTART};
    sigaction(SIGINT, &ctrlcstruct, NULL);
    while(true) {
        std::cout << smash.promptDisplay();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}

void sigalarmhandler(int signal){
    shell_access -> AlarmTriggered(time(NULL));
}

void MyctrlCHandler(int signal){
    if(shell_access->job_in_fg != nullptr && shell_access->job_in_fg->getPid() != getpid()) {
        kill(shell_access->job_in_fg->getPid(), SIGKILL);
        if(shell_access->job_in_fg->is_timed) {
            shell_access->removeTimedJob(shell_access->job_in_fg->getPid());
        }
        std::cout << "smash: process " << shell_access->job_in_fg->getPid() << " was killed" << std::endl;
    }
}

void MyctrlZHandler(int signail) {

}