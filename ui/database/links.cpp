#include "links.h"

#include "logger.h"
#include "serializeddata.h"

#include <fstream>

#ifdef WITH_J2534
#include "lt/link/passthru.h"
#endif

#ifdef WITH_SOCKETCAN
#include "lt/link/socketcan.h"
#endif

#include <QString>
#include <lt/link/elm.h>

struct LinkData
{
    std::string type;
    std::string name;
    std::string port;
    int baudrate;
};

namespace serialize
{
template <typename D> void deserialize(D & d, LinkData & link)
{
    d.deserialize(link.type);
    d.deserialize(link.name);
    d.deserialize(link.port);
    d.deserialize(link.baudrate);
}

template <typename S> void serialize(S & s, const LinkData & link)
{
    s.serialize(link.type);
    s.serialize(link.name);
    s.serialize(link.port);
    s.serialize(link.baudrate);
}
} // namespace serialize

void Links::load()
{
    if (path_.empty())
    {
        throw std::runtime_error("database path has not been set");
    }

    std::ifstream file(path_, std::ios::binary | std::ios::in);
    if (!file.is_open())
    {
        // Does not exist
        return;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                             (std::istreambuf_iterator<char>()));
    std::vector<LinkData> links;

    serialize::Deserializer<serialize::InputBufferAdapter<std::vector<char>>>
        des(buffer);
    des.load(links);

    for (const LinkData & link : links)
    {
        if (link.type == "socketcan")
        {
#ifdef WITH_SOCKETCAN
            manualLinks_.emplace_back(
                std::make_unique<lt::SocketCanLink>(link.name, link.port));
#else
            Logger::warning(
                "SocketCAN is unuspported on this platform, ignoring link.");
#endif
        }
        else if (link.type == "elm")
        {
            manualLinks_.emplace_back(std::make_unique<lt::ElmDataLink>(
                link.name, link.port, link.baudrate));
        }
        else
        {
            throw std::runtime_error("Unknown datalink type: " + link.type);
        }
    }
}

void Links::save() const
{
    if (path_.empty())
    {
        throw std::runtime_error("database path has not been set");
    }

    // Save manual links
    std::vector<LinkData> links;
    for (const lt::DataLinkPtr & link : manualLinks_)
    {
        std::string type;
        if (link->type() == lt::DataLinkType::SocketCan)
        {
            type = "socketcan";
        }
        else if (link->type() == lt::DataLinkType::Elm)
        {
            type = "elm";
        }
        LinkData data;
        data.type = type;
        data.name = link->name();
        data.port = link->port();
        data.baudrate = link->baudrate();
        links.emplace_back(std::move(data));
    }

    std::vector<char> buffer;
    serialize::Serializer<serialize::OutputBufferAdapter<std::vector<char>>>
        ser(buffer);
    ser.save(links);

    std::ofstream file(path_, std::ios::binary | std::ios::out);
    file.write(buffer.data(), buffer.size());
    file.close();
}

void Links::detect()
{
    detectedLinks_.clear();
#ifdef WITH_J2534
    for (lt::PassThruLinkPtr & link : lt::detect_passthru_links())
    {
        detectedLinks_.emplace_back(std::unique_ptr<lt::DataLink>(
            static_cast<lt::DataLink *>(link.release())));
    }
#endif
}

void Links::add(lt::DataLinkPtr && link)
{
    beginInsertRows(createIndex(1, 0), manualLinks_.size(),
                    manualLinks_.size());
    manualLinks_.emplace_back(std::move(link));
    endInsertRows();
}

lt::DataLink * Links::get(int index) const
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    if (static_cast<std::size_t>(index) < detectedLinks_.size())
    {
        return detectedLinks_[index].get();
    }
    return manualLinks_[index - detectedLinks_.size()].get();
}

lt::DataLink * Links::getDetected(int index) const
{
    if (index < 0 || index >= detectedCount())
    {
        return nullptr;
    }

    return detectedLinks_[index].get();
}

