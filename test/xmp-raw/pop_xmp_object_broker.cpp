#include "pop_xmp_object_broker.h"

#include "paroc_combox.h"
#include "pop_buffer.h"
#include "paroc_exception.h"
#include "pop_xmp_object.h"

#include <stdio.h>

// Generated by the compiler 


bool POPXMPObject_popcobject_Broker::invoke(unsigned method[3], pop_buffer &_popc_broker, paroc_connection *_popc_connection)
{
  if (*method == CLASSUID_POPXMPObject) {
    switch(method[1]) {
      case 10: Invoke_POPXMPObject_10(_popc_broker, _popc_connection); 
        return true;
      case 13: Invoke_execute_xmp_1_13(_popc_broker, _popc_connection); 
        return true;
      case 14: Invoke_set_value_14(_popc_broker, _popc_connection); 
        return true;
      default: return false;
    }
  }
  return POPC_GroupBroker::invoke(method, _popc_broker, _popc_connection);
}

POPXMPObject_popcobject_Broker::POPXMPObject_popcobject_Broker()
{
  printf("POPXMPObject_popcobject_Broker created\n"); 
  static popc_method_info _popc_minfo[3] = { {10,(char*)"POPXMPObject"}, {12,(char*)"execute_xmp_1"}, {13,(char*)"set_value"} };
  add_method_info(CLASSUID_POPXMPObject, _popc_minfo, 3);
}


// Collective, therefore the last process send a response
void POPXMPObject_popcobject_Broker::Invoke_POPXMPObject_10(pop_buffer &_popc_buffer, paroc_connection *_popc_connection)
{
  printf("Invoke_POPXMPObject_10 - Constructor\n");  
  try {
    _popc_internal_object = new POPXMPObject_popcobject();
  } catch(std::exception& e) {
    printf("POP-C++ Warning: Exception '%s' raised in method 'POPXMPObject' of class 'POPXMPObject'\n",e.what()); 
    throw;
  }
  
  if (_popc_connection != NULL) {
    _popc_buffer.Reset();
    paroc_message_header _popc_message_header("POPXMPObject");
    _popc_buffer.SetHeader(_popc_message_header);

    popc_send_response(_popc_buffer, _popc_connection, true); 
    _popc_connection->reset();
  }
}



void POPXMPObject_popcobject_Broker::Invoke_execute_xmp_1_13(pop_buffer &_popc_buffer, paroc_connection *_popc_connection)
{
  POPXMPObject_popcobject* _popc_object = dynamic_cast<POPXMPObject_popcobject *>(_popc_internal_object);
  try {
    _popc_object->execute_xmp_1();
  } catch(std::exception& e) {
    printf("POP-C++ Warning: Exception '%s' raised in method 'execute_xmp_1' of class 'POPXMPObject'\n", e.what()); 
    throw;
  }
  
  if (_popc_connection != NULL)  {
    _popc_buffer.Reset();
    paroc_message_header _popc_message_header("execute_xmp_1");
    _popc_buffer.SetHeader(_popc_message_header);
    popc_send_response(_popc_buffer, _popc_connection, true); 
    _popc_connection->reset();   
  }
}

// Generate method set_value
void POPXMPObject_popcobject_Broker::Invoke_set_value_14(pop_buffer &_popc_buffer, paroc_connection *_popc_connection)
{
  int val;
  _popc_buffer.Push("val","int", 1);
  _popc_buffer.UnPack(&val,1);
  _popc_buffer.Pop();

  POPXMPObject_popcobject * _popc_object = dynamic_cast<POPXMPObject_popcobject *>(_popc_internal_object);
  try {
    _popc_object->set_value(val);
  } catch(std::exception& e) {
    printf("POP-C++ Warning: Exception '%s' raised in method 'set_value' of class 'POPXMPObject'\n",e.what()); 
    throw;
  }
  
  if (_popc_connection != NULL) {
    _popc_buffer.Reset();
    paroc_message_header _popc_message_header("set_value");
    _popc_buffer.SetHeader(_popc_message_header);

    popc_send_response(_popc_buffer, _popc_connection, false);     
    _popc_connection->reset();      
  }
}

POPC_GroupBroker* POPXMPObject_popcobject_Broker::_init() { return new POPXMPObject_popcobject_Broker; }
POPC_GroupBrokerFactory POPXMPObject_popcobject_Broker::_popc_factory(_init, "POPXMPObject");
