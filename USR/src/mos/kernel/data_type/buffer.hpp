#ifndef _MOS_BUFFER_
#define _MOS_BUFFER_

#include "../utils.hpp"
#include "../sync.hpp"

namespace MOS::DataType
{
	template <typename T, size_t N>
	struct Buffer_t
	{
		using Raw_t = T[N];
		using Cnt_t = volatile int32_t;

		Raw_t raw;
		Cnt_t len = 0;

		MOS_INLINE inline auto
		get_raw() const { return raw; }

		MOS_INLINE inline bool
		full() const volatile { return len >= N; }

		MOS_INLINE inline bool
		empty() const volatile { return len == 0; }

		MOS_INLINE inline void
		add(T data) volatile { raw[len++] = data; }

		MOS_INLINE inline auto
		back() const volatile
		{
			return empty() ? 0 : raw[len - 1];
		}

		MOS_INLINE inline auto
		begin() const { return &raw[0]; }

		MOS_INLINE inline auto
		end() const { return &raw[len]; }

		MOS_INLINE inline void
		pop() volatile
		{
			if (!empty()) {
				raw[--len] = '\0';
			}
		}

		void clear() volatile
		{
			if (!empty()) {
				Utils::memset((void*) raw, 0, sizeof(raw));
				len = 0;
			}
		}
	};

	template <size_t N>
	using RxBuf_t = Buffer_t<char, N>;

	template <size_t N>
	struct SyncRxBuf_t : public RxBuf_t<N>
	{
		MOS_INLINE inline void
		wait() { sema.down(); }

		MOS_INLINE inline void
		signal() { sema.up(); }

		MOS_INLINE inline void
		signal_from_isr() { sema.up_from_isr(); }

		MOS_INLINE inline auto
		as_str() const { return this->get_raw(); }

		MOS_INLINE inline auto
		recv()
		{
			struct TmpRecvObj_t
			{
				using TmpBuf_t = SyncRxBuf_t<N>;

				MOS_INLINE inline TmpRecvObj_t(TmpBuf_t& ref): tmp_ref(ref) { tmp_ref.wait(); }
				MOS_INLINE inline ~TmpRecvObj_t() { tmp_ref.clear(); }

				MOS_INLINE inline auto
				as_str() const { return tmp_ref.as_str(); }

				TmpBuf_t& tmp_ref;
			};

			return TmpRecvObj_t {*this};
		}

	private:
		Kernel::Sync::Sema_t sema {0};
	};
}

#endif