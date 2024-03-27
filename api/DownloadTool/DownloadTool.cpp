//
// Created by xtx on 2023/6/4.
//

#include "DownloadTool.h"

DownloadTool::DownloadTool(const QString &download_url,
                           const QString &software_name,
                           const QString &save_path,
                           QObject *parent)
    : QObject(parent) {
    m_downloadUrl = download_url;
    m_savePath = save_path;
    m_fileName = software_name+".exe";
}

DownloadTool::~DownloadTool() {}

void DownloadTool::StartDownload() {
    const QUrl newUrl = QUrl::fromUserInput(m_downloadUrl);

    if (!newUrl.isValid()) {
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Invalid URL: %1: %2").arg(m_downloadUrl, newUrl.errorString());
#endif // DOWNLOAD_DEBUG
        return;
    }

//    QString fileName = newUrl.fileName();
//
//    if (fileName.isEmpty()) fileName = defaultFileName;
    if (m_savePath.isEmpty()) { m_savePath = QApplication::applicationDirPath() + "/TEMP"; }
    if (!QFileInfo(m_savePath).isDir()) {
        QDir dir;
        dir.mkpath(m_savePath);
    }

    m_fileName.prepend(m_savePath + '/');
    if (QFile::exists(m_fileName)) { QFile::remove(m_fileName); }
    file = openFileForWrite(m_fileName);
    if (!file) { return; }

    startRequest(newUrl);
}

[[maybe_unused]] void DownloadTool::cancelDownload() {
    httpRequestAborted = true;
    reply->abort();
}

void DownloadTool::HttpFinished() {
    QFileInfo fi;
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }

    if (httpRequestAborted) {
        return;
    }

    if (reply->error()) {
        QFile::remove(fi.absoluteFilePath());
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Download failed: %1.").arg(reply->errorString());
#endif // DOWNLOAD_DEBUG
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());
        file = openFileForWrite(fi.absoluteFilePath());
        if (!file) { return; }
        startRequest(redirectedUrl);
        return;
    }

    Q_EMIT sigDownloadFinished();

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloaded %1 bytes to %2 in %3")
                            .arg(fi.size()).arg(fi.fileName(), QDir::toNativeSeparators(fi.absolutePath())));
    qDebug() << "Finished";
#endif // DOWNLOAD_DEBUG
}

void DownloadTool::HttpReadyRead() {
    if (file) { file->write(reply->readAll()); }
}

void DownloadTool::NetworkReplyProgress(qint64 bytes_read, qint64 total_bytes) {
    qreal progress = qreal(bytes_read) / qreal(total_bytes);
    Q_EMIT sigProgress(bytes_read, total_bytes, progress);

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString::number(progress * 100, 'f', 2) << "%    "
             << bytes_read / (1024 * 1024) << "MB" << "/" << total_bytes / (1024 * 1024) << "MB";
#endif // DOWNLOAD_DEBUG
}

void DownloadTool::startRequest(const QUrl &requestedUrl) {
    url = requestedUrl;
    httpRequestAborted = false;

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &DownloadTool::HttpFinished);
    connect(reply, &QIODevice::readyRead, this, &DownloadTool::HttpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadTool::NetworkReplyProgress);

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloading %1...").arg(url.toString()));
#endif // DOWNLOAD_DEBUG
}

std::unique_ptr<QFile> DownloadTool::openFileForWrite(const QString &fileName) {
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Unable to save the file %1: %2.")
            .arg(QDir::toNativeSeparators(fileName), file->errorString());
#endif // DOWNLOAD_DEBUG
        return nullptr;
    }
    return file;
}