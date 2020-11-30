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
            exit(EXIT_FAILURE);
        }
        if(smash.job_in_fg->is_timed) {
            smash.removeTimedJob(fg_pid);
        }
        std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
    }
}

void ctrlZHandler(int singal) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smash = SmallShell::getInstance();
    if(smash.job_in_fg == nullptr) {
        return;
    }
    pid_t fg_pid = smash.job_in_fg->getPid();
    if(fg_pid != getpid()) {
        if(kill(fg_pid, SIGSTOP)) {
            perror("smash error: kill failed");
            exit(EXIT_FAILURE);
        }
        JobsList::JobEntry fg_job = *smash.job_in_fg;
        smash.job_list.addJob(fg_job.getPid(), fg_job.start_time, fg_job.cmd_line, fg_job.is_timed);
        cout << "smash: process " << fg_pid << " was stopped" << endl;
    }
}
