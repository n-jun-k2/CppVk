#pragma once
#include <initializer_list>
#include <memory>
#include <cassert>
#include <vector>
#include <functional>
namespace cppvk
{
  template<class T>
  class InfoWrap{
    protected:
      T& m_info;
    public:
      InfoWrap(T& v) : m_info(v) {}
  };

  class Nondynamicallocation
  {
  protected:
    Nondynamicallocation() = default;
    ~Nondynamicallocation() = default;
    static void *operator new(size_t) {}
    static void operator delete(void *) {}
  };

  class Noncopyable
  {
  protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
  };

  template <typename Type, typename Wtype,
            template <typename E, typename Allocator = std::allocator<E>> class Container>
  union WrapContainer
  {
    Container<Wtype> values;
    Container<Type> raw;

    WrapContainer() : values() {}
    WrapContainer(const uint32_t size) : raw(size) {}
    WrapContainer(std::initializer_list<Wtype> init) : values(std::begin(init), std::end(init)) {}
    WrapContainer(std::initializer_list<Type> init) : raw(std::begin(init), std::end(init)) {}
    WrapContainer(const uint32_t n, const Wtype &value) : values(n, value) {}
    WrapContainer(const uint32_t n, const Type &value) : raw(n, value) {}
    ~WrapContainer() {}
  };

  template <typename T>
  class Singleton : Noncopyable
  {
  private:
    static std::weak_ptr<T> instance;

  protected:
    Singleton() = default;
    virtual T *createInstance() = 0;

  public:
    virtual ~Singleton() = default;
    static std::shared_ptr<T> getInstance()
    {
      auto ptr = instance.lock();
      if (!ptr)
      {
        ptr = std::shared_ptr<T>(createInstance());
        instance = ptr;
      }
      return ptr;
    }
  };

  template <typename E>
  union PoolResource
  {
    E live;
    std::add_pointer_t<PoolResource<E>> next;
  };

  template <class _Ty, typename... _Args>
  using is_class_constructible = std::conjunction<std::is_class<_Ty>, std::is_constructible<_Ty, _Args...>>;

  template <typename E, template <typename T, typename Allocator = std::allocator<T>> class Container = std::vector>
  class Pool : public std::enable_shared_from_this<Pool<E, Container>>
  {
  public:
    using Element_Type = E;
    using Element_NonPointer_Type = std::remove_pointer_t<Element_Type>;
    using InstancePtr = std::shared_ptr<Element_NonPointer_Type>;

  private:
    WrapContainer<Element_Type, PoolResource<Element_Type>, Container> buffer;
    PoolResource<Element_Type> *firstAvailable;

    void initilize() noexcept
    {
      firstAvailable = &buffer.values[0];

      auto it = std::begin(buffer.values);
      auto end = std::prev(std::end(buffer.values), 2);
      for (; it <= end; it = std::next(it))
      {
        it->next = &(*std::next(it));
      }
      std::prev(std::end(buffer.values))->next = nullptr;
    }

    void releasepool(PoolResource<Element_Type> *item) noexcept
    {
      if (firstAvailable == nullptr)
      {
        firstAvailable = item;
        firstAvailable->next = nullptr;
        return;
      }

      if (item < firstAvailable)
      {
        item->next = firstAvailable;
        firstAvailable = item;
      }
      else
      {
        item->next = firstAvailable->next;
        firstAvailable->next = item;
      }
    }

    template <class _T = Element_Type, class _Rx,
              std::enable_if_t<std::is_pointer_v<_T>> * = nullptr>
    InstancePtr makeShared(_T *element, _Rx rel)
    {
      return InstancePtr(*element, std::move(rel));
    }

    template <class _T = Element_Type, class _Rx,
              std::enable_if_t<!std::is_pointer_v<_T>> * = nullptr>
    InstancePtr makeShared(_T *element, _Rx rel)
    {
      return InstancePtr(element, std::move(rel));
    }

    template <typename _Rx, typename _InitFunc>
    InstancePtr __getInstance(_Rx rel, _InitFunc init)
    {
      assert(firstAvailable != nullptr);

      auto element = &firstAvailable->live;
      firstAvailable = firstAvailable->next;
      init(element);
      return makeShared(element, std::move(rel));
    }

    template <typename _T = Element_Type, typename _Rx, typename... _Args,
              std::enable_if_t<is_class_constructible<_T, _Args...>::value> * = nullptr>
    InstancePtr _getInstance(_Rx rel, _Args &&...args)
    {
      return this->__getInstance(std::move(rel), [&](_T **ptr)
                                 { **ptr = _T(std::forward<_Args>(args)...); });
    }

