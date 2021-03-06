/**
 *
 * Copyright (c) 2005-2012 POP-C++ project - GRID & Cloud Computing group, University of Applied Sciences of western
 *Switzerland.
 * http://gridgroup.hefr.ch/popc
 *
 * @author Valentin Clement
 * @date 2012/12/04
 * @brief Implementation of the abstraction of a UDS communication
 *
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#include "pop_combox_uds.h"
#include "pop_logger.h"

// Constant declaration
const char* popc_combox_uds::UDS_PROTOCOL_NAME = "uds";

/**
 * UDS Combox constructor initialize internal values
 */
popc_combox_uds::popc_combox_uds()
    : _socket_fd(-1),
      _is_server(false),
      _active_connection_nb(0),
      _timeout(-1),
      _connected(false),
      _is_first_connection(true) {
}

/**
 * UDS Combox destructor: Close the connection in case this combox has been connected.
 */
popc_combox_uds::~popc_combox_uds() {
    if (_connected || _is_server) {
        Close();
    }
}

/**
 * Create a combox client or server with a port number. NOT USED FOR UDS COMBOX
 * @param port    Port number on which the combox should be create
 * @param server  FALSE for a client combox and TRUE for a server combox
 * @return FALSE in any cases
 */
bool popc_combox_uds::Create(int, bool) {
    return false;
}

bool popc_combox_uds::Create(const char* address, bool server) {
    LOG_DEBUG_T("UDS", "Create %s", address);

    _is_server = server;

    // 1. Create a socket
    _socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (_socket_fd < 0) {
        LOG_ERROR_T("UDS", "socket() failed");
        return false;
    }

    LOG_DEBUG_T("UDS", "socket created");

    if (_is_server) {
        _timeout = -1;

        // If no address are provided, it is necessary to find one
        if (!address) {
            // note: This is ugly and probably wrong as well. This should be improved
            std::size_t i = 0;
            for (; i < 16384; ++i) {
                std::string str_address = ".uds_0." + std::to_string(i);

                // 2. Make sure the address is clear
                memset(&_sock_address, 0, sizeof(struct sockaddr_un));
                _sock_address.sun_family = AF_UNIX;
                strcpy(_sock_address.sun_path, str_address.c_str());

                if (!bind(_socket_fd, (struct sockaddr*)&_sock_address, sizeof(struct sockaddr_un))) {
                    _uds_address = str_address;
                    LOG_DEBUG_T("UDS", "Selected address: %s", _uds_address.c_str());
                    break;
                }
            }

            if (i >= 16384) {
                char* tmp = getenv("POPC_TEMP");

                std::string tmp_folder;
                if (tmp) {
                    tmp_folder = tmp;
                } else {
                    tmp_folder = "/tmp";
                }

                i = 0;
                for (; i < 16384; ++i) {
                    std::string str_address = tmp_folder + "/uds_0." + std::to_string(i);

                    // 2. Make sure the address is clear
                    memset(&_sock_address, 0, sizeof(struct sockaddr_un));
                    _sock_address.sun_family = AF_UNIX;
                    strcpy(_sock_address.sun_path, str_address.c_str());

                    if (!bind(_socket_fd, (struct sockaddr*)&_sock_address, sizeof(struct sockaddr_un))) {
                        _uds_address = str_address;
                        LOG_DEBUG_T("UDS", "Selected address: %s", _uds_address.c_str());
                        break;
                    }
                }

                if (i >= 16384) {
                    LOG_ERROR_T("UDS", "unable to find file for UDS socket");
                    return false;
                }
            }
        } else {
            _uds_address = address;

            if (_uds_address.find("uds://") == 0) {
                _uds_address = _uds_address.substr(6);
            }

            unlink(_uds_address.c_str());

            // 2. Make sure the address is clear
            memset(&_sock_address, 0, sizeof(struct sockaddr_un));
            _sock_address.sun_family = AF_UNIX;
            strcpy(_sock_address.sun_path, _uds_address.c_str());

            if (bind(_socket_fd, (struct sockaddr*)&_sock_address, sizeof(struct sockaddr_un)) != 0) {
                LOG_WARNING_T("UDS", "bind() failed");
                return false;
            }
        }

        LOG_DEBUG_T("UDS", "socket bound");

        if (listen(_socket_fd, 10) != 0) {
            LOG_WARNING_T("UDS", "listen() failed");
            return false;
        }

        LOG_DEBUG_T("UDS", "socket listened");

        add_fd_to_poll(_socket_fd);
    } else {
        std::string address_str(address);
        if (address_str.find("uds://") == 0) {
            address_str = address_str.substr(6);
        }

        _uds_address = address_str;

        // 2. Make sure the address is clear
        memset(&_sock_address, 0, sizeof(struct sockaddr_un));
        _sock_address.sun_family = AF_UNIX;
        strcpy(_sock_address.sun_path, _uds_address.c_str());
    }

    //_uds_address = address;

    return true;
}

