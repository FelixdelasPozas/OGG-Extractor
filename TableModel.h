/*
 File: TableModel.h
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

#ifndef __TABLE_MODEL_H_
#define __TABLE_MODEL_H_

// Qt
#include <QAbstractItemModel>

// C++
#include <cmath>

struct OGGData;

/** \class TableModel
 * \brief Implements the model for the table view. 
 * 
 */
class TableModel
: public QAbstractItemModel
{
  Q_OBJECT
  public:
    /** \brief TableModel class constructor. 
     * \param[in] parent Raw pointer of the QObject parent of this one. 
     * 
    */
    explicit TableModel(QObject *parent = nullptr);

    /** \brief TableModel class virtual destructor. 
     * 
     * 
     */
    virtual ~TableModel(){};

    /** \brief Sets the data for the model. 
     * \param[in] data Vector of OGGData structs reference. 
     * 
    */
    void setModelData(std::vector<OGGData> &data);

    /** \brief Sets the page to show on the table.
     * \param[in] pageIndex Page number. 
     * 
    */
    void setPage(const unsigned int pageIndex);

    /** \brief Sets the size of each page to show. 
     * \param[in] pageSize Number of items in each page. 
     * 
     */
    void setPageSize(const unsigned int pageSize);

    /** \brief Returns the current page size. 
     * 
    */
    inline unsigned int pageSize() const
    { return m_pageSize; }

    /** \brief Return the current page. 
     * 
     */
    inline unsigned int page() const
    { return m_page; }

    /** \brief Helper method to return the maximum index of current page. 
     * 
    */
    unsigned int pageItems() const
    {
      if(m_data)
      {
        const auto remain =  m_data->size() - (m_page * m_pageSize);
        
        if(remain < m_pageSize)
          return remain;

        return m_pageSize;
      }

      return 0;
    }

    unsigned int maxPage() const
    {
      if(m_data)
      {
        return std::ceil(static_cast<float>(m_data->size()) / m_pageSize);
      }

      return 0;
    }

    /** \brief Resets the model pointer. 
     * 
     */
    void clearModel();

    // Reimplemented from base class.
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;    
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /** \brief Helper method that returns the displayRole data for the given OGGData.
     * \param[in] data OGGData struct reference. 
     * \param[in] column Table column index. 
     * 
     */
    QVariant dataDisplayRole(const OGGData &data, int column) const;

   private:
    /** \brief Updates the contents of the table with the items of the current page. 
     * 
     */
    void updateTableContents();

    /** \brief Helper method that returns the tooltipRole data for the given column.
     * \param[in] column Table column index. 
     * 
     */
    QVariant dataTooltipRole(int column) const;

    struct ContainerCache
    {
      QString baseName;   // container filename.
      int     fieldWidth; // size number width.
    };

    std::vector<OGGData>                          *m_data;     /** pointer to model data. */ 
    mutable std::map<std::wstring, ContainerCache> m_cache;    /** containers data cache. */
    unsigned int                                   m_page;     /** current page. */
    unsigned int                                   m_pageSize; /** max items per page. */
};

#endif