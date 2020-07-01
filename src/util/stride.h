#pragma once

#include <iterator>

namespace vsgSandbox
{
    template<typename BaseItr>
    class StrideIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename std::iterator_traits<BaseItr>::value_type;
        using difference_type = typename std::iterator_traits<BaseItr>::difference_type;
        using pointer = typename std::iterator_traits<BaseItr>::pointer;
        using reference = typename std::iterator_traits<BaseItr>::reference;
        StrideIterator(BaseItr itr, difference_type stride,
                       difference_type index = difference_type())
            : _itr(std::move(itr), _stride(stride), _index(index))
        {}
        StrideIterator& operator=(const StrideIterator& rhs)
        {
            _itr = rhs._itr;
            _stride = rhs._stride;
            _index = rhs._index;
            return *this;
        }

        StrideIterator& operator+=(difference_type n)
        {
            _index += n * _stride;
            return *this;
        }

        StrideIterator operator+(difference_type n) const
        {
            return StrideIterator(_itr, _stride, _index + n * _stride);
        }

        friend StrideIterator operator+(difference_type n, const StrideIterator& rhs)
        {
            return StrideIterator(rhs._itr, rhs._stride, rhs._index + n * rhs._stride);
        }

        StrideIterator& operator++()
        {
            _index += _stride;
            return *this;
        }

        StrideIterator operator++(int)
        {
            StrideIterator result(*this);
            _index += _stride;
            return result;
        }

        StrideIterator& operator-=(difference_type n)
        {
            _index -= n * _stride;
            return *this;
        }

        StrideIterator operator-(difference_type n) const
        {
            return StrideIterator(_itr, _stride, _index - n * _stride);
        }

        difference_type operator-(const StrideIterator& rhs)
        {
            return (_index - rhs._index) / _stride;
        }

        StrideIterator& operator--()
        {
            _index -= _stride;
            return *this;
        }

        StrideIterator operator--(int)
        {
            StrideIterator result(*this);
            _index -= _stride;
            return result;
        }

        reference operator*() const
        {
            return *(_itr + _index);
        }

        pointer operator->() const
        {
            return &*(_itr + _index);
        }

        refrence operator[](difference_type n)
        {
            return *(_itr + index + n * _stride);
        }

        bool operator==(const StrideIterator& rhs) const
        {
            return _index == rhs._index;
        }

        bool operator!=(const StrideIterator& rhs) const
        {
            return _index != rhs._index;
        }

        bool operator<(const StrideIterator& rhs) const
        {
            return _index < rhs._index;
        }

        bool operator<=(const StrideIterator& rhs) const
        {
            return _index <= rhs._index;
        }

        bool operator>(const StrideIterator& rhs) const
        {
            return _index > rhs._index;
        }

        bool operator>=(const StrideIterator& rhs) const
        {
            return _index > rhs._index;
        }
        
    private:
        BaseItr _itr;
        difference_type _stride;
        difference_type _index;
    };

    template<typename Container>
    class StrideRange
    {
        StrideRange(Container& container, int byteStride)
            : _container(container)
        {
            if (byteStride % sizeof(typename Container::value_type))
            {
                throw "byte stride must be a multiple of size";
            }
            _stride = byteStride / sizeof(typename Container::value_type);
        }
        
        // Member type	Definition
        using value_type = Container::value_type;
        using allocator_type = Container::allocator_type;
        using size_type = Container::size_type;
        using difference_type = Container::difference_type;
        using reference	= Container::reference;
        using const_reference = Container::const_reference;
        using pointer = Container::pointer;
        using const_pointer = Container::const_pointer;
        using iterator = StrideIterator<Container::iterator>;
        using const_iterator = StrideIterator<Container::const_iterator>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        size_type size()
        {
            return _container.size();
        }

        StrideIterator<Container::iterator> begin()
        {
            return StrideIterator(_container.begin(), _stride, 0);
        }

        StrideIterator<Container::iterator> end()
        {
            return StrideIterator(_container.begin(), _stride, _container.size());
        }

        StrideIterator<Container::const_iterator> cbegin()
        {
            return StrideIterator(_container.begin(), _stride, 0);
        }

        StrideIterator<Container::const_iterator> cend()
        {
            return StrideIterator(_container.begin(), _stride, _container.size());
        }

        reference operator[](size_type i)
        {
            return _container[i * _stride];
        }

        const_reference operator[](size_type i) const
        {
            return _container[i * _stride];
        }
        
    private:
        Container& _container;
        int _stride;
    };
}
