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

#include "ssl_client.h"
#include "transport_record.h"

#include <QCoreApplication>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QIODeviceBase>
#include <QAbstractSocket>
#include <QTimer>

#include "hex_dump.h"

SslClient::SslClient(QWidget *parent) : QObject(parent), socket(nullptr) {
    light_remote_data.set_var_st_handler(remote_data_handler, this);
}

SslClient::~SslClient() {}

void SslClient::remote_data_handler(const light_controller_remote_hdr_t *,
                                    const void *var_part,
                                    const void *closure) {
    SslClient *this_ssl_connection = (SslClient *)closure;

    light_controller_remote_transport_body_t const *remote_transport_struct =
        (light_controller_remote_transport_body_t const *)var_part;
    light_controller_object::light_controller_values_t remote_ui_light_controller_object_values;

    remote_ui_light_controller_object_values.dimmer_setting = remote_transport_struct->remote_values.dimmer_setting;
    remote_ui_light_controller_object_values.light_intensity = remote_transport_struct->remote_values.light_intensity;
    remote_ui_light_controller_object_values.power_supply_voltage = remote_transport_struct->remote_values.power_supply_voltage;
    remote_ui_light_controller_object_values.physical_light_state = remote_transport_struct->remote_values.physical_light_state;
    remote_ui_light_controller_object_values.user_set_light_state = remote_transport_struct->remote_values.user_set_light_state;
    remote_ui_light_controller_object_values.PIR_detected_presence = remote_transport_struct->remote_values.PIR_detected_presence;

    emit this_ssl_connection->remote_data_ready(remote_ui_light_controller_object_values);
}

void SslClient::setup() {
    if (!socket) {
        socket = new QSslSocket(this);
    } /* endif */
}

void SslClient::slot_mode_changed(QSslSocket::SslMode mode) {
    if (mode == QSslSocket::SslClientMode) {
        qDebug("SslClient::slot_mode_changed: To SslClientMode.");
    } else if (mode == QSslSocket::SslServerMode) {
        qDebug("SslClient::slot_mode_changed: To SslServerMode.");
    } else {
        qDebug("SslClient::slot_mode_changed to Unknown mode (%d)???", mode);
    }

}

void SslClient::slot_secure_connect(void) {
    if (!is_connected()) {
        QSslConfiguration client_server_configuration = QSslConfiguration::defaultConfiguration();
        /* Add server certificates and CA (from minica... from ~/ssl_stuff/minica/aaker.org */
        client_server_configuration.setCaCertificates(
            QSslCertificate::fromPath(QStringLiteral(":/light_controller_CA.pem")));
        QSslConfiguration::setDefaultConfiguration(client_server_configuration);
        socket->setLocalCertificate(":/light_controller_client.pem");
        socket->setPrivateKey(":/light_controller_client.key", QSsl::Rsa, QSsl::Pem, "var6look");

        connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
                SLOT(slot_state_changed(QAbstractSocket::SocketState)));
        connect(socket, SIGNAL(encrypted()), this, SLOT(slot_encrypted()));
        connect(socket, &QSslSocket::modeChanged, this, &SslClient::slot_mode_changed);
//        connect(socket, SIGNAL(modeChanged(QSslSocket::SslMode newMode)), this,
//                SLOT(slot_mode_changed(QSslSocket::SslMode newMode)));
        connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slot_ssl_errors(QList<QSslError>)));
        connect(socket, SIGNAL(peerVerifyError(QSslError)), this, SLOT(peer_verify_ssl_error(QSslError)));
        connect(socket, &QSslSocket::readyRead, this, &SslClient::slot_ready_read);
//        connect(socket, SIGNAL(readyRead()), this, SLOT(slot_ready_read()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
//        connect(&reconnect_timer, SIGNAL(timeout()), this, SLOT(secure_connect()));

        QList<QSslCertificate> light_controller_cert = QSslCertificate::fromPath(":/light_controller_server.pem");
        QSslError self_signed_error(QSslError::SelfSignedCertificate, light_controller_cert.at(0));
        QSslError host_name_mismatch(QSslError::HostNameMismatch, light_controller_cert.at(0));
        QList<QSslError> expected_ssl_errors;
        expected_ssl_errors.append(self_signed_error);
        expected_ssl_errors.append(host_name_mismatch);
        socket->ignoreSslErrors(expected_ssl_errors);
        socket->setPauseMode(QAbstractSocket::PauseNever);
    } /* endif */
    socket->connectToHostEncrypted(light_controller_host_name_or_ip,
                                   light_controller_port,
                                   QIODevice::ReadWrite,
                                   QAbstractSocket::IPv6Protocol);
}

void SslClient::slot_secure_disconnect() {
    if (socket != nullptr) {
        socket->disconnectFromHost();
        delete socket;
        socket = nullptr;
    }
}

