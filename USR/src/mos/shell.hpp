//////////////////////////////////////////////////////////////////////////
//                          The MOS Shell                               //
//                  Eplankton (wikechao@gmail.com)                      //
//                   https://github.com/Eplankton                       //
//               East China Normal University, Shanghai                 //
//                  The Apache License, Version 2.0                     //
//////////////////////////////////////////////////////////////////////////

#ifndef _MOS_SHELL_
#define _MOS_SHELL_

#include "kernel/utils.hpp"
#include "kernel/task.hpp"
#include "kernel/data_type/buffer.hpp"

namespace MOS::Shell
{
	using Utils::DisIntrGuard_t;
	using Utils::strlen;
	using Utils::strncmp;

	struct Command_t
	{
		using Ret_t  = void;
		using Text_t = const char*;
		using Argv_t = const char*;
		using Fn_t   = Ret_t (*)(Argv_t);

		Text_t text;
		Fn_t callback;

		MOS_INLINE inline uint32_t
		len() const { return strlen(text); }

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
				return (str[xlen] == ' ' ||
				        str[xlen] == '\0') &&
				       strncmp(str, text, xlen) == 0;
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
					MOS_MSG("Unknown task '%s'", name);
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
					MOS_MSG("Task '%s' terminated", name);
				}
				else {
					MOS_MSG("Unknown task '%s'", name);
				}
			}
			else {
				MOS_MSG("Invalid Arguments");
			}
		}

		static inline void date_cmd(Argv_t argv)
		{
			if (auto tcb = Task::find("Calendar")) {
				Task::resume(tcb);
			}
			else {
				MOS_MSG("Calendar not found!");
			}
		}

		static inline void uname_cmd(Argv_t argv)
		{
			DisIntrGuard_t guard;
			kprintf(
			    " A_A       _\n"
			    "o'' )_____//  Version @ %s\n"
			    " `_/  MOS  )  Build   @ %s, %s\n"
			    " (_(_/--(_/   Chip    @ %s, %s\n",
			    MOS_VERSION,
			    __TIME__, __DATE__,
			    MOS_MCU, MOS_ARCH
			);
		}

		static inline void reboot_cmd(Argv_t argv)
		{
			MOS_MSG("Reboot!\n\n");
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

	using SyncRxBuf_t = DataType::SyncRxBuf_t<Macro::RX_BUF_SIZE>;

	void launch(SyncRxBuf_t& rx_buf)
	{
		using Text_t = Command_t::Text_t;

		auto parser = [](Text_t str) {
			for (const auto& cmd: cmds) {
				if (auto argv = cmd.match(str)) {
					return cmd.run(argv);
				}
			}
			MOS_MSG("Unknown command '%s'", str);
		};

		CmdCall::uname_cmd(nullptr);
		Task::print_all();

		while (true) {
			rx_buf.wait(); // Sync from ISR
			rx_buf.pop();  // Parsing begins
			auto rx = rx_buf.c_str();
			kprintf("> %s\n", rx);
			parser(rx);
			rx_buf.clear(); // Parsing ends
		}
	}
}

#endif