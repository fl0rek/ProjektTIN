/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#pragma once

std::mutex libnet_mutex;

namespace util {

class TagNotFoundException : public std::runtime_error {
	using runtime_error::runtime_error;
};

bool tag_exists(std::vector<unsigned char> buffer) {
	return !!buffer.size();
}

template<typename T>
T extract_tag(std::vector<unsigned char> buffer, size_t offset = 0) {
	if(!tag_exists(buffer))
		throw new TagNotFoundException("");
	T ret;
	memcpy(&ret, &buffer[offset], sizeof(T));
	return ret;
}

int get_client_id(Tlv msg) {
	auto client_id_buffer = msg.getTagData(tag::internal_tags::client_id);
	if(!tag_exists(client_id_buffer))
		return -1;

	return extract_tag<int>(client_id_buffer);
}

bool tag_equal(const unsigned char ltag[4], const unsigned char rtag[4]) {
	return ltag[0] == rtag[0]
		&& ltag[1] == rtag[1]
		&& ltag[2] == rtag[2]
		&& ltag[3] == rtag[3];
}

void dump(const void *mem, size_t n) {
	const unsigned char *p = reinterpret_cast<const unsigned char*>(mem);
	std::cout << std::endl << "====" << std::endl;
	for(size_t i = 0; i < n; i++) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex << int(p[i]) << " ";
	}
	std::cout << std::endl << "====" << std::endl;
}
}



namespace libnet_helper {
void sendAuthError(int client_id) {
	unsigned char buffer[sizeof(tag::internal_tags::authentication_error) +3];
	memset(buffer, 0, sizeof * buffer);
	memcpy(buffer, tag::internal_tags::authentication_error, sizeof(tag::internal_tags::authentication_error));
	buffer[sizeof(tag::internal_tags::authentication_error) + 1] = 1;

	{
		std::lock_guard<std::mutex> lock{libnet_mutex};
		if(! libnet_send_to(client_id, tag::internal, sizeof(buffer), buffer)) {
			log_warn("Error sending auth error message, client will probably get notified on next message");
		}
	}
}

void sendPlayerHisId(int client_id) {
	unsigned char buffer[sizeof tag::internal_tags::client_id + sizeof client_id + 2];
	memset(buffer, 0, sizeof * buffer);
	memcpy(buffer, tag::internal_tags::client_id, sizeof(tag::internal_tags::client_id));
	buffer[sizeof(tag::internal_tags::client_id) + 1] = 1;
	memcpy(buffer + sizeof(tag::internal_tags::client_id) +2, &client_id, sizeof client_id);

	{
		std::lock_guard<std::mutex> lock{libnet_mutex};
		if(! libnet_send_to(client_id, tag::internal, sizeof(buffer), buffer)) {
			log_warn("Error sending auth status back to player");
		}
	}
}
}
