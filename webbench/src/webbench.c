#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>



void usage(void) {
    fprintf(stderr,
	"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"         
    );
};

/* Allow: GET, HEAD, OPTIONS, TRACE */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"

/* internal */
char host[MAXHOSTNAMELEN];
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];

int force = 0;
int reload = 0;
int method = METHOD_GET;
int clients = 1;
int http10 = 1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
int benchtime = 30;
char* proxyhost = NULL;
int proxyport=80;

static const struct option long_options[]=
{
    {"force", no_argument, &force, 1},
    {"reload", no_argument, &reload, 1},
    {"time", required_argument, NULL, 't'},
    {"help", no_argument, NULL, '?'},
    {"http09", no_argument, NULL, '9'},
    {"http10", no_argument, NULL, '1'},
    {"http11", no_argument, NULL, '2'},
    {"get", no_argument, &method, METHOD_GET},
    {"head", no_argument, &method, METHOD_HEAD},
    {"options", no_argument, &method, METHOD_OPTIONS},
    {"trace", no_argument, &method, METHOD_TRACE},
    {"version", no_argument, NULL, 'V'},
    {"proxy", required_argument, NULL, 'p'},
    {"clients", required_argument, NULL,'c'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char *argv[]) {
    int option_ind = 0;
    int opt = 0;
    char* tmp = NULL;

    if (argc == 1) {
        usage();
        return 2;
    }

    const char* shortopts = "912Vfrt:p:c:?h";
    while ((opt = getopt_long(argc, argv, shortopts, &long_options, &option_ind)) != EOF ) {
        switch (opt) {
            case  0  : break;
            case 'f' : force  = 1; break;
            case 'r' : reload = 1; break;
            case '9' : http10 = 0; break;
            case '1' : http10 = 1; break;
            case '2' : http10 = 2; break;
            case 'V' : printf(PROGRAM_VERSION"\n"); exit(0);
            case 't' : benchtime = atoi(optarg); break;
            case 'p' : 
                /* proxy server parsing  server:port */
                tmp = strrchr(optarg, ':');
                printf("=====%s", tmp);
                printf("==---%s", optarg);
                proxyhost = optarg;
                if (tmp == NULL) {
                    break; /* default port is 80 */
                }
                if (tmp == optarg) {
                    fprintf(stderr, "Error in option --proxy %s: Missing hostname.\n", optarg);
                    return 2;
                }
                if (tmp == (optarg + strlen(optarg) - 1)) {
                    fprintf(stderr, "Error in option --proxy %s: Missing port.\n", optarg);
                    return 2;
                }
                *tmp = '\0';
                proxyport=atoi(tmp+1); break;
            case ':' :
            case 'h' :
            case '?' : usage(); return 2; break;
            case 'c' : clients=atoi(optarg); break;
        }       
    }

    if (optind == argc) {
        fprintf(stderr,"webbench: Missing URL!\n");
        usage();
        return 2;
    }

    if(clients==0) clients=1;
    if(benchtime==0) benchtime=60;

    fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
        "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n"
        );
    printf("\nBenchmarking: ");
    switch (method)
    {
        case METHOD_GET:
        default:
            printf("GET");break;
        case METHOD_OPTIONS:
            printf("OPTIONS");break;
        case METHOD_HEAD:
            printf("HEAD");break;
        case METHOD_TRACE:
            printf("TRACE");break;
    }
    printf(" %s",argv[optind]);

    switch (http10) {
        case 0: printf(" (using HTTP/0.9)\n");break;
        case 2: printf(" (using HTTP/1.1)\n");break;
    }

    printf("%d clients, running %d sec", clients, benchtime);
    if (force) printf(", early socket close");
    if (proxyhost != NULL) printf(", via proxy server %s:%d", proxyhost, proxyport);
    printf(".\n");
}

void build_request(const char *url) {

    memset(host, 0, MAXHOSTNAMELEN);
    memset(request, 0, REQUEST_SIZE);

    if(reload && proxyhost!=NULL && http10<1) http10=1;
    if(method==METHOD_HEAD && http10<1) http10=1;
    if(method==METHOD_OPTIONS && http10<2) http10=2;
    if(method==METHOD_TRACE && http10<2) http10=2;

    switch(method)
    {
        default:
        case METHOD_GET: strcpy(request,"GET");break;
        case METHOD_HEAD: strcpy(request,"HEAD");break;
        case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
        case METHOD_TRACE: strcpy(request,"TRACE");break;
    }

    strcat(request," ");
    
}