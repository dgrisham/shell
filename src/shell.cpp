/*
 * Name: David Grisham
 * Operating Systems -- Project 2 (Shell)
 */


#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "builtins.h"

// Potentially useful #includes (either here or in builtins.h):
//   #include <dirent.h>
//   #include <errno.h>
//   #include <fcntl.h>
//   #include <signal.h>
//   #include <sys/errno.h>
//   #include <sys/param.h>
//   #include <sys/types.h>
//   #include <sys/wait.h>
//   #include <unistd.h>

const int STDIN_CONST = dup(0);
const int STDOUT_CONST = dup(1);

using namespace std;


// The characters that readline will use to delimit words
const char* const WORD_DELIMITERS = " \t\n\"\\'`@><=;|&{(";

// An external reference to the execution environment
extern char** environ;

// Define 'command' as a type for built-in commands
typedef int (*command)(vector<string>&);

// A mapping of internal commands to their corresponding functions
map<string, command> builtins;

// Variables local to the shell
map<string, string> localvars;

// map to hold aliases command (as the key) and corresponding definition (as the
// value)
map<string, string> aliases;

// Handles external commands, redirects, and pipes
int execute_external_command(vector<string> tokens) {
    // the path delimeter
    const char *path_delim = new char[2];;
    path_delim = ":";

    // get path env variable
    string path = getenv("PATH");

    // string vector to hold path directories
    vector<string> path_dirs;

    // char* to hold a single directory in the path
    char *dir = new char[PATH_MAX + 1];

    // use strtok to find all paths in PATH, separated by path_delim (":")
    if (a(dir = strtok((char*)path.c_str(), path_delim)) == NULL) {
        // no path_delim found in path
        if (path == "\0") {
            // path is empty, return 1
            perror("PATH is empty ");
            return 1;
        } else {
            // path had one entry, push it onto path_dirs
            path_dirs.push_back(path);
        }
    } else {
        // path_delim found in path, which means there are multiple dirs in path
        
        // push the first directory (found in the first call to strtok) onto path_dirs
        path_dirs.push_back(dir);

        // find all of the other dirs in path
        while ((dir = strtok(NULL, path_delim)) != NULL) {
            // ...and push each onto path_dirs
            path_dirs.push_back(dir);
        }
    }

    // char *arg = (char *)malloc(tokens.size());
    char *args[(tokens.size() + 1)*sizeof(char*)];

    string cmd = tokens[0];

    // look for input in path
    for (int i = 0; i < path_dirs.size(); ++i) {
        // string to hold path + filename that *may* be what we're looking for
        string test_loc = path_dirs[i];

        // append filename to end of path
        test_loc += "/";
        test_loc += cmd;
        /*
         * if (access(test_loc.c_str(), F_OK)==0)
         */
        struct stat *file_info;

        // use stat() to test if file exists
        if (access(test_loc.c_str(), F_OK) == 0) {

            int j = 0;
            args[j] = (char *)(malloc((test_loc.length() + 1) * sizeof(char)));
            strcpy(args[j], test_loc.c_str());
            ++j;

            for (; j < tokens.size(); ++j) {
                args[j] = (char *)malloc((tokens[j].length() + 1) * sizeof(char));
                strcpy(args[j], tokens[j].c_str());
            }

            args[tokens.size()] = (char*)malloc(1);
            args[tokens.size()] = NULL;

            pid_t cpid, cpid2;
            int status;

            if ((cpid = fork()) == -1) {
                perror("process fork failed");
                return 1;
            }

            if (cpid == 0) {
                execvp(args[0], args);
                perror("exec failed");
                return -1;
            } else {
                cpid2 = waitpid(cpid, &status, 0);

                return WEXITSTATUS(status);
            }
        }
    }

    fprintf(stderr, "error: command not found\n");
    return 1;
}

char *prompt_state(int return_value) {
    char *state = new char[64];

    if (return_value == 0) {
        sprintf(state, "+");
    } else {
        sprintf(state, "-");
    }

    return state;
}

// Return a string representing the prompt to display to the user. It needs to
// include the current working directory and should also use the return value to
// indicate the result (success or failure) of the last command.
char *get_prompt(int return_value) {
    char *prompt = new char[1024];

    char *cwd = new char[PATH_MAX + 1];
    getcwd(cwd, sizeof(cwd));

     // TODO: use strpbrk to get two nearest ancestor directories

    strcpy(prompt, prompt_state(return_value));
    return (char *)"prompt > "; // replace with your own code
}


// Return one of the matches, or NULL if there are no more
char* pop_match(vector<string>& matches) {
    if (matches.size() > 0) {
        const char* match = matches.back().c_str();

        // Delete the last element
        matches.pop_back();

        // We need to return a copy, because readline deallocates when done
        char* copy = (char*) malloc(strlen(match) + 1);
        strcpy(copy, match);

        return copy;
    }

    // No more matches
    return NULL;
}


// Generates environment variables for readline completion. This function will
// be called multiple times by readline and will return a single cstring each
// time.
char* environment_completion_generator(const char* text, int state) {
    // A list of all the matches;
    // Must be static because this function is called repeatedly
    static vector<string> matches;

    // If this is the first time called, construct the matches list with
    // all possible matches
    if (state == 0) {
        // TODO: YOUR CODE GOES HERE
    }

    // Return a single match (one for each time the function is called)
    return pop_match(matches);
}


// Generates commands for readline completion. This function will be called
// multiple times by readline and will return a single cstring each time.
char* command_completion_generator(const char* text, int state) {
    // A list of all the matches;
    // Must be static because this function is called repeatedly
    static vector<string> matches;

    // If this is the first time called, construct the matches list with
    // all possible matches
    if (state == 0) {
        // TODO: YOUR CODE GOES HERE
    }

    // Return a single match (one for each time the function is called)
    return pop_match(matches);
}


// This is the function we registered as rl_attempted_completion_function. It
// attempts to complete with a command, variable name, or filename.
char** word_completion(const char* text, int start, int end) {
    char** matches = NULL;

    if (start == 0) {
        rl_completion_append_character = ' ';
        matches = rl_completion_matches(text, command_completion_generator);
    } else if (text[0] == '$') {
        rl_completion_append_character = ' ';
        matches = rl_completion_matches(text, environment_completion_generator);
    } else {
        rl_completion_append_character = '\0';
        // We get directory matches for free (thanks, readline!)
    }

    return matches;
}


// Transform a C-style string into a C++ vector of string tokens, delimited by
// whitespace
vector<string> tokenize(const char* line) {
    vector<string> tokens;
    string token;

    // istringstream allows us to treat the string like a stream
    istringstream token_stream(line);

    while (token_stream >> token) {
        tokens.push_back(token);
    }

    // Search for quotation marks, which are explicitly disallowed
    for (size_t i = 0; i < tokens.size(); i++) {

        if (tokens[i].find_first_of("\"'`") != string::npos) {
            cerr << "\", ', and ` characters are not allowed." << endl;

            tokens.clear();
        }
    }

    return tokens;
}

// Examines each token and sets an env variable for any that are in the form
// of key=value
void local_variable_assignment(vector<string>& tokens) {
    vector<string>::iterator token = tokens.begin();

    // we don't want to process the arguments to an 'alias' command
    if (*token == "alias") return;

    while (token != tokens.end()) {
        string::size_type eq_pos = token->find("=");

        // If there is an equal sign in the token, assume the token is var=value
        if (eq_pos != string::npos) {
            string name = token->substr(0, eq_pos);
            string value = token->substr(eq_pos + 1);

            localvars[name] = value;

            token = tokens.erase(token);
        } else {
            ++token;
        }
    }
}

// Substitutes any tokens that start with a $ with their appropriate value, or
// with an empty string if no match is found
void variable_substitution(vector<string>& tokens) {
    vector<string>::iterator token;

    for (token = tokens.begin(); token != tokens.end(); ++token) {

        if (token->at(0) == '$') {
            string var_name = token->substr(1);

            if (getenv(var_name.c_str()) != NULL) {
                *token = getenv(var_name.c_str());
            } else if (localvars.find(var_name) != localvars.end()) {
                *token = localvars.find(var_name)->second;
            } else {
                *token = "";
            }
        }
    }
}

void redirect_output(vector<string>& tokens) {
    FILE *out;

    // sorry for the repetitive code, I'll fix it for the final submission
    for (vector<string>::iterator s = tokens.begin(); s != tokens.end(); ++s) {
        if (strcmp(s->c_str(), ">") == 0) {
            advance(s, 1);
            out = fopen(s->c_str(), "w");
            dup2(fileno(out), STDOUT_FILENO);
            fclose(out);
        } else if (strcmp(s->c_str(), ">>") == 0) {
            advance(s, 1);
            out = fopen(s->c_str(), "a+");
            dup2(fileno(out), STDOUT_FILENO);
            fclose(out);
        }
    }

    for (vector<string>::iterator s2 = tokens.begin(); s2 != tokens.end(); ++s2) {
        if (strcmp(s2->c_str(), ">") == 0) {
            tokens.erase(s2, tokens.end());
            break;
        } else if (strcmp(s2->c_str(), ">>") == 0) {
            tokens.erase(s2, tokens.end());
            break;
        }
    }
}

vector<string> handle_piping(vector<string> tokens) {
    vector<string>::iterator s;
    for (s = tokens.begin(); s != tokens.end(); ++s) {
        if (strcmp(s->c_str(), "|") == 0) {
            break;
        }
    }

    if (s == tokens.end()) {
        return tokens;
    }

    int pipefd[2];
    pid_t cpid;
    string output;
    char buf;

    vector<string> child_tokens = tokens;


    if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    if ((cpid = fork()) == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        tokens.erase(tokens.begin(), ++s);

        close(pipefd[1]);

        string input;

        char c;
        while(read(STDIN_FILENO, &c, 1) > 0) {
            input += c;
        }

        close(pipefd[0]);

        return tokens;
    } else {
        dup2(pipefd[1], STDOUT_FILENO);

        close(pipefd[1]);
        close(pipefd[0]);

        tokens.erase(s, tokens.end());

        return tokens;
    }
}

// general function to process a line
vector<string> process_line(const char *line) {
    vector<string> tokens = tokenize(line);

    // handle piping...
    tokens = handle_piping(tokens);

    // handle local vars...
    local_variable_assignment(tokens);

    // sub variable refs...
    variable_substitution(tokens);

    // take care of file redirection...
    redirect_output(tokens);

    return tokens;
}

// Executes a line of input by either calling execute_external_command or
// directly invoking the built-in command
int execute_line(vector<string>& tokens, map<string, command>& builtins)
{
    int return_value = 0;

    if (tokens.size() != 0) {
        map<string, command>::iterator cmd = builtins.find(tokens[0]);

        if (cmd == builtins.end()) {
            map<string, string>::iterator alias = aliases.find(tokens[0]);

            if (alias == aliases.end()) {
                // not a builtin or alias, try external commands
                return_value = execute_external_command(tokens);
            } else {

                // handle aliases

                // get alias definition
                const char *alias_cmd = alias->second.c_str();

                // process the command and get tokens
                vector<string> alias_tokens = process_line(alias_cmd);

                // insert the tokens from the alias replacement into the start of
                // the (original) tokens array
                tokens.erase(tokens.begin());
                tokens.insert(tokens.begin(), alias_tokens.begin(), alias_tokens.end());

                // execute the command
                return_value = execute_line(tokens, builtins);
            }
        } else {
            return_value = ((*cmd->second)(tokens));
        }
    }

    return return_value;
}

char *bang(char *line) {
    // stores the offset (from history_base) of the command we want
    int offset;
    
    char *bad_input = new char[1];
    bad_input[0] = '\0';

    if (line[1] == '!') {
        // make sure there's a previous command to execute
        if (history_length == 0) {
            return bad_input;
        }

        // call last command
        offset = history_length - 1;
    } else if (line[1] == '-') {
        int N = line[2] - '0';

        if ((N <= 0) || (N > history_length)) {
            printf("invalid input (use !-N, where N is an integer)\n");
            return bad_input;
        }

        offset = history_length - N;
    } else {
        // call Nth line in history
        int N = line[1] - '0';

        if ((N <= 0) || (N > history_length - history_base + 1)) {
            printf("invalid input (use !N, where N is an integer)\n");
            return bad_input;
        }

        offset = N - 1;
    }

    // deallocate line's memory
    free(line);
    // get new buffer size for line
    int buf_size = strlen(history_get(history_base + offset)->line);
    // allocate space for line
    line = new char[buf_size + 1];

    strcpy(line, history_get(history_base + offset)->line);
    printf("%s\n", line);

    return line;
}

// The main program
int main() {
    // Populate the map of available built-in functions
    builtins["ls"] = &com_ls;
    builtins["cd"] = &com_cd;
    builtins["pwd"] = &com_pwd;
    builtins["alias"] = &com_alias;
    builtins["unalias"] = &com_unalias;
    builtins["echo"] = &com_echo;
    builtins["exit"] = &com_exit;
    builtins["history"] = &com_history;

    // Specify the characters that readline uses to delimit words
    rl_basic_word_break_characters = (char *) WORD_DELIMITERS;

    // Tell the completer that we want to try completion first
    rl_attempted_completion_function = word_completion;

    // The return value of the last command executed
    int return_value = 0;

    char *line;

    vector<string> tokens;

    pid_t main_pid = getpid();

    // Loop for multiple successive commands 
    while (true) {

        // Get the prompt to show, based on the return value of the last command
        string prompt = get_prompt(return_value);

        // Read a line of input from the user
        line = readline(prompt.c_str());

        // If the pointer is null, then an EOF has been received (ctrl-d)
        if (!line) {
            break;
        }

        // If the command is non-empty, attempt to execute it
        if (line[0]) {
            if (line[0] == '!') {
                line = bang(line);

                if (!line[0]) {
                    continue;
                }
            }

            // Add this command to readline's history
            add_history(line);

            // split line into tokens, handle variables, etc.
            tokens = process_line(line);

            // Execute the line
            return_value = execute_line(tokens, builtins);

            // empty tokens before next round
            tokens.clear();

            // reset output (in case redirected before)
            if (STDOUT_FILENO != STDOUT_CONST) {
                close(STDOUT_FILENO);
                dup2(STDOUT_CONST, STDOUT_FILENO);
            }

            if (STDIN_FILENO != STDIN_CONST) {
                close(STDIN_FILENO);
                dup2(STDIN_CONST, STDIN_FILENO);
            }

            if (getpid() != main_pid) {
                _exit(EXIT_SUCCESS);
            } else {
                wait(NULL);
            }
        }
    }
    return 0;
}
