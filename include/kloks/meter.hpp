#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace kloks {
struct MusicalPosition {
  std::uint64_t bar;
  std::uint32_t beat;
  std::uint32_t tick;
};
class Meter {
public:
  Meter(std::uint32_t numerator, std::uint32_t denominator, std::uint32_t ppqn)
      : numerator_(numerator), denominator_(denominator), ppqn_(ppqn) {
    if (numerator == 0 || ppqn == 0 || denominator == 0 ||
        (denominator & (denominator - 1)) != 0)
      throw std::invalid_argument("invalid meter or PPQN");
    const std::uint64_t quarter_ticks = static_cast<std::uint64_t>(ppqn) * 4;
    if (quarter_ticks % denominator != 0 || quarter_ticks / denominator == 0 ||
        quarter_ticks / denominator > std::numeric_limits<std::uint32_t>::max())
      throw std::invalid_argument("meter beat cannot be represented in ticks");
    ticks_per_beat_ = static_cast<std::uint32_t>(quarter_ticks / denominator);
  }
  std::uint32_t numerator() const noexcept { return numerator_; }
  std::uint32_t denominator() const noexcept { return denominator_; }
  std::uint32_t ppqn() const noexcept { return ppqn_; }
  std::uint32_t ticks_per_beat() const noexcept { return ticks_per_beat_; }
  MusicalPosition locate(std::uint64_t ticks) const noexcept {
    const auto elapsed_beats = ticks / ticks_per_beat_;
    return {elapsed_beats / numerator_,
            static_cast<std::uint32_t>(elapsed_beats % numerator_),
            static_cast<std::uint32_t>(ticks % ticks_per_beat_)};
  }
  // Inverse of locate(): reject noncanonical positions and overflow.
  std::uint64_t to_ticks(MusicalPosition position) const {
    if (position.beat >= numerator_ || position.tick >= ticks_per_beat_)
      throw std::invalid_argument("position outside meter");
    const auto max = std::numeric_limits<std::uint64_t>::max();
    if (position.bar > max / numerator_)
      throw std::overflow_error("musical position overflow");
    const auto beats = position.bar * numerator_ + position.beat;
    if (beats < position.beat || beats > max / ticks_per_beat_)
      throw std::overflow_error("musical position overflow");
    const auto base = beats * ticks_per_beat_;
    if (position.tick > max - base)
      throw std::overflow_error("musical position overflow");
    return base + position.tick;
  }
private:
  std::uint32_t numerator_, denominator_, ppqn_, ticks_per_beat_ = 0;
};
} // namespace kloks
