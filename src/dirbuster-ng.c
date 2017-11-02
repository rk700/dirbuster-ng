#include "dirbuster-ng.h"

/*we split the argument parsing from main()
because it's more clear */
void parse_arguments(int argc, char **argv)
{
    extern dbng_config conf0;
    char *cvalue = NULL;
    int index;
    int c;
    int opterr = 0;    
    while ((c = getopt(argc, argv, "hfqvVw:d:e:n:t:X:K:u:U:W:")) != -1) {
        switch (c) {
            case 'v':
                return;
            case 'w':
                conf0.nb_workers = atoi(optarg);
                break;
            case 'q':
                conf0.quiet = 1;
                break;
            case 'h':
                usage();
                break;
            case 'f':
                conf0.followRedirect = 1;
                break;
            case 'd':
                conf0.dict = optarg;
                break;
            case 'e':
                conf0.ext = explode(optarg, ",");
                break;
            case 'V':
                conf0.verbose = 1;
                break;
            case 't':
                conf0.timeout = atoi(optarg);
                break;
            case 'X':
                conf0.proxy = optarg;
                break;
            case 'A':
                conf0.http_auth = optarg;
                break;    
            case 'K':
                conf0.proxy_auth = optarg;
                break;
            case 'u':
                conf0.uagent = (char*) resolve_ua(optarg);
                break;
            case 'U':
                conf0.uagent = optarg;
                break;
            case 'W':
                conf0.output_file = fopen(optarg,"w+");
                if (! conf0.output_file) {
                    fprintf(stderr,"ERROR: cannot open output file\n");
                    exit(1);
                }
                break;
            default:
                break;
        }
    }
}

void output(char* fmt,...) {
    va_list args;
    extern dbng_config conf0;
    if (conf0.quiet)
        return;    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void outputToFile(char* fmt,...) {
    va_list args;
    extern dbng_config conf0;
    if (!conf0.output_file) 
        return;
    va_start(args, fmt);
    vfprintf(conf0.output_file, fmt, args);
    va_end(args);
}



static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    return (size_t)(size * nmemb);
}

void* dbng_engine(void* queue_arg) {
    struct queue* db_queue = (struct queue*) queue_arg;
    extern dbng_config conf0;
    CURL *curl;
    CURLcode response;
    long http_code;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, conf0.timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, conf0.followRedirect);

    if (conf0.proxy != NULL) {
        curl_easy_setopt(curl, CURLOPT_PROXY, conf0.proxy);
    }

    if (conf0.uagent != NULL) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, conf0.uagent);
    }

    if (conf0.proxy_auth != NULL) {
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, conf0.proxy_auth);
    }

    if (conf0.http_auth != NULL) {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST);
        curl_easy_setopt(curl, CURLOPT_USERPWD, conf0.http_auth);
    }

    char buffer[256];
    buffer[sizeof(buffer)-1] = '\0';
    char url[512];
    url[sizeof(url)-1] = '\0';
    struct elt *head;

loop: 
    pthread_mutex_lock(db_queue->mutex);
    head = db_queue->head;
    if(head == NULL) {
        pthread_mutex_unlock(db_queue->mutex);
        return NULL;
    }
    strncpy(buffer, head->entry, sizeof(buffer)-1);
    queue_rem(db_queue);
    pthread_mutex_unlock(db_queue->mutex);

    strncpy(url, conf0.host, sizeof(url)-1);
    strncat(url, buffer, sizeof(url)-strlen(url)-1);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    response = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code == 200 || http_code == 403) {
        output("FOUND %s (response code %d)\n", url, http_code);
        outputToFile("%s (HTTP code %d)\n", url, http_code);
    }  
    if(conf0.verbose) {
        output("[%d] %s\n", http_code, url);
    }
    goto loop;
}

void load_dict(struct queue* db_queue) {
    extern dbng_config conf0;    
    FILE* dict_fh;
    char buffer[256];
    // char * buffer = (char*) malloc(4096 * sizeof(char));
    dict_fh = fopen(conf0.dict, "r");

    if (!dict_fh) {
        fprintf(stderr,"ERROR: cannot open dictionary file %s, quitting..\n", conf0.dict);
        exit(1);
    }

    int i;
    char **ext;
    while (!feof(dict_fh)) {
        if(!fgets(buffer, sizeof(buffer), dict_fh)) {
            break;
        }
        //fgets would also store the trailing newline
        size_t entry_len = strlen(buffer);
        if(buffer[entry_len-1]=='\n') {
            buffer[entry_len-1] = '\0';
            entry_len -= 1;
        }
        if(conf0.ext == NULL) {
            queue_add(db_queue, buffer);
            continue;
        }
        for(ext=conf0.ext; *ext!=NULL; ++ext) {
            if(sizeof(buffer) - entry_len > strlen(*ext)+1) {
                strcpy(buffer+entry_len, *ext);
                queue_add(db_queue, buffer);
            }
            else {
                fprintf(stderr, "WARNNING: not enough space for entry %s and extension %s\n", buffer, *ext);
            }
        }
    }
}


