/*
 * COMAR Unified Core - Process Management Engine
 * Copyright (c) 2026, Ergün Salman (Poyraz76)
 *
 * ALTYAPI: POSIX Process Management, Unified Core 4.0.
 * GÜVENLİK: Graceful Shutdown & Zombie Process Protection.
 */

#include "process.h"
#include "log.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

struct Proc my_proc;

/**
 * Yeni bir alt süreci envantere mühürler ve ekler.
 */
struct ProcChild *add_child(pid_t pid, int from, DBusMessage *bus_msg) {
    int i = my_proc.nr_children;
    if (i >= my_proc.max_children) {
        my_proc.max_children = (i == 0) ? 4 : my_proc.max_children * 2;
        my_proc.children = realloc(my_proc.children, my_proc.max_children * sizeof(struct ProcChild));
        if (!my_proc.children) oom();
    }

    memset(&my_proc.children[i], 0, sizeof(struct ProcChild));
    my_proc.children[i].from = from;
    my_proc.children[i].pid = pid;
    my_proc.children[i].bus_msg = bus_msg;
    ++my_proc.nr_children;
    
    log_debug("Süreç Eklendi (PID: %d). Aktif Çocuk: %d\n", pid, my_proc.nr_children);
    return &my_proc.children[i];
}

/**
 * Tamamlanan süreci envanterden temizler ve mühür sökülür.
 */
void rem_child(int nr) {
    int status;
    // Zombi oluşumunu engellemek için waitpid kullanılır.
    waitpid(my_proc.children[nr].pid, &status, WNOHANG);
    close(my_proc.children[nr].from);
    
    --my_proc.nr_children;
    if (my_proc.nr_children > 0 && nr != my_proc.nr_children) {
        my_proc.children[nr] = my_proc.children[my_proc.nr_children];
    }
}

/**
 * Süreç yönetim motorunu başlatır.
 */
int proc_init() {
    memset(&my_proc, 0, sizeof(struct Proc));
    my_proc.parent.from = -1;
    my_proc.max_children = 8;
    my_proc.children = calloc(my_proc.max_children, sizeof(struct ProcChild));
    if (!my_proc.children) oom();
    return 0;
}

/**
 * Aktif olan tüm alt süreçleri güvenli bir şekilde durdurur.
 */
void stop_children(void) {
    log_debug("Tüm alt süreçler durduruluyor...\n");
    for (int i = 0; i < my_proc.nr_children; i++) {
        kill(my_proc.children[i].pid, SIGTERM);
    }
}

/**
 * Çekirdek sonlanırken tüm süreci mühürler ve temizler.
 */
void proc_finish(void) {
    if (my_proc.nr_children) stop_children();
    log_info("Süreç motoru kapatıldı.\n");
    exit(0);
}

/**
 * Sistemden gelen kapatma sinyallerini işler.
 */
static void handle_sigterm(int signum) {
    log_info("Kapatma sinyali alındı (%d). Çıkış yapılıyor...\n", signum);
    proc_finish();
}

/**
 * Sinyal yakalama mekanizmasını mühürler.
 */
void handle_signals(void) {
    struct sigaction act;
    struct sigaction ign;

    act.sa_handler = handle_sigterm;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART; // Kesilen sistem çağrılarını otomatik devam ettir.

    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ign.sa_flags = 0;

    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);  // Kullanıcı kesmesi (Ctrl+C) desteği.
    sigaction(SIGPIPE, &ign, NULL); // Kırık boru hatalarını yoksay.
}

/**
 * Betiği yeni bir alt süreçte çatallayarak (fork) çalıştırır.
 */
struct ProcChild *proc_fork(void (*child_func)(DBusMessage *msg), DBusMessage *msg) {
    pid_t pid;
    int pfd[2];

    dbus_message_ref(msg);
    if (pipe(pfd) == -1) return NULL;

    pid = fork();
    if (pid == -1) {
        close(pfd[0]);
        close(pfd[1]);
        return NULL;
    }

    if (pid == 0) { // Alt Süreç (Child)
        close(pfd[1]);
        fcntl(pfd[0], F_SETFD, FD_CLOEXEC);

        // Alt süreç için Proc yapısını sıfırla
        memset(&my_proc, 0, sizeof(struct Proc));
        my_proc.parent.from = pfd[0];
        my_proc.parent.pid = getppid();
        my_proc.bus_msg = msg;

        handle_signals();
        child_func(msg);
        
        close(pfd[0]);
        proc_finish();
        _exit(0); // Güvenli çıkış
    } else { // Ana Süreç (Parent)
        close(pfd[0]);
        return add_child(pid, pfd[1], msg);
    }
    return NULL;
}
