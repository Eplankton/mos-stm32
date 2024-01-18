//////////////////////////////////////////////////////////////////////////
//                          The MOS Shell
//                  Eplankton (wikechao@gmail.com)
//                   https://github.com/Eplankton
//               East China Normal University, Shanghai
//                  The Apache License, Version 2.0
//////////////////////////////////////////////////////////////////////////

#ifndef _MOS_SHELL_
#define _MOS_SHELL_

#include "kernel/utils.hpp"
#include "kernel/task.hpp"

namespace MOS::Shell
{
	struct Command_t
	{
		using Ret_t  = void;
		using Text_t = const char*;
		using Argv_t = const char*;
		using Fn_t   = Ret_t (*)(Argv_t);

		Text_t text;
		Fn_t callback;

		MOS_INLINE inline uint32_t
		len() const { return Utils::strlen(text); }

		MOS_INLINE inline void
		run(Argv_t argv) const { callback(argv); }

		inline Argv_t match(Text_t str) const
		{
			const auto xlen = len(); // The length of a command

			// Skip all blank
			auto skip = [](Text_t str) {
				while (*str == ' ') ++str;
				return str;
			};

			// Check whether match or not
			auto check = [&](Text_t str) {
				return (str[xlen] == ' ' || str[xlen] == '\0') &&
				       Utils::strncmp(str, text, xlen) == 0;
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
		using Argv_t = Command_t::Argv_t;

		static inline void ls_cmd(Argv_t argv)
		{
			auto name = argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					// todo!()
				}
				else {
					MOS_MSG("Unknown task '%s'\n", name);
				}
			}
			else { // No arguments provided
				Task::print_all();
			}
		}

		static inline void kill_cmd(Argv_t argv)
		{
			auto name = argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					Task::terminate(tcb);
					MOS_MSG("Task '%s' terminated\n", name);
				}
				else {
					MOS_MSG("Unknown task '%s'\n", name);
				}
			}
			else {
				MOS_MSG("Invalid Arguments\n");
			}
		}

		static inline void date_cmd(Argv_t argv)
		{
			if (auto tcb = Task::find("Calendar")) {
				Task::resume(tcb);
			}
			else {
				MOS_MSG("No Calendar found!\n");
			}
		}

		static inline void uname_cmd(Argv_t argv)
		{
			kprintf(" A_A       _\n"
			        "o'' )_____//  Version @ %s\n"
			        " `_/  MOS  )  Build   @ %s, %s\n"
			        " (_(_/--(_/   Chip    @ %s, %s\n",
			        MOS_VERSION,
			        __TIME__, __DATE__,
			        MOS_MCU, MOS_ARCH);
		}

		static inline void reboot_cmd(Argv_t argv)
		{
			MOS_MSG("Reboot!\n\n\n");
			MOS_REBOOT();
		}
	}

	// Add more commands here with {"text", CmdCall::callback}
	static constexpr Command_t cmds[] = {
	        {    "ls",     CmdCall::ls_cmd},
	        {  "kill",   CmdCall::kill_cmd},
	        {  "date",   CmdCall::date_cmd},
	        { "uname",  CmdCall::uname_cmd},
	        {"reboot", CmdCall::reboot_cmd},
	};

	inline void launch(void* input_buf)
	{
		using Text_t     = Command_t::Text_t;
		using RxBufPtr_t = DataType::RxBuffer_t<Macro::RX_BUF_SIZE>*;

		static auto parser = [](Text_t str) {
			for (const auto& cmd: cmds) {
				if (auto argv = cmd.match(str)) {
					return cmd.run(argv);
				}
			}
			MOS_MSG("Unknown command '%s'\n", str);
		};

		CmdCall::uname_cmd(nullptr);
		Task::print_all();

		auto rx_buf = (RxBufPtr_t) input_buf;

		while (true) {
			// Valid input should end with '\n'
			if (rx_buf->back() == '\n') {
				rx_buf->pop();
				auto rx = rx_buf->c_str();
				kprintf("> %s\n", rx);
				parser(rx);
				rx_buf->clear();
			}
			else {
				Task::delay(5);
			}
		}
	}
}

#endif