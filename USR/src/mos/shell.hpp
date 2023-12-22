#ifndef _MOS_SHELL_
#define _MOS_SHELL_

#include "kernel/util.hpp"
#include "kernel/task.hpp"

namespace MOS::Shell
{
	struct Command
	{
		using Ret_t  = void;
		using Text_t = const char*;
		using Argv_t = const char*;
		using Fn_t   = Ret_t (*)(Argv_t);

		Text_t text;
		Fn_t callback;

		__attribute__((always_inline)) inline auto
		len() const { return Util::strlen(text); }

		__attribute__((always_inline)) inline void
		run(Argv_t argv) const { callback(argv); }

		inline Argv_t match(Text_t str) const
		{
			uint32_t xlen = len();

			auto skip = [](Text_t str) {
				while (*str == ' ') ++str;
				return str;
			};

			auto check = [&](Text_t str) {
				return (str[xlen] == ' ' || str[xlen] == '\0') &&
				       Util::strncmp(str, text, xlen) == 0;
			};

			str = skip(str);

			if (check(str)) {
				// Return first argument
				return skip(str + xlen);
			}
			else {
				return nullptr;
			}
		}
	};

	namespace CmdCall
	{
		using Argv_t = Command::Argv_t;

		static inline void ls_cmd(Argv_t argv)
		{
			auto name = argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					// todo()
				}
				else {
					MOS_MSG("[MOS]: Unknown task '%s'\n", name);
				}
			}
			else {// No arguments provided
				Task::print_all_tasks();
			}
		}

		static inline void kill_cmd(Argv_t argv)
		{
			auto name = argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					Task::terminate(tcb);
					MOS_MSG("[MOS]: Task '%s' terminated\n", name);
				}
				else {
					MOS_MSG("[MOS]: Unknown task '%s'\n", name);
				}
			}
			else {
				MOS_MSG("[MOS]: Invalid Arguments\n");
			}
		}

		static inline void reboot_cmd(Argv_t argv)
		{
			MOS_MSG("[MOS]: Reboot\n\n\n");
			MOS_REBOOT();
		}

		static inline void uname_cmd(Argv_t argv)
		{
			MOS_MSG(" A_A       _\n"
			        "o'' )_____//  Version  @ %s\n"
			        " `_/  MOS  )  Platform @ %s, %s\n"
			        " (_(_/--(_/   Build    @ %s, %s\n",
			        MOS_VERSION, MOS_DEVICE, MOS_CPU, __TIME__, __DATE__);
		}
	}

	// Add more cmds here with {"text", CmdCall::callback}
	static constexpr Command cmds[] = {
	        {    "ls",     CmdCall::ls_cmd},
	        {  "kill",   CmdCall::kill_cmd},
	        { "uname",  CmdCall::uname_cmd},
	        {"reboot", CmdCall::reboot_cmd},
	};

	inline void launch(void* argv)
	{
		using KernelGlobal::rx_buf;

		auto parser = [](Command::Text_t str) {
			for (auto& cmd: cmds) {
				if (auto argv = cmd.match(str)) {
					return cmd.run(argv);
				}
			}
			MOS_MSG("[MOS]: Unknown command '%s'\n", str);
		};

		CmdCall::uname_cmd(nullptr);
		Task::print_all_tasks();

		while (true) {
			// Valid input should end with '\n'
			if (rx_buf.back() == '\n') {
				rx_buf.pop();
				auto rx_str = rx_buf.c_str();
				MOS_MSG("> %s\n", rx_str);
				parser(rx_str);
				rx_buf.clear();
			}
			else {
				Task::yield();
			}
		}
	}
}

#endif