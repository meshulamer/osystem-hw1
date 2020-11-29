#include <iostream>
#include <signal.h>
#include <time.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>


using namespace std;

void sigalarmhandler(int signal){
    SmallShell& smash = SmallShell::getInstance();
    smash.AlarmTriggered(time(NULL));
}

void ctrlCHandler(int signal) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell& smash = SmallShell::getInstance();
    if(smash.job_in_fg == nullptr)
        return;
    pid_t fg_pid = smash.job_in_fg->getPid();
    if(fg_pid != getpid()) {
        if(kill(fg_pid, SIGKILL)) {
            perror("smash error: kill failed");
        }
        if(smash.job_in_fg->is_timed) {
            smash.removeTimedJob(fg_pid);
        }
        std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
    }
}

void MyctrlZHandler(int signail) {

}
