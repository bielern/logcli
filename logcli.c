/**
 * Logbook: lets you write down in a simple way you thoughts that
 * you have, while browsing through your file system.
 * It save the date and your current path
 *
 * Written by Noah Bieler. Inspired by memoir
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <regex.h>
#include <error.h>
#include <errno.h>
#include "utils.h"

char usage[] = "l [log|help|dir REGEX|date REGEX|body REGEX] \n\n"
"This programm lets you log your thoughts together with the date \n"
"(YYYYMMDDhhmm) and the current working directory.\n"
"You can also search for specific date, directory or words in the log entry\n"
"with 'dir', 'date' or 'body'.\n"
"'help' prints this meassage.\n"
"No command logs all arguments written.\n\n"
"Written by Noah Bieler\n";

#define CONFFOLDER "logcli"
#define CONFFILE "logcli.conf"
#define LOGFILE "logcli.log"
#define STRINGSIZE 1024
#define Bool int
#define true 1
#define false 0

/*** Structures ***/
typedef struct files {
    char * confdirname; /* directory with all relevant files */
    DIR * confdir;      /* directory with all relevant files */
    FILE * conffile;    /* File handler for the config file */
    FILE * logfile;     /* File handler for the log file */
} Files;

typedef enum command {
    log,                /* log into the file (default") */
    oneliner,           /* log a one liner into the file */
    dir,                /* search for a directory */
    date,               /* search for a date */
    body,               /* search in the logs */
    help,               /* Print help message */
} Command;

typedef struct regex {
    regex_t comp_regex; /* The comiled regular expression */
    char * myregex;     /* The string with the regular expression */
} Regex;

typedef struct conf {
    Files files;
    Command command;
    Regex regex;
} Conf;

/*** utils ***/
/**
 * clean up the file pointers
 */
void
clean_conf(Files files){
    if(files.confdirname){
        free(files.confdirname);
    }
    if(files.conffile){
        fclose(files.conffile);
    }
    if(files.logfile){
        fclose(files.logfile);
    }
    if(files.confdir){
        closedir(files.confdir);
    }
}

/*** Initialization ***/
/**
 * initializes all file handlers and config direcory
 */
int
init_conf(Conf * conf){
    Bool firsttime = false;
    char * home = getenv("XDG_CONFIG_HOME");
    if(home == NULL){
        home = getenv("HOME");
        cat(&home, "/.config");
    }
    DEBUG("Home: %s\n", home);
    conf->files.confdirname = strdup(home);
    cat(&(conf->files.confdirname), "/");
    cat(&conf->files.confdirname, CONFFOLDER);
    DEBUG("confdirname: %s\n", conf->files.confdirname);

    conf->files.confdir = opendir(conf->files.confdirname);
    if(conf->files.confdir == NULL){
        if((conf->command != log) && (conf->command != oneliner)){
            error(1, 0, "There are no log entries yet!\n");
        }
        mode_t proc_mask = umask(0);
        if(mkdir(conf->files.confdirname, 
                    S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH| S_IXOTH) != 0){
            fprintf(stderr, "ERROR: Could not create configuration "
                            "directory %s\n", conf->files.confdirname);
            exit(1);
        }
        firsttime = true;
        umask(proc_mask);
    }

    char * filename;
    filename = strdup(conf->files.confdirname);
    cat(&filename, "/");
    cat(&filename, CONFFILE);
    DEBUG("conf file name: %s\n", filename);
    if(firsttime){
        conf->files.conffile = fopen(filename, "w");
        fclose(conf->files.conffile);
    }
    conf->files.conffile = fopen(filename, "r");

    free(filename);
    filename = strdup(conf->files.confdirname);
    cat(&filename, "/");
    cat(&filename, LOGFILE);
    DEBUG("log file name: %s\n", filename);
    if((conf->command == log) || (conf->command == oneliner)){
        conf->files.logfile = fopen(filename, "a");
        DEBUG("Logfile with write access\n");
    }
    else {
        conf->files.logfile = fopen(filename, "r");
        DEBUG("Logfile with read access\n");
    }
    
    free(filename);

    return 0;
}

