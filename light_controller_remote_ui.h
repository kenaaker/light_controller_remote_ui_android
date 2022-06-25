#ifndef LIGHT_CONTROLLER_REMOTE_UI_H
#define LIGHT_CONTROLLER_REMOTE_UI_H

#include <QMainWindow>
#include <QApplication>
#include <ssl_client.h>
#include "qzeroconf.h"
#include "transport_record.h"
#include "light_controller_object.h"

namespace Ui {
class light_controller_remote_ui;
}

class light_controller_remote_ui : public QMainWindow {
    Q_OBJECT

public:
    explicit light_controller_remote_ui(QWidget *parent = 0);
    ~light_controller_remote_ui();
public slots:
    void socket_up();
    void socket_down();
    void exit_button_pressed();
    void read_remote_light_controller();

private slots:
    void toggle_on_off_clicked(bool);
    void light_controller_dimmer_set(int value);
    void on_reset_to_default_clicked();
    void remote_data_update(light_controller_object::light_controller_values_t remote_values);
    void addService(const QZeroConfService &);
    void updateService(const QZeroConfService &);
    void removeService(const QZeroConfService &);

private:
    Ui::light_controller_remote_ui *ui;
    SslClient light_controller_connection;
    QZeroConf *light_controller_zero_conf;
    QTimer *light_state_poll;
    light_controller_object::light_controller_values_t remote_ui_light_controller_object_values;
};

#endif // LIGHT_CONTROLLER_REMOTE_UI_H
