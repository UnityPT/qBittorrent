/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2023  Vladimir Golovnev <glassez@yandex.ru>
 * Copyright (C) 2006  Christophe Dumez <chris@qbittorrent.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "statusfilterwidget.h"

#include <QListWidgetItem>
#include <QMenu>

#include "base/bittorrent/session.h"
#include "base/global.h"
#include "base/preferences.h"
#include "base/torrentfilter.h"
#include "gui/transferlistwidget.h"
#include "gui/uithememanager.h"

StatusFilterWidget::StatusFilterWidget(QWidget *parent, TransferListWidget *transferList)
    : BaseFilterWidget(parent, transferList)
{
    // Add status filters
    auto *all = new QListWidgetItem(this);
    all->setData(Qt::DisplayRole, tr("All (0)", "this is for the status filter"));
    all->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"filter-all"_qs, u"filterall"_qs));
    auto *downloading = new QListWidgetItem(this);
    downloading->setData(Qt::DisplayRole, tr("Downloading (0)"));
    downloading->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"downloading"_qs));
    auto *seeding = new QListWidgetItem(this);
    seeding->setData(Qt::DisplayRole, tr("Seeding (0)"));
    seeding->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"upload"_qs, u"uploading"_qs));
    auto *completed = new QListWidgetItem(this);
    completed->setData(Qt::DisplayRole, tr("Completed (0)"));
    completed->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"checked-completed"_qs, u"completed"_qs));
    auto *resumed = new QListWidgetItem(this);
    resumed->setData(Qt::DisplayRole, tr("Resumed (0)"));
    resumed->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"torrent-start"_qs, u"media-playback-start"_qs));
    auto *paused = new QListWidgetItem(this);
    paused->setData(Qt::DisplayRole, tr("Paused (0)"));
    paused->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"stopped"_qs, u"media-playback-pause"_qs));
    auto *active = new QListWidgetItem(this);
    active->setData(Qt::DisplayRole, tr("Active (0)"));
    active->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"filter-active"_qs, u"filteractive"_qs));
    auto *inactive = new QListWidgetItem(this);
    inactive->setData(Qt::DisplayRole, tr("Inactive (0)"));
    inactive->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"filter-inactive"_qs, u"filterinactive"_qs));
    auto *stalled = new QListWidgetItem(this);
    stalled->setData(Qt::DisplayRole, tr("Stalled (0)"));
    stalled->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"filter-stalled"_qs, u"filterstalled"_qs));
    auto *stalledUploading = new QListWidgetItem(this);
    stalledUploading->setData(Qt::DisplayRole, tr("Stalled Uploading (0)"));
    stalledUploading->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"stalledUP"_qs));
    auto *stalledDownloading = new QListWidgetItem(this);
    stalledDownloading->setData(Qt::DisplayRole, tr("Stalled Downloading (0)"));
    stalledDownloading->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"stalledDL"_qs));
    auto *checking = new QListWidgetItem(this);
    checking->setData(Qt::DisplayRole, tr("Checking (0)"));
    checking->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"force-recheck"_qs, u"checking"_qs));
    auto *moving = new QListWidgetItem(this);
    moving->setData(Qt::DisplayRole, tr("Moving (0)"));
    moving->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"set-location"_qs));
    auto *errored = new QListWidgetItem(this);
    errored->setData(Qt::DisplayRole, tr("Errored (0)"));
    errored->setData(Qt::DecorationRole, UIThemeManager::instance()->getIcon(u"error"_qs));

    const QVector<BitTorrent::Torrent *> torrents = BitTorrent::Session::instance()->torrents();
    update(torrents);
    connect(BitTorrent::Session::instance(), &BitTorrent::Session::torrentsUpdated
            , this, &StatusFilterWidget::update);

    const Preferences *const pref = Preferences::instance();
    connect(pref, &Preferences::changed, this, &StatusFilterWidget::configure);

    const int storedRow = pref->getTransSelFilter();
    if (item((storedRow < count()) ? storedRow : 0)->isHidden())
        setCurrentRow(TorrentFilter::All, QItemSelectionModel::SelectCurrent);
    else
        setCurrentRow(storedRow, QItemSelectionModel::SelectCurrent);

    toggleFilter(pref->getStatusFilterState());
}

