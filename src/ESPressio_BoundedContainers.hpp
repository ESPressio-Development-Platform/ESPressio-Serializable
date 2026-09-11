#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace ESPressio::Serializable {

/// <summary>Inline sequence with exactly N storage slots; capacity failure leaves the input and sequence unchanged.</summary>
/// <remarks>All slots are value-initialized. Element constructors/assignments retain their own exception contract.
/// Primitive eligibility separately checks the complete element graph. No allocator or growth path exists.</remarks>
template<class T, std::size_t N>
class BoundedVector {
    std::array<T, N> _items{};
    std::size_t _size = 0;
public:
    using value_type = T;
    /// <summary>Returns the fixed number of inline slots.</summary>
    static constexpr std::size_t capacity() noexcept { return N; }
    /// <summary>Returns the number of active elements.</summary>
    constexpr std::size_t size() const noexcept { return _size; }
    /// <summary>Reports whether the sequence has no active elements.</summary>
    constexpr bool empty() const noexcept { return _size == 0; }
    /// <summary>Returns contiguous storage; only the first size() entries are active.</summary>
    constexpr T* data() noexcept { return _items.data(); }
    constexpr const T* data() const noexcept { return _items.data(); }
    /// <summary>Returns an iterator delimiting the active elements.</summary>
    constexpr T* begin() noexcept { return data(); }
    constexpr const T* begin() const noexcept { return data(); }
    constexpr T* end() noexcept { return N == 0 ? data() : data() + _size; }
    constexpr const T* end() const noexcept { return N == 0 ? data() : data() + _size; }
    /// <summary>Accesses an active element; index must be smaller than size().</summary>
    constexpr T& operator[](std::size_t index) noexcept { return _items[index]; }
    constexpr const T& operator[](std::size_t index) const noexcept { return _items[index]; }
    /// <summary>Copies one element if a slot is free. Never grows storage.</summary>
    constexpr bool push_back(const T& item) {
        if (_size == N) return false;
        _items[_size] = item;
        ++_size;
        return true;
    }
    /// <summary>Moves one element only after checking capacity.</summary>
    constexpr bool push_back(T&& item) {
        if (_size == N) return false;
        _items[_size] = std::move(item);
        ++_size;
        return true;
    }
    /// <summary>Resets active slots to their default values, retaining the inline storage.</summary>
    constexpr void clear() {
        while (_size != 0) _items[--_size] = T{};
    }
    /// <summary>Compares active values, ignoring unused storage.</summary>
    friend constexpr bool operator==(const BoundedVector& a, const BoundedVector& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) if (!(a[i] == b[i])) return false;
        return true;
    }
};

/// <summary>Inline binary bytes. N bounds payload bytes, with no terminator or hidden capacity.</summary>
template<std::size_t N> using BoundedBytes = BoundedVector<std::uint8_t, N>;