/**
 * Make the header
 */
char *
mkheader(char * header){
    char * line;
    line = (char *) xmalloc((STRINGSIZE + 1) * sizeof(char));
    time_t cur_time;
    struct tm *loctime;

    cur_time = time(NULL);
    loctime = localtime(&cur_time);
    strftime(header, STRINGSIZE, "%Y%m%d%H%M", loctime);
    if(getcwd(line, STRINGSIZE) == NULL){
        error(1, 0, "Could not obtain current working directory");
    }
    DEBUG("cwd in mkheader: %s\n", line);
    cat(&header, "\n");
    cat(&header, line);
    cat(&header, "\n");
    free(line);
    return header;
}

/* Commands */
/**
 * Log the entry. Stop, when empty line is found
 */
int 
log_entry(Conf * conf){
    char * logentry;
    char * line;
    char * header;
    int byte_read;
    int bytes_sofar = 0;
    size_t log_size = sizeof(char) * STRINGSIZE;
    size_t line_size = sizeof(char) * STRINGSIZE;

    logentry = (char *) xmalloc(line_size);
    line = (char *) xmalloc(line_size);
    header = (char *) xmalloc(line_size);
    strcpy(logentry, "");

    header = mkheader(header);
    while(true){
        byte_read = getline(&line, &line_size, stdin);
        if(byte_read > 0){
            bytes_sofar += byte_read;
            if(byte_read < log_size){
                cat(&logentry, line);
            }
        }
        if(strcmp(line,"\n") == 0){
            if(strcmp(logentry, "\n") != 0){
                fprintf(conf->files.logfile, "%s", header);
                fprintf(conf->files.logfile, "%s", logentry);
                free(line);
                free(header);
                free(logentry);
                return 0;
            }
            else {
                printf("Nothing logged.\n");
                free(line);
                free(header);
                free(logentry);
                return 0;
            }
        }
    }

    // We should not be here
    free(line);
    free(header);
    free(logentry);
    return 1;
}

/**
 * print the one liner
 */
int
log_oneline(int argc, char *argv[], Conf * conf){
    char * logentry;
    char * header;

    logentry = strdup("");
    header = (char *) xmalloc((STRINGSIZE + 1) * sizeof(char));
    header = mkheader(header);

    DEBUG("Logging a oneliner\n");

    int i;
    for(i = 1; i < argc; i++){
        cat(&logentry, argv[i]);
        cat(&logentry, " ");
    }

    DEBUG("header in oneliner:\n%s", header);
    fprintf(conf->files.logfile, "%s", header);
    fprintf(conf->files.logfile, "%s\n\n", logentry);

    free(logentry);
    free(header);
    return 0;
}

/**
 * Search for a date or a directory given by a regex
 */