void init_config(dbng_config* conf0) {
    conf0->quiet = 0;
    conf0->nb_workers = DEFAULT_WORKERS;
    conf0->timeout = DEFAULT_TIMEOUT;
    conf0->host = NULL;
    conf0->dict = NULL;
    conf0->proxy = NULL;
    conf0->uagent = NULL;
    conf0->proxy_auth = NULL;
    conf0->http_auth = NULL;
    conf0->output_file = NULL;
    conf0->ext = NULL;
    conf0->verbose = 0;
    conf0->followRedirect = 0;
}

void doRequests(struct queue* db_queue) {
    extern dbng_config conf0;
    int i;
    conf0.workers = (pthread_t*) malloc(conf0.nb_workers * sizeof(pthread_t));
    for (i=0; i<conf0.nb_workers; i++) {
        pthread_create(&conf0.workers[i], NULL, dbng_engine, (void*) db_queue);
    }

    void *res;
    for (i=0; i<conf0.nb_workers; i++) {
        pthread_join(conf0.workers[i], &res);
    }
}

void init_workloads(struct queue* db_queue) {
    
    extern dbng_config conf0;
    extern const char* def_dict[];
    extern int dd_nbentries;
    int i;

    //no external dictionary defined, using the internal
    if (NULL == conf0.dict) {
        for(i=0; i<dd_nbentries; i++) {
            queue_add(db_queue, (char*)def_dict[i]);
        }
    }
    else {
        load_dict(db_queue);
    }
}

void usage() {
    printf("Usage: dirbuster-ng [options...] <url>\n\
    Options:\n\
    -w <nb_threads>\tDefines the number of threads to use to make the attack(4 by default)\n\
    -U <user_agent>\tDefines a custom user-agent\n\
    -A <username:password>\tSets username and password for HTTP auth\n\
    -X <proxy_server:port>\tUse an HTTP proxy server to perform the queries\n\
    -K <username:password>\tSets an username/password couple for proxy auth\n\
    -d <dict>\tLoads an external textfile to use as a dictionary\n\
    -e <ext>\tSpecify a list of extensions that would be appended to each word in the dict; seperated by comma; omitting this option would lead to dict entries unchanged\n\
    -t <seconds>\tSets the timeout in seconds for each http query(10 by default)\n\
    -W <file>\tSaves the program's result inside a file\n\
    -u <ua>\tuse a predefined user-agent, corresponding to the most used browsers/crawlers:\n\
    \t\tff: Mozilla Firefox\n\
    \t\tchr: Google Chrome\n\
    \t\tsaf: Apple Safari\n\
    \t\tope: Opera\n\
    \t\topem: Opera Mobile\n\
    \t\tie: Microsoft Internet Explorer\n\
    \t\tfen: Mozilla Fennec\n\
    \t\tbb: RIM BlackBerry Browser\n\
    \t\tgbot: Google Bot\n\
    \t\tbing: Microsoft Bing Crawler\n\
    \t\tbspid: Baidu Spider\n\
    -q\tEnable quiet mode (relevent only with the -W flag)\n\
    -h\tPrints this help then exits\n\
    -V \tVerbose. Print each request url and response code\n\
    -v\tPrints the program version then exits\n\
    -f\tFollow redirects(302 or 301)\n");

    exit(0);
}

void version() {
    printf("{Dirbuster NG 0.2} (c)2017 WintermeW, Ruikai Liu\n");
}


int main(int argc, char **argv) {
    extern dbng_config conf0;
    struct queue* db_queue;

    if(argc < 2)
        usage();

    init_config(&conf0);
    conf0.host = argv[argc -1];    
    db_queue = queue_new();
    db_queue->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(db_queue->mutex, NULL);

    parse_arguments(argc, argv);
    if(!conf0.quiet)
        version();

    init_workloads(db_queue);
    doRequests(db_queue);
}