lt::DataLink * Links::getManual(int index) const
{
    if (index < 0 || index >= manualCount())
    {
        return nullptr;
    }

    return manualLinks_[index].get();
}

lt::DataLink * Links::getFirst() const { return get(0); }

void Links::remove(lt::DataLink * link)
{
    beginResetModel();
    manualLinks_.erase(std::remove_if(manualLinks_.begin(), manualLinks_.end(),
                                      [link](const lt::DataLinkPtr & l) {
                                          return l.get() == link;
                                      }),
                       manualLinks_.end());
    endResetModel();
}

QModelIndex Links::index(int row, int column, const QModelIndex & parent) const
{
    if (parent.isValid() && parent.internalId() == 0)
    {
        return createIndex(row, column, parent.row() + 1);
    }
    return createIndex(row, column);
}

QModelIndex Links::parent(const QModelIndex & child) const
{
    if (child.internalId() != 0)
    {
        return createIndex(child.internalId() - 1, 0);
    }
    return QModelIndex();
}

int Links::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
    {
        return 2;
    }

    if (parent.internalId() == 0)
    {
        if (parent.row() == 0)
        {
            // Auto-detected
            return detectedCount();
        }
        else if (parent.row() == 1)
        {
            // Manully added
            return manualCount();
        }
        return 0;
    }

    return 0;
}

int Links::columnCount(const QModelIndex & /*parent*/) const { return 2; }

QString typeToString(lt::DataLinkType type)
{
    switch (type)
    {
    case lt::DataLinkType::PassThru:
        return "PassThru";
    case lt::DataLinkType::SocketCan:
        return "SocketCAN";
    case lt::DataLinkType::Elm:
        return "ELM327/ST";
    default:
        return QObject::tr("Invalid");
    }
}

QVariant Links::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || index.column() > 1 || index.column() < 0)
    {
        return QVariant();
    }

    if (index.internalId() == 0)
    {
        if (index.column() != 0)
        {
            return QVariant();
        }
        // root node
        if (role != Qt::DisplayRole)
        {
            return QVariant();
        }

        if (index.row() == 0)
        {
            return tr("Auto-detected");
        }
        else if (index.row() == 1)
        {
            return tr("Manually added");
        }
        return QVariant();
    }

    lt::DataLink * link;

    int parentRow = index.internalId() - 1;
    if (parentRow == 0)
    {
        // Auto-detected
        if (index.row() >= detectedCount())
        {
            return QVariant();
        }
        link = detectedLinks_[index.row()].get();
    }
    else if (parentRow == 1)
    {
        if (index.row() >= manualCount())
        {
            return QVariant();
        }
        link = manualLinks_[index.row()].get();
    }
    else
    {
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
        if (index.column() == 0)
        {
            return QString::fromStdString(link->name());
        }
        else if (index.column() == 1)
        {
            return typeToString(link->type());
        }
        break;
    case Qt::UserRole:
        return QVariant::fromValue(link);
    default:
        break;
    }
    return QVariant();
}

QVariant Links::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (section == 0)
    {
        return tr("Name");
    }
    else if (section == 1)
    {
        return tr("Type");
    }
    return QVariant();
}

Links::Links(Links && database) noexcept
    : manualLinks_{std::move(database.manualLinks_)},
      detectedLinks_{std::move(database.detectedLinks_)}
{
}

int LinksListModel::rowCount(const QModelIndex & /*parent*/) const
{
    return links_.count();
}

QVariant LinksListModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() > links_.count())
    {
        return QVariant();
    }

    lt::DataLink * link = links_.get(index.row());

    if (role == Qt::DisplayRole)
    {
        return QString::fromStdString(link->name());
    }
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(link);
    }
    return QVariant();
}
LinksListModel::LinksListModel(const Links & links) : links_(links)
{
    connect(&links_, &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex & parent, int first, int last) {
                beginResetModel();
                endResetModel();
            });
}
