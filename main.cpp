//
// Copyright (c) 2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2017
//

#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

using namespace std;

//------------------------------------------------------------------------------
//
// Message Container
//
//------------------------------------------------------------------------------

/// Holds an HTTP message
template<bool isRequest, class Body, class Fields>
struct message;

/// Holds an HTTP request
template<class Body, class Fields>
struct message<true, Body, Fields> : Fields,
    private Body::value_type
{
	int version;

    string_view method() const
    {
        return this->get_method();
    }

    void method(string_view s)
    {
        this->set_method(s);
    }

    string_view target() const
    {
        return this->get_target();
    }

    void target(string_view s)
    {
        this->set_target(s);
    }

	typename Body::value_type const&
    body() const
    {
        return *this;
    }

    typename Body::value_type&
    body()
    {
        return *this;
    }
};

/// Holds an HTTP response
template<class Body, class Fields>
struct message<false, Body, Fields> : Fields,
    private Body::value_type
{
	int version;
	int status;

    string_view reason() const
    {
        return this->get_reason();
    }

    void reason(string_view s)
    {
        this->set_reason(s);
    }

	typename Body::value_type const&
    body() const
    {
        return *this;
    }

    typename Body::value_type&
    body()
    {
        return *this;
    }
};

//------------------------------------------------------------------------------
//
// Body
//
//------------------------------------------------------------------------------

/// `true_type` if B meets the requirements of Body
template<class B, class = void>
struct is_body : false_type
{
};

template<class B>
struct is_body<B, void_t<
    typename B::value_type,
    decltype(
        B::write(
            declval<ostream&>(),
            declval<typename B::value_type const&>()),
        (void)0)
    >> : true_type
{
};

/// A Body that uses a string container
struct string_body
{
	using value_type = string;

	static void write(
		ostream& os, string const& body)
	{
        os << body;
	}
};

/// A Body that uses a vector container
template<class T>
struct vector_body
{
	using value_type = vector<T>;

	static void write(
		ostream& os, vector<T> const& body)
	{
		os.write(body.data(), body.size());
	}
};

/// A Body that uses a list container
template<class T>
struct list_body
{
	using value_type = list<T>;

	static void write(
		ostream& os, list<T> const& body)
	{
		for(auto const& t : body)
			os << t;
	}
};

/// A Body using file contents
struct file_body
{
	using value_type = string; // Path to file
	
	static void write(
		ostream& os, string const& path)
	{
		size_t n;
		char buf[4096];
		FILE* f = fopen(path.c_str(), "rb");
		while((n = fread(buf, 1, sizeof(buf), f)))
			os.write(buf, n);
		fclose(f);
	}
};

/// An empty body
struct empty_body
{
    struct value_type {};

	static void write(ostream&, value_type const&)
    {
        // Do nothing
    }
};

//------------------------------------------------------------------------------
//
// Fields Container
//
//------------------------------------------------------------------------------

/// A container for HTTP fields
template<class Allocator>
struct basic_fields
{
    void set(string_view, string_view);
    string_view operator[](string_view) const;

protected:
    string_view get_method() const noexcept;
    void set_method(string_view);
    
    string_view get_target() const noexcept;
    void set_target(string_view);

    string_view get_reason() const noexcept;
    void set_reason(string_view);
};

//------------------------------------------------------------------------------
//
// Type Aliases
//
//------------------------------------------------------------------------------

/// A container for HTTP fields using the default allocator
using fields = basic_fields<std::allocator<char>>;

/// Holds an HTTP request
template<class Body, class Fields = fields>
using request = message<true, Body, Fields>;

/// Holds an HTTP response
template<class Body, class Fields = fields>
using response = message<false, Body, Fields>;

//------------------------------------------------------------------------------
//
// Serialization
//
//------------------------------------------------------------------------------

/// Serialize the HTTP header to an ostream
template<bool isRequest, class Body, class Fields>
void write_header(ostream&,
    message<isRequest, Body, Fields> const&)
{
}

/// Serialize an HTTP Message
template<bool isRequest, class Body, class Fields>
void write(ostream& os,
    message<isRequest, Body, Fields> const& msg)
{
    static_assert(is_body<Body>::value,
        "Body requirements not met");
    write_header(os, msg);
    Body::write(os, msg.body());
}

//------------------------------------------------------------------------------
//
// Tests
//
//------------------------------------------------------------------------------

// Error: missing value_type
struct invalid_body_1
{
    using body_type = string;

    static void write(
		ostream& os, body_type const& body)
	{
		os.write(body.data(), body.size());
	}
};

// Error: wrong signature for write
struct invalid_body_2
{
    using value_type = string;

    static void write(ostream& os)
	{
		os << "void";
	}
};

static_assert(is_body<string_body>::value, "");
static_assert(is_body<vector_body<char>>::value, "");
static_assert(is_body<list_body<string>>::value, "");
static_assert(is_body<file_body>::value, "");
static_assert(is_body<empty_body>::value, "");

static_assert(! is_body<invalid_body_1>::value, "");
static_assert(! is_body<invalid_body_2>::value, "");

int
main()
{
    {
        request<string_body> req;
        write(cout, req);
    }
    {
        response<vector_body<char>> res;
        write(cout, res);
    }
    {
        response<list_body<string>> res;
        write(cout, res);
    }
    {
        response<empty_body> res;
        write(cout, res);
    }
}
