#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MyTest

#include "Tlv.h"

#include <boost/test/unit_test.hpp>
#include <boost/range.hpp>
#include <vector>
#include <iostream>

/*
project libtlv from MASM
author Michal Citko
date 24.05.2017
*/

BOOST_AUTO_TEST_CASE(normal_find) {
	Tlv o;
	unsigned char first_buffer[] = {3, 3, 3};
	unsigned char second_buffer[] = {0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	o.add(12, 0, boost::size(first_buffer), first_buffer);
	o.add(14, 1, boost::size(second_buffer), second_buffer);
	// 0 0 0 13 - tag  0 - no embedded tags 1 - size 2 - data
	std::vector<unsigned char> buffer = o.getTagData(13);

	BOOST_REQUIRE_EQUAL(buffer.size(), 1); // check if data size == 1 
	BOOST_CHECK_EQUAL(buffer[0], 2); // check if data == 2
}


BOOST_AUTO_TEST_CASE(normal_find2) {
	Tlv o;
	unsigned char first_buffer[] = {3, 3, 3};
	unsigned char second_buffer[] = {0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	o.add(2147483647, 0, boost::size(first_buffer), first_buffer);
	o.add(14, 1, boost::size(second_buffer), second_buffer);

	std::vector<unsigned char> buffer = o.getTagData(2147483647);

	BOOST_REQUIRE_EQUAL(buffer.size(), 3); // check if data size == 3
	BOOST_CHECK_EQUAL(buffer[0], 3); // check if data == 3, 3, 3
	BOOST_CHECK_EQUAL(buffer[1], 3);
	BOOST_CHECK_EQUAL(buffer[2], 3);
}

BOOST_AUTO_TEST_CASE(normal_find3) {
	Tlv o;
	unsigned char first_buffer[] = {3, 3, 3};
	unsigned char second_buffer[] = {1, 2, 3, 4, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	o.add(13, 0, boost::size(first_buffer), first_buffer);
	o.add(14, 1, boost::size(second_buffer), second_buffer);

	std::vector<unsigned char> buffer = o.getTagData(Tlv::getTag(1, 2, 3, 4));
	// 1, 2, 3, 4 is 00000001000000100000001100000100 which is 16909060 
	// 1<<24 | 2<<16 | 3<<8 | 4
	// this is what getTag returns
	
	BOOST_REQUIRE_EQUAL(buffer.size(), 2); // check if data size == 2
	BOOST_CHECK_EQUAL(buffer[0], 4); // check if data == 4, 4
	BOOST_CHECK_EQUAL(buffer[1], 4);
}

BOOST_AUTO_TEST_CASE(nested_find) {
	Tlv o;
	unsigned char first_buffer[] = {3, 3, 3};
	unsigned char second_buffer[] = {0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	o.add(12, 0, boost::size(first_buffer), first_buffer);
	o.add(14, 1, boost::size(second_buffer), second_buffer);
	std::vector<unsigned char> buffer = o.getTagData(14);
	// 0 0 0 14 - tag  1 - embedded tags 1 - size 15 data -  0 0 0 11 0 2 4 4 0 0 0 13 0 1 2
	BOOST_REQUIRE_EQUAL(buffer.size(), 15); //check if data size == 15

	int j = 0; //check if data == contents of second_buffer
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, second_buffer[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(add_data_get_all) {
	Tlv o;
	std::vector<unsigned char> input = {0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output = {0, 0, 0, 15, 1, 21, 0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};

	o.add(15, 1, input.size(), input.data());

	std::vector<unsigned char> buffer = o.getAllData();

	BOOST_REQUIRE_EQUAL(buffer.size(), output.size());// check if size of returned buffer equals size of output

	int j = 0;// check if contents match
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, output[j]);
		++j;
	}
}


BOOST_AUTO_TEST_CASE(parse_and_get) {
	unsigned char input[] = {0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_get_all = {0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};

	Tlv o(input, boost::size(input));
	std::vector<unsigned char> buffer = o.getAllData();

	BOOST_REQUIRE_EQUAL(buffer.size(), output_get_all.size()); //check sizes

	int j = 0;//check content
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, output_get_all[j]);
		++j;
	}
}


BOOST_AUTO_TEST_CASE(parse_and_find) {
	unsigned char input[] = {0, 0, 0, 14, 1, 13, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_find_14 = {0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};

	Tlv o(input, boost::size(input));
	std::vector<unsigned char> buffer = o.getTagData(14); // returns data without header
	// 0 0 0 14 1 13 is header, rest is data

	BOOST_REQUIRE_EQUAL(buffer.size(), output_find_14.size());

	int j = 0;
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, output_find_14[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(hex_test) {
	unsigned char foo[] = { 0x11, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01 };
	Tlv client_id_tag(foo, sizeof(foo));

	std::vector<unsigned char> client_id_vector = client_id_tag.getTagData(Tlv::getTag(0x11, 0x00, 0x00, 0x01));

	BOOST_REQUIRE_EQUAL(client_id_vector.size(), 4); // 4 is size of data which iss 0x00 0x00 0x00 0x01
	BOOST_CHECK_EQUAL(client_id_vector[0], 0);
	BOOST_CHECK_EQUAL(client_id_vector[1], 0);
	BOOST_CHECK_EQUAL(client_id_vector[2], 0);
	BOOST_CHECK_EQUAL(client_id_vector[3], 1);
}