int
search(Conf * conf){
    int errcode;
    Bool match = false;
    int byte_read = 0;
    int bytes_sofar;
    int body_size;
    size_t buffer_size = sizeof(char) * (STRINGSIZE + 1);
    char * buffer;
    char * date_str = NULL;
    char * dir_str = NULL;
    char * body_str;
    buffer = (char *) xmalloc(buffer_size);
    body_str = (char *) xmalloc(buffer_size);
    strcpy(body_str, "");
    body_str = (char *) xrealloc(body_str, buffer_size);
    body_size = STRINGSIZE;

    DEBUG("I am in search\n");
    if((conf->files.logfile == NULL) && ((conf->command != log) || (conf->command != oneliner))){
        error(1, 0, "No log file found!");
    }

    errcode = regcomp(&(conf->regex.comp_regex), conf->regex.myregex, 
                      REG_ICASE);
    if(errcode){
        int new_size;
        new_size = regerror(errcode, &(conf->regex.comp_regex), 
                            buffer, buffer_size);
        if(new_size > buffer_size){
            xrealloc(buffer, new_size);
            buffer_size = new_size;
            new_size = regerror(errcode, &(conf->regex.comp_regex), 
                                buffer, buffer_size);
        }
        regfree(&(conf->regex.comp_regex));
        error(1, 0, buffer);
    }

    strcpy(buffer, "\n");
    while((strcmp(buffer, "\n") == 0) && (byte_read != -1)){
        byte_read = getline(&buffer, &buffer_size, conf->files.logfile);
    }

    while(byte_read != -1){
        DEBUG("I am reading the file\n");
        if(date_str){
            free(date_str);
        }
        if(dir_str){
            free(dir_str);
        }
        date_str = strdup(buffer);
        byte_read = getline(&buffer, &buffer_size, conf->files.logfile);
        dir_str = strdup(buffer);

        bytes_sofar = 0;
        byte_read = getline(&buffer, &buffer_size, conf->files.logfile);
        bytes_sofar += byte_read;
        while((strcmp(buffer, "\n") != 0) && (byte_read != -1)){
            DEBUG("Reading the body\n");
            if(bytes_sofar > body_size){
                DEBUG("Reallocating memory\n");
                body_size = bytes_sofar + STRINGSIZE;
                xrealloc(body_str, body_size);
            }
            if(conf->command == body){
                DEBUG("We search in the body\n");
                if(regexec(&(conf->regex.comp_regex), buffer, 0, NULL, 0) == 0){
                    match = true;
                }
            }
            cat(&body_str, buffer);
            byte_read = getline(&buffer, &buffer_size, conf->files.logfile);
        }
        DEBUG("date: %s\n", date_str);
        DEBUG("dir: %s\n", dir_str);  
        DEBUG("body: %s\n", body_str);

        if(conf->command == dir){
            if(regexec(&(conf->regex.comp_regex), dir_str, 0, NULL, 0) == 0){
                match = true;
            }
        }
        else if(conf->command == date){
            if(regexec(&(conf->regex.comp_regex), date_str, 0, NULL, 0) == 0){
                match = true;
            }
        }
        if(match){
            printf("%s", date_str);
            printf("%s", dir_str);
            printf("%s\n", body_str);
            fflush(stdout);
            match = false;
        }

        strcpy(body_str, "");
        byte_read = getline(&buffer, &buffer_size, conf->files.logfile);
    }
    
    free(buffer);
    free(date_str);
    free(dir_str);
    free(body_str);
    regfree(&(conf->regex.comp_regex));
    return 0;
}

/**
 * Main
 */
int
main(int argc, char *argv[]){
    Conf conf;

    if(argc == 1){
        conf.command = log;
    }
    else{
        if(strcmp(argv[1],"log") == 0){
            conf.command = log;
        }
        else if((strcmp(argv[1],"date") == 0) || (strcmp(argv[1],"t") == 0)){
            conf.command = date;
        }
        else if((strcmp(argv[1],"dir") == 0) || (strcmp(argv[1],"d") == 0)){
            conf.command = dir;
        }
        else if((strcmp(argv[1],"body") == 0) || (strcmp(argv[1],"b") == 0)){
            conf.command = body;
        }
        else if(strcmp(argv[1],"help") == 0){
            conf.command = help;
        }
        else {
            conf.command = oneliner;
        }
    }
    if(conf.command == help){
        puts(usage);
        return 0;
    }
    if(init_conf(&conf)){
        clean_conf(conf.files);
        error(0, 0, "in setting configuration files!");
    }

    if(conf.command == log){
        log_entry(&conf);
    }
    if(conf.command == oneliner){
        log_oneline(argc, argv, &conf);
    }
    if((conf.command == date) || (conf.command == body) || (conf.command == dir)){
        if(argc < 3){
            error(0, 0, "No regular expression given");
        }
        if(argc > 3){
            fprintf(stderr, "Too many regular expressions. " 
                            "Will only consider first\n");
        }
        conf.regex.myregex = argv[2];
        if(search(&conf)){
            clean_conf(conf.files);
            error(1, 0, "Couldn't search");
        }
    }
    clean_conf(conf.files);

    return 0;
}

/* EOF */
