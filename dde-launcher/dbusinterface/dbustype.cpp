#include "dbustype.h"

CategoryInfo::CategoryInfo(){

}

CategoryInfo::~CategoryInfo(){

}

void CategoryInfo::registerMetaType()
{
    qRegisterMetaType<CategoryInfo>("CategoryInfo");
    qDBusRegisterMetaType<CategoryInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const CategoryInfo &obj)
{
    argument.beginStructure();
    argument << obj.key << obj.id << obj.items;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, CategoryInfo &obj)
{
    argument.beginStructure();
    argument >> obj.key >> obj.id >> obj.items;
    argument.endStructure();
    return argument;
}


AppFrequencyInfo::AppFrequencyInfo(){

}

AppFrequencyInfo::~AppFrequencyInfo(){

}

void AppFrequencyInfo::registerMetaType()
{
    qRegisterMetaType<AppFrequencyInfo>("AppFrequencyInfo");
    qDBusRegisterMetaType<AppFrequencyInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const AppFrequencyInfo &obj)
{
    argument.beginStructure();
    argument << obj.key << obj.count;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, AppFrequencyInfo &obj)
{
    argument.beginStructure();
    argument >> obj.key >> obj.count;
    argument.endStructure();
    return argument;
}


AppInstalledTimeInfo::AppInstalledTimeInfo(){

}

AppInstalledTimeInfo::~AppInstalledTimeInfo(){

}

void AppInstalledTimeInfo::registerMetaType()
{
    qRegisterMetaType<AppInstalledTimeInfo>("AppInstalledTimeInfo");
    qDBusRegisterMetaType<AppInstalledTimeInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const AppInstalledTimeInfo &obj)
{
    argument.beginStructure();
    argument << obj.key << obj.intsalledTime;
    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, AppInstalledTimeInfo &obj)
{
    argument.beginStructure();
    argument >> obj.key >> obj.intsalledTime;
    argument.endStructure();
    return argument;
}

ItemInfo::ItemInfo(){

}

ItemInfo::~ItemInfo(){

}

void ItemInfo::registerMetaType()
{
    qRegisterMetaType<ItemInfo>("ItemInfo");
    qDBusRegisterMetaType<ItemInfo>();
}


QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &obj)
{
    argument.beginStructure();
    argument << obj.url << obj.name;
    argument << obj.key << obj.iconKey;
    argument << obj.id << obj.intsalledTime;

    argument.endStructure();
    return argument;
}


const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &obj)
{
    argument.beginStructure();
    argument >> obj.url >> obj.name;
    argument >> obj.key >> obj.iconKey;
    argument >> obj.id >> obj.intsalledTime;
    argument.endStructure();
    return argument;
}
