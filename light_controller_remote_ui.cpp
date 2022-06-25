
#include "light_controller_remote_ui.h"
#include "ui_light_controller_remote_ui.h"
#include "ssl_client.h"
#include "hex_dump.h"

light_controller_remote_ui::light_controller_remote_ui(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::light_controller_remote_ui) {
    ui->setupUi(this);
    light_controller_connection.setup();
    connect(&light_controller_connection, SIGNAL(socket_up()), this, SLOT(socket_up()));
    connect(&light_controller_connection, SIGNAL(socket_down()), this, SLOT(socket_down()));
    connect(&light_controller_connection, &SslClient::remote_data_ready, this, &light_controller_remote_ui::remote_data_update);

    light_controller_zero_conf = new QZeroConf(this);

    connect(light_controller_zero_conf, &QZeroConf::serviceAdded, this, &light_controller_remote_ui::addService);
    connect(light_controller_zero_conf, &QZeroConf::serviceRemoved, this, &light_controller_remote_ui::removeService);
    connect(light_controller_zero_conf, &QZeroConf::serviceUpdated, this, &light_controller_remote_ui::updateService);
    light_controller_zero_conf->startBrowser("_light_controllerui._tcp", QAbstractSocket::IPv6Protocol);

    connect(ui->exit_button, &QPushButton::pressed, this, &light_controller_remote_ui::exit_button_pressed);
    connect(ui->dimmer_slider, &QSlider::valueChanged, this, &light_controller_remote_ui::light_controller_dimmer_set);
    connect(ui->toggle_on_off, &QPushButton::toggled,
            this, &light_controller_remote_ui::toggle_on_off_clicked);
    light_state_poll = new QTimer(this);
    QObject::connect(light_state_poll, &QTimer::timeout,
                     this, &light_controller_remote_ui::read_remote_light_controller);

    light_state_poll->start(1000);

}

light_controller_remote_ui::~light_controller_remote_ui() {
    delete ui;
    delete light_controller_zero_conf;
}

void light_controller_remote_ui::socket_up() {
    ui->light_controller_controls->setEnabled(true);
}

void light_controller_remote_ui::socket_down() {
    ui->light_controller_controls->setEnabled(false);
}

void light_controller_remote_ui::remote_data_update(light_controller_object::light_controller_values_t remote_values) {
    ui->presence_detected->setEnabled(remote_values.PIR_detected_presence);
    //ui->dimmer_slider->setValue(remote_values.dimmer_setting);
    if (remote_values.physical_light_state == light_on) {
        ui->light_state->setEnabled(true);
        ui->light_state->setText("Light is On");
    } else if (remote_values.physical_light_state == light_off) {
        ui->light_state->setEnabled(true);
        ui->light_state->setText("Light is Off");
    } else {
        ui->light_state->setText("Don't Know what light state is.");
        ui->light_state->update();
        ui->light_state->setEnabled(false);
    }

    ui->light_intensity->setNum((int)remote_values.light_intensity);

    ui->power_supply_voltage->setNum((int)remote_values.power_supply_voltage);
}


void light_controller_remote_ui::toggle_on_off_clicked(bool) {
    light_controller_object::light_controller_values_t values_to_send;

    values_to_send = remote_ui_light_controller_object_values;
    light_controller_connection.send_remote_values(command_opcode_toggle_on_off, &values_to_send);
}

void light_controller_remote_ui::exit_button_pressed() {
    qApp->quit();
}

void light_controller_remote_ui::read_remote_light_controller(void) {
    light_controller_object::light_controller_values_t values_to_send = { 0, 0, 0, undefined_light_state, undefined_light_state, false };

    /* This sends zero values for everything to the remote, since nothing on the remote */
    /* processes any of the values sent when the opcode is "command_opcode_get_light_state. */
    light_controller_connection.send_remote_values(command_opcode_get_light_state, &values_to_send);
}

void light_controller_remote_ui::light_controller_dimmer_set(int value) {
    light_controller_object::light_controller_values_t values_to_send;

    values_to_send = remote_ui_light_controller_object_values;
    values_to_send.dimmer_setting = value;
    light_controller_connection.send_remote_values(command_opcode_dimmer_setting, &values_to_send);
}

void light_controller_remote_ui::on_reset_to_default_clicked() {
    light_controller_object::light_controller_values_t values_to_send;

    values_to_send = remote_ui_light_controller_object_values;
    light_controller_connection.send_remote_values(command_opcode_reset_to_default, &values_to_send);
}

// ---------- Discovery Callbacks ----------

void light_controller_remote_ui::addService(const QZeroConfService &zcs) {
    QString controller_ip = zcs->ip().toString();

    qDebug() << "Added service: " << zcs;
    qDebug() << "Added service name = " << zcs->name();
    qDebug() << "Added service ip = " << controller_ip;
    light_controller_connection.set_server_ip(controller_ip);
    light_controller_connection.set_server_port(zcs->port());
    qDebug() << "Added service port = " << zcs->port();
    if (!light_controller_connection.is_connected()) {
        light_controller_connection.slot_secure_connect();
    }
}

void light_controller_remote_ui::removeService(const QZeroConfService &zcs) {
    qDebug() << "Service removed: " << zcs;
    //light_controller_connection.secure_disconnect();
}

void light_controller_remote_ui::updateService(const QZeroConfService &zcs) {
    QString controller_ip = zcs->ip().toString();

    qDebug() << "Service updated: " << zcs;
    qDebug() << "Updated service name = " << zcs->name();
    qDebug() << "Updated service ip = " << zcs->ip();
    light_controller_connection.set_server_ip(controller_ip);
    light_controller_connection.set_server_port(zcs->port());
    qDebug() << "Updated service port = " << zcs->port();
    if (!light_controller_connection.is_connected()) {
        light_controller_connection.slot_secure_connect();
    }
}
