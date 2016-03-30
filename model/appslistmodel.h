#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>

class AppsManager;
class CalculateUtil;
class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AppItemRole {
        ItemSizeHintRole = Qt::SizeHintRole,
        AppNameRole = Qt::DisplayRole,
        AppIconRole = Qt::DecorationRole,
        AppReserveRole = Qt::UserRole,

        // custom role start
        AppRawItemInfoRole,
        AppKeyRole,
        AppIconKeyRole,
        AppDesktopRole,
        AppCategoryRole,
        AppGroupRole,
        AppAutoStartRole,
        AppNewInstallRole,
        AppIsOnDesktopRole,
        AppIsOnDockRole,
        AppIconSizeRole,
        AppFontSizeRole,
        AppItemIsDragingRole,
    };

    enum AppCategory {
        All,
        Custom,
        Search,

        // apps category
        Chat,
        Internet,
        Music,
        Video,
        Graphics,
        Game,
        Office,
        Reading,
        Development,
        System,
        Others,
    };

public:
    explicit AppsListModel(const AppCategory& category, QObject *parent = 0);

    inline AppCategory category() const {return m_category;}
    void setDragingIndex(const QModelIndex &index);
    void dropInsert(const QString &appKey, const int pos);
    void dropSwap(const int nextPos);

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

public slots:
    void clearDragingIndex();

protected:
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void dataChanged(const AppsListModel::AppCategory category);
    void layoutChanged(const AppsListModel::AppCategory category);

private:
    AppsManager *m_appsManager;
    CalculateUtil *m_calcUtil;

    QModelIndex m_dragingIndex = QModelIndex();
    AppCategory m_category = All;
};

Q_DECLARE_METATYPE(AppsListModel::AppCategory)

#endif // APPSLISTMODEL_H