/// <summary>Inline text with at most N bytes plus one permanently reserved terminator.</summary>
/// <remarks>Embedded zero bytes are preserved by view(); c_str() is a convenience for text without them.</remarks>
template<std::size_t N>
class BoundedString {
    static_assert(N < static_cast<std::size_t>(-1), "BoundedString capacity leaves room for its terminator");
    std::array<char, N + 1> _bytes{};
    std::size_t _size = 0;
public:
    using value_type = char;
    /// <summary>Returns the maximum text length, excluding the terminator.</summary>
    static constexpr std::size_t capacity() noexcept { return N; }
    /// <summary>Returns the stored byte count.</summary>
    constexpr std::size_t size() const noexcept { return _size; }
    constexpr bool empty() const noexcept { return _size == 0; }
    /// <summary>Returns immutable text storage or its length-preserving view.</summary>
    constexpr const char* data() const noexcept { return _bytes.data(); }
    constexpr const char* c_str() const noexcept { return data(); }
    constexpr std::string_view view() const noexcept { return {data(), _size}; }
    constexpr const char* begin() const noexcept { return data(); }
    constexpr const char* end() const noexcept { return data() + _size; }
    /// <summary>Assigns text after checking the entire length; oversize leaves the value unchanged.</summary>
    constexpr bool assign(std::string_view text) noexcept {
        if (text.size() > N) return false;
        // A same-buffer subview starts at or after destination, so forward copy is safe.
        for (std::size_t i = 0; i < text.size(); ++i) _bytes[i] = text[i];
        _size = text.size();
        _bytes[_size] = '\0';
        return true;
    }
    /// <summary>Empties the text without changing capacity.</summary>
    constexpr void clear() noexcept { _size = 0; _bytes[0] = '\0'; }
    /// <summary>Appends one byte if space remains, always retaining the terminator.</summary>
    constexpr bool push_back(char value) noexcept {
        if (_size==N) return false;
        _bytes[_size++]=value; _bytes[_size]='\0'; return true;
    }
    /// <summary>Compares the complete stored byte sequences.</summary>
    friend constexpr bool operator==(const BoundedString& a, const BoundedString& b) noexcept { return a.view() == b.view(); }
    friend constexpr bool operator<(const BoundedString& a, const BoundedString& b) noexcept { return a.view() < b.view(); }
};

/// <summary>Fixed-capacity unique-value collection. Duplicate/full insertion returns false without modification.</summary>
/// <remarks>Iteration is insertion ordered. Only immutable iterators are exposed to preserve uniqueness.</remarks>
template<class T, std::size_t N>
class BoundedSet {
    BoundedVector<T, N> _items;
public:
    using value_type = T;
    static constexpr std::size_t capacity() noexcept { return N; }
    constexpr std::size_t size() const noexcept { return _items.size(); }
    constexpr const T* begin() const noexcept { return _items.begin(); }
    constexpr const T* end() const noexcept { return _items.end(); }
    /// <summary>Looks up a value by equality in at most N comparisons.</summary>
    constexpr bool contains(const T& value) const {
        for (const auto& item : _items) if (item == value) return true;
        return false;
    }
    /// <summary>Copies a previously absent value into one free inline slot.</summary>
    constexpr bool insert(const T& value) { return !contains(value) && _items.push_back(value); }
    /// <summary>Clears all active values.</summary>
    constexpr void clear() { _items.clear(); }
    /// <summary>Compares membership independently of insertion order.</summary>
    friend constexpr bool operator==(const BoundedSet& a, const BoundedSet& b) {
        if (a.size() != b.size()) return false;
        for (const auto& item : a) if (!b.contains(item)) return false;
        return true;
    }
};

/// <summary>Fixed-capacity unique-key map with bounded linear lookup and immutable entry iteration.</summary>
template<class K, class V, std::size_t N>
class BoundedMap {
    BoundedVector<std::pair<K, V>, N> _items;
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<K, V>;
    static constexpr std::size_t capacity() noexcept { return N; }
    constexpr std::size_t size() const noexcept { return _items.size(); }
    constexpr const value_type* begin() const noexcept { return _items.begin(); }
    constexpr const value_type* end() const noexcept { return _items.end(); }
    /// <summary>Returns the mapped value for a key, or null without inserting.</summary>
    constexpr const V* find(const K& key) const {
        for (const auto& item : _items) if (item.first == key) return &item.second;
        return nullptr;
    }
    /// <summary>Inserts a new key/value only if capacity is available and the key is absent.</summary>
    constexpr bool insert(const K& key, const V& value) {
        if (find(key) != nullptr || size() == N) return false;
        return _items.push_back(value_type{key, value});
    }
    /// <summary>Clears all active entries.</summary>
    constexpr void clear() { _items.clear(); }
    /// <summary>Compares mappings independently of insertion order.</summary>
    friend constexpr bool operator==(const BoundedMap& a, const BoundedMap& b) {
        if (a.size() != b.size()) return false;
        for (const auto& item : a) {
            const auto* found = b.find(item.first);
            if (found == nullptr || !(*found == item.second)) return false;
        }
        return true;
    }
};
}
