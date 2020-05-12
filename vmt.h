#pragma once
#include <vector>

namespace util
{
	struct vmt_hook
	{
		vmt_hook() = default;

		vmt_hook(void *object) { init(reinterpret_cast<uintptr_t **>(object)); }

		vmt_hook(const uintptr_t object)
		{
			init(reinterpret_cast<uintptr_t **>(object));
		}

		~vmt_hook() { unhook(); }

		void unhook()
		{
			if (!_object)
				return;

			*_object = _orig_vmt;
			_object  = nullptr;
		}

		void hook_func(const size_t idx, const void *func)
		{
			if (idx >= (_custom_vmt.size() - 1u))
				return;

			_custom_vmt[idx + 1u] = reinterpret_cast<uintptr_t>(func);
		}

		void unhook_func(const size_t idx)
		{
			if (idx >= (_custom_vmt.size() - 1u))
				return;

			_custom_vmt[idx + 1u] = _orig_vmt[idx];
		}

		uintptr_t orig_func(const size_t idx) const { return _orig_vmt[idx]; }

		void init(uintptr_t **object)
		{
			_object         = object;
			_orig_vmt       = *object;
			auto func_count = 0u;
			while (_orig_vmt[func_count])
				++func_count;

			_custom_vmt.resize(func_count + 1u);
			std::copy(_orig_vmt - 1u, _orig_vmt + func_count, _custom_vmt.begin());
			*object = _custom_vmt.data() + 1u;
		}

		void init(void *object) { init(reinterpret_cast<uintptr_t **>(object)); }

		void init(const uintptr_t object)
		{
			init(reinterpret_cast<uintptr_t **>(object));
		}

		protected:
		uintptr_t *_orig_vmt               = nullptr;
		uintptr_t **_object                = nullptr;
		std::vector<uintptr_t> _custom_vmt = {};
	};
}  // namespace util
