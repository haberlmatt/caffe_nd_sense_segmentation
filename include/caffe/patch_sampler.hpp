#ifndef CAFFE_PATCH_SAMPLER_HPP_
#define CAFFE_DATA_SAMPLER_HPP_

#include <map>
#include <string>
#include <vector>
#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/internal_thread.hpp"
#include "caffe/util/blocking_queue.hpp"
#include "caffe/util/db.hpp"

namespace caffe {

  template <typename Dtype>
  class Batch_data {
   public:
    Blob<Dtype> data_;
    Blob<Dtype> label_;
  };
 template <typename Dtype>
  class QueuePair_Batch {
   public:
    explicit QueuePair_Batch(const LayerParameter& param);
    ~QueuePair_Batch();

    BlockingQueue<Batch_data<Dtype>*> free_;
    BlockingQueue<Batch_data<Dtype>*> full_;

  DISABLE_COPY_AND_ASSIGN(QueuePair_Batch);
  };
  // A single body is created per source
  template <typename Dtype>
  class Runner : public InternalThread {
   public:
    explicit Runner(const LayerParameter& param);
    virtual ~Runner();

   protected:
    void InternalThreadEntry();
    //void read_one(db::Cursor* cursor, QueuePair* qp);

    const LayerParameter param_;
    BlockingQueue<shared_ptr<QueuePair_Batch<Dtype> > > new_queue_pairs_;

    friend class PatchSampler;

  DISABLE_COPY_AND_ASSIGN(Runner);
  };
/**
 * @brief warp patches from a data_reader_general to queues available to PatchSamplerLayer layers.
 * A single reading thread is created per source, even if multiple solvers
 * are running in parallel, e.g. for multi-GPU training. This makes sure
 * databases are read sequentially, and that each solver accesses a different
 * subset of the database. Data is distributed to solvers in a round-robin
 * way to keep parallel training deterministic.
 */
template <typename Dtype>
class PatchSampler {
 public:
  explicit PatchSampler(const LayerParameter& param);
  ~PatchSampler();

  inline BlockingQueue<Blob<Dtype>*>& free() const {
    return queue_pair_->free_;
  }
  inline BlockingQueue<Blob<Dtype>*>& full() const {
    return queue_pair_->full_;
  }

 protected:

  // Queue pairs are shared between a runner and its readers
  //template <typename Dtype>



  // A source is uniquely identified by its layer name + path, in case
  // the same database is read from two different locations in the net.
  static inline string source_key(const LayerParameter& param) {
    return param.name() + ":" + param.data_param().source();
  }

  const shared_ptr<QueuePair_Batch<Dtype> > queue_pair_;
  shared_ptr<Runner<Dtype> > runner_;

  static map<const string, boost::weak_ptr<Runner<Dtype> > > runners_;

DISABLE_COPY_AND_ASSIGN(PatchSampler);
};

}  // namespace caffe

#endif  // CAFFE_DATA_READER_HPP_
