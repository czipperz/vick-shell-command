#include <functional>
#include <string>

namespace vick {
class contents;

namespace shell_command {

/*!
 * \brief Defines exec_shell_command(), which has four overloads for
 * different usages.
 */

/*!
 * \brief Runs the command given by \c cmd and puts stdout into \c out
 * and stderr into \c err.
 */
void exec_shell_command(const std::string& cmd, std::string* out,
                        std::string* err);

/*!
 * \brief Runs the command given by \c cmd and puts stdout and stderr
 * into \c out.
 */
void exec_shell_command(const std::string& cmd, std::string* out);

/*!
 * \brief Runs the command given by \c cmd and occasionally calls \out
 * with the output of stdout and stderr, as a string with a max length
 * given by \c buffer_max_size.
 *
 * \param [in] cmd The shell command to run to get the output of.
 * \param [out] out The function to be occasionally called with the
 * output of stdout and stderr, represented as a string with a max
 * length given by \c buffer_max_size.
 * \param [in] buffer_max_size The maximum size of the string that out
 * will be called with.  This includes the null character terminator,
 * so it should be at least two!
 */
void exec_shell_command(const std::string& cmd,
                        std::function<void(std::string)> out,
                        size_t buffer_max_size = 1024);

/*!
 * \brief Runs the command given by \c cmd and calls \c out and \c err
 * with the output of stdout and stderr, respectively, as a string
 * with a max length given by \c buffer_max_size.
 *
 * The arguments out and err may be called in a seemingly random
 * order, so for example this call to exec_shell_command():
 *
 * \code 
 * exec_shell_command("echo exec_shell_command ;"
                      "echo can have strange behavior 1>&2;"
                      "echo ending my statement",
                      [](std::string str){printf("<<<%s>>>", str.c_str());},
                      [](std::string str){printf("(((%s)))", str.c_str());});
 * \endcode
 *
 * May output the lines in any order, it is completely undefined
 * behavior.  Also, the ``>>>``s can be outputted separately from the
 * ``<<<%s`` part!
 *
 * then out may be called with "exec_shell_command()\necho ending my
 * statement" before err is called with "can have strange behavior".
 * This is a problem with side effects in multithreading!
 *
 * \param [in] cmd The shell command to run to get the output of.
 * \param [out] out The function to be occasionally called with the
 * output of stdout, represented as a string with a max length given by
 * \c buffer_max_size.
 * \param [out] err The function to be occasionally called with the
 * output of stderr, represented as a string with a max length given
 * by \c buffer_max_size.
 * \param [in] buffer_max_size The maximum size of the string that out
 * and err will be called with.  This includes the null character
 * terminator, so it should be at least two!
 */
void exec_shell_command(const std::string& cmd,
                        std::function<void(std::string)> out,
                        std::function<void(std::string)> err,
                        size_t buffer_max_size = 1024);

/*!
 * \brief Runs the command given by \c cmd and puts stdout and stderr
 * into ``contents.cont``.
 */
void exec_shell_command(const std::string& cmd, contents& contents);

}
}
