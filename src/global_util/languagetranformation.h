#ifndef LANGUAGETRANSFORMATION_H
#define LANGUAGETRANSFORMATION_H

#include <QObject>

class LanguageTransformation : public QObject
{
    Q_OBJECT
public:
    explicit LanguageTransformation(QObject *parent = Q_NULLPTR);
    static LanguageTransformation *instance();

    QString zhToPinYin(const QString &chinese);
    QString zhToJianPin(const QString &chinese);
    void readConfigFile();

private:
    static LanguageTransformation *m_instance;

    QStringList m_pinyinStrList;
    QStringList m_jianpingStrList;
};

#endif
