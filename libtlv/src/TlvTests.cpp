#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MyTest

#include "Tlv.h"

#include <boost/test/unit_test.hpp>
#include <boost/range.hpp>
#include <vector>
#include <iostream>
#include <memory>


BOOST_AUTO_TEST_CASE(getTagTest1) {
	int tag = Tlv::getTag(1, 2, 3, 5);
	int tag2 = 1 << 24 | 2 << 16 | 3 << 8 | 5;
	BOOST_CHECK_EQUAL(tag, tag2); 
	
}

BOOST_AUTO_TEST_CASE(getTagTest2) {
	int tag = Tlv::getTag(8, 99, 31, 21);
	int tag2 = 8 << 24 | 99 << 16 | 31 << 8 | 21;
	BOOST_CHECK_EQUAL(tag, tag2); 
}

BOOST_AUTO_TEST_CASE(getTagTest3) {
	int tag = Tlv::getTag(6, 5, 5, 5);
	int tag2 = 6 << 24 | 5 << 16 | 5 << 8 | 5;
	BOOST_CHECK_EQUAL(tag, tag2); 
}

BOOST_AUTO_TEST_CASE(getTagTestArray) {
	unsigned char tag_array[4] = {55, 2, 100, 10};
	int tag = Tlv::getTag(tag_array);
	int tag2 = 55 << 24 | 2 << 16 | 100 << 8 | 10;
	BOOST_CHECK_EQUAL(tag, tag2); 
}

BOOST_AUTO_TEST_CASE(getTagTestArray2) {
	unsigned char tag_array[4] = {1, 2, 3, 10};
	int tag = Tlv::getTag(tag_array);
	int tag2 = 1 << 24 | 2 << 16 | 3 << 8 | 10;
	BOOST_CHECK_EQUAL(tag, tag2); 
}

BOOST_AUTO_TEST_CASE(getTagArray_and_getTag) {
	unsigned char tag_array[4] = {1, 6, 8, 10};
	int tag0 = Tlv::getTag(1, 6, 8, 10);
	int tag = Tlv::getTag(tag_array);
	int tag2 = 1 << 24 | 6 << 16 | 8 << 8 | 10;
	BOOST_CHECK_EQUAL(tag, tag0); 
	BOOST_CHECK_EQUAL(tag2, tag0); 
}

BOOST_AUTO_TEST_CASE(getTagArray_and_getTag2) {
	unsigned char tag_array[4] = {2, 200, 22, 10};
	int tag0 = Tlv::getTag(2, 200, 22, 10);
	int tag = Tlv::getTag(tag_array);
	int tag2 = 2 << 24 | 200 << 16 | 22 << 8 | 10;
	BOOST_CHECK_EQUAL(tag, tag0); 
	BOOST_CHECK_EQUAL(tag2, tag0); 
}

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
	// which is what getTag returns
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
	unsigned char input[] = {0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
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


BOOST_AUTO_TEST_CASE(hex) {
	unsigned char data[] = { 0x11, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01 };
	Tlv client_id_tag(data, sizeof(data));

	std::vector<unsigned char> buffer = client_id_tag.getTagData(Tlv::getTag(0x11, 0x00, 0x00, 0x01));

	BOOST_REQUIRE_EQUAL(buffer.size(), 4); 
	// 0x00 0x00 0x00 0x01 is data
	
	int j = 6;
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, data[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(hex2) {
	unsigned char data[] = { 0x23, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00};
	Tlv client_id_tag(data, sizeof(data));

	std::vector<unsigned char> buffer = client_id_tag.getTagData(Tlv::getTag(0x23, 0x00, 0x00, 0x02));

	BOOST_REQUIRE_EQUAL(buffer.size(), 2); 
	// 0x00 0x00 0x00 0x01 is data
	
	int j = 6; // data start at index 6
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, data[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(hex3) {
	unsigned char data[] = { 0x22, 0x01, 0x01, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x22, 0x12, 0x11 };
	Tlv client_id_tag(data, sizeof(data));

	std::vector<unsigned char> buffer = client_id_tag.getTagData(Tlv::getTag(0x22, 0x01, 0x01, 0x01));

	BOOST_REQUIRE_EQUAL(buffer.size(), 8); 
	// 0x00 0x00 0x00 0x01 is data
	
	int j = 6;
	for(auto i : buffer) {
		BOOST_CHECK_EQUAL(i, data[j]);
		++j;
	}
}

BOOST_AUTO_TEST_CASE(not_found) {
	unsigned char input[] = {0, 0, 0, 14, 1, 15, 0, 0, 0, 11, 0, 2, 4, 4, 0, 0, 0, 13, 0, 1, 2};
	std::vector<unsigned char> output_find_22 = {};

	Tlv o(input, boost::size(input));
	std::vector<unsigned char> buffer = o.getTagData(22); // returns data without header
	// there is no data with this tag

	BOOST_REQUIRE_EQUAL(buffer.size(), output_find_22.size());
}

BOOST_AUTO_TEST_CASE(hex_not_found) {
	unsigned char input[] = { 0x22, 0x01, 0x01, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x22, 0x12, 0x11 };
	std::vector<unsigned char> output = {};

	Tlv o(input, boost::size(input));
	std::vector<unsigned char> buffer = o.getTagData(127); // returns data without header
	// there is no data with this tag

	BOOST_REQUIRE_EQUAL(buffer.size(), output.size());
}