StatusFilterWidget::~StatusFilterWidget()
{
    Preferences::instance()->setTransSelFilter(currentRow());
}

QSize StatusFilterWidget::sizeHint() const
{
    int numVisibleItems = 0;
    for (int i = 0; i < count(); ++i)
    {
        if (!item(i)->isHidden())
            ++numVisibleItems;
    }

    return {
        // Width should be exactly the width of the content
        sizeHintForColumn(0),
        // Height should be exactly the height of the content
        static_cast<int>((sizeHintForRow(0) + 2 * spacing()) * (numVisibleItems + 0.5))};
}

void StatusFilterWidget::updateTorrentStatus(const BitTorrent::Torrent *torrent)
{
    TorrentFilterBitset &torrentStatus = m_torrentsStatus[torrent];

    const auto update = [torrent, &torrentStatus](const TorrentFilter::Type status, int &counter)
    {
        const bool hasStatus = torrentStatus[status];
        const bool needStatus = TorrentFilter(status).match(torrent);
        if (needStatus && !hasStatus)
        {
            ++counter;
            torrentStatus.set(status);
        }
        else if (!needStatus && hasStatus)
        {
            --counter;
            torrentStatus.reset(status);
        }
    };

    update(TorrentFilter::Downloading, m_nbDownloading);
    update(TorrentFilter::Seeding, m_nbSeeding);
    update(TorrentFilter::Completed, m_nbCompleted);
    update(TorrentFilter::Resumed, m_nbResumed);
    update(TorrentFilter::Paused, m_nbPaused);
    update(TorrentFilter::Active, m_nbActive);
    update(TorrentFilter::Inactive, m_nbInactive);
    update(TorrentFilter::StalledUploading, m_nbStalledUploading);
    update(TorrentFilter::StalledDownloading, m_nbStalledDownloading);
    update(TorrentFilter::Checking, m_nbChecking);
    update(TorrentFilter::Moving, m_nbMoving);
    update(TorrentFilter::Errored, m_nbErrored);

    m_nbStalled = m_nbStalledUploading + m_nbStalledDownloading;
}

void StatusFilterWidget::updateTexts()
{
    const qsizetype torrentsCount = BitTorrent::Session::instance()->torrentsCount();
    item(TorrentFilter::All)->setData(Qt::DisplayRole, tr("All (%1)").arg(torrentsCount));
    item(TorrentFilter::Downloading)->setData(Qt::DisplayRole, tr("Downloading (%1)").arg(m_nbDownloading));
    item(TorrentFilter::Seeding)->setData(Qt::DisplayRole, tr("Seeding (%1)").arg(m_nbSeeding));
    item(TorrentFilter::Completed)->setData(Qt::DisplayRole, tr("Completed (%1)").arg(m_nbCompleted));
    item(TorrentFilter::Resumed)->setData(Qt::DisplayRole, tr("Resumed (%1)").arg(m_nbResumed));
    item(TorrentFilter::Paused)->setData(Qt::DisplayRole, tr("Paused (%1)").arg(m_nbPaused));
    item(TorrentFilter::Active)->setData(Qt::DisplayRole, tr("Active (%1)").arg(m_nbActive));
    item(TorrentFilter::Inactive)->setData(Qt::DisplayRole, tr("Inactive (%1)").arg(m_nbInactive));
    item(TorrentFilter::Stalled)->setData(Qt::DisplayRole, tr("Stalled (%1)").arg(m_nbStalled));
    item(TorrentFilter::StalledUploading)->setData(Qt::DisplayRole, tr("Stalled Uploading (%1)").arg(m_nbStalledUploading));
    item(TorrentFilter::StalledDownloading)->setData(Qt::DisplayRole, tr("Stalled Downloading (%1)").arg(m_nbStalledDownloading));
    item(TorrentFilter::Checking)->setData(Qt::DisplayRole, tr("Checking (%1)").arg(m_nbChecking));
    item(TorrentFilter::Moving)->setData(Qt::DisplayRole, tr("Moving (%1)").arg(m_nbMoving));
    item(TorrentFilter::Errored)->setData(Qt::DisplayRole, tr("Errored (%1)").arg(m_nbErrored));
}

