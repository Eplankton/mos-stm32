#ifndef _MOS_USER_TEST_
#define _MOS_USER_TEST_

#include "src/mos/kernel/task.hpp"
#include "src/mos/kernel/sync.hpp"
#include "src/mos/kernel/ipc.hpp"
#include "global.hpp"

namespace MOS::User::Test
{
	using namespace Kernel;
	using namespace Utils;

	void MutexTest()
	{
		static Sync::Mutex_t mutex;

		static auto mtx_test = [](uint32_t ticks) {
			auto name = Task::current()->get_name();
			while (true) {
				mutex.exec([&] {
					for (auto _: Range(0, 5)) {
						kprintf("%s is working...\n", name);
						Task::delay(100_ms);
					}
				});
				Task::delay(ticks);
			}
		};

		static auto launch = [] {
			Task::create(mtx_test, 10_ms, 3, "Mtx3");
			Task::delay(5_ms);
			Task::create(mtx_test, 20_ms, 2, "Mtx2");
			Task::delay(5_ms);
			Task::create(mtx_test, 30_ms, 1, "Mtx1");
		};

		Task::create(launch, nullptr, Macro::PRI_MAX, "MutexTest");
	}

	void MsgQueueTest()
	{
		static IPC::MsgQueue_t<int, 3> msg_q;

		static auto producer = [](int& msg) {
			while (true) {
				msg_q.send(msg++);
				Task::delay(50_ms);
			}
		};

		static auto consumer = [] {
			while (true) {
				int msg  = -1;
				auto res = msg_q.recv(msg, 100_ms);

				IntrGuard_t guard;
				kprintf(res ? "" : "Timeout!\n");
				// kprintf(res ? "%d\n" : "Timeout!\n", msg);
			}
		};

		static auto launch = [] {
			// Create a Consumer
			Task::create(consumer, nullptr, 4, "recv");

			// Mutable Data Sequences
			static int data[] = {5, 6, 7, 8, 9};

			// Create some Producers
			for (auto& i: data) {
				Task::create(
				    producer, &i, (Task::Prior_t) i, "send"
				);
			}
		};

		Task::create(
		    launch, nullptr, Macro::PRI_MAX, "MsgQueueTest"
		);
	}

	void SDCardTest()
	{
		using Driver::Device::SD_t;
		using Global::sd;

		enum TestStatus : bool
		{
			FAILED = true,
			PASSED = !FAILED
		};

		static constexpr auto BLOCK_SIZE        = 512; /* Block Size in Bytes */
		static constexpr auto NUMBER_OF_BLOCKS  = 4;   /* For Multi Blocks operation (Read/Write) */
		static constexpr auto MULTI_BUFFER_SIZE = BLOCK_SIZE * NUMBER_OF_BLOCKS;

		static uint8_t
		    single_tx_buf[BLOCK_SIZE],
		    single_rx_buf[BLOCK_SIZE];

		static uint8_t
		    multi_tx_buf[MULTI_BUFFER_SIZE],
		    multi_rx_buf[MULTI_BUFFER_SIZE];

		static auto buf_cmp =
		    [](uint8_t* buf_1, uint8_t* buf_2,
		       uint32_t length) {
			    while (length--) {
				    if (*buf_1 != *buf_2) {
					    return FAILED;
				    }

				    buf_1++;
				    buf_2++;
			    }
			    return PASSED;
		    };

		static auto fill_buf =
		    [](uint8_t* buf, uint32_t length, uint32_t offset) {
			    /* Put in global buffer same values */
			    for (uint16_t index = 0; index < length; index++) {
				    buf[index] = index + offset;
			    }
		    };

		/*------------------- Block Read/Write --------------------------*/
		static auto SingleBlockTest = [] {
			/* Fill the buffer to send */
			fill_buf(single_tx_buf, BLOCK_SIZE, 0x320F);

			static SD_t::Error err = SD_t::RESPONSE_NO_ERROR;

			if (err == SD_t::RESPONSE_NO_ERROR) {
				/* Write block of 512 bytes on address 0 */
				err = sd.write_block(single_tx_buf, 0x00, BLOCK_SIZE);
				/* Check if the Transfer is finished */
			}

			if (err == SD_t::RESPONSE_NO_ERROR) {
				/* Read block of 512 bytes from address 0 */
				err = sd.read_block(single_rx_buf, 0x00, BLOCK_SIZE);
			}

			/* Check the correctness of written data */
			if (err == SD_t::RESPONSE_NO_ERROR) {
				if (buf_cmp(single_tx_buf, single_rx_buf, BLOCK_SIZE) == PASSED) {
					MOS_MSG("SD SingleBlockTest -> Pass!");
				}
				else {
					MOS_MSG("SD SingleBlockTest -> Failed!");
				}
			}
		};

		/*--------------- Multiple Block Read/Write ---------------------*/
		static auto MultiBlockTest = [] {
			/* Fill the buffer to send */
			fill_buf(multi_tx_buf, MULTI_BUFFER_SIZE, 0x0);

			SD_t::Error err = SD_t::RESPONSE_NO_ERROR;

			if (err == SD_t::RESPONSE_NO_ERROR) {
				/* Write multiple block of many bytes on address 0 */
				err = sd.write_multi_block(multi_tx_buf, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
				/* Check if the Transfer is finished */
			}

			if (err == SD_t::RESPONSE_NO_ERROR) {
				/* Read block of many bytes from address 0 */
				err = sd.read_multi_block(multi_rx_buf, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
				/* Check if the Transfer is finished */
			}

			/* Check the correctness of written data */
			if (err == SD_t::RESPONSE_NO_ERROR) {
				if (buf_cmp(multi_tx_buf, multi_rx_buf, MULTI_BUFFER_SIZE) == PASSED) {
					MOS_MSG("SD MultiBlockTest -> Pass!");
				}
				else {
					MOS_MSG("SD MultiBlockTest -> Failed!");
				}
			}
		};

		auto ShowInfo = [] {
			switch (sd.type) {
				case SD_t::Type::V1: kprintf("Type: SDSC V1.0\n"); break;
				case SD_t::Type::V2HC: kprintf("Type: SDHC V2.0\n"); break;
				default: kprintf("Type: Unknown\n"); break;
			}

			kprintf("ManufacturerID: %d\n"
			        "Capacity: %d MB\n"
			        "BlockSize: %d B\n",
			        sd.info.cid.ManufacturerID,          //制造商ID
			        (uint32_t) (sd.info.capacity >> 20), //显示容量
			        sd.info.block_size);                 //显示块大小
		};

		ShowInfo();
		SingleBlockTest();
		MultiBlockTest();
	}
}

#endif