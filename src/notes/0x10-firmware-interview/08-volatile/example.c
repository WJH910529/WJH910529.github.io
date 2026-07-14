#include <signal.h>
#include <stdio.h>

static volatile sig_atomic_t event_flag;
static volatile int simulated_status = 3;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    event_flag = 1;
}

static void simulate_hardware_update(void)
{
    ++simulated_status;
}

int main(void)
{
    int first_read;
    int second_read;
    int snapshot;

    if (signal(SIGINT, handle_signal) == SIG_ERR) {
        fputs("failed to install signal handler\n", stderr);
        return 1;
    }

    printf("event flag before signal: %d\n", (int)event_flag);
    if (raise(SIGINT) != 0) {
        fputs("failed to raise signal\n", stderr);
        return 1;
    }
    printf("event flag after signal : %d\n", (int)event_flag);

    first_read = simulated_status;
    simulate_hardware_update();
    second_read = simulated_status;
    printf("two reads: %d then %d, product = %d\n",
           first_read,
           second_read,
           first_read * second_read);

    simulated_status = 3;
    snapshot = simulated_status;
    simulate_hardware_update();
    printf("one snapshot: %d, square = %d, register is now %d\n",
           snapshot,
           snapshot * snapshot,
           simulated_status);

    return 0;
}
