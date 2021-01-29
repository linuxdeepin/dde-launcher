#pragma once

#include <QString>
#include <QVariant>
#include <QList>

enum class Category {
    AudioVideo,
    Audio,
    Video,
    Development,
    Education,
    Game,
    Graphics,
    Network,
    Office,
    Science,
    Settings,
    System,
    Utility,
    Other,
};

typedef QList<Category> Categories;
Q_DECLARE_METATYPE(Category);
Q_DECLARE_METATYPE(Categories);

class AppData : public QObject {
    Q_OBJECT
    Q_PROPERTY(Categories categories READ categories WRITE setCategories NOTIFY categoriesChanged)
    Q_PROPERTY(QVariant data READ data WRITE setData NOTIFY dataChanged)
public:
    explicit AppData(const QString& id, QObject *parent = nullptr)
        : QObject(parent)
        , m_id(id)
    {
    }

    inline QString id() {
        return m_id;
    }

    inline Categories categories() {
        return m_categories;
    }

    inline QVariant data() {
        return m_data;
    }

    void setCategories(const Categories categories) {
        m_categories = categories;
    }

    void setData(const QVariant &data) {
        m_data = data;
    }

Q_SIGNALS:
    void dataChanged(const QVariant &data);
    void categoriesChanged(const Categories &categories);

private:
    QString m_id;
    Categories m_categories;

    // If category is Other, will read category in data.
    // 'Category': [{'zh_CN': 'Audio'}]
    // data is used for plugins to store the data they need
    QVariant m_data;
};
