#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_REQUEST_LENGTH 1024
#define PROGRAM_VERSION "1.5"

char host[MAXHOSTNAMELEN];
char request[MAX_REQUEST_LENGTH];
int port = 80;
int client = 1;
int speed, bytes, failed;
int expired = 0;
int benchtime = 5 ;

static int Socket(char *host, int port);
static void generate_request(const char *url);
static int bench();
static void bench_core(const char *host, int port, const char *req);

static void alarm_handler() {
    expired = 1;
}

int main(int argc, char *argv[]) {
    char url[] = "http://www.baidu.com/";
    generate_request(url);
    bench_core(host, port, request);
//    return bench();
    printf("\nspeed: %d ,bytes: %d ,failed %d \n", speed, bytes, failed);
}


/* generate request header
 * "http://learn.microsoft.com:9090/zh-cn/"
 * */
static void generate_request(const char *url)   {
    char tmp[10];
    int i;
    char *ch1, *ch2;
    memset(host, 0, sizeof(host));
    memset(tmp, 0, sizeof(tmp));
    memset(request, 0, sizeof(request));

    ch1 = strstr(url, "://");
    if (ch1 == NULL) {
        fprintf(stderr, "invalid url syntax.\n");
        exit(1);
    }
    i = ch1 - url + 3;
    if (strchr(url + i, '/') == NULL) {
        fprintf(stderr, "invalid url syntax: url do not end with '/'.\n");
        exit(2);
    }

    strcat(request,"GET ");
    strcat(request, url + i + strcspn(url + i, "/"));
    strcat(request, " HTTP/1.1\r\n");

    memcpy(host, url + i, strcspn(url + i, "/"));
    strcat(request, "Host: ");
    strcat(request, host);
    strcat(request, "\r\n");
    strcat(request, "User-Agent: WebBench "PROGRAM_VERSION"\r\n");

    strcat(request, "Connection: close\r\n");
    strcat(request,"\r\n");
    printf("====\n%s", request);

}

static int bench() {
    int a,b,c;
    int mypipe[2];
    pid_t pid;
    int s, n;
    FILE *f;
    size_t i;
    s = Socket(host, port);
    if (s <= 0) {
        fprintf(stderr, "\nConnect to server failed. Aborting WebBench.\n");
        return 1;
    }
    close(s);
    if (pipe(mypipe)) {
        perror("create pipe failed.");
        return 3;
    }

    for(i = 0; i < client; i++) {
        pid = fork();
        if (pid <= 0) break;
    }

    if (pid < 0) {
        perror("fork failed.");
        return 3;
    }

    if (pid == 0) {
        bench_core(host, port, request);
        f = fdopen(mypipe[1], "w");
        if (f == NULL) {
            perror("open pipe for writing failed.");
            return 3;
        }
        fprintf(f, "%d %d %d\n", speed, bytes, failed);
        fclose(f);
    } else {
        close(mypipe[1]);
        f = fdopen(mypipe[0], "r");
        if (f == NULL) {
            perror("Open pipe for reading failed.");
            return 3;
        }
        speed = bytes = failed = 0;
        while (1) {
            n = fscanf(f, "%d %d %d", &a, &b, &c);
            speed += a;
            bytes += b;
            failed += c;
            if (--client == 0) break;
        }
        printf("\nspeed: %d ,bytes: %d ,failed %d \n", speed, bytes, failed);
    }
}

static void bench_core(const char *host, int port, const char *req) {
    int s;
    int b;
    int rlen;
    rlen = strlen(req);
    char buffer[15000];
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL)) exit(3);
    alarm(benchtime);

    nexttry:
    while (1) {
        if (expired) {
//            if (failed > 0) --failed;
            return;
        }
        s = Socket(host, port);
        if (s <= 0) {
            printf("1\n");
            failed++;
            continue;
        }

        if (rlen != write(s, req, rlen)) {
            printf("2\n");
            failed++;
            close(s);
            continue;
        }
//        shutdown(s,1);
//        FILE *f = fopen("a.html", "a");
        while (1) {

            if (expired)  {break;}
            b = read(s, buffer, sizeof(buffer));

            if (b < 0) {
                printf("3\n");
                failed++;
                close(s);
                goto nexttry;
            }
            if (b == 0) break;
//            fwrite(buffer, sizeof(char), sizeof(buffer), f);
            bytes += b;
        }
        if (close(s) == -1) {printf("4\n"); failed++; continue;}
        speed++;
    }
}


/* Create socket to connect remote socket， return socket file descriptor */
static int Socket(char *host, int port) {
    int sock;
    unsigned long addr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_port = htons(port);
    addr = inet_addr(host);
    if (addr != INADDR_NONE) {
        memcpy(&ad.sin_addr, &addr, sizeof(addr));
    } else {
        hp = gethostbyname(host);
        memcpy(&ad.sin_addr, hp->h_addr, sizeof(hp->h_length));
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Create socket failed!");
        return sock;
    }

    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}

