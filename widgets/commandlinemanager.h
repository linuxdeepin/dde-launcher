/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H

#include <QCommandLineOption>
#include <QJsonObject>
#include <QMap>
#include <QList>
class QCommandLineParser;
class QCoreApplication;

class CommandLineManager
{

public:

    static CommandLineManager* instance(){
        static CommandLineManager instance;
        return &instance;
    }

    static void initOptions();

    bool isSet(const QString& name) const;
    QString value(const QString& name) const;
    void process();

    void addOption(const QCommandLineOption& option);
    void addOptions(const QList<QCommandLineOption> & options);

private:
    explicit CommandLineManager();
   ~CommandLineManager();
   CommandLineManager(const CommandLineManager &);
   CommandLineManager & operator = (const CommandLineManager &);
   QCommandLineParser* m_commandParser;
   QMap<QString, QCommandLineOption> m_options;
};

#endif // COMMANDLINEMANAGER_H
