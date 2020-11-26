#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

void sigalarmhandler(int signal);
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
//    sigalarmstruct.sa_handler = sigalarmhandler;
//    sigalarmstruct.sa_sigaction = NULL;
//    sigalarmstruct.sa_mask = SA_NODEFER;
//    sigalarmstruct.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sigalarmstruct, NULL);

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
