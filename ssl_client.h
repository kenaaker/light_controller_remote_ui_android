/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SSLCLIENT_H
#define SSLCLIENT_H

#include <QtWidgets/QWidget>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslSocket>
#include <QTimer>
#include "transport_record.h"

class SslClient : public QObject {
    Q_OBJECT
public:
    SslClient(QWidget *parent = 0);
    ~SslClient();
    void set_server_ip(QString &host_name_or_ip) {
        /* Add the "scope" to the host_name or IP address if the address is a IPV6 link local address. */
        if (host_name_or_ip.startsWith("fe80::", Qt::CaseSensitivity::CaseInsensitive)) {
            light_controller_host_name_or_ip = host_name_or_ip + "%br0";
        } else {
            light_controller_host_name_or_ip = host_name_or_ip;
        }
    }
    QString &get_server_ip(void) {
        return light_controller_host_name_or_ip;
    }
    bool is_server_ip_set(void) {
        return !light_controller_host_name_or_ip.isEmpty();
    }
    bool is_socket_encrypted(void) {
        return ((socket->mode() == QSslSocket::SslClientMode) ||
                (socket->mode() == QSslSocket::SslClientMode));
    }
    void set_server_port(unsigned short port) {
        light_controller_port = port;
    }
    QAbstractSocket::SocketState get_socket_state(void) {
        QAbstractSocket::SocketState ret_value;

        if (socket != nullptr) {
            ret_value = socket->state();
        } else {
            ret_value = QAbstractSocket::UnconnectedState;
        }
        return ret_value;
    }
    light_controller_object::light_controller_values_t get_remote_data(void);
    void setup(void);
    bool is_connected(void) {
        bool ret_value;
        QAbstractSocket::SocketState working_state;

        working_state = get_socket_state();
        if (working_state == QAbstractSocket::UnconnectedState) {
            ret_value = false;
        } else {
            ret_value = true;
        }
        return ret_value;
    }
    void send_remote_values(command_opcode_enum_t remote_opcode,
                            light_controller_object::light_controller_values_t *remote_values);

signals:
    void socket_up();
    void socket_down();
    void remote_data_ready(light_controller_object::light_controller_values_t light_remote_data);

public slots:
    void slot_secure_connect(void);
    void slot_secure_disconnect();

private slots:
    void slot_state_changed(QAbstractSocket::SocketState state);
    void slot_encrypted();
    void slot_mode_changed(QSslSocket::SslMode mode);
    void slot_ready_read();
    void slot_disconnected();
    void slot_ssl_errors(const QList<QSslError> &errors);
    void peer_verify_ssl_error(const QSslError &error);

private:
    static void remote_data_handler(const light_controller_remote_hdr_t *,
                                    const void *var_part,
                                    const void *closure);
    QString light_controller_host_name_or_ip;
    unsigned short light_controller_port;
    QSslSocket *socket;
    light_controller_remote_transport_struct_t light_controller_transported_object_data;
    var_st_xport<light_controller_remote_hdr_t, 'L', 4,
                 sizeof(light_controller_remote_transport_body_t),
                 light_controller_remote_transport_body_t> light_remote_data;
};

#endif
