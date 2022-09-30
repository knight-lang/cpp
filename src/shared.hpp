#pragma once

#include <memory>

namespace kn {
	template<typename T>
	class shared {
		std::shared_ptr<T> ptr;
	public:
		shared(std::shared_ptr<T> t): ptr(t){}

		T& operator*() const noexcept { return *ptr; }
		T* operator->() const noexcept { return &*ptr; }
		bool ptr_eq(shared<T> const& rhs) const noexcept { return ptr == rhs.ptr; }

		bool operator==(const shared<T>& rhs) const {
			return *ptr == *rhs;
		}
	};

	template<class T, class... Args>
	shared<T> make_shared( Args&&... args ) {
		return shared(std::make_shared<T>(std::forward<Args>(args)...));
	}
}
