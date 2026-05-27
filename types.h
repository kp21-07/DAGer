#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

//
// Data Structures
//

template <typename T>
struct vector {
  T* m_data;
  size_t m_size;
  size_t m_capacity;

  vector() : m_data(nullptr), m_size(0), m_capacity(0) {}

  ~vector() {
    delete[] m_data;
  }

  vector(const vector& other) : m_data(nullptr), m_size(0), m_capacity(0) {
    if (other.m_capacity > 0) {
      m_data = new T[other.m_capacity];
      m_capacity = other.m_capacity;
      m_size = other.m_size;
      for (size_t i = 0; i < m_size; ++i) {
        m_data[i] = other.m_data[i];
      }
    }
  }

  vector& operator=(const vector& other) {
    if (this != &other) {
      delete[] m_data;
      m_data = nullptr;
      m_size = 0;
      m_capacity = 0;
      if (other.m_capacity > 0) {
        m_data = new T[other.m_capacity];
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        for (size_t i = 0; i < m_size; ++i) {
          m_data[i] = other.m_data[i];
        }
      }
    }
    return *this;
  }

  vector(vector&& other) noexcept : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  vector& operator=(vector&& other) noexcept {
    if (this != &other) {
      delete[] m_data;
      m_data = other.m_data;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_data = nullptr;
      other.m_size = 0;
      other.m_capacity = 0;
    }
    return *this;
  }

  void reserve(size_t new_cap) {
    if (new_cap <= m_capacity) return;
    T* new_data = new T[new_cap];
    for (size_t i = 0; i < m_size; ++i) {
      new_data[i] = static_cast<T&&>(m_data[i]);
    }
    delete[] m_data;
    m_data = new_data;
    m_capacity = new_cap;
  }

  void push_back(const T& val) {
    if (m_size >= m_capacity) {
      reserve(m_capacity == 0 ? 4 : m_capacity * 2);
    }
    m_data[m_size++] = val;
  }

  void push_back(T&& val) {
    if (m_size >= m_capacity) {
      reserve(m_capacity == 0 ? 4 : m_capacity * 2);
    }
    m_data[m_size++] = static_cast<T&&>(val);
  }

  size_t size() const { return m_size; }
  bool empty() const { return m_size == 0; }

  T& operator[](size_t index) { return m_data[index]; }
  const T& operator[](size_t index) const { return m_data[index]; }

  T* begin() { return m_data; }
  T* end() { return m_data + m_size; }
  const T* begin() const { return m_data; }
  const T* end() const { return m_data + m_size; }
};

struct string {
  char *m_data;
  size_t m_size;
  size_t m_capacity;

  string() : m_data(nullptr), m_size(0), m_capacity(0) {}

  string(const char *str) {
    m_size = str ? strlen(str) : 0;
    m_capacity = m_size;
    m_data = (char *)malloc(m_size + 1);
    if (str) {
      memcpy(m_data, str, m_size + 1);
    } else {
      if (m_data)
        m_data[0] = '\0';
    }
  }

  string(const char *str, size_t len) {
    m_size = len;
    m_capacity = len;
    m_data = (char *)malloc(len + 1);
    if (str && len > 0) {
      memcpy(m_data, str, len);
    }
    if (m_data)
      m_data[len] = '\0';
  }

  const char *data() const { return m_data; }

  size_t length() const { return m_size; }

  size_t size() const { return m_size; }

  bool is_empty() const { return m_size == 0; }

  void reserve(size_t new_cap) {
    if (new_cap <= m_capacity)
      return;
    char *new_data = (char *)realloc(m_data, new_cap + 1);
    if (new_data) {
      m_data = new_data;
      m_capacity = new_cap;
    }
  }

  void append(const char *str, size_t len) {
    if (!str || len == 0)
      return;
    if (m_size + len > m_capacity) {
      reserve((m_size + len) * 2);
    }
    memcpy(m_data + m_size, str, len);
    m_size += len;
    m_data[m_size] = '\0';
  }

	vector<string> split(const char delimiter) const {
		vector<string> tokens;
    if (!m_data || m_size == 0) {
      return tokens;
    }
    size_t start = 0;
    for (size_t i = 0; i <= m_size; ++i) {
      if (i == m_size || m_data[i] == delimiter) {
        tokens.push_back(string(m_data + start, i - start));
        start = i + 1;
      }
    }
    return tokens;
	}

  bool operator==(const char *str) const {
    return strcmp(data(), str ? str : "") == 0;
  }

  friend string operator+(const string& lhs, const string& rhs) {
    string result;
    size_t total_len = lhs.length() + rhs.length();
    result.reserve(total_len);
    if (lhs.data()) {
      result.append(lhs.data(), lhs.length());
    }
    if (rhs.data()) {
      result.append(rhs.data(), rhs.length());
    }
    return result;
  }

};

struct binary_buffer {
    char* m_data;
    size_t m_size;
    size_t m_capacity;

    binary_buffer() : m_data(nullptr), m_size(0), m_capacity(0) {}

    binary_buffer(size_t cap) : m_data(nullptr), m_size(0), m_capacity(cap) {
        if (cap > 0) {
            m_data = (char*)malloc(cap);
        }
    }

    const char* data() const { return m_data; }
    char* data() { return m_data; }
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    void reserve(size_t new_cap) {
        if (new_cap <= m_capacity) return;
        char* new_data = (char*)realloc(m_data, new_cap);
        if (new_data) {
            m_data = new_data;
            m_capacity = new_cap;
        }
    }

    void append(const char* data, size_t len) {
        if (!data || len == 0) return;
        if (m_size + len > m_capacity) {
            reserve((m_size + len) * 2);
        }
        memcpy(m_data + m_size, data, len);
        m_size += len;
    }
};

#endif
