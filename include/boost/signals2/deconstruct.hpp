#ifndef BOOST_SIGNALS2_DECONSTRUCT_HPP
#define BOOST_SIGNALS2_DECONSTRUCT_HPP

//  deconstruct.hpp
//  based on make_shared.hpp and make_shared_access patch from Michael Marcin
//
//  Copyright (c) 2007, 2008 Peter Dimov
//  Copyright (c) 2008 Michael Marcin
//  Copyright (c) 2009 Frank Mori Hess
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  See http://www.boost.org
//  for more information

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/deconstruct_ptr.hpp>
#include <boost/type_traits/type_with_alignment.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <cstddef>
#include <new>

namespace boost
{

namespace signals2
{

namespace detail
{

template< std::size_t N, std::size_t A > struct sp_aligned_storage
{
    union type
    {
        char data_[ N ];
        typename boost::type_with_alignment< A >::type align_;
    };
};

template< class T > class deconstruct_deleter
{
private:

    typedef typename sp_aligned_storage< sizeof( T ), ::boost::alignment_of< T >::value >::type storage_type;

    bool initialized_;
    storage_type storage_;

private:

    void destroy()
    {
        if( initialized_ )
        {
            T* p = reinterpret_cast< T* >( storage_.data_ );
            detail::do_predestruct(p);
            p->~T();
            initialized_ = false;
        }
    }

public:

    deconstruct_deleter(): initialized_( false )
    {
    }

    // this copy constructor is an optimization: we don't need to copy the storage_ member,
    // and shouldn't be copying anyways after initialized_ becomes true
    deconstruct_deleter(const deconstruct_deleter &): initialized_( false )
    {
    }

    ~deconstruct_deleter()
    {
        destroy();
    }

    void operator()( T * )
    {
        destroy();
    }

    void * address()
    {
        return storage_.data_;
    }

    void set_initialized()
    {
        initialized_ = true;
    }
};

template< class T > T forward( T t )
{
    return t;
}

class deconstruct_access
{
public:

    template< class T >
    static boost::shared_ptr< T > deconstruct()
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T();
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

#if defined( BOOST_HAS_VARIADIC_TMPL ) && defined( BOOST_HAS_RVALUE_REFS )

    // Variadic templates, rvalue reference

    template< class T, class... Args >
    static boost::shared_ptr< T > deconstruct( Args && ... args )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( detail::forward<Args>( args )... );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

#else

    template< class T, class A1 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4, class A5 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4, a5 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4, class A5, class A6 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4, a5, a6 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4, a5, a6, a7 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

    template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
    static boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9 )
    {
        boost::shared_ptr< T > pt( static_cast< T* >( 0 ), detail::deconstruct_deleter< T >() );

        detail::deconstruct_deleter< T > * pd = boost::get_deleter< detail::deconstruct_deleter< T > >( pt );

        void * pv = pd->address();

        new( pv ) T( a1, a2, a3, a4, a5, a6, a7, a8, a9 );
        pd->set_initialized();

        boost::shared_ptr< T > retval( pt, static_cast< T* >( pv ) );
        detail::do_postconstruct(retval.get());
        return retval;
    }

#endif
};
}  // namespace detail

// Zero-argument versions
//
// Used even when variadic templates are available because of the new T() vs new T issue

template< class T > boost::shared_ptr< T > deconstruct()
{
	return detail::deconstruct_access::deconstruct<T>();
}

#if defined( BOOST_HAS_VARIADIC_TMPL ) && defined( BOOST_HAS_RVALUE_REFS )

// Variadic templates, rvalue reference

template< class T, class... Args > boost::shared_ptr< T > deconstruct( Args && ... args )
{
    return detail::deconstruct_access::deconstruct( detail::forward<Args>( args )... );
}

#else

// C++03 version

template< class T, class A1 >
boost::shared_ptr< T > deconstruct( A1 const & a1 )
{
	return detail::deconstruct_access::deconstruct<T>(a1);
}

template< class T, class A1, class A2 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2 )
{
   return detail::deconstruct_access::deconstruct<T>(a1,a2);
}

template< class T, class A1, class A2, class A3 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3 )
{
	return detail::deconstruct_access::deconstruct<T>(a1,a2,a3);
}

template< class T, class A1, class A2, class A3, class A4 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4);
}

template< class T, class A1, class A2, class A3, class A4, class A5 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4,a5);
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4,a5,a6);
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4,a5,a6,a7);
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4,a5,a6,a7,a8);
}

template< class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9 >
boost::shared_ptr< T > deconstruct( A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6, A7 const & a7, A8 const & a8, A9 const & a9 )
{
    return detail::deconstruct_access::deconstruct<T>(a1,a2,a3,a4,a5,a6,a7,a8,a9);
}

using detail::deconstruct_access;

#endif

} // namespace signals2

} // namespace boost

#endif // #ifndef BOOST_SIGNALS2_DECONSTRUCT_HPP
