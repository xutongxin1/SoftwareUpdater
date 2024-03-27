//
// Created by xtx on 2023/5/30.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainWindow.h" resolved

#include <QJsonParseError>
#include <QJsonObject>
#include <QMessageBox>
#include <QFile>
#include "mainwindow.h"
#include "ui_mainWindow.h"
#include "QNetworkReply"
#include "DownloadTool.h"

mainWindow::mainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::mainWindow) {
    ui->setupUi(this);

    this->setWindowTitle("WirelessMonitor更新器 V1.0");
    manager = new QNetworkAccessManager(this);
    config_main_ini_ = new ConfigClass("main.ini", QSettings::IniFormat);
    current_version_ = config_main_ini_->value("/Device/Version").toString();
    ui->nowVersion->setText("目前版本：" + current_version_);
    update_json_url_ = config_main_ini_->value("/Device/UpdateJsonURL").toString();
    ui->StartUpdate->setEnabled(false);
    ui->progressBar->hide();
    connect(manager, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply) {
              QString str = reply->readAll();//读取接收到的数据

              QJsonParseError err_rpt;
              QJsonDocument root_doc = QJsonDocument::fromJson(str.toUtf8(), &err_rpt);//字符串格式化为JSON
              if (err_rpt.error != QJsonParseError::NoError) {
//        qDebug() << "root格式错误";
                  QMessageBox::critical(this, "检查失败", "服务器地址错误或JSON格式错误!");
              }
              if (root_doc.isObject()) {
                  QJsonObject root_obj = root_doc.object();   //创建JSON对象，不是字符串
                  latest_version_ = root_obj.value("LatestVersion").toString();  //V1.0
                  if (latest_version_ > current_version_) {
                      software_name_ = root_obj.value("SoftwareName").toString();
                      QString cdn = root_obj.value("CDN").toString();
                      update_url_ =
                          cdn + "https://raw.githubusercontent.com/" +
                              root_obj.value("Repositories").toString() + "/"
                              + root_obj.value("SoftwareName").toString() + "_"
                              + root_obj.value("LatestVersion").toString() + ".exe";
                      QString update_time = root_obj.value("UpdateTime").toString();
                      QString release_note = root_obj.value("ReleaseNote").toString();

                      ui->updateInfo->setText(
                          "检测到新版本!\n版本号：" + latest_version_ + "\n" + "更新时间：" + update_time + "\n" + "更新说明："
                              + release_note);
                      ui->StartUpdate->setEnabled(true);
                  } else {
                      ui->updateInfo->setText("当前已经是最新版本!");
                      ui->StartUpdate->setEnabled(false);
                  }
                  ui->Check->setEnabled(true);
              }

              reply->deleteLater();               //销毁请求对象
            }
    );
    connect(ui->Check, &QPushButton::clicked, this, [&] {
      ui->Check->setEnabled(false);
      ui->StartUpdate->setEnabled(false);
      if (update_json_url_ != "") {
          QNetworkRequest quest;
          quest.setUrl(QUrl(update_json_url_)); //包含最新版本软件的下载地址
          manager->get(quest);    //发送get网络请求
      }
    });
    connect(ui->StartUpdate, &QPushButton::clicked, this, [&] {
              ui->Check->setEnabled(false);
              ui->StartUpdate->setEnabled(false);
              if (update_url_ != "") {
                  ui->progressBar->show();
                  dt_ = new DownloadTool(update_url_, software_name_);
                  connect(dt_, &DownloadTool::sigProgress, this, [&](qint64 bytes_read, qint64 total_bytes, qreal progress) {
                    ui->progressBar->setValue((int) (progress * 100));
                  });
                  connect(dt_, &DownloadTool::sigDownloadFinished, this, [&] {
                            disconnect(dt_, &DownloadTool::sigDownloadFinished, this, nullptr);
                            disconnect(dt_, &DownloadTool::sigProgress, this, nullptr);

                            if (QFile::exists("./" + software_name_ + ".exe")) {
                                bool is_success = QFile::remove("./" + software_name_ + ".exe");
                                if (!is_success) {
                                    QMessageBox::warning(this, "提示", "请先关闭" + software_name_ + "程序再开始更新");
                                }
                            }
                            if (!QFile::exists("./" + software_name_ + ".exe")) {
                                if (!QFile::copy("./TEMP/" + software_name_ + ".exe", "./" + software_name_ + ".exe")) {
                                    qDebug("file move failed");
                                }
                                config_main_ini_->setValue("/Device/Version", latest_version_);
                                current_version_ = latest_version_;
                                qDebug("Finish");
                                QMessageBox::information(this, "提示", "更新完成");
                            }
                            ui->Check->setEnabled(true);
                            ui->StartUpdate->setEnabled(true);

                          }
                  );
                  dt_->StartDownload();
              }
            }
    );
}

mainWindow::~mainWindow() {
    delete ui;
}