/**
 * Connect to a server combox with its path
 * @param url Path to the file representing the socket to connect to.
 * @return TRUE if the connection is successful. FALSE in any other cases.
 */
bool popc_combox_uds::Connect(const char* param) {
    if (connect(_socket_fd, (struct sockaddr*)&_sock_address, sizeof(struct sockaddr_un)) != 0) {
        LOG_WARNING("Connect failed: %s", _uds_address.c_str());
        perror("Connect failed");
        return false;
    }

    LOG_DEBUG_T("UDS", "Connected to %s (param: %s)", _uds_address.c_str(), param);

    _connected = true;
    active_connection[0].fd = _socket_fd;
    active_connection[0].events = POLLIN;
    active_connection[0].revents = 0;
    _connection = new popc_connection_uds(_socket_fd, this);

    return true;
}

/**
 * Return the connection initialized with "Connect(const char* url)"
 * @return A pointer to the connection or nullptr if this combox has no connection
 */
pop_connection* popc_combox_uds::get_connection() {
    if (!_connected) {
        return nullptr;
    }

    return _connection;
}

/**
 * Send bytes to another combox without connection. NOT USED IN UDS COMBOX
 */
int popc_combox_uds::Send(const char*, int) {
    LOG_ERROR_T("UDS", "Send(const char*, int) should not be called in UDS mode");

    return 0;
}

/**
 * Send bytes to another combox represented by a connection.
 * @param s           Pointer to the bytes to be written.
 * @param len         Number of bytes to write.
 * @param connection  Connection representing the endpoint to write bytes.
 * @return Number of bytes sent
 */
int popc_combox_uds::Send(const char* s, int len, pop_connection* connection) {
    if (!connection) {
        LOG_ERROR_T("UDS", "Cannot send (connection == nullptr)");
        return -1;
    }

    if (len > 0) {
        std::string ver(s, len);
        LOG_DEBUG_T("UDS", "Write %d bytes (%s)", len, ver.c_str());

        int socket_fd = dynamic_cast<popc_connection_uds*>(connection)->get_fd();
        int wbytes = write(socket_fd, s, len);
        if (wbytes < 0) {
            perror("UDS Combox: Cannot write to socket");
        }

        return wbytes;
    } else {
        return 0;
    }
}

/**
 * Receive bytes from another combox without connection. NOT USED IN UDS COMBOX
 */
int popc_combox_uds::Recv(char*, int) {
    LOG_ERROR_T("UDS", "Recv(char*, int) should not be called in UDS mode");

    return 0;
}

/**
 * Receive bytes from another combox represented by a connection.
 * @param s           Pointer to the buffer where bytes must be written.
 * @param len         Number of bytes to read.
 * @param connection  Connection representing the endpoint from where to read bytes.
 * @return Number of bytes read
 */
int popc_combox_uds::Recv(char* s, int len, pop_connection* connection) {
    if (!connection) {
        connection = Wait();
        if (!connection) {
            LOG_ERROR("[CORE] Wait failed with code");
            return -1;
        }
    }

    int socket_fd = dynamic_cast<popc_connection_uds*>(connection)->get_fd();

    int nbytes;
    do {
        nbytes = read(socket_fd, s, len);
        if (nbytes < 0) {
            perror("Read");
        }
    } while (nbytes < 0);

    if (len > 0) {
        std::string ver(s, len);
        LOG_DEBUG_T("UDS", "Read %d bytes (%s)", len, ver.c_str());
    } else {
        LOG_DEBUG_T("UDS", "Read %d bytes", len);
    }

    return nbytes;
}

/**
 * Add a file descriptor to the poll fd array
 * @param fd  File descriptor to add.
 */
void popc_combox_uds::add_fd_to_poll(int fd) {
    active_connection[_active_connection_nb].fd = fd;
    active_connection[_active_connection_nb].events = POLLIN;
    active_connection[_active_connection_nb].revents = 0;
    _active_connection_nb++;
}

/**
 * Wait for a new connection or data from an existing connection
 * @return A pointer to the ready connection
 */
