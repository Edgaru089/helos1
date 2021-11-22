#pragma once

#include <stdint.h>
#include <string.h>
#include "../runtime/unicode.h"
#include <utility>

namespace helos {
namespace runtime {


// String is a UTF-8 encoded string with a fixed size.
class String {
public:
	// Default constructor. Creates an empty string.
	String(): end(0), str(&end), size(0) {}

	// Construct an copy from a C string.
	String(const char *cstr): end(0) {
		size = strlen(cstr);
		str  = new char[size + 1];
		memcpy(str, cstr, size + 1);
	}

	// Construct an copy from a C string and a size.
	String(const char *cstr, uintptr_t csize): end(0) {
		size = csize;
		str  = new char[size + 1];
		memcpy(str, cstr, size);
		str[size] = 0;
	}

	// Construct an copy from an existing String object.
	String(const String &other): end(0), size(other.size) {
		str = new char[size + 1];
		memcpy(str, other.str, size + 1);
	}

	// Construct a new UTF-8 String from a Unicode codepoint.
	String(uint32_t unicode): end(0) {
		char buf[4];
		size = utf8_Encode(buf, unicode);

		str = new char[size + 1];
		memcpy(str, buf, size);
		str[size] = 0;
	}


	// Move-Construct a new String.
	String(String &&move): end(0), size(move.size), str(move.str) {
		move.size = 0;
		move.str  = &move.end;
	}

	// Deconstructor.
	~String() {
		if (str && str != &end)
			delete[] str;
	}

public:
	// Compare operators.
	bool operator==(const String &other) const { return strcmp(str, other.str) == 0; }
	bool operator==(const char *other) const { return strcmp(str, other) == 0; }
	bool operator!=(const String &other) const { return !((*this) == other); }
	bool operator!=(const char *other) const { return !((*this) == other); }
	bool operator<(const String &other) const { return strcmp(str, other.str) < 0; }
	bool operator<(const char *other) const { return strcmp(str, other) < 0; }
	bool operator<=(const String &other) const { return strcmp(str, other.str) <= 0; }
	bool operator<=(const char *other) const { return strcmp(str, other) <= 0; }
	bool operator>(const String &other) const { return strcmp(str, other.str) > 0; }
	bool operator>(const char *other) const { return strcmp(str, other) > 0; }
	bool operator>=(const String &other) const { return strcmp(str, other.str) >= 0; }
	bool operator>=(const char *other) const { return strcmp(str, other) >= 0; }

	// Copy operator.
	const String &operator=(const String &other) {
		if (str && str != &end)
			delete[] str;

		size = other.size;
		str  = new char[size + 1];
		memcpy(str, other.str, size + 1);

		return *this;
	}

	// Move operator.
	const String &operator=(String &&move) {
		if (str && str != &end)
			delete[] str;

		size = move.size;
		str  = move.str;

		move.size = 0;
		move.str  = &move.end;

		return *this;
	}

	// Swap.
	void swap(String &other) {
		std::swap(str, other.str);
		std::swap(size, other.size);
	}

public:
	// Return the length of the string.
	uintptr_t Length() const { return size; }

	// Dereference.
	char       &operator[](size_t offset) { return str[offset]; }
	const char &operator[](size_t offset) const { return str[offset]; }

	// C string.
	const char *C() const { return str; }

	// Substring.
	String Substring(uintptr_t offset, uintptr_t count) const {
		String str;
		str.size = count;
		str.str  = new char[count + 1];
		memcpy(str.str, this->str + offset, count);
		str.str[count] = 0;
		return str;
	}

public:
	// Iterator for UTF-8 decoding.
	//
	// Use this like:
	//    for(UnicodeIterator i(str); !i.End(); i++) { uint32_t code = *i; }
	struct UnicodeIterator {
	public:
		explicit UnicodeIterator(const String &parent): parent(parent), i(0) {
			adv = utf8_Decode(parent.C(), parent.Length(), &cur);
		}

		// Operator * returns the codepoint at the current position.
		uint32_t operator*() { return cur; }

		// Prefix addition
		const UnicodeIterator &operator++() {
			i += adv;
			if (i < parent.Length())
				adv = utf8_Decode(parent.C() + i, parent.Length() - i, &cur);
			return *this;
		}

		// Postfix addition
		UnicodeIterator operator++(int postfix) {
			UnicodeIterator previous = *this;
			i += adv;
			if (i < parent.Length())
				adv = utf8_Decode(parent.C() + i, parent.Length() - i, &cur);
			return previous;
		}

		// Tells if the iterator is past-the-end.
		bool End() { return i >= parent.Length(); }

	private:
		const String &parent;
		uintptr_t     i;
		uint32_t      cur;
		int           adv;
	};

private:
	char      end;
	char     *str;
	uintptr_t size;
};


} // namespace runtime
} // namespace helos


// Swap function for std::swap
inline void swap(helos::runtime::String &str1, helos::runtime::String &str2) {
	str1.swap(str2);
}
