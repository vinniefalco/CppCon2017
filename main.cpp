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
#include <string>
#include <type_traits>
#include <vector>

using namespace std;

#ifndef _MSC_VER
template<class... Ts>
struct make_void
{
    using type = void;
};
template<typename... Ts>
using void_t = typename make_void<Ts...>::type;
#endif

template<bool isRequest, class Body>
struct message;

/// Holds an HTTP request
template<class Body>
struct message<true, Body>
{
	int					version;
	string				method;
	string		        target;
	map<string,string>	fields;

	typename Body::value_type body;	
};

/// Holds an HTTP response
template<class Body>
struct message<false, Body>
{
	int					version;
	int					status;
	string				reason;
	map<string,string>	fields;

	typename Body::value_type body;	
};

/// Serialize the HTTP header to an ostream
template<bool isRequest, class Body>
void write_header(ostream&,
    message<isRequest, Body> const&)
{
}

/// Holds an HTTP request
template<class Body>
using request = message<true, Body>;

/// Holds an HTTP response
template<class Body>
using response = message<false, Body>;

/// A Body that uses a string container
struct string_body
{
	using value_type = string;

	static void write(
		ostream& os, value_type const& body)
	{
		os.write(body.data(), body.size());
	}
};

/// A Body that uses a vector container
template<class T>
struct vector_body
{
	using value_type = vector<T>;

	static void write(
		ostream& os, value_type const& body)
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
		ostream& os, value_type const& body)
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
		ostream& os, value_type const& body)
	{
		size_t n;
		char buf[4096];
		FILE* f = fopen(body.c_str(), "rb");
		while(n = fread(buf, 1, sizeof(buf), f))
			os.write(buf, n);
		fclose(f);
	}
};

/// Serialize an HTTP Message
template<bool isRequest, class Body>
void write(ostream& os,
    message<isRequest, Body> const& msg)
{
    write_header(os, msg);
    Body::write(os, msg.body);
}

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

struct invalid_body_1
{
    using body_type = string;

    static void write(
		ostream& os, body_type const& body)
	{
		os.write(body.data(), body.size());
	}
};

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
}
