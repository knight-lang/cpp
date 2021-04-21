#pragma once

#include <string>
#include <string_view>
#include <memory>

namespace kn {
	class String {
		std::shared_ptr<std::string> str;

	public:
		String() : String(0) {}
		String(size_t length) : String(std::string(length, '\0')) {}
		String(std::string string) : String(std::make_shared<std::string>(string)) {}
		String(std::shared_ptr<std::string> string) : str(string) {}
		static String fetch(std::string_view view) {
			return String(std::make_shared<std::string>(std::string(view)));
		}

		void cache() const;

		template<typename T>
		bool operator==(T other) {
			return *str == other;
		}

		std::string& fetch() { return *str; }
		std::string const& fetch() const { return *str; }

		std::string& operator*() {
			return *str;
		}

		std::string* operator->() { return &*str; }
		std::string const* operator->() const { return &*str; }

		operator std::string_view() const noexcept {
			return *str;
		}
	};
}