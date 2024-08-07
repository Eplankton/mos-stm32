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
	using namespace Utils;
	using namespace DataType;

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
		ls_cmd(Argv_t _) { Task::print_all(); }

		static inline void
		kill_cmd(Argv_t argv)
		{
			task_ctrl_cmd(
			    argv, [](auto tcb) {
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
			    argv, [](auto tcb) {
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
			    argv, [](auto tcb) {
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

		static inline void
		help_cmd(Argv_t argv);

		// Add more commands to here by {"text", callback}
		static constexpr Command_t sys_cmds[] = {
		    {    "ls",     ls_cmd}, // List all tasks
		    {  "kill",   kill_cmd}, // Kill a task
		    { "block",  block_cmd}, // Block a task
		    {"resume", resume_cmd}, // Resume a task
		    {  "help",   help_cmd}, // Show help info
		    { "uname",  uname_cmd}, // Show system info
		    {"reboot", reboot_cmd}, // Reboot system
		};

		using UsrCmds_t = Buffer_t<Command_t, Macro::SHELL_USR_CMD_SIZE>;
		UsrCmds_t usr_cmds; // For applications to register

		static inline void
		help_cmd(Argv_t argv)
		{
			static auto show = [](const auto& cmds) {
				for (auto [text, _]: cmds) {
					kprintf("%s, ", text);
				}
			};

			kprintf("{");
			show(sys_cmds);
			show(usr_cmds);
			kprintf("}\n");
		}
	}

	using Input_t = SyncRxBuf_t<Macro::SHELL_BUF_SIZE>;
	using CmdCall::sys_cmds;
	using CmdCall::usr_cmds;

	void launch(Input_t& input)
	{
		static auto parser = [](auto str) {
			kprintf("> %s\n", str); // Echo
			if (str[0] != '\0') {
				for (auto& cmd: sys_cmds) { // Search in System Commands
					if (auto argv = cmd.match(str)) {
						return cmd.run(argv);
					}
				}

				for (auto& cmd: usr_cmds) { // Search in User Commands
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
			parser(input.recv().as_str());
		}
	}
}

#endif