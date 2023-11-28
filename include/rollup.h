/*
    TickTock is an open-source Time Series Database, maintained by
    Yongtao You (yongtao.you@gmail.com) and Yi Lin (ylin30@gmail.com).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <queue>
#include "dp.h"
#include "mmap.h"
#include "type.h"


namespace tt
{


class QueryTask;
class Tsdb;


class __attribute__ ((__packed__)) RollupManager
{
public:
    RollupManager();
    RollupManager(const RollupManager& copy);   // copy constructor
    RollupManager(Timestamp tstmap, uint32_t cnt, double min, double max, double sum);
    ~RollupManager();

    void copy_from(const RollupManager& other);
    RollupManager& operator=(const RollupManager& other);

    static void init();
    static void shutdown();

    // process in-order dps only
    void add_data_point(MetricId mid, TimeSeriesId tid, DataPoint& dp);
    void flush(MetricId mid, TimeSeriesId tid);
    void close(TimeSeriesId tid);   // called during TT shutdown

    inline Timestamp get_tstamp() const { return m_tstamp; }

    // return true if data-point found; false if no data
    bool query(RollupType type, DataPointPair& dp);

    static void add_data_file_size(off_t size);
    static off_t get_rollup_data_file_size();
    static int get_rollup_bucket(MetricId mid);
    static RollupDataFile *get_data_file(MetricId mid, Timestamp tstamp);
    static void get_data_files(MetricId mid, TimeRange& range, std::vector<RollupDataFile*>& files);
    static void query(MetricId mid, TimeRange& range, std::vector<QueryTask*>& tasks, RollupType rollup);
    static void query_no_lock(MetricId mid, TimeRange& range, std::vector<QueryTask*>& tasks, RollupType rollup);
    static double query(struct rollup_entry *entry, RollupType type);
    static void rotate();

private:
    Timestamp step_down(Timestamp tstamp);

    uint32_t m_cnt;
    double m_min;
    double m_max;
    double m_sum;
    Timestamp m_tstamp;

    // used during shutdown/restart of TT
    static RollupDataFile *m_backup_data_file;

    static std::mutex m_lock;
    static std::unordered_map<uint64_t, RollupDataFile*> m_data_files;
    static std::queue<off_t> m_sizes;   // sizes of 'recent' data files
    static off_t m_size_hint;
};


}
