#include "commandlinemanager.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CommandLineManager::CommandLineManager():
    m_commandParser(new QCommandLineParser)
{

}

void CommandLineManager::process(){
    m_commandParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    m_commandParser->addHelpOption();
    m_commandParser->addVersionOption();
    m_commandParser->process(*qApp);
}

void CommandLineManager::initOptions(){
    instance()->process();
}

void CommandLineManager::addOption(const QCommandLineOption &option){
    m_commandParser->addOption(option);
    foreach (QString key , option.names()) {
        m_options.insert(key, option);
    }
}

void CommandLineManager::addOptions(const QList<QCommandLineOption> &options){
    foreach (QCommandLineOption option, options) {
        addOption(option);
    }
}

bool CommandLineManager::isSet(const QString &name) const{
    return m_commandParser->isSet(name);
}

QString CommandLineManager::value(const QString &name) const{
    if (isSet(name)){
        return m_commandParser->value(name);
    }
    return "";
}

CommandLineManager::~CommandLineManager(){

}
