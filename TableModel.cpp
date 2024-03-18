/*
 File: TableModel.cpp
 Created on: 14 mar. 2024
 Author: Felix de las Pozas Alvarez

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project
#include <OGGContainerWrapper.h>
#include <TableModel.h>

// Qt
#include <QFileInfo>
#include <QString>

// C++ 
#include <array>
#include <string>
#include <filesystem>
#include <cassert>

//----------------------------------------------------------------------------
TableModel::TableModel(QObject *parent)
: QAbstractItemModel(parent)
, m_data{nullptr}
, m_page{0}
, m_pageSize{1000}
{
}

//----------------------------------------------------------------------------
void TableModel::setModelData(std::vector<OGGData> &data)
{
  if(data.empty())
    m_data = nullptr;
  else
    m_data = &data;

  m_page = 0;
  updateTableContents();
}

//----------------------------------------------------------------------------
void TableModel::setPage(const unsigned int pageIndex)
{
  if(m_data && pageIndex < maxPage())
  {
    m_page = pageIndex;
    updateTableContents();
  }
}

//----------------------------------------------------------------------------
void TableModel::setPageSize(const unsigned int pageSize)
{
  m_pageSize = std::min(pageSize, 1000U);
  m_page = 0;
  updateTableContents();
}

//----------------------------------------------------------------------------
void TableModel::clearModel()
{
  beginResetModel();
  m_data = nullptr;
  endResetModel();
}

//----------------------------------------------------------------------------
QVariant TableModel::data(const QModelIndex &index, int role) const
{
  if(m_data && index.isValid() && (index.row() < pageItems()) && (index.column() < 8))
  {
    const auto row = (m_page * m_pageSize) + index.row();
    assert(row < m_data->size());
    const auto &data = m_data->at(row);

    switch(role)
    {
      case Qt::EditRole:
      case Qt::DisplayRole:
        return dataDisplayRole(data, index.column());
      break;
      case Qt::ToolTipRole:
        return dataTooltipRole(index.column());
      break;
      case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
        break;
    }

  }

  return QVariant();
}

//----------------------------------------------------------------------------
bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(m_data && index.isValid())
  {
    const auto row = (m_page * m_pageSize) + index.row();
    switch(role)
    {
      case Qt::DisplayRole:
        if(index.column() == 7)
        {
          const auto row = (m_page * m_pageSize) + index.row();
          m_data->at(row).error = value.toString().toStdString();
        }
        break;
      case Qt::EditRole:
        if(index.column() == 1)
        {
          m_data->at(row).name = value.toString().toStdWString();
        }
        break;
      default:
        break;        
    }
  }

  return false;
}

//----------------------------------------------------------------------------
Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
  if(index.column() == 1)
    return Qt::ItemIsEnabled|Qt::ItemIsEditable;

  return Qt::ItemIsEnabled;    
}

//----------------------------------------------------------------------------
int TableModel::columnCount(const QModelIndex &parent) const
{
  return 8;
}

//----------------------------------------------------------------------------
int TableModel::rowCount(const QModelIndex &parent) const
{
  if(m_data)
    return pageItems();

  return 0;
}

//----------------------------------------------------------------------------
QModelIndex TableModel::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

//----------------------------------------------------------------------------
QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row, column, nullptr);
}

//----------------------------------------------------------------------------
QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  const std::vector<QString> headers = {"Selected", "Filename", "Channels", "Bitrate", 
                                        "Duration", "Container", "Play", "Errors" };

  if(role != Qt::DisplayRole)
    return QAbstractItemModel::headerData(section, orientation, role);

  switch(orientation)
  {
    default:
    case Qt::Horizontal:
      if(section < headers.size()) return headers[section];
      break;
    case Qt::Vertical:
      if(m_data && section <= pageItems()) return (m_page * m_pageSize) + section + 1;
      break;
  }

  return QVariant();
}

//----------------------------------------------------------------------------
void TableModel::updateTableContents()
{
  // Nothing to do, just signal.
  beginResetModel();
  endResetModel();
}

//----------------------------------------------------------------------------
QVariant TableModel::dataDisplayRole(const OGGData &data, int column) const
{
  switch (column)
  {
    case 1: // Filename
      {
        if(!data.name.empty())
          return QString::fromStdWString(data.name);

        auto it = m_cache.find(data.container);
        if (it == m_cache.cend())
        {
          const QFileInfo fileInfo(QString::fromStdWString(data.container));
          m_cache.emplace(data.container, ContainerCache{fileInfo.completeBaseName(), QString::number(fileInfo.size(), 16).length()});
        }
        it = m_cache.find(data.container);

        const auto cache = (*it).second;
        return QString("%1 0x%2-0x%3 (%4)").arg(cache.baseName).arg(data.start, cache.fieldWidth, 16, QLatin1Char('0')).arg(data.end, cache.fieldWidth, 16, QLatin1Char('0')).arg(data.end - data.start);
      }
      break;
    case 2: // Channels
      return QString::number(data.channels);
      break;
    case 3: // Bitrate
      return QString::number(data.rate);
      break;
    case 4: // Duration
      {
        qulonglong iSeconds = static_cast<qulonglong>(data.duration);
        return QString("%1:%2:%3,%4").arg(iSeconds / 3600, 2, 10, QChar('0'))
                                    .arg(iSeconds / 60, 2, 10, QChar('0'))
                                    .arg(iSeconds % 60, 2, 10, QChar('0'))
                                    .arg(static_cast<qulonglong>((data.duration - iSeconds)*1000), 3, 10, QChar('0'));
      }
      break;
    case 5: // Container
      {
        QString containerName = "Unknown";
        const auto file = std::filesystem::path(data.container);
        return QString::fromStdWString(file.filename());
      }
      break;
    case 7: // Errors
      return (data.error.empty() ? tr("No error") : QString::fromStdString(data.error));
      break;
    case 0: // Selected
    case 6: // Play
    default:
      break;
  }

  return QVariant();
}

//----------------------------------------------------------------------------
QVariant TableModel::dataTooltipRole(int column) const
{
  switch(column)
  {
    case 0:
      return QString("Check to select audio file for extraction.");
    case 1:
      return QString("Output audio file name. Double-click to edit.");
    case 2:
      return QString("Number of channels of audio file.");
    case 3:
      return QString("Bitrate of audio file.");
    case 4:
      return QString("Duration of audio file.");
    case 5:
      return QString("File name of the container of this audio file.");
    case 6:
      return QString("Click to play/stop.");
    case 7:
      return QString("Errors of the audio file processing.");
    default:
      break;
  }

  return QVariant();
}
