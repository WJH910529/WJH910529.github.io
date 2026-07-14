#include <signal.h>
#include <stdio.h>

#define PI 3.141592653589793

static volatile sig_atomic_t work_pending = 0;
static double requested_radius = 0.0;

static void simulated_timer_isr(void)
{
    work_pending = 1;
}

static void process_deferred_work(void)
{
    double area = PI * requested_radius * requested_radius;

    printf("main: deferred area for radius %.1f = %.3f\n",
           requested_radius, area);
}

int main(void)
{
    requested_radius = 3.0;
    puts("main: trigger simulated interrupt");
    simulated_timer_isr();

    printf("main: pending flag = %d\n", (int)work_pending);
    if (work_pending != 0) {
        work_pending = 0;
        process_deferred_work();
    }
    printf("main: pending flag after work = %d\n", (int)work_pending);

    return 0;
}
