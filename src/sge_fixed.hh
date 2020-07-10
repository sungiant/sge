#pragma once

namespace sge::fixed {

//--------------------------------------------------------------------------------------------------------------------//
// just some of this: https://gcc.gnu.org/onlinedocs/gcc-4.6.3/libstdc++/api/a00752_source.html
// with minor changes
template<typename TP, size_t SZ> struct array {
    typedef TP                                      value_type;
    typedef TP*                                     pointer;
    typedef const TP*                               const_pointer;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef value_type*                             iterator;
    typedef const value_type*                       const_iterator;
    typedef size_t                                  size_type;
    typedef std::ptrdiff_t                          difference_type;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;


    void                      fill        (const value_type& x) { std::fill_n (begin (), size (), x); }
    void                      swap        (array& x)            { std::swap_ranges (begin (), end (), x.begin ()); }

    iterator                  begin       ()       { return iterator (std::addressof (content[0])); }
    const_iterator            begin       () const { return const_iterator (std::addressof (content[0])); }
    iterator                  end         ()       { return iterator (std::addressof (content[SZ])); }
    const_iterator            end         () const { return const_iterator (std::addressof (content[SZ])); }
    reverse_iterator          rbegin      ()       { return reverse_iterator (end ()); }
    const_reverse_iterator    rbegin      () const { return const_reverse_iterator (end ()); }
    reverse_iterator          rend        ()       { return reverse_iterator (begin ()); }
    const_reverse_iterator    rend        () const { return const_reverse_iterator (begin ()); }
    const_iterator            cbegin      () const { return const_iterator (std::addressof (content[0])); }
    const_iterator            cend        () const { return const_iterator (std::addressof (content[SZ])); }
    const_reverse_iterator    crbegin     () const { return const_reverse_iterator (end ()); }
    const_reverse_iterator    crend       () const { return const_reverse_iterator (begin ()); }

    constexpr size_type       size        () const { return SZ; }
    constexpr size_type       max_size    () const { return SZ; }
    constexpr bool            empty       () const { return size () == 0; }

    reference                 operator[]  (size_type n)       { return content[n]; }
    const_reference           operator[]  (size_type n) const { return content[n]; }

    reference                 at          (size_type n)       { assert (n < SZ); return content[n]; }
    const_reference           at          (size_type n) const { assert (n < SZ); return content[n]; }

    reference                 front       ()       { return *begin (); }
    const_reference           front       () const { return *begin (); }
    reference                 back        ()       { return SZ ? *(end () - 1) : *end (); }
    const_reference           back        () const { return SZ ? *(end () - 1) : *end (); }
    TP*                       data        ()       { return std::addressof (content[0]); }
    const TP*                 data        () const { return std::addressof (content[0]); }
    
private:
    value_type content[SZ ? SZ : 1];
};

template<typename TP, size_t SZ> inline bool operator==(const array<TP, SZ>& left, const array<TP, SZ>& right) { return std::equal (left.begin (), left.end (), right.begin ()); }
template<typename TP, size_t SZ> inline bool operator!=(const array<TP, SZ>& left, const array<TP, SZ>& right) { return !(left == right); }
template<typename TP, size_t SZ> inline bool operator< (const array<TP, SZ>& left, const array<TP, SZ>& right) { return std::lexicographical_compare (left.begin (), left.end (), right.begin (), right.end ()); }
template<typename TP, size_t SZ> inline bool operator> (const array<TP, SZ>& left, const array<TP, SZ>& right) { return right < left; }
template<typename TP, size_t SZ> inline bool operator<=(const array<TP, SZ>& left, const array<TP, SZ>& right) { return !(left > right); }
template<typename TP, size_t SZ> inline bool operator>=(const array<TP, SZ>& left, const array<TP, SZ>& right) { return !(left < right); }
template<typename TP, size_t SZ> inline void swap      (array<TP, SZ>& left, array<TP, SZ>& right)             { left.swap (right); }

//--------------------------------------------------------------------------------------------------------------------//

template<typename TP, size_t SZ> struct vector {

    typedef TP                                      value_type;
    typedef TP*                                     pointer;
    typedef const TP*                               const_pointer;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef value_type*                             iterator;
    typedef const value_type*                       const_iterator;
    typedef size_t                                  size_type;
    typedef std::ptrdiff_t                          difference_type;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    

    ~vector () { clear(); }

    void                      fill        (const value_type& x) { std::fill_n (begin (), SZ, x); current_size = SZ; }
    void                      swap        (vector& x)           { assert (false); }

    
    iterator                  begin       ()       { return iterator (std::addressof (content[0])); }
    const_iterator            begin       () const { return const_iterator (std::addressof (content[0])); }
    iterator                  end         ()       { return iterator (std::addressof (content[current_size])); }
    const_iterator            end         () const { return const_iterator (std::addressof (content[current_size])); }
    reverse_iterator          rbegin      ()       { return reverse_iterator (end ()); }
    const_reverse_iterator    rbegin      () const { return const_reverse_iterator (end ()); }
    reverse_iterator          rend        ()       { return reverse_iterator (begin ()); }
    const_reverse_iterator    rend        () const { return const_reverse_iterator (begin ()); }
    const_iterator            cbegin      () const { return const_iterator (std::addressof (content[0])); }
    const_iterator            cend        () const { return const_iterator (std::addressof (content[current_size])); }
    const_reverse_iterator    crbegin     () const { return const_reverse_iterator (end ()); }
    const_reverse_iterator    crend       () const { return const_reverse_iterator (begin ()); }
    
    constexpr size_type       size        () const { return current_size; }
    constexpr size_type       max_size    () const { return SZ; }
    constexpr bool            empty       () const { return current_size == 0; }

    reference                 operator[]  (size_type n)       { assert (n < current_size); return content[n]; }
    const_reference           operator[]  (size_type n) const { assert (n < current_size); return content[n]; }

    reference                 at          (size_type n)       { assert (n < current_size); return content[n]; }
    const_reference           at          (size_type n) const { assert (n < current_size); return content[n]; }

    reference                 front       ()       { return *begin (); }
    const_reference           front       () const { return *begin (); }
    reference                 back        ()       { return SZ ? *(end () - 1) : *end (); }
    const_reference           back        () const { return SZ ? *(end () - 1) : *end (); }
    TP*                       data        ()       { return std::addressof (content[0]); }
    const TP*                 data        () const { return std::addressof (content[0]); }

private:
    size_type current_size;
    value_type content[SZ ? SZ : 1];
};

    
}
