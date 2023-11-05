#ifndef _MOS_SHELL_
#define _MOS_SHELL_

#include "config.h"
#include "util.hpp"
#include "task.hpp"

namespace MOS::Shell
{
	struct Command
	{
		using Ret_t  = void;
		using Text_t = const char*;
		using Argv_t = const char*;
		using Fn_t   = Ret_t (*)(Argv_t);

		Text_t text;
		Fn_t fn;

		__attribute__((always_inline)) inline auto
		len() const { return strlen(text); }

		__attribute__((always_inline)) inline void
		run(const char* argv) const { fn(argv); }

		inline const char* match(const char* str) const
		{
			auto xlen = len();
			if (strncmp(str, text, xlen) == 0) {
				const char* argv = str + xlen;
				while (*argv == ' ') ++argv;
				return argv;
			}
			else {
				return nullptr;
			}
		}
	};

	namespace CmdCall
	{
		static inline void ls_cmd(const char* argv)
		{
			auto name = (const char*) argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					MOS_DISABLE_IRQ();
					MOS_MSG("-----------------------------------\n");
					Task::print_task(tcb->node);

					// for (auto t = tcb; t != nullptr; t = t->get_parent()) {
					// 	printf("%s", t->get_name());
					// 	if (t->get_parent() != nullptr) {
					// 		printf("->");
					// 	}
					// }

					MOS_MSG("-----------------------------------\n");
					MOS_ENABLE_IRQ();
				}
				else {
					MOS_MSG("[MOS]: Unknown task '%s'\n", name);
				}
			}
			else {// No arguments provided
				Task::print_all_tasks();
			}
		}

		static inline void kill_cmd(const char* argv)
		{
			auto name = (const char*) argv;
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

		static inline void reboot_cmd(const char* argv)
		{
			MOS_MSG("[MOS]: Reboot\n\n");
			MOS_REBOOT();
		}

		static inline void uname_cmd(const char* argv)
		{
			MOS_MSG(" A_A       _\n"
			        "o'' )_____//   Build    = %s, %s\n"
			        " `_/  MOS  )   Version  = %s\n"
			        " (_(_/--(_/    Platform = %s\n",
			        __TIME__, __DATE__, MOS_VERSION, MOS_DEVICE);
		}
	}

	// Add more cmds here...
	static constexpr Command cmds[] = {
	        {    "ls",     CmdCall::ls_cmd},
	        {  "kill",   CmdCall::kill_cmd},
	        { "uname",  CmdCall::uname_cmd},
	        {"reboot", CmdCall::reboot_cmd},
	};

	inline void launch(void* argv)
	{
		using KernelGlobal::rx_buf;

		static auto parser = [](const char* str) {
			for (auto& cmd: cmds) {
				if (auto argv = cmd.match(str)) {
					cmd.run(argv);
					return;
				}
			}
			MOS_MSG("[MOS]: Unknown command '%s'\n", str);
		};

		CmdCall::uname_cmd(nullptr);
		Task::print_all_tasks();

		while (true) {
			if (!rx_buf.empty()) {
				auto rx_str = rx_buf.c_str();
				MOS_MSG("< %s\n", rx_str);
				parser(rx_str);
				rx_buf.clear();
			}
		}
	}
}

#endif