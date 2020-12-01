#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits>
#include "Commands.h"
#include "signals.h"
#include <limits>


int main(int argc, char* argv[]) {
    SmallShell& smash = SmallShell::getInstance();
    if(signal(SIGTSTP,ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT,ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction sigalarmstruct;
    memset(&sigalarmstruct,0,sizeof(sigalarmstruct));
    sigalarmstruct.sa_handler = sigalarmhandler;
    sigalarmstruct.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &sigalarmstruct, NULL);
    while(true) {
        std::cout << smash.promptDisplay();
        std::string cmd_line;
        std::cin.clear();
        std::fflush(stdin);
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());

    }
    return 0;
}
