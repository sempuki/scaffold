/* ui_login.hpp - simple login ui
*
* Jonne Nauha
*/

#ifndef UI_LOGIN_H_
#define UI_LOGIN_H_

#include <QWidget>
#include <QMap>
#include <QString>
#include <QLineEdit>
#include <QLabel>

namespace ViewerPlugin
{
    class LoginWidget : public QWidget
    {
        Q_OBJECT

        public:
            LoginWidget (QWidget *parent = 0);

        signals:
            void login (QMap<QString,QString>);
            void exit ();

        public slots:
            void set_connected (bool connected);
            void set_status (const QString &status);

        private slots:
            void get_login_parameters ();

        private:
            void initialize ();
            void write_config ();
            QMap <QString, QString> read_config ();

        private:
            QLineEdit   *edit_name_;
            QLineEdit   *edit_pwd_;
            QLineEdit   *edit_host_;
            QLabel      *label_status_;
    };
}

#endif