void SslClient::slot_state_changed(QAbstractSocket::SocketState state) {
    if (state == QAbstractSocket::UnconnectedState) {
        emit socket_down();
        socket->deleteLater();
//        socket = nullptr;
//        reconnect_timer.start(2000); /* Try to reconnect every two seconds */
    } else if (state == QAbstractSocket::ConnectedState) {
        emit socket_up();
//        reconnect_timer.stop();
    } else {
    } /* endif */
}

void SslClient::slot_encrypted() {
    if (socket != nullptr) {
        qDebug() << __func__ << ":" << __LINE__ << "socket is encrypted";
        return;
    } /* endif */
}

void SslClient::slot_ready_read() {
    int remote_bytes_ready = socket->bytesAvailable();

    qDebug() << __func__ << ":" << __LINE__ << "Got Ready Read signal.";
    qDebug() << __func__ << ":" << __LINE__ << "remote_bytes_ready =" << remote_bytes_ready;
    light_remote_data.add_bytes(socket->readAll().data(), remote_bytes_ready);
}

light_controller_object::light_controller_values_t SslClient::get_remote_data(void) {
    light_controller_object::light_controller_values_t remote_data_values;
    transported_light_controller_values_t transported_remote_values;

    transported_remote_values =
        light_controller_transported_object_data.light_controller_remote_transport_body.remote_values;
    remote_data_values.dimmer_setting = transported_remote_values.dimmer_setting;
    remote_data_values.light_intensity = transported_remote_values.light_intensity;
    remote_data_values.power_supply_voltage = transported_remote_values.power_supply_voltage;
    remote_data_values.physical_light_state = transported_remote_values.physical_light_state;
    remote_data_values.user_set_light_state = transported_remote_values.user_set_light_state;
    remote_data_values.PIR_detected_presence = transported_remote_values.PIR_detected_presence;
    return remote_data_values;
}

void SslClient::slot_disconnected() {
    QCoreApplication::quit(); /* Stop the whole application, let a shell loop restart it if required. */
}

void SslClient::send_remote_values(command_opcode_enum_t remote_opcode, light_controller_object::light_controller_values_t *remote_values) {
    light_controller_remote_transport_struct_t remote_values_with_header;

//    if (remote_opcode == command_opcode_get_light_state) {
//        hex_dump(__func__, __LINE__, "remote values on entry to function", remote_values, sizeof(*remote_values));
//    }
    remote_values_with_header.light_controller_remote_transport_body.remote_values.dimmer_setting = remote_values->dimmer_setting;
    remote_values_with_header.light_controller_remote_transport_body.remote_values.light_intensity = remote_values->light_intensity;
    remote_values_with_header.light_controller_remote_transport_body.remote_values.power_supply_voltage = remote_values->power_supply_voltage;
    remote_values_with_header.light_controller_remote_transport_body.remote_values.physical_light_state = remote_values->physical_light_state;
    remote_values_with_header.light_controller_remote_transport_body.remote_values.user_set_light_state = remote_values->user_set_light_state;
    remote_values_with_header.light_controller_remote_transport_body.remote_values.PIR_detected_presence = remote_values->PIR_detected_presence;
    remote_values_with_header.light_controller_remote_transport_body.command_opcode = remote_opcode;
//    if (remote_opcode == command_opcode_get_light_state) {
//        hex_dump(__func__, __LINE__, "remote values with header (after setup)", &remote_values_with_header, sizeof(remote_values_with_header));
//    }

    if (socket->mode() == QSslSocket::SslClientMode) {
        qDebug() << __func__ << ":" << __LINE__ << "SslClientMode is active";
        socket->write((char *)&remote_values_with_header, sizeof(remote_values_with_header));
        qDebug() << __func__ << ":" << __LINE__ << "Wrote remote_values.";
        socket->flush();
//        hex_dump(__func__, __LINE__, "remote values sent from server",
//                 &remote_values_with_header, sizeof(remote_values_with_header));
    }
}

void SslClient::slot_ssl_errors(const QList<QSslError> &errors) {
    foreach (const QSslError &error, errors) {
        qDebug() << error.errorString();
    } /* endfor */

    socket->ignoreSslErrors();

    // did the socket state change?
    if (socket->state() != QAbstractSocket::ConnectedState) {
        slot_state_changed(socket->state());
    } /* endif */
}

void SslClient::peer_verify_ssl_error(const QSslError &error) {
    qDebug() << error.errorString();

    // did the socket state change?
    if (socket->state() != QAbstractSocket::ConnectedState) {
        slot_state_changed(socket->state());
    } /* endif */
}

#if 0
void SslClient::pre_shared_key_authentication_required(QSslPreSharedKeyAuthenticator *authenticator) {
    qDebug() << "Need pre shared key authentication.";

}
#endif
