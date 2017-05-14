//#include "TlvObject.hpp"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include "TlvObject.h"

BOOST_AUTO_TEST_CASE(normal_find)
{
	Tlv o;
	unsigned char t[] = {3, 3, 3};
	unsigned char tt[] = {0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	o.add(12, 0, 3, t);
	o.add(14, 1, 13, tt);
	std::vector<unsigned char> buffer = o.findTagData(13);

	BOOST_REQUIRE_EQUAL(buffer.size(), 1);
	BOOST_CHECK_EQUAL(buffer[0], 2);
}

BOOST_AUTO_TEST_CASE(nested_find)
{
	Tlv o;
	unsigned char t[] = {3, 3, 3};
	unsigned char tt[] = {0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	o.add(12, 0, 3, t);
	o.add(14, 1, 13, tt);
	std::vector<unsigned char> buffer = o.findTagData(14);

	BOOST_REQUIRE_EQUAL(buffer.size(), 13);

	int j = 0;
	for(auto i : buffer)
	{
		BOOST_CHECK_EQUAL(i, tt[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(add_data_get_all)
{
	Tlv o;
	std::vector<unsigned char> input = {0, 0, 14, 1, 13, 0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output = {0, 0, 15, 1, 18, 0, 0, 14, 1, 13, 0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};

	o.add(15, 1, 18, input.data());

	std::vector<unsigned char> buffer = o.getAllData();

	BOOST_REQUIRE_EQUAL(buffer.size(), output.size());

	int j = 0;
	for(auto i : buffer)
	{
		BOOST_CHECK_EQUAL(i, output[j]);
		++j;
	}
}


BOOST_AUTO_TEST_CASE(parse_and_get)
{
	unsigned char input[] = {0, 0, 14, 1, 13, 0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_get_all = {0, 0, 14, 1, 13, 0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_find_14 = {0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};

	Tlv o(input, 18);
	std::vector<unsigned char> buffer = o.getAllData();

	BOOST_REQUIRE_EQUAL(buffer.size(), output_get_all.size());

	int j = 0;
	for(auto i : buffer)
	{
		BOOST_CHECK_EQUAL(i, output_get_all[j]);
		++j;
	}
}


BOOST_AUTO_TEST_CASE(parse_and_find)
{
	unsigned char input[] = {0, 0, 14, 1, 13, 0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_find_14 = {0, 0, 11, 0, 2, 4, 4, 0, 0, 13, 0, 1, 2};

	Tlv o(input, 18);
	std::vector<unsigned char> buffer = o.findTagData(14);

	BOOST_REQUIRE_EQUAL(buffer.size(), output_find_14.size());

	int j = 0;
	for(auto i : buffer)
	{
		BOOST_CHECK_EQUAL(i, output_find_14[j]);
		++j;
	}
}

