#include "iteminfo.h"

ItemInfo::ItemInfo()
{

}

ItemInfo::~ItemInfo()
{

}

void ItemInfo::registerMetaType()
{
    qRegisterMetaType<ItemInfo>("ItemInfo");
    qDBusRegisterMetaType<ItemInfo>();
    qRegisterMetaType<ItemInfoList>("ItemInfoList");
    qDBusRegisterMetaType<ItemInfoList>();
}

QDebug operator<<(QDebug argument, const ItemInfo &info)
{
    argument << info.m_id << info.m_installedTime;
    argument << info.m_url << info.m_name << info.m_key << info.m_iconKey;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &info)
{
    argument.beginStructure();
    argument << info.m_url << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_id << info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo &info)
{
    argument << info.m_url << info.m_name << info.m_key << info.m_iconKey;
    argument << info.m_id << info.m_installedTime;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &info)
{
    argument.beginStructure();
    argument >> info.m_url >> info.m_name >> info.m_key >> info.m_iconKey;
    argument >> info.m_id >> info.m_installedTime;
    argument.endStructure();

    return argument;
}
