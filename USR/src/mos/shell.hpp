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
	using namespace Kernel;
	using Utils::IntrGuard_t;
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
		len() const volatile { return strlen(text); }

		MOS_INLINE inline void
		run(Argv_t argv) const volatile { callback(argv); }

		MOS_INLINE inline void
		operator=(const Command_t& cmd) volatile
		{
			text     = cmd.text;
			callback = cmd.callback;
		}

		Argv_t match(Text_t str) const volatile
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
			return check(str) ? skip(str + xlen) : nullptr;
		}
	};

	namespace CmdCall
	{
		using Argv_t = Command_t::Argv_t;

		MOS_INLINE static inline void
		bad_argv_err()
		{
			MOS_MSG("Invalid Arguments");
		}

		static inline void
		task_ctrl_cmd(
		    Argv_t argv, auto&& ok, auto&& err
		)
		{
			auto name = argv;
			if (*name != '\0') {
				if (auto tcb = Task::find(name)) {
					ok(tcb);
				}
				else {
					MOS_MSG("Unknown task '%s'", name);
				}
			}
			else { // No arguments provided
				err();
			}
		}

		static inline void
		ls_cmd(Argv_t argv)
		{
			task_ctrl_cmd(
			    argv,
			    [](auto tcb) { /* Todo */ },
			    [] { Task::print_all(); }
			);
		}

		static inline void
		kill_cmd(Argv_t argv)
		{
			task_ctrl_cmd(
			    argv,
			    [](auto tcb) {
				    MOS_MSG("Task '%s' terminated", tcb->get_name());
				    Task::terminate(tcb);
			    },
			    bad_argv_err
			);
		}

		static inline void
		block_cmd(Argv_t argv)
		{
			task_ctrl_cmd(
			    argv,
			    [](auto tcb) {
				    MOS_MSG("Task '%s' blocked", tcb->get_name());
				    Task::block(tcb);
			    },
			    bad_argv_err
			);
		}

		static inline void
		resume_cmd(Argv_t argv)
		{
			task_ctrl_cmd(
			    argv,
			    [](auto tcb) {
				    MOS_MSG("Task '%s' resumed", tcb->get_name());
				    Task::resume(tcb);
			    },
			    bad_argv_err
			);
		}

		static inline void
		uname_cmd(Argv_t argv)
		{
			IntrGuard_t guard;
			kprintf(
			    " A_A       _  Version @ %s\n"
			    "o'' )_____//  Build   @ %s, %s\n"
			    " `_/  MOS  )  Chip    @ %s, %s\n"
			    " (_(_/--(_/   2023-2024 Copyright by Eplankton\n",
			    MOS_VERSION,
			    __TIME__, __DATE__,
			    MOS_MCU, MOS_ARCH
			);
		}

		static inline void
		reboot_cmd(Argv_t argv)
		{
			MOS_MSG("Reboot!\n\n");
			MOS_REBOOT();
		}
	}

	// Add more commands here with {"text", CmdCall::callback}
	static constexpr Command_t sys_cmds[] = {
	    {    "ls",     CmdCall::ls_cmd},
	    {  "kill",   CmdCall::kill_cmd},
	    { "block",  CmdCall::block_cmd},
	    {"resume", CmdCall::resume_cmd},
	    { "uname",  CmdCall::uname_cmd},
	    {"reboot", CmdCall::reboot_cmd},
	};

	using SyncRxBuf_t = DataType::SyncRxBuf_t<Macro::SHELL_BUF_SIZE>;
	using UsrCmds_t   = DataType::Buffer_t<Command_t, Macro::SHELL_USR_CMD_SIZE>;

	UsrCmds_t usr_cmds; // For applications to register

	void launch(SyncRxBuf_t& input)
	{
		using Text_t = Command_t::Text_t;

		static auto parse = [](Text_t str) {
			kprintf("> %s\n", str); // Echo
			if (str[0] != '\0') {
				// Search in System Commands
				for (auto& cmd: sys_cmds) {
					if (auto argv = cmd.match(str)) {
						return cmd.run(argv);
					}
				}

				// Search in User Commands
				for (auto& cmd: usr_cmds.raw) {
					if (auto argv = cmd.match(str)) {
						return cmd.run(argv);
					}
				}

				MOS_MSG("Unknown command '%s'", str);
			}
		};

		CmdCall::uname_cmd(nullptr);
		Task::print_all();

		while (true) {
			input.wait();
			parse(input.as_str());
			input.clear();
		}
	}
}

#endif