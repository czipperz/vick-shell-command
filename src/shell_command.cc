#include <mutex>
#include <unistd.h>
#include <functional>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>

#include "operating_system_macros.hh"
#include "contents.hh"
#include "split.hh"
#include "file_contents.hh"

namespace vick {
namespace shell_command {

/// Not `static` for testing terms
std::string quote_string_windows(const std::string& s) {
    std::string ret;
    ret.reserve(s.size());

    // escape by wrapping double quotes and escaping double quotes
    // inside arg
    ret += '"';
    std::string::const_iterator i = s.begin(), e = s.end(), p;
beg:
    p = i;
    i = std::find(i, e, '"');
    ret.append(p, i);
    if (i != e) {
        ret += '\\';
        ret += '"';
        ++i;
        goto beg;
    }
    ret += '"';

    return ret;
}

/// Not `static` for testing terms
std::string quote_string_linux(const std::string& s) {
    std::string ret;
    ret.reserve(s.size());

    // escape by wrapping single quotes and escaping single quotes and
    // raw `\n`s
    ret += '\'';
    std::string::const_iterator i = s.begin(), e = s.end(), p;
beg:
    p = i;
    i = std::find(i, e, '\'');
    ret.append(p, i);
    if (i != e) {
        ret += "'\\''";
        ++i;
        goto beg;
    }
    ret += '\'';

    return ret;
}

std::string quote_string(const std::string& s) {
    if (IS_OS_WIN) {
        return quote_string_windows(s);
    } else {
        return quote_string_linux(s);
    }
}

class exec_shell_command_exception : public std::exception {
    std::string message;

public:
    exec_shell_command_exception(std::string message)
        : message(std::move(message)) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

void exec_shell_command(const std::string& cmd, std::string* out_,
                        std::string* err_) {
    *out_ = "";
    *err_ = "";

    int out[2], err[2];

    if (pipe(out) == -1)
        throw exec_shell_command_exception(
            "Couldn't open stdout pipe");
    if (pipe(err) == -1)
        throw exec_shell_command_exception(
            "Coudln't open stderr pipe");

    switch (fork()) {
        case -1:
            throw exec_shell_command_exception(
                "Forking error on stdout");
        case 0: {
            close(out[0]);
            close(err[0]);
            dup2(out[1], STDOUT_FILENO);
            dup2(err[1], STDERR_FILENO);
            system(cmd.c_str());
            _Exit(-1);
        }
        default: {
            close(out[1]);
            close(err[1]);
            char readbuffer[1024];
            memset(readbuffer, '\0', sizeof(readbuffer));
            int nbytes = sizeof(readbuffer) - 1;
            while (std::find(readbuffer, readbuffer + nbytes, EOF) ==
                       readbuffer + nbytes and
                   nbytes) {
                nbytes =
                    read(out[0], readbuffer, sizeof(readbuffer) - 1);
                readbuffer[nbytes] = 0;
                *out_ += readbuffer;
            }
            close(out[0]);

            memset(readbuffer, '\0', sizeof(readbuffer));
            nbytes = sizeof(readbuffer) - 1;
            while (std::find(readbuffer, readbuffer + nbytes, EOF) ==
                       readbuffer + nbytes and
                   nbytes) {
                nbytes =
                    read(err[0], readbuffer, sizeof(readbuffer) - 1);
                readbuffer[nbytes] = 0;
                *err_ += readbuffer;
            }
        }
    }
}

void exec_shell_command(const std::string& cmd, std::string* out_) {
    *out_ = "";

    int out[2];

    if (pipe(out) == -1) {
        throw exec_shell_command_exception(
            "Couldn't generate a pipe");
    }

    switch (fork()) {
        case -1:
            throw exec_shell_command_exception(
                "Couldn't fork the process");
        case 0: { // pump data into pipe
            close(out[0]);
            dup2(out[1], STDOUT_FILENO);
            dup2(out[1], STDERR_FILENO);
            std::vector<const char*> command = {"sh", "-c",
                                                cmd.c_str(), nullptr};
            execv("/bin/sh", const_cast<char* const*>(
                                 static_cast<const char* const*>(
                                     command.data())));
            _Exit(-1);
        }
        default: { // transfer pipe to param out_
            close(out[1]);
            char readbuffer[1024];
            memset(readbuffer, '\0', sizeof(readbuffer));
            int nbytes = sizeof(readbuffer) - 1;
            while (std::find(readbuffer, readbuffer + nbytes, EOF) ==
                       readbuffer + nbytes and
                   nbytes) {
                nbytes =
                    read(out[0], readbuffer, sizeof(readbuffer) - 1);
                readbuffer[nbytes] = 0;
                *out_ += std::string(readbuffer);
            }
        }
    }
}

void exec_shell_command(const std::string& cmd,
                        std::function<void(std::string)> out_,
                        size_t buffer_size) {
    int out[2];

    if (pipe(out) == -1) {
        throw exec_shell_command_exception(
            "Couldn't generate a pipe");
    }

    switch (fork()) {
        case -1:
            throw exec_shell_command_exception(
                "Couldn't fork the process");
        case 0: {
            close(out[0]);
            dup2(out[1], STDOUT_FILENO);
            dup2(out[1], STDERR_FILENO);
            const char* const args[] = {"/bin/sh", "-c", cmd.c_str(),
                                        nullptr};
            execv("/bin/sh", const_cast<char* const*>(args));
            _Exit(1);
        }
        default: {
            close(out[1]);
            char readbuffer[buffer_size];
            memset(readbuffer, 0, sizeof(readbuffer));
            int nbytes = buffer_size;
            bool cont = false;
            while (nbytes and
                   std::find(readbuffer, readbuffer + nbytes, EOF) ==
                       readbuffer + nbytes) {
                if (cont) {
                    out_(readbuffer);
                    cont = false;
                }

                nbytes = read(out[0], readbuffer, buffer_size);
                readbuffer[nbytes] = 0;

                if (readbuffer[0] == '\0')
                    cont = true;
                else
                    out_(readbuffer);
            }
        }
    }
}

void exec_shell_command(const std::string& cmd,
                        std::function<void(std::string)> out_,
                        std::function<void(std::string)> err_,
                        size_t buffer_size) {
    int out[2], err[2];

    if (pipe(out) == -1 or pipe(err) == -1) {
        throw exec_shell_command_exception(
            "Couldn't generate a pipe");
    }

    pid_t p_read, p_err;
    if ((p_read = fork()) == -1 or (p_err = fork()) == -1)
        throw exec_shell_command_exception(
            "Couldn't fork the process");
    if (not p_read and p_err)
        _Exit(-1);
    if (not p_read) {
        close(out[0]);
        dup2(out[1], STDOUT_FILENO);
        dup2(err[1], STDERR_FILENO);
        std::vector<const char*> command = {"sh", "-c", cmd.c_str(),
                                            nullptr};
        execv("/bin/sh",
              const_cast<char* const*>(
                  static_cast<const char* const*>(command.data())));
        _Exit(-1);
    } else if (p_err) {
        close(out[0]);
        close(out[1]);
        close(err[1]);
        char readbuffer[buffer_size + 1];
        memset(readbuffer, '\0', buffer_size);
        int nbytes = buffer_size;
        bool cont = false;
        while (nbytes and
               std::find(readbuffer, readbuffer + nbytes, EOF) ==
                   readbuffer + nbytes) {
            if (cont) {
                err_(readbuffer);
                cont = false;
            }

            nbytes = read(err[0], readbuffer, buffer_size);
            readbuffer[nbytes] = 0;
            if (readbuffer[0] == '\0')
                cont = true;
            else
                err_(readbuffer);
        }
    } else {
        close(out[1]);
        close(err[0]);
        close(err[1]);
        char readbuffer[buffer_size + 1];
        memset(readbuffer, '\0', buffer_size);
        int nbytes = buffer_size;
        bool cont = false;
        while (nbytes and
               std::find(readbuffer, readbuffer + nbytes, EOF) ==
                   readbuffer + nbytes) {
            if (cont) {
                out_(readbuffer);
                cont = false;
            }

            nbytes = read(out[0], readbuffer, buffer_size);
            readbuffer[nbytes] = 0;

            if (readbuffer[0] == '\0')
                cont = true;
            else
                out_(readbuffer);
        }
        _Exit(-1);
    }
}

void exec_shell_command(const std::string& cmd, contents& cont) {
    cont.cont.clear();
    cont.x = 0;
    cont.y = 0;
    std::mutex mut;
    bool pb_empty;
    exec_shell_command(cmd,
                       [&cont, &mut, &pb_empty](std::string str) {
                           std::lock_guard<std::mutex> _(mut);
                           if (pb_empty) {
                               pb_empty = false;
                               cont.cont.push_back("");
                           }

                           if ((str.empty()) or
                               (str.size() == 1 and str[0] == '\n'))
                               ;
                           else if (str.find('\n') ==
                                    std::string::npos) {
                               if (cont.cont.empty())
                                   cont.cont.push_back(str);
                               else
                                   cont.cont.back() += str;
                           } else {
                               std::vector<std::string> splitted =
                                   split(str, '\n');
                               auto itr = splitted.begin();
                               if (cont.cont.empty())
                                   cont.cont.push_back(*itr);
                               else
                                   cont.cont.back() += *itr;
                               ++itr;
                               bool contin = false;
                               for (auto end = splitted.end();
                                    itr != end; ++itr) {
                                   if (contin) {
                                       contin = false;
                                       cont.cont.push_back(*itr);
                                   }
                                   if (itr->empty()) {
                                       contin = true;
                                       continue;
                                   }
                                   cont.cont.push_back(*itr);
                               }
                           }

                           if (str.size() and
                               str[str.size() - 1] == '\n')
                               pb_empty = true;
                           print_contents(cont);
                       },
                       1024);
}
}
}
