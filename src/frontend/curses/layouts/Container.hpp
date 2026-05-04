#pragma once

#include <memory>
#include <ranges>
#include <span>
#include <vector>

#include "View.hpp"

namespace frontend::curses {

class Container : public View {
public:
  enum class GrowthType { Vertical, Horizontal };
  class ChildArrangement {
  public:
    enum class ArrangementType { Forward, Backward, FillRest };

    explicit ChildArrangement(ArrangementType type, DisplayLength marginBefore, DisplayLength marginAfter)
        : _Arrangement(type), _MarginBefore(marginBefore), _MarginAfter(marginAfter) {}
    ~ChildArrangement() = default;

    ArrangementType GetArrangement() const { return _Arrangement; }
    DisplayLength GetMarginBefore() const { return _MarginBefore; }
    DisplayLength GetMarginAfter() const { return _MarginAfter; }

  private:
    ArrangementType _Arrangement;
    DisplayLength _MarginBefore;
    DisplayLength _MarginAfter;
  };

  class Child {
  public:
    virtual ~Child() = default;
    virtual ChildArrangement::ArrangementType GetArrangement() const = 0;
    virtual DisplayLength GetSize() const = 0;
    virtual DisplayLength GetMarginBefore() const = 0;
    virtual DisplayLength GetMarginAfter() const = 0;
    virtual View& GetView() = 0;

    virtual void DrawPrepare(const UpdateContext& attrs) { GetView().DrawPrepare(attrs); }

    // This child will be removed at container's DrawPrepare
    void MarkForDeletion() { _MarkForDeletion = true; }
    bool IsMarkForDeletion() const { return _MarkForDeletion; }

  private:
    bool _MarkForDeletion = false;
  };

  explicit Container(GrowthType growth) : View(), _Growth(growth) {}
  ~Container() override {}

  auto GetChildren() {
    return std::span(_Children) | std::views::filter([](auto child) { return !child->IsMarkForDeletion(); });
  }

  bool SetLayout(Layout offset, Layout layout) override;

  bool OnKey(TermKeyCode key) override;
  void DrawPrepare(const UpdateContext& attrs) override;
  void DrawContent(const UpdateContext& attrs) override;

  void AppendChild(std::shared_ptr<Child> view);
  void CalculateLayout();
  void MakeSimilarTo(Container& other);

  static DisplayLength& ParrelGrowth(GrowthType growth, Layout& layout) {
    return growth == GrowthType::Vertical ? layout.Height : layout.Width;
  }
  static DisplayLength& PerpGrowth(GrowthType growth, Layout& layout) {
    return growth == GrowthType::Vertical ? layout.Width : layout.Height;
  }

  static DisplayLength ParrelGrowth(GrowthType growth, const Layout& layout) {
    return growth == GrowthType::Vertical ? layout.Height : layout.Width;
  }
  static DisplayLength PerpGrowth(GrowthType growth, const Layout& layout) {
    return growth == GrowthType::Vertical ? layout.Width : layout.Height;
  }

private:
  void CalculateChildLayout(std::shared_ptr<Child> view);

  // Data members
public:
  const GrowthType _Growth;

protected:
  std::vector<std::shared_ptr<Child>> _Children;

private:
  DisplayLength _AllcatedForward = 0;
  DisplayLength _AllcatedBackward = 0;
};

} // namespace frontend::curses
