#pragma once

#include <vector>
#include <algorithm>
#include <functional>

namespace cppvk {

  template <  typename RAW_TYPE, typename TYPE, typename SIZE = int, template< typename E, typename Allocator = std::allocator<E> > class Container = std::vector>
  void containerToCPtr(SIZE& ref_size, RAW_TYPE** ptr_value, Container<TYPE>& container) {
    ref_size = static_cast<SIZE>(container.size());
    *ptr_value = nullptr;
    if (!container.empty()){
      *ptr_value = container.data();
    }
  }

  template< template< typename E, typename Allocator = std::allocator<E> > class Container = std::vector>
  void stringListToCPtrList(const Container<std::string>& src, Container<const char*>& dst) {
    dst.reserve(src.size());
    std::transform(std::begin(src), std::end(src), std::back_inserter(dst), [](const std::string& s){
      return s.c_str();
    });
  }

  template < typename T, template< typename E, typename Allocator = std::allocator<E> > class Container = std::vector>
  void foreach(Container<T>& container, std::function<void(T&)> update, const int offset = 0, const unsigned int count = 0) {
    const auto n = count <= 0 ? container.size() : count;
    auto begin = std::begin(container);
    std::advance(begin, offset);
    std::for_each_n(begin, n, update);
  }

  template< typename fT, typename sT, template< typename E, typename Allocator = std::allocator<E> > class Container = std::vector>
  void foreachZip(Container<fT>& first_container, Container<sT>& second_container, std::function<void (fT&, sT&)> update, const int offset = 0, const unsigned int count = 0) {
    auto size = std::min(first_container.size(), second_container.size());
    if(0 < count && count <= size) size = count;
    for (auto i = offset; i < size; ++i) update(first_container[i], second_container[i]);
  }

}