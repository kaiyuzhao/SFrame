#ifndef _CANVAS_STREAMING_TRANSFORMATION
#define _CANVAS_STREAMING_TRANSFORMATION

#include <parallel/lambda_omp.hpp>
#include <unity/lib/toolkit_class_macros.hpp>
#include <unity/lib/toolkit_function_macros.hpp>

#define TRANSFORMATION_REGISTRATION(TRANSFORMER) \
    REGISTER_CLASS_MEMBER_FUNCTION(TRANSFORMER::eof) \
    REGISTER_CLASS_MEMBER_FUNCTION(TRANSFORMER::get) \
    REGISTER_CLASS_MEMBER_FUNCTION(TRANSFORMER::init, "source") \
    REGISTER_GETTER("rows_processed", TRANSFORMER::get_rows_processed) \

namespace graphlab {
namespace _canvas {
namespace streaming {

template<typename InputIterable,
         typename Output,
         typename Transformer,
         size_t BATCH_SIZE>
class transformation : public toolkit_class_base {
  protected:
    InputIterable m_source;
    Transformer m_transformer;
    size_t m_currentIdx = 0;
    bool m_initialized = false;

  private:
    void check_init(const char * msg, bool initialized) const {
      if (initialized != m_initialized) {
        log_and_throw(msg);
      }
    }
    void require_init() const {
      check_init("Transformer must be initialized before performing this operation.", true);
    }

  protected:
    /* Subclasses may override: */
    /* Get the current result (without iterating over any new values) */
    virtual Output get_current() {
      return m_transformer;
    }
    /* Create multiple transformers from input */
    virtual std::vector<Transformer> split_input(size_t num_threads) {
      return std::vector<Transformer>(num_threads);
    }
    /* Merge multiple transformers into output */
    virtual void merge_results(std::vector<Transformer>& transformers) = 0;

  public:
    virtual void init(const InputIterable& source) {
      check_init("Transformer is already initialized.", false);
      m_source = source;
      m_currentIdx = 0;
      m_initialized = true;
    }
    virtual bool eof() const {
      require_init();
      DASSERT_LE(m_currentIdx, m_source.size());
      return m_currentIdx == m_source.size();
    }
    virtual flex_int get_rows_processed() const {
      require_init();
      DASSERT_LE(m_currentIdx, m_source.size());
      return m_currentIdx;
    }

    virtual Output get() {
      require_init();
      if (this->eof()) {
        // bail out, done streaming
        return this->get_current();
      }

      const size_t num_threads_reported = thread_pool::get_instance().size();
      const size_t start = m_currentIdx;
      const size_t input_size = std::min(BATCH_SIZE, m_source.size() - m_currentIdx);
      const size_t end = start + input_size;
      auto transformers = this->split_input(num_threads_reported);
      const auto& source = this->m_source;
      in_parallel(
        [&transformers, &source, input_size, start]
        (size_t thread_idx, size_t num_threads) {

        DASSERT_LE(transformers.size(), num_threads);
        if (thread_idx >= transformers.size()) {
          // this operation isn't parallel enough to use all threads.
          // bail out on this thread.
          return;
        }

        auto& transformer = transformers[thread_idx];
        size_t thread_input_size = input_size / transformers.size();
        size_t thread_start = start + (thread_idx * thread_input_size);
        size_t thread_end = thread_idx == transformers.size() - 1 ?
          start + input_size :
          thread_start + thread_input_size;
        DASSERT_LE(thread_end, start + input_size);
        for (const auto& value : source.range_iterator(thread_start, thread_end)) {
          transformer.update(value);
        }
      });

      this->merge_results(transformers);
      m_currentIdx = end;

      return this->get_current();
    }
};

}}}

#endif
