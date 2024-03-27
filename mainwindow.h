//
// Created by xtx on 2023/5/30.
//

#ifndef SOFTWAREUPDATER_MAINWINDOW_H
#define SOFTWAREUPDATER_MAINWINDOW_H

#include "ConfigClass.h"
#include "DownloadTool.h"
#include <QNetworkAccessManager>
#include <QWidget>
#include <qdebug.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class mainWindow;
}
QT_END_NAMESPACE

class mainWindow : public QWidget {
 Q_OBJECT

 public:
    explicit mainWindow(QWidget *bytes_read = nullptr);
    ~mainWindow() override;

 private:
    Ui::mainWindow *ui;
    QNetworkAccessManager *manager;//定义网络请求对象
    int parse_UpdateJSON(QString str); //解析数据函数的声明
    void replyFinished(QNetworkReply *reply); //网络数据接收完成槽函数的声明
    QString current_version_ = "", update_json_url_ = ""; //定义当前软件的版本号
    QString update_url_;
    ConfigClass *config_main_ini_;
    DownloadTool* dt_;
    QString software_name_;
    QString latest_version_;
};

#endif//SOFTWAREUPDATER_MAINWINDOW_H
