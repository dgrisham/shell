/*
 * Name: David Grisham
 * Operating Systems -- Project 2 (Shell)
 * Note on datatypes: I used more char *'s instead of strings, mostly so I could
 * practice initializing and using the datatype char * (this happens in, e.g.,
 * the com_cd function)
 */

#include "builtins.h"

using namespace std;


int com_ls(vector<string>& tokens) {
    // if no directory is given, use the local directory
    if (tokens.size() < 2) {
        tokens.push_back(".");
    }

    const char *target = tokens[1].c_str();

    // open the directory
    DIR *dir = opendir(target);

    // catch an errors opening the directory
    if (!dir) {
        // print the error from the last system call with the given prefix
        perror("ls error: ");

       // return error
        return 1;
    }

    // output each entry in the directory
    for (dirent* current = readdir(dir); current; current = readdir(dir)) {
        cout << current->d_name << endl;
    }

    // close directory stream
    if (closedir(dir)) {
        perror("ls error: ");

        return 1;
    }

    // return success
    return 0;
}


int com_cd(vector<string>& tokens) {
    // if no directory given, go to the home directory
    if (tokens.size() < 2) {
        tokens.push_back(getenv("HOME"));
    }

    // store target directory
    const char *target = tokens[1].c_str();

    // try to change into dir, print errors (if any)
    if (chdir(target)) {
        perror("cd error: ");

        return 1;
    }

    char cwd[PATH_MAX + 1];

    // get the cwd and make sure no errors occurred
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("error updating pwd after cd: ");

        return 1;
    }

    // update PWD environment variable w/ new cwd
    setenv("PWD", cwd, 1);

    return 0;
}


int com_pwd(vector<string>& tokens) {
    // print output of pwd()
    cout << pwd() << endl;

    return 0;
}


int com_alias(vector<string>& tokens) {
    if (tokens.size() == 1) {
        // 'alias' called by itself, print all aliases
        map<string, string>::iterator it;

        for (it = aliases.begin(); it != aliases.end(); ++it) {
            cout << "alias " << it->first << "=" << it->second << endl;
        }

        return 0;
    }

    // arg holds the entire argument to 'alias'. cmd and def hold the left/right
    // sides of the alias (i.e. alias <cmd>=<def>)
    char *arg, *cmd, *def;

    arg = new char[1 + tokens[1].length()];
    strcpy(arg, tokens[1].c_str());

    if ((cmd = strtok(arg, "=")) == NULL) {
        perror("alias error, bad input ");

        return 1;
    }

    if ((def = strtok(NULL, "=")) == NULL) {
        perror("alias error, invalid input ");

        return 1;
    }

    aliases.insert(make_pair(cmd, def));
    delete[] arg;

    *arg = *cmd = *def = '\0';

    return 0;
}


int com_unalias(vector<string>& tokens) {
    if (tokens.size() == 2) {
        if (tokens[1] == "-a") {
            aliases.clear();
        } else {
            map<string, string>::iterator alias = aliases.find(tokens[1]);

            if (alias == aliases.end()) {
                perror("unalias error, invalid input ");
            } else {
                aliases.erase(alias);
            }
        }
    } else {
        perror("unalias error, invalid input ");
    }

    return 0;
}


int com_echo(vector<string>& tokens) {
    // remove 'echo' from tokens, since we don't want to print it
    tokens.erase(tokens.begin());

    // output everything in tokens
    for (vector<string>::const_iterator t = tokens.begin(); t != tokens.end();
         ++t)
    {
        cout << *t << ' ';
        
    }

    // print out a newline at the end, since that what echo does
    cout << '\n';

    return 0;
}


int com_exit(vector<string>& tokens) {
    exit(EXIT_SUCCESS);
    return 0;
}


int com_history(vector<string>& tokens) {
    int start = 0;
    int end = history_length;

    if (tokens.size() == 2) {
        start = history_length - strtol(tokens[1].c_str(), NULL, 10);

        if (!end) {
            perror("history error: ");

            return 1;
        }
    } else {
        // if no value specified, output everything
        end = history_length;
    }

    for (int i = start; i < end; ++i) {
        HIST_ENTRY* cmd = history_get(history_base + i);

        if (cmd) {
            cout << i+1 << ' ' << cmd->line << endl;
        }
    }

    return 0;
}


string pwd() {
    // updating PWD should be handled by other functions (namely cd), this
    // function simply grabs the current PWD environment variable
    return getenv("PWD");
}
