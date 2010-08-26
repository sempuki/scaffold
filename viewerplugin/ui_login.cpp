/* ui_login.hpp - simple login ui
*
* Jonne Nauha
*/
#include "stdheaders.hpp"
#include "application.hpp"
#include "session.hpp"
#include "viewerplugin/logic.hpp"
#include "viewerplugin/ui_login.hpp"

#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMessageBox>
#include <QFont>

namespace ViewerPlugin
{
    LoginWidget::LoginWidget (QWidget *parent) :
        QWidget (parent)
    {
        initialize ();
    }

    QMap <QString, QString> LoginWidget::read_config ()
    {
        QMap<QString, QString> ui_values;
        QSettings login_config(QSettings::IniFormat, QSettings::UserScope, "scaffold", "config/login");

        if (login_config.childGroups().count() == 0)
        {
            login_config.beginGroup("avatar");
            login_config.setValue("names", "");
            login_config.setValue("password", "");
            login_config.endGroup();
            login_config.beginGroup("world");
            login_config.setValue("host", "http://localhost:8002");
            login_config.sync();

            cout << "initialised login credentials config file" << endl;
            ui_values["names"] = "";
            ui_values["password"] = "";
            ui_values["host"] = "http://localhost:8002";
            return ui_values;
        }

        ui_values["names"] = login_config.value("avatar/names").toString();
        ui_values["password"] = login_config.value("avatar/password").toString();
        ui_values["host"] = login_config.value("world/host").toString();

        cout << "read login credentials from config" << endl;
        return ui_values;
    }

    void LoginWidget::write_config ()
    {
        QSettings login_config(QSettings::IniFormat, QSettings::UserScope, "scaffold", "config/login");
        login_config.beginGroup("avatar");
        login_config.setValue("names", edit_name_->text());
        login_config.setValue("password", edit_pwd_->text());
        login_config.endGroup();
        login_config.beginGroup("world");
        login_config.setValue("host", edit_host_->text());
        login_config.sync();

        cout << "wrote credentials to config" << endl;
    }

    void LoginWidget::initialize ()
    {
        QMap<QString, QString> ui_values = read_config();

        QVBoxLayout *main_layout = new QVBoxLayout();

        // Input elements
        QGridLayout *input_layout = new QGridLayout();

        QLabel *label_name = new QLabel("First Last");
        QLabel *label_pwd = new QLabel("Password");
        QLabel *label_host = new QLabel("Host");

        edit_name_ = new QLineEdit(ui_values["names"]);
        edit_pwd_ = new QLineEdit(ui_values["password"]);
        edit_host_ = new QLineEdit(ui_values["host"]);
        edit_pwd_->setEchoMode(QLineEdit::Password);

        input_layout->addWidget(label_name, 0, 0);
        input_layout->addWidget(edit_name_, 0, 1);
        input_layout->addWidget(label_pwd, 1, 0);
        input_layout->addWidget(edit_pwd_, 1, 1);
        input_layout->addWidget(label_host, 2, 0);
        input_layout->addWidget(edit_host_, 2, 1);

        // Controls and info
        label_status_ = new QLabel("Offline");
        label_status_->setAlignment(Qt::AlignCenter);
        label_status_->setFont(QFont("Arial", 14));

        QPushButton *button_login = new QPushButton("Login");
        QPushButton *button_exit = new QPushButton("Exit");
        connect(button_login, SIGNAL(clicked()), this, SLOT(get_login_parameters()));
        connect(button_exit, SIGNAL(clicked()), this, SIGNAL(exit()));

        QHBoxLayout *button_layout = new QHBoxLayout();
        button_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
        button_layout->addWidget(button_login);
        button_layout->addWidget(button_exit);

        // Setup
        main_layout->addWidget(label_status_);
        main_layout->addLayout(input_layout);
        main_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
        main_layout->addLayout(button_layout);

        setLayout(main_layout);
    }

    void LoginWidget::get_login_parameters ()
    {
        QStringList names = edit_name_->text().split(" ");;
        QString pwd = edit_pwd_->text();
        QString host = edit_host_->text();

        if (names.count() != 2 )
        {
            QMessageBox::information(this, "Missing values", "Name cannot be empty and must be like <First Last>");
            return;
        }

        else if (host.isEmpty())
        {
            QMessageBox::information(this, "Missing values", "Host cannot be empty");
            return;
        }

        Connectivity::LoginParameters params;
        params.insert ("first", names.at(0));
        params.insert ("last", names.at(1));
        params.insert ("pass", pwd);
        params.insert ("service", host);

        set_status("Connecting...");

        emit start_login (params);
    }

    void LoginWidget::set_connected (bool connected)
    {
        QString status;
        if (connected)
        {
            status = "Connected to " + edit_host_->text();
            write_config();
        }
        else
            status = "Offline";
        label_status_->setText(status);
    }

    void LoginWidget::set_status (const QString &status)
    {
        label_status_->setText(status);
    }
}