void StatusFilterWidget::hideZeroItems()
{
    item(TorrentFilter::Downloading)->setHidden(m_nbDownloading == 0);
    item(TorrentFilter::Seeding)->setHidden(m_nbSeeding == 0);
    item(TorrentFilter::Completed)->setHidden(m_nbCompleted == 0);
    item(TorrentFilter::Resumed)->setHidden(m_nbResumed == 0);
    item(TorrentFilter::Paused)->setHidden(m_nbPaused == 0);
    item(TorrentFilter::Active)->setHidden(m_nbActive == 0);
    item(TorrentFilter::Inactive)->setHidden(m_nbInactive == 0);
    item(TorrentFilter::Stalled)->setHidden(m_nbStalled == 0);
    item(TorrentFilter::StalledUploading)->setHidden(m_nbStalledUploading == 0);
    item(TorrentFilter::StalledDownloading)->setHidden(m_nbStalledDownloading == 0);
    item(TorrentFilter::Checking)->setHidden(m_nbChecking == 0);
    item(TorrentFilter::Moving)->setHidden(m_nbMoving == 0);
    item(TorrentFilter::Errored)->setHidden(m_nbErrored == 0);

    if (currentItem() && currentItem()->isHidden())
        setCurrentRow(TorrentFilter::All, QItemSelectionModel::SelectCurrent);
}

void StatusFilterWidget::update(const QVector<BitTorrent::Torrent *> torrents)
{
    for (const BitTorrent::Torrent *torrent : torrents)
        updateTorrentStatus(torrent);

    updateTexts();

    if (Preferences::instance()->getHideZeroStatusFilters())
    {
        hideZeroItems();
        updateGeometry();
    }
}

void StatusFilterWidget::showMenu()
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction(UIThemeManager::instance()->getIcon(u"torrent-start"_qs, u"media-playback-start"_qs), tr("Resume torrents")
        , transferList(), &TransferListWidget::startVisibleTorrents);
    menu->addAction(UIThemeManager::instance()->getIcon(u"torrent-stop"_qs, u"media-playback-pause"_qs), tr("Pause torrents")
        , transferList(), &TransferListWidget::pauseVisibleTorrents);
    menu->addAction(UIThemeManager::instance()->getIcon(u"list-remove"_qs), tr("Remove torrents")
        , transferList(), &TransferListWidget::deleteVisibleTorrents);

    menu->popup(QCursor::pos());
}

void StatusFilterWidget::applyFilter(int row)
{
    transferList()->applyStatusFilter(row);
}

void StatusFilterWidget::handleTorrentsLoaded(const QVector<BitTorrent::Torrent *> &torrents)
{
    for (const BitTorrent::Torrent *torrent : torrents)
        updateTorrentStatus(torrent);

    updateTexts();
}

void StatusFilterWidget::torrentAboutToBeDeleted(BitTorrent::Torrent *const torrent)
{
    const TorrentFilterBitset status = m_torrentsStatus.take(torrent);

    if (status[TorrentFilter::Downloading])
        --m_nbDownloading;
    if (status[TorrentFilter::Seeding])
        --m_nbSeeding;
    if (status[TorrentFilter::Completed])
        --m_nbCompleted;
    if (status[TorrentFilter::Resumed])
        --m_nbResumed;
    if (status[TorrentFilter::Paused])
        --m_nbPaused;
    if (status[TorrentFilter::Active])
        --m_nbActive;
    if (status[TorrentFilter::Inactive])
        --m_nbInactive;
    if (status[TorrentFilter::StalledUploading])
        --m_nbStalledUploading;
    if (status[TorrentFilter::StalledDownloading])
        --m_nbStalledDownloading;
    if (status[TorrentFilter::Checking])
        --m_nbChecking;
    if (status[TorrentFilter::Moving])
        --m_nbMoving;
    if (status[TorrentFilter::Errored])
        --m_nbErrored;

    m_nbStalled = m_nbStalledUploading + m_nbStalledDownloading;

    updateTexts();
}

void StatusFilterWidget::configure()
{
    if (Preferences::instance()->getHideZeroStatusFilters())
    {
        hideZeroItems();
    }
    else
    {
        for (int i = 0; i < count(); ++i)
            item(i)->setHidden(false);
    }

    updateGeometry();
}
