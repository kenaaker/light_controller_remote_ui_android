#include <QApplication>
#include <QMessageBox>
#include "light_controller_remote_ui.h"

int main(int argc, char *argv[]) {

    Q_INIT_RESOURCE(light_controller_remote_ui);

    QApplication a(argc, argv);

    if (!QSslSocket::supportsSsl()) {
        QMessageBox::information(0, "Secure Socket Client",
                                 "This system does not support OpenSSL.");
        return -1;
    } else {
        light_controller_remote_ui w;
        w.show();

        return a.exec();
    } /* endif */
}
