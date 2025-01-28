/*
 File: Utils.h
 Created on: 28/01/2025
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

#ifndef _UTILS_H_
#define _UTILS_H_

// Qt
#include <QLabel>

/** \class ClickableHoverLabel
 * \brief ClickableLabel subclass that changes the mouse cursor when hovered.
 *
 */
class ClickableHoverLabel
    : public QLabel
{
    Q_OBJECT
  public:
    /** \brief ClickableHoverLabel class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \f Widget flags.
     *
     */
    explicit ClickableHoverLabel(QWidget *parent = 0, Qt::WindowFlags f = 0);

    /** \brief ClickableHoverLabel class constructor.
     * \param[in] text Label text.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \f Widget flags.
     *
     */
    explicit ClickableHoverLabel(const QString &text, QWidget *parent = 0, Qt::WindowFlags f = 0);

    /** \brief ClickableHoverLabel class virtual destructor.
     *
     */
    virtual ~ClickableHoverLabel();

  signals:
    void clicked();

  protected:
    void mousePressEvent(QMouseEvent *e)
    {
      emit clicked();
      QLabel::mousePressEvent(e);
    }

    virtual void enterEvent(QEvent *event) override
    {
      setCursor(Qt::PointingHandCursor);
      QLabel::enterEvent(event);
    }

    virtual void leaveEvent(QEvent *event) override
    {
      setCursor(Qt::ArrowCursor);
      QLabel::leaveEvent(event);
    }
};

#endif // UTILS_H_