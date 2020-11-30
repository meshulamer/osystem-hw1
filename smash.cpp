#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits>
#include "Commands.h"
#include "signals.h"



int main(int argc, char* argv[]) {
    SmallShell& smash = SmallShell::getInstance();
    if(signal(SIGTSTP,ctrlZHandler)==SIG_ERR) {
        std::cout << "fuk off" << std::endl;
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT,ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction sigalarmstruct{{sigalarmhandler},SA_NODEFER,SA_RESTART};
    sigaction(SIGALRM, &sigalarmstruct, NULL);
    /*struct sigaction ctrlzstruct{{MyctrlZHandler},SA_NODEFER,SA_RESTART};
    sigaction(SIGTSTP, &ctrlzstruct, NULL);
    struct sigaction ctrlcstruct{{MyctrlCHandler},SA_NODEFER,SA_RESTART};
    sigaction(SIGINT, &ctrlcstruct, NULL);*/
    while(true) {
        std::cout << smash.promptDisplay();
        std::string cmd_line;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max());
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}
