/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#pragma once

#include "SimpleStatusObserver.hh"
#include "util.hh"

class Clients : public SimpleStatusObserver {
public:
	class Client {
	public:
		enum State {
			Connected, Chatter, Player
		};
		Client() : name("UNKNOWN"), state(Connected) {}

		template<State S>
		bool is() {
			return state = S;
		}

		bool isPlayer() const {
			return state == Player;
		}
		bool isChatter() const {
			return state == Chatter;
		}

		bool isUnauthenticated() const {
			return state == Connected;
		}

		void authenticate(const State s, const std::string &name = "") {
			this->state = s;
			if(!name.empty())
				this->name = name;
		}

		std::string getName() const {
			return name;
		}
	private:
		std::string name;
		State state;
	};

	Client& getClient(int id) {
		if(id > max_id) {
			clients.resize(id+1);
			max_id = id;
		}
		return clients[id];
	}

	void request_while_replay_for_client(int client_id) {
		requested_whole_replay.push_back(client_id);
	}

	virtual void notify(const unsigned  char tag[4], size_t length, unsigned char *value) {
		if(util::tag_equal(tag, tag::game_tags::resync_response)) {
			for(const int client_id : requested_whole_replay) {
				std::lock_guard<std::mutex> lock{libnet_mutex};
				if(!libnet_send_to(client_id, tag::game, length, value)) {
					log_warn("Could not send resync response to some clients, they'll probably rerequest?");
				}
			}

		}

		if(util::tag_equal(tag, tag::game_tags::step)) {
			if(!libnet_send(tag::game, length, value)) {
				log_warn("Could not send game state to some clients, let's hope they resync on next send or request resync manually");
			}
		}
	}

private:
	int max_id = 0;
	std::vector<Client> clients;

	std::vector<int> requested_whole_replay;
};