pop_connection* popc_combox_uds::Wait() {
    if (_is_server) {
        socklen_t address_length;
        address_length = sizeof(_sock_address);
        int poll_back;
        _timeout = timeout;
        do {
            poll_back = poll(active_connection, _active_connection_nb, _timeout);
            if (_active_connection_nb >= 199) {
                LOG_WARNING("TOO MANY CONNECTIONS");
            }
        } while ((poll_back == -1) && (errno == EINTR));
        LOG_DEBUG_T("UDS", "Poll %s", _uds_address.c_str());
        if (poll_back > 0) {
            for (int i = 0; i < _active_connection_nb; i++) {
                if (active_connection[i].revents & POLLIN) {
                    if (i == 0) {  // New connection
                        // A new connection can be received
                        int connection_fd;
                        connection_fd = accept(_socket_fd, (struct sockaddr*)&_sock_address, &address_length);
                        if (connection_fd < 0) {
                            perror("UDS Combox accept:");
                        } else {
                            add_fd_to_poll(connection_fd);
                            active_connection[i].revents = 0;
                            return new popc_connection_uds(connection_fd, this, true);
                        }
                    } else {
                        if (active_connection[i].revents & POLLHUP) {  // POLLIN and POLLHUP
                            LOG_DEBUG_T("UDS", "write and disconnect");
                            int tmpfd = active_connection[i].fd;
                            if (_active_connection_nb == 2) {
                                _active_connection_nb = 1;
                                active_connection[i].fd = 0;
                                active_connection[i].events = 0;
                                active_connection[i].revents = 0;
                            } else {
                                // Modify connection tab
                                _active_connection_nb--;
                                active_connection[i].fd = active_connection[_active_connection_nb].fd;
                                active_connection[i].events = active_connection[_active_connection_nb].events;
                                active_connection[i].revents = active_connection[_active_connection_nb].revents;
                            }
                            LOG_DEBUG_T("UDS", "POLLON %s %d", _uds_address.c_str(), active_connection[i].fd);
                            return new popc_connection_uds(tmpfd, this);
                        } else {  // Just POLLIN
                            LOG_DEBUG_T("UDS", "POLLIN %s %d", _uds_address.c_str(), active_connection[i].fd);
                            active_connection[i].revents = 0;
                            return new popc_connection_uds(active_connection[i].fd, this);
                        }
                    }
                } else if (active_connection[i].revents & POLLHUP) {
                    LOG_DEBUG_T("UDS", "%d fd is disconnected", active_connection[i].fd);
                    if (_active_connection_nb == 2) {
                        _active_connection_nb = 1;
                        active_connection[i].fd = 0;
                        active_connection[i].events = 0;
                        active_connection[i].revents = 0;
                        return nullptr;
                    } else {
                        // Modify connection tab
                        _active_connection_nb--;
                        active_connection[i].fd = active_connection[_active_connection_nb].fd;
                        active_connection[i].events = active_connection[_active_connection_nb].events;
                        active_connection[i].revents = active_connection[_active_connection_nb].revents;
                        return nullptr;
                    }
                }
            }
        } else if (poll_back == 0) {
            perror("combox: timeout");
            return nullptr;
        } else {
            perror("poll");
            return nullptr;
        }
    } else {
        int poll_back = poll(active_connection, 1, _timeout);
        if (poll_back > 0) {
            if (active_connection[0].revents & POLLIN) {
                return new popc_connection_uds(active_connection[0].fd, this);
            }
        } else if (poll_back == 0) {
            perror("timeout");
            return nullptr;
        } else {
            perror("poll");
            return nullptr;
        }
    }
    return nullptr;
}

/**
 * Close the combox
 * For server: Close all open connection to this combox
 * For client: Close its own connection
 */
void popc_combox_uds::Close() {
    if (_is_server) {
        LOG_DEBUG_T("UDS", "Close (server) %s", _uds_address.c_str());

        // Close all connections FD (socket_fd is already inside the list)
        for (std::size_t i = 0; i < std::size_t(_active_connection_nb); ++i) {
            if (close(active_connection[i].fd)) {
                LOG_WARNING("close failed: %s", _uds_address.c_str());
                perror("close failed");
            }
        }

        // Unlink the uds file
        if (unlink(_uds_address.c_str())) {
            LOG_WARNING("unlink failed: %s", _uds_address.c_str());
            perror("unlink failed");
        }
    } else {
        LOG_DEBUG_T("UDS", "Close (client) %s", _uds_address.c_str());

        if (close(_socket_fd)) {
            LOG_WARNING("close failed: %s", _uds_address.c_str());
            perror("close failed");
        }

        _connected = false;
    }
}

/**
 * Get the protocol name for this combox
 * @return the protocol name
 */
std::string popc_combox_uds::GetProtocol() {
    return UDS_PROTOCOL_NAME;
}

/**
 * Get the current url of this combox. Format of the url is uds://uds_rank.object_id
 * @param Reference to a string object to store the url.
 * @return FALSE if the combox has no url. TRUE otherwise.
 */
std::string popc_combox_uds::GetUrl() {
    return "uds://" + _uds_address;
}
