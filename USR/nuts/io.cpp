#include "io.hpp"

// 重定向c库函数 printf到串口，重定向后可使用 printf函数
extern "C" int
fputc(int ch, FILE* f) noexcept
{
	auto& serial_out = USART_t::convert(OUT_PORT);
	serial_out.send_data(ch);                                   /* 发送一个字节数据到串口 */
	while (serial_out.get_flag_status(USART_FLAG_TXE) == RESET) /* 等待发送完毕 */
		;
	return (ch);
}

// 重定向c库函数 scanf到串口，重写向后可使用 scanf、getchar等函数
extern "C" int
fgetc(FILE* f) noexcept
{
	auto& serial_in = USART_t::convert(IN_PORT);
	while (serial_in.get_flag_status(USART_FLAG_RXNE) == RESET) /* 等待串口输入数据 */
		;
	return (int) serial_in.receive_data();
}