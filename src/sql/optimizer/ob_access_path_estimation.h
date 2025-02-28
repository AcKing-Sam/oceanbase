/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef OB_ACCESS_PATH_ESTIMATION_H
#define OB_ACCESS_PATH_ESTIMATION_H

#include "sql/optimizer/ob_opt_est_cost.h"
#include "sql/optimizer/ob_optimizer_context.h"
#include "sql/optimizer/ob_join_order.h"
#include "sql/optimizer/ob_dynamic_sampling.h"
#include "sql/optimizer/ob_opt_selectivity.h"

namespace oceanbase {
namespace sql {
class AccessPath;

struct ObBatchEstTasks
{
  ObAddr addr_;
  obrpc::ObEstPartArg arg_;
  obrpc::ObEstPartRes res_;
  ObArray<AccessPath *> paths_;

  bool check_result_reliable() const;

  TO_STRING_KV(K_(addr));
};

class ObAccessPathEstimation
{
public:
  static int estimate_rowcount(ObOptimizerContext &ctx,
                               common::ObIArray<AccessPath*> &paths,
                               const bool is_inner_path,
                               const ObIArray<ObRawExpr*> &filter_exprs,
                               ObBaseTableEstMethod &method);
  static int inner_estimate_rowcount(ObOptimizerContext &ctx,
                                      common::ObIArray<AccessPath *> &paths,
                                      const bool is_inner_path,
                                      const ObIArray<ObRawExpr*> &filter_exprs,
                                      bool &is_use_ds);
  static int estimate_full_table_rowcount(ObOptimizerContext &ctx,
                                          const ObTablePartitionInfo &table_part_info,
                                          ObTableMetaInfo &meta);

  static bool is_retry_ret(int ret);
  static int is_storage_estimation_enabled(const ObLogPlan* log_plan,
                                          ObOptimizerContext &ctx,
                                          uint64_t table_id,
                                          uint64_t ref_table_id,
                                          bool &can_use);
private:
  static int inner_estimate_rowcount(ObOptimizerContext &ctx,
                                     common::ObIArray<AccessPath *> &paths,
                                     const bool is_inner_path,
                                     const ObIArray<ObRawExpr*> &filter_exprs,
                                     ObBaseTableEstMethod &method);

  static inline uint64_t choose_one_est_method(ObBaseTableEstMethod valid_methods, const ObBaseTableEstMethod est_priority[], uint64_t cnt)
  {
    ObBaseTableEstMethod ret = EST_INVALID;
    for (int64_t i = 0; EST_INVALID == ret && i < cnt; i ++) {
      if ((valid_methods & est_priority[i]) == est_priority[i]) {
        ret = est_priority[i];
      }
    }
    return ret;
  }

  static int get_valid_est_methods(ObOptimizerContext &ctx,
                                   common::ObIArray<AccessPath*> &paths,
                                   const ObIArray<ObRawExpr*> &filter_exprs,
                                   bool is_inner_path,
                                   ObBaseTableEstMethod &valid_methods,
                                   ObBaseTableEstMethod &hint_specify_methods);

  static int check_can_use_dynamic_sampling(ObOptimizerContext &ctx,
                                            const ObLogPlan &log_plan,
                                            const OptTableMeta &table_meta,
                                            const ObIArray<ObRawExpr*> &filter_exprs,
                                            ObBaseTableEstMethod &valid_methods,
                                            ObBaseTableEstMethod &specify_methods);

  static int choose_best_est_method(ObOptimizerContext &ctx,
                                    common::ObIArray<AccessPath*> &paths,
                                    const ObIArray<ObRawExpr*> &filter_exprs,
                                    const ObBaseTableEstMethod &valid_methods,
                                    ObBaseTableEstMethod& method);

  static int do_estimate_rowcount(ObOptimizerContext &ctx,
                                  common::ObIArray<AccessPath*> &paths,
                                  const ObIArray<ObRawExpr*> &filter_exprs,
                                  ObBaseTableEstMethod &valid_methods,
                                  ObBaseTableEstMethod &method);

  static int process_common_estimate_rowcount(ObOptimizerContext &ctx,
                                              common::ObIArray<AccessPath *> &paths);

  static int64_t get_get_range_count(const ObIArray<ObNewRange> &ranges);

  static int64_t get_scan_range_count(const ObIArray<ObNewRange> &ranges);

  static int check_path_can_use_storage_estimation(const AccessPath *path,
                                                   bool &can_use,
                                                   ObOptimizerContext &ctx);

  static int choose_leader_replica(const ObCandiTabletLoc &part_loc_info,
                                   const bool can_use_remote,
                                   const ObAddr &local_addr,
                                   EstimatedPartition &best_partition);

  static int process_external_table_default_estimation(AccessPath *path);
  static int process_vtable_default_estimation(AccessPath *path);

  static int process_table_force_default_estimation(AccessPath *path);
  static int process_table_default_estimation(ObOptimizerContext &ctx, ObIArray<AccessPath *> &path);

  /// following functions are mainly uesd by statistics estimation
  static int process_statistics_estimation(AccessPath *path);

  static int process_statistics_estimation(ObIArray<AccessPath *> &paths);

  /// following functions are mainly used by storage estimation
  static int process_storage_estimation(ObOptimizerContext &ctx,
                                        ObIArray<AccessPath *> &paths,
                                        bool &is_success);

  static int process_dynamic_sampling_estimation(ObOptimizerContext &ctx,
                                                 ObIArray<AccessPath *> &paths,
                                                 const ObIArray<ObRawExpr*> &filter_exprs,
                                                 bool only_ds_basic_stat,
                                                 bool &is_success);

  static int calc_skip_scan_prefix_ndv(AccessPath &ap, double &prefix_ndv);

  static int get_skip_scan_prefix_exprs(ObIArray<ColumnItem> &column_items,
                                        int64_t skip_scan_offset,
                                        ObIArray<ObRawExpr*> &prefix_exprs);

  static int update_use_skip_scan(ObCostTableScanInfo &est_cost_info,
                                  ObIArray<ObExprSelPair> &all_predicate_sel,
                                  OptSkipScanState &use_skip_scan);

  static int reset_skip_scan_info(ObCostTableScanInfo &est_cost_info,
                                  ObIArray<ObExprSelPair> &all_predicate_sel,
                                  OptSkipScanState &use_skip_scan);

  static int do_storage_estimation(ObOptimizerContext &ctx,
                                   ObBatchEstTasks &tasks);

  static int get_task(ObIArray<ObBatchEstTasks *>& tasks,
                      const ObAddr &addr,
                      ObBatchEstTasks *&task);

  static int create_task(ObIAllocator &allocator,
                         const ObAddr &addr,
                         ObBatchEstTasks *&task);


  static int add_index_info(ObOptimizerContext &ctx,
                            ObIAllocator &allocator,
                            ObBatchEstTasks *task,
                            const EstimatedPartition &part,
                            AccessPath *ap);

  static int construct_scan_range_batch(ObIAllocator &allocator,
                                        const ObIArray<ObNewRange> &scan_ranges,
                                        ObSimpleBatch &batch);

  static int construct_geo_scan_range_batch(ObIAllocator &allocator,
                                            const ObIArray<ObNewRange> &scan_ranges,
                                            ObSimpleBatch &batch);

  static bool is_multi_geo_range(const ObNewRange &range);

  static int estimate_prefix_range_rowcount(
      const obrpc::ObEstPartResElement &result,
      ObCostTableScanInfo &est_cost_info);

  static int fill_cost_table_scan_info(ObCostTableScanInfo &est_cost_info);

  static int get_key_ranges(ObOptimizerContext &ctx,
                            ObIAllocator &allocator,
                            const ObTabletID &tablet_id,
                            AccessPath *ap,
                            ObIArray<common::ObNewRange> &new_ranges);

  static int convert_agent_vt_key_ranges(ObOptimizerContext &ctx,
                                         ObIAllocator &allocator,
                                         AccessPath *ap,
                                         ObIArray<common::ObNewRange> &new_ranges);

  static int gen_agent_vt_table_convert_info(const uint64_t vt_table_id,
                                             const ObTableSchema *vt_table_schema,
                                             const ObTableSchema *real_index_schema,
                                             const ObIArray<ColumnItem> &range_columns,
                                             int64_t &tenant_id_col_idx,
                                             ObIArray<ObObjMeta> &key_types);

  static int convert_physical_rowid_ranges(ObOptimizerContext &ctx,
                                           ObIAllocator &allocator,
                                           const ObTabletID &tablet_id,
                                           const uint64_t index_id,
                                           ObIArray<common::ObNewRange> &new_ranges);
  static int storage_estimate_full_table_rowcount(ObOptimizerContext &ctx,
                                                  const ObCandiTabletLoc &part_loc_info,
                                                  ObTableMetaInfo &meta);

  static int estimate_full_table_rowcount_by_meta_table(ObOptimizerContext &ctx,
                                                        const ObIArray<ObTabletID> &all_tablet_ids,
                                                        const ObIArray<ObLSID> &all_ls_ids,
                                                        ObTableMetaInfo &meta);

  static int get_need_dynamic_sampling_columns(const ObLogPlan* log_plan,
                                               const int64_t table_id,
                                               const ObIArray<ObRawExpr*> &filter_exprs,
                                               const bool need_except_filter,
                                               const bool depend_on_join_filter,
                                               ObIArray<ObRawExpr *> &ds_column_exprs);

  static int update_column_metas_by_ds_col_stat(const int64_t rowcount,
                                                const common::ObOptDSStat::DSColStats &ds_col_stats,
                                                ObIArray<OptColumnMeta> &col_metas);

  static int add_ds_result_items(ObIArray<AccessPath *> &paths,
                                 const ObIArray<ObRawExpr*> &filter_exprs,
                                 const bool specify_ds,
                                 ObIArray<ObDSResultItem> &ds_result_items,
                                 bool only_ds_basic_stat,
                                 bool only_ds_filter);

  static int update_table_stat_info_by_dynamic_sampling(AccessPath *path,
                                                        int64_t ds_level,
                                                        ObIArray<ObDSResultItem> &ds_result_items,
                                                        bool only_ds_filter,
                                                        bool &no_ds_data);
  static int update_table_stat_info_by_default(AccessPath *path);

  static int estimate_path_rowcount_by_dynamic_sampling(const uint64_t table_id,
                                                        ObIArray<AccessPath *> &paths,
                                                        ObIArray<ObDSResultItem> &ds_result_items);
  static int classify_paths(common::ObIArray<AccessPath *> &paths,
                             common::ObIArray<AccessPath *> &normal_paths,
                             common::ObIArray<AccessPath *> &geo_paths);
};

}
}

#endif // OB_ACCESS_PATH_ESTIMATION_H
