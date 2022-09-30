#pragma once

#include <memory>

namespace kn {

// My version of `shared_ptr`, except that the `==` operator acts on the values
// themselves, not on the pointers
template<typename T>
class shared {
	std::shared_ptr<T> ptr;
public:
	shared(std::shared_ptr<T> t) noexcept : ptr(t) {}

	T& operator*() const noexcept { return *ptr; }
	T* operator->() const noexcept { return &*ptr; }

	bool ptr_eq(shared<T> const& rhs) const noexcept { return ptr == rhs.ptr; }
	bool operator==(const shared<T>& rhs) const { return *ptr == *rhs; }
};

// The equivalent of `std::make_shared` for `shared`.
template<class T, class... Args>
shared<T> make_shared( Args&&... args ) {
	return shared(std::make_shared<T>(std::forward<Args>(args)...));
}

} // namespace kn
