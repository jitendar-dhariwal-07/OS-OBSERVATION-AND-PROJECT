/* TSCOS - Traffic Signal Control Operating System (simulation)
 * Usage:  ./tscos        run the scripted five-scenario demo
 *         ./tscos -i     interactive mode (type "help")
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
 
#define NL 4
typedef enum { RED, YELLOW, GREEN } State;
static const char *LN[] = {"NORTH", "SOUTH", "EAST", "WEST"};
static const char *SN[] = {"RED", "YELLOW", "GREEN"};
 
typedef struct { int id; State state; int density; int priority; int remaining; } PCB;
 
/* Shared-memory structure written by sensor module, read by scheduler */
typedef struct {
    int density[NL];
    int emergency_lane;   /* -1 = none */
    int ped_lane;         /* -1 = none */
    int manual_lane;      /* -1 = none, 4 = all-red hold */
    int rush_hour;
} Shared;
 
static PCB lane[NL];
static pthread_mutex_t intersection_lock = PTHREAD_MUTEX_INITIALIZER;
static int clk = 8 * 3600;
 
static void say(const char *tag, const char *fmt, ...) {
    va_list a; va_start(a, fmt);
    printf("[%-10s] [%02d:%02d:%02d] ", tag, clk/3600, (clk/60)%60, clk%60);
    vprintf(fmt, a); printf("\n"); va_end(a);
}
 
static void show(const char *extra) {
    printf("              N:%-6s S:%-6s E:%-6s W:%-6s %s\n",
        SN[lane[0].state], SN[lane[1].state], SN[lane[2].state], SN[lane[3].state], extra);
}
 
/* Mutual exclusion: only one approach may be GREEN at a time */
static int safe_to_green(int l) {
    for (int i = 0; i < NL; i++) if (i != l && lane[i].state != RED) return 0;
    return 1;
}
 
static void give_green(int l, int secs, const char *why) {
    pthread_mutex_lock(&intersection_lock);
    if (!safe_to_green(l)) { say("SAFETY", "Conflict blocked for %s", LN[l]); pthread_mutex_unlock(&intersection_lock); return; }
    lane[l].state = GREEN; lane[l].remaining = secs;
    say("SIGNAL SET", "%s: GREEN (%ds) [%s]", LN[l], secs, why);
    show("");
    pthread_mutex_unlock(&intersection_lock);
    clk += secs;
}
 
static void end_green(int l) {
    pthread_mutex_lock(&intersection_lock);
    lane[l].state = YELLOW; say("TRANSITION", "%s: YELLOW (5s)", LN[l]); clk += 5;
    lane[l].state = RED;    say("TRANSITION", "%s: RED", LN[l]);
    pthread_mutex_unlock(&intersection_lock);
}
 
static int dyn_green(int d, int rush) {
    if (!rush) return 30;
    int g = 20 + (d * 2) / 3;           /* time quantum grows with load */
    return g > 60 ? 60 : g;
}
 
static void scheduler_step(Shared *s) {
    /* Step 5-6: manual override (Level 1) */
    if (s->manual_lane >= 0) {
        say("OPERATOR", "Manual override received. Automatic scheduler suspended.");
        if (s->manual_lane < NL) { lane[s->manual_lane].state = GREEN;
            say("SIGNAL SET", "%s: GREEN (locked by operator) | OTHERS: RED", LN[s->manual_lane]); show(""); }
        return;
    }
    /* Step 7-8: emergency preemption (Level 2) */
    if (s->emergency_lane >= 0) {
        say("RULE MATCH", "Step 7-8: Emergency vehicle on %s. Preempting cycle.", LN[s->emergency_lane]);
        give_green(s->emergency_lane, 25, "EMERGENCY_PREEMPT");
        say("INFO", "Emergency vehicle cleared intersection.");
        end_green(s->emergency_lane); s->emergency_lane = -1; return;
    }
    /* Step 9-10: pedestrian phase */
    if (s->ped_lane >= 0) {
        say("RULE MATCH", "Step 9-10: Pedestrian request at %s crossing.", LN[s->ped_lane]);
        say("SIGNAL SET", "VEHICLES: ALL RED | %s PEDESTRIAN: WALK (15s)", LN[s->ped_lane]);
        clk += 15; say("TRANSITION", "Pedestrian phase expired. Returning to vehicle loop.");
        s->ped_lane = -1; return;
    }
    /* Step 13-17: density / round robin */
    int max = 0, min = 1 << 30, best = 0;
    for (int i = 0; i < NL; i++) { lane[i].density = s->density[i];
        if (s->density[i] > max) { max = s->density[i]; best = i; }
        if (s->density[i] < min) min = s->density[i]; }
    say("SENSOR", "Density -> N:%d S:%d E:%d W:%d", s->density[0], s->density[1], s->density[2], s->density[3]);
    if (max - min <= 1) {
        say("RULE MATCH", "Step 16-17: Equal density. Round-Robin fallback (20s quantum).");
        int order[NL] = {0, 2, 1, 3};       /* N -> E -> S -> W */
        for (int k = 0; k < NL; k++) { give_green(order[k], 20, "ROUND_ROBIN"); end_green(order[k]); }
    } else {
        say("RULE MATCH", "Step 11-15: Rush hour=%s, target %s (density %d).", s->rush_hour ? "YES" : "NO", LN[best], max);
        int g = dyn_green(max, s->rush_hour);
        give_green(best, g, "ADAPTIVE_DENSITY"); end_green(best);
    }
}
 
static void run_demo(void) {
    for (int i = 0; i < NL; i++) { lane[i].id = i; lane[i].state = RED; lane[i].priority = 1; }
    printf("=== TSCOS v1.0 : Traffic Signal Control Operating System ===\n");
    say("SYSTEM INIT", "Defaults: Green 30s, Yellow 5s, Red 65s. Mode: AUTOMATIC.");
    Shared s = {{0}, -1, -1, -1, 1};
 
    printf("\n--- Scenario 1: Adaptive control (rush hour) ---\n");
    clk += 5; s.density[0]=45; s.density[1]=12; s.density[2]=8; s.density[3]=15; scheduler_step(&s);
 
    printf("\n--- Scenario 2: Emergency vehicle preemption ---\n");
    say("SENSOR", "AMBULANCE detected on EAST approach!"); s.emergency_lane = 2; scheduler_step(&s);
 
    printf("\n--- Scenario 3: Pedestrian crossing ---\n");
    clk += 0; say("SENSOR", "Pedestrian push-button pressed at WEST crossing."); s.ped_lane = 3; scheduler_step(&s);
 
    printf("\n--- Scenario 4: Equal low traffic (round-robin) ---\n");
    clk += 5; s.density[0]=5; s.density[1]=5; s.density[2]=4; s.density[3]=5; s.rush_hour=0; scheduler_step(&s);
 
    printf("\n--- Scenario 5: Manual override by police ---\n");
    s.manual_lane = 1; scheduler_step(&s);
    clk += 60; say("OPERATOR", "Override released. Safe transition, resuming automatic mode.");
    end_green(1); s.manual_lane = -1;
    say("SYSTEM", "Mode: AUTOMATIC. Simulation complete.");
}

/* ---------- interactive mode (live commands) ---------- */

static int lane_from_letter(char c) {
    switch (c) {
        case 'N': case 'n': return 0;
        case 'S': case 's': return 1;
        case 'E': case 'e': return 2;
        case 'W': case 'w': return 3;
        default: return -1;
    }
}

static void print_help(void) {
    printf("Commands:\n");
    printf("  density N S E W     set vehicle counts for the four approaches\n");
    printf("  rush 0|1            turn rush-hour profile off/on\n");
    printf("  emergency N|S|E|W   emergency vehicle on an approach (preempts)\n");
    printf("  ped N|S|E|W         pedestrian button pressed at a crossing\n");
    printf("  override N|S|E|W    police force one approach green\n");
    printf("  release             release police override (safe transition)\n");
    printf("  help                show this list\n");
    printf("  quit                stop the system\n");
}

static void run_interactive(void) {
    Shared s = {{0}, -1, -1, -1, 1};
    char line[256];
    printf("=== TSCOS v1.0 : interactive mode ===\n");
    print_help();
    for (int i = 0; i < NL; i++) { lane[i].id = i; lane[i].state = RED; lane[i].priority = 1; }
    while (printf("tscos> "), fflush(stdout), fgets(line, sizeof line, stdin)) {
        char cmd[32] = {0}; char arg[8] = {0};
        int a = 0, b = 0, c = 0, d = 0;
        if (sscanf(line, "%31s", cmd) != 1) continue;
        if (!strcmp(cmd, "quit")) break;
        else if (!strcmp(cmd, "help")) print_help();
        else if (!strcmp(cmd, "density") && sscanf(line, "%*s %d %d %d %d", &a, &b, &c, &d) == 4) {
            s.density[0] = a; s.density[1] = b; s.density[2] = c; s.density[3] = d;
            clk += 5; scheduler_step(&s);
        }
        else if (!strcmp(cmd, "rush") && sscanf(line, "%*s %d", &a) == 1) {
            s.rush_hour = a ? 1 : 0; say("SYSTEM", "Rush-hour profile %s.", s.rush_hour ? "ON" : "OFF");
        }
        else if (!strcmp(cmd, "emergency") && sscanf(line, "%*s %7s", arg) == 1 && lane_from_letter(arg[0]) >= 0) {
            say("SENSOR", "AMBULANCE detected on %s approach!", LN[lane_from_letter(arg[0])]);
            s.emergency_lane = lane_from_letter(arg[0]); scheduler_step(&s);
        }
        else if (!strcmp(cmd, "ped") && sscanf(line, "%*s %7s", arg) == 1 && lane_from_letter(arg[0]) >= 0) {
            say("SENSOR", "Pedestrian push-button pressed at %s crossing.", LN[lane_from_letter(arg[0])]);
            s.ped_lane = lane_from_letter(arg[0]); scheduler_step(&s);
        }
        else if (!strcmp(cmd, "override") && sscanf(line, "%*s %7s", arg) == 1 && lane_from_letter(arg[0]) >= 0) {
            s.manual_lane = lane_from_letter(arg[0]); scheduler_step(&s);
        }
        else if (!strcmp(cmd, "release")) {
            if (s.manual_lane < 0) say("SYSTEM", "No override is active.");
            else {
                int l = s.manual_lane;
                clk += 60; say("OPERATOR", "Override released. Safe transition, resuming automatic mode.");
                end_green(l); s.manual_lane = -1;
            }
        }
        else say("ERROR", "Unknown or malformed command. Type 'help'.");
    }
    say("SYSTEM", "Mode: AUTOMATIC. Stopped.");
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "-i")) { run_interactive(); return 0; }
    run_demo();
    return 0;
}
