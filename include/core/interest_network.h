/**
 * @file	interest_network.h
 * @brief	Network of interest
 * @author	Loïc Monney
 * @version	1.0
 */

#ifndef INTEREST_NETWORK_H_
#define INTEREST_NETWORK_H_

#include "pop_base.h"
#include "pop_buffer.h"
#include "pop_accesspoint.h"
#include <list>
#include <string>

/**
 * @class	InterestNetwork
 * @brief	An interest network is used with a given application and allows
 * 			its users to share resources between computers in the same
 * 			interest network.
 *
 * 			Each interest network is defined with:
 * 				- a unique id which identifies the network
 * 				- a list of friends
 */
class InterestNetwork : public POPBase
{
	private:
		std::string _id;
		std::list<pop_accesspoint> _friends;

	public:

		/**
		 * @brief Constructor
		 *
		 * Constructs a new empty InterestNetwork
		 */
		InterestNetwork();

		/**
		 * @brief Constructor
		 *
		 * Constructs a new InterestNetwork based on the given parameter
		 */
		InterestNetwork(const InterestNetwork &p);

		/**
		 * @brief Destructor
		 *
		 * Destructs this InterestNetwork
		 */
		~InterestNetwork();

		/**
		 * @brief Compare this network of interest with the one given in parameter
		 *
		 * @return true if the two networks have the same id
		 */
		bool operator ==(const InterestNetwork &p) const;

		/**
		 * @brief Assignment
		 */
		InterestNetwork& operator =(const InterestNetwork &p);

		/**
		 * @brief Assignment
		 */
		InterestNetwork& operator =(const InterestNetwork* const p);

		/**
		 * @brief Serialize and unserialize this InterestNetwork to/from the given buffer
		 *
		 * @param buf	:	the buffer
		 * @param pack	:	true for serialize, false for unserialize
		 */
		virtual void Serialize(POPBuffer &buf, bool pack);

		/**
		 * @brief Change the id
		 */
		void setId(const std::string &id);

		/**
		 * @brief Get the id of this network
		 */
		std::string getId() const;

		/**
		 * @brief Add the reference to a friend
		 *
		 * @param friendContact   : ip address + port + protocol (e.g. socket://10.0.0.11:2711)
		 */
		void addFriend(const std::string &friendContact);

		/**
		 * @brief Add the reference to a friend
		 *
		 * @param friendAccessPoint : access point of the friend's job manager
		 */
		void addFriend(const pop_accesspoint &friendAccessPoint);

		/**
		 * @brief Remove the given friend
		 *
		 * @param friendContact  : ip address + port + protocol (e.g. socket://10.0.0.11:2711)
		 */
		void removeFriend(const std::string &friendContact);

		/**
		 * @brief Remove the given friend
		 *
		 * @param friendAccessPoint : access point of the friend's job manager
		 */
		void removeFriend(const pop_accesspoint &friendAccessPoint);

		/**
		 * @brief Get the list of all friends that are registred in this network
		 *
		 * @return the list of all friends
		 */
		const std::list<pop_accesspoint> getFriends() const;
};

/**
 * @class	InterestNetworkCollection
 * @brief	Collection of network of interests
 *
 * 			This class is useful to encapsulate a list of network of interest.
 * 			Thus it may be sent between POP objects
 */
class InterestNetworkCollection : public POPBase
{
	private:
		std::list<InterestNetwork> _interests;

	public:
		InterestNetworkCollection();
		InterestNetworkCollection(const std::list<InterestNetwork>& interests);

		/**
		 * @brief get the list of networks of friends contained in this collection
		 *
		 * @return list of networks of friends
		 */
		std::list<InterestNetwork> get() const;

		/**
		 * @brief change the list of networks of friends contained in this collection
		 *
		 * @param interests :   the new list
		 */
		void set(const std::list<InterestNetwork>& interests);

		/**
		 * @brief Serialize and unserialize this collection to/from the given buffer
		 *
		 * @param buf	:	the buffer
		 * @param pack	:	true for serialize, false for unserialize
		 */
		virtual void Serialize(pop_buffer &buf, bool pack);

		/**
		 * @brief load and replace the list of networks of friends contained in this
		 * 		  collection with the one stored in the given file
		 *
		 * @return true if the file has been successfully loaded
		 */
		bool loadFromConfiguration(const char* file);

		/**
		 * @brief store the list of networks of friends contained in this
		 * 		  collection to the given file
		 *
		 * @return true if the file has been successfully stored
		 */
		bool saveToConfiguration(const char* file);
};


#endif /* INTEREST_NETWORK_H_ */
