// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Util/Utils.h"
#include "devicedrivercore.h"

#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQmlContext>

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // TODO see other comment about paths
    QFile file(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../logs/app.log")));

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Could not open log file for writing";
        return;
    }

    QTextStream out(&file);

    QString timestamp = QDateTime::currentDateTime().toString(
        QStringLiteral("yyyy-MM-dd hh:mm:ss:zzz"));
    QString fileName = QFileInfo(QString::fromUtf8(context.file)).fileName();

    // Log type as a string
    QString logType;
    switch (type) {
    case QtDebugMsg:
        logType = QStringLiteral("DEBUG");
        break;
    case QtInfoMsg:
        logType = QStringLiteral("INFO");
        break;
    case QtWarningMsg:
        logType = QStringLiteral("WARNING");
        break;
    case QtCriticalMsg:
        logType = QStringLiteral("CRITICAL");
        break;
    case QtFatalMsg:
        logType = QStringLiteral("FATAL");
        break;
    }

    // Format the log message
    out << QStringLiteral("[%1] [%2] (%3:%4) %5")
               .arg(timestamp)
               .arg(logType)
               .arg(fileName)
               .arg(context.line)
               .arg(msg)
        << Qt::endl;

    // Also write to the console
    // TODO make this optional
    QTextStream console(stdout);
    console << QStringLiteral("[%1] [%2] (%3:%4) %5")
                   .arg(timestamp)
                   .arg(logType)
                   .arg(fileName)
                   .arg(context.line)
                   .arg(msg)
            << Qt::endl;
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    DeviceDriverCore core;

// TODO let the user set those paths. Do not hardcode them depending on the build path
#ifdef WASM_BUILD
    bool isWasm = true;
    core.setNodeSetPath(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("/data")));
    core.setMustacheTemplatePath(QDir(QCoreApplication::applicationDirPath())
                                     .filePath(QStringLiteral("/templates/open62541.mustache")));
    core.setCmakeMustacheTemplatePath(
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("/templates/CMakeLists.mustache")));
#else
    bool isWasm = false;
    core.setNodeSetPath(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../UA-Nodeset/")));
    core.setMustacheTemplatePath(
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("../../templates/open62541.mustache")));
    core.setCmakeMustacheTemplatePath(
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("../../templates/CMakeLists.mustache")));
#endif

    core.setOutputFilePath(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../templates/")));
    core.setJsonOutputFilePath(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../templates/")));
    core.setCmakeOutputFilePath(
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../templates/")));

    engine.rootContext()->setContextProperty(QStringLiteral("core"), &core);
    engine.rootContext()->setContextProperty(QStringLiteral("isWasm"), isWasm);

    qmlRegisterSingletonType<Utils>("Utils", 1, 0, "Utils", &Utils::create);
    qmlRegisterType<TreeItem>("TreeItem", 1, 0, "TreeItem");

#ifndef WASM_BUILD
    qInstallMessageHandler(messageHandler);
#endif

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("open62541devicedriver", "Main");

    return app.exec();
}
