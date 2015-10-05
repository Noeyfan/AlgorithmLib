#ifndef _NOEY_LRU_H
#define _NOEY_LRU_H

#include <memory>
using std::shared_ptr;
using std::make_shared;

namespace noey{
  template <typename T, typename U = int>
    class __lru
    {
      struct __node
      {
        T _M_key; U _M_value;
        shared_ptr<__node> prev, next;

	__node() = default;

        __node(const T& __k, const U& __v,
	       const shared_ptr<__node>& p,
	       const shared_ptr<__node>& n)
        : _M_key(__k), _M_value(__v), prev(p), next(n){ }

	void remove() {
	  auto p = prev;
	  auto n = next;
	  p->next = n;
	  n->prev = p;
	}

	~__node() = default;
      };

      struct __buf
      {
	shared_ptr<__node> _M_head;

	// API
	__buf() {
	  _M_head = make_shared<__node>();
	  _M_head->prev = _M_head;
	  _M_head->next = _M_head;
	}

	~__buf() {
	  while (!empty()) {
	    back()->remove();
	  }
	  _M_head->prev.reset();
	  _M_head->next.reset();
	}

	bool empty() {
	  return _M_head->next == _M_head;
	}

	auto& back() {
	  return _M_head->prev;
	}

	auto& front() {
	  return _M_head->next;
	}

	void remove(const T& key) {
	  auto p = _M_head->next;
	  while (p != _M_head) {
	    if (p->_M_key == key) {
	      p->remove();
	      break;
	    }
	    p = p->next;
	  }
	}

	U get(const T& key) {
	  auto p = _M_head->next;
	  while (p != _M_head) {
	    if (p->_M_key == key) {
	      return p->_M_value;
	    }
	    p = p->next;
	  }
	  return -1;
	}

	void append(const T& key, const T& value) {
	  auto n = make_shared<__node>(key, value, back(), _M_head);
	  back()->next = n;
	  _M_head->prev = n;
	}
      };

    private:
      __buf _M_buf;
      int _M_capacity, _M_used;

    public:
      __lru(int cap) : _M_capacity(cap), _M_used(cap) { }

      U get(const T& key) {
	U ret = _M_buf.get(key);
	if (ret != -1) {
	  // hit
	  _M_buf.remove(key);
	  _M_buf.append(key, ret);
	}
	return ret;
      }

      void set(const T& key, const U& value) {
	assert(_M_capacity >= value);
	int v = get(key);
	if (v != -1) {
	  // hit
	  _M_used += v;
	  _M_buf.remove(key);
	}
	// miss
	while (!_M_buf.empty() && (_M_used - value) < 0) {
	  _M_used += _M_buf.front()->_M_value;
	  _M_buf.front()->remove();
	}
	_M_buf.append(key, value);
	_M_used -= value;
      }
    };
}

#endif
