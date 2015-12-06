
#include "interest_network.h"
#include "pop_logger.h"
#include "pugixml.h"

InterestNetwork::InterestNetwork()
{
}

InterestNetwork::InterestNetwork(const InterestNetwork& p)
{
	_id = p._id;
	_friends = p._friends;
}

InterestNetwork::~InterestNetwork()
{
}

bool InterestNetwork::operator ==(const InterestNetwork& p) const
{
	return ( _id.compare( p._id)) == 0;
}

InterestNetwork& InterestNetwork::operator =(const InterestNetwork& p)
{
	if (this != &p)
	{
		_id = p._id;
		_friends = p._friends;
	}
	return *this;
}

InterestNetwork& InterestNetwork::operator =(const InterestNetwork* const p)
{
	if (this != p)
	{
		_id = p->_id;
		_friends = p->_friends;
	}
	return *this;
}

void InterestNetwork::Serialize(pop_buffer &buf, bool pack)
{
	/* serialize */
	if(pack)
	{
		buf.Pack(&_id, 1);

		int friendsNb = _friends.size();
		buf.Pack(&friendsNb, 1);
		for(std::list<pop_accesspoint>::iterator i = _friends.begin(); i != _friends.end() ; i++)
		{
			i->Serialize(buf, pack);
		}
	}

	/* unserialize */
	else
	{
		buf.UnPack(&_id, 1);

		int friendsNb;
		buf.UnPack(&friendsNb, 1);
		_friends.clear();
		for(int i = 0; i < friendsNb; i++)
		{
			pop_accesspoint ap;
			ap.Serialize(buf, pack);
			_friends.push_back(ap);
		}
	}
}

void InterestNetwork::setId(const std::string &id)
{
	_id = id;
}

std::string InterestNetwork::getId() const
{
	return _id;
}

void InterestNetwork::addFriend(const std::string &friendContact)
{
	pop_accesspoint ap;
	ap.SetAccessString(friendContact);
	addFriend(ap);
}

void InterestNetwork::addFriend(const pop_accesspoint &friendAccessPoint)
{
	_friends.push_back(friendAccessPoint);
}

void InterestNetwork::removeFriend(const std::string &friendContact)
{
	pop_accesspoint ap;
	ap.SetAccessString(friendContact);
	removeFriend(ap);
}

void InterestNetwork::removeFriend(const pop_accesspoint &friendAccessPoint)
{
	_friends.remove(friendAccessPoint);
}

const std::list<pop_accesspoint> InterestNetwork::getFriends() const
{
	return _friends;
}

InterestNetworkCollection::InterestNetworkCollection()
{}

InterestNetworkCollection::InterestNetworkCollection(const std::list<InterestNetwork>& interests)
{
	_interests = interests;
}

std::list<InterestNetwork> InterestNetworkCollection::get() const
{
	return _interests;
}


void InterestNetworkCollection::set(const std::list<InterestNetwork>& insterests)
{
	_interests = insterests;
}

void InterestNetworkCollection::Serialize(pop_buffer &buf, bool pack)
{
	/* serialize */
	if(pack)
	{
		int interestNb = _interests.size();
		buf.Pack(&interestNb, 1);
		for(std::list<InterestNetwork>::iterator i = _interests.begin(); i != _interests.end() ; i++)
		{
			i->Serialize(buf, pack);
		}
	}

	/* unserialize */
	else
	{
		int interestNb;
		buf.UnPack(&interestNb, 1);
		_interests.clear();
		for(int i = 0; i < interestNb; i++)
		{
			InterestNetwork interest;
			interest.Serialize(buf, pack);
			_interests.push_back(interest);
		}
	}
}

bool InterestNetworkCollection::loadFromConfiguration(const char* file)
{
	pugi::xml_document doc;

	/* Loading the configuration of the networks */
	pugi::xml_parse_result result = doc.load_file(file);

	/* If an error occurred */
	if (!result)
	{
		return false;
	}

	/* Loading the networks */
	pugi::xml_node rootNode = doc.child("interests");
	for (pugi::xml_node interestNode = rootNode.child("interest"); interestNode; interestNode = interestNode.next_sibling("interest"))
	{
		std::string id = interestNode.child_value("id");
		InterestNetwork interest;
		interest.setId(id);

		// friends
		for (pugi::xml_node friendNode = interestNode.child("friend"); friendNode; friendNode = friendNode.next_sibling("friend"))
		{
			std::string friendInfo = friendNode.text().get();
			interest.addFriend(friendInfo);
		}

		_interests.push_back(interest);
	}

	return true;
}

bool InterestNetworkCollection::saveToConfiguration(const char* file)
{
	pugi::xml_document doc;
	pugi::xml_node rootNode = doc.append_child("interests");

	for(std::list<InterestNetwork>::iterator i = _interests.begin(); i != _interests.end(); i++)
	{
		pugi::xml_node interestNode = rootNode.append_child("interest");
		interestNode.append_child("id").append_child(pugi::node_pcdata).set_value(i->getId().c_str());

		std::list<pop_accesspoint> friends = i->getFriends();
		for(std::list<pop_accesspoint>::iterator f = friends.begin(); f != friends.end(); f++)
		{
			interestNode.append_child("friend").append_child(pugi::node_pcdata).set_value((f->GetAccessString()).c_str());
		}
	}
	return doc.save_file(file);
}