    template <typename _T = Element_Type, typename _Rx, typename... _Args,
              std::enable_if_t<std::is_pointer_v<_T>> * = nullptr>
    InstancePtr _getInstance(_Rx rel, _Args &&...args)
    {
      return this->__getInstance(std::move(rel), [&](Element_NonPointer_Type **ptr)
                                 { *ptr = new Element_NonPointer_Type(std::forward<_Args>(args)...); });
    }

    template <typename _T = Element_Type, typename _Rx,
              std::enable_if_t<std::is_pointer_v<_T>> * = nullptr>
    InstancePtr _getInstance(_Rx rel, _T value)
    {
      return this->__getInstance(std::move(rel), [&](Element_NonPointer_Type **ptr)
                                 { *ptr = std::move(value); });
    }

  public:
    Pool() = delete;
    ~Pool() = default;

    explicit Pool(const uint32_t size) noexcept
        : buffer(size)
    {
      this->initilize();
    }
    explicit Pool(std::initializer_list<PoolResource<Element_Type>> il) noexcept
        : buffer(il)
    {
      this->initilize();
    }
    explicit Pool(std::initializer_list<Element_Type> il) noexcept
        : buffer(il)
    {
      this->initilize();
    }
    explicit Pool(const uint32_t n, const PoolResource<Element_Type> &value) noexcept
        : buffer(n, value)
    {
      this->initilize();
    }
    explicit Pool(const uint32_t n, const Element_Type &value) noexcept
        : buffer(n, value)
    {
      this->initilize();
    }

    static std::shared_ptr<Pool<Element_Type, Container>> createPool(const uint32_t size) noexcept
    {
      return std::make_shared<Pool<Element_Type, Container>>(size);
    }
    static std::shared_ptr<Pool<Element_Type, Container>> createPool(std::initializer_list<PoolResource<Element_Type>> il) noexcept
    {
      return std::make_shared<Pool<Element_Type, Container>>(il);
    }
    static std::shared_ptr<Pool<Element_Type, Container>> createPool(std::initializer_list<Element_Type> il) noexcept
    {
      return std::make_shared<Pool<Element_Type, Container>>(il);
    }
    static std::shared_ptr<Pool<Element_Type, Container>> createPool(const uint32_t n, const PoolResource<Element_Type> &value) noexcept
    {
      return std::make_shared<Pool<Element_Type, Container>>(n, value);
    }
    static std::shared_ptr<Pool<Element_Type, Container>> createPool(const uint32_t n, const Element_Type &value) noexcept
    {
      return std::make_shared<Pool<Element_Type, Container>>(n, value);
    }

    template <typename _Ty = Element_Type, typename _Dx, typename... _Args,
              std::enable_if_t<std::is_invocable_v<_Dx, Element_Type &>> * = nullptr>
    InstancePtr getInstance(_Dx deleter, _Args &&...args)
    {
      struct _CRelease
      {
        std::shared_ptr<Pool<Element_Type, Container>> pool;
        _Dx del;
        explicit _CRelease(std::shared_ptr<Pool<Element_Type, Container>> pPool, _Dx _del)
            : pool(pPool), del(std::move(_del)) {}
        void operator()(Element_NonPointer_Type *pItem)
        {
          auto releaseResource = reinterpret_cast<PoolResource<Element_Type> *>(&pItem);
          del(releaseResource->live);
          pool->releasepool(releaseResource);
        }
      };
      return this->_getInstance(_CRelease(this->shared_from_this(), std::move(deleter)), args...);
    }

    template <typename _T = Element_Type, class... Args,
              std::enable_if_t<std::negation_v<std::is_pointer<_T>>> * = nullptr>
    InstancePtr getInstance(Args &&...args)
    {
      struct _Release
      {
        std::shared_ptr<Pool<_T, Container>> pool;
        explicit _Release(std::shared_ptr<Pool<_T, Container>> pPool)
            : pool(pPool) {}
        void operator()(std::remove_pointer_t<_T> *pItem)
        {
          auto releaseResource = reinterpret_cast<PoolResource<Element_Type> *>(pItem);
          pool->releasepool(releaseResource);
        }
      };
      return this->_getInstance(_Release(this->shared_from_this()), args...);
    }

    size_t size() const noexcept
    {
      return buffer.raw.size();
    }

    Element_Type *data() noexcept
    {
      return buffer.raw.data();
    }
  };

}