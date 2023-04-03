#pragma once

#include <memory>
#include <vector>

#include "View.hpp"

namespace frontend::curses {

class Container : public View {
public:
  enum class GrowthType { TopDown, LeftRight };
  enum class ArrangementType { Forward, Backward, FillRest };

  class Context {
  public:
    explicit Context() = default;
    virtual ~Context() = default;

    virtual ArrangementType GetArrangement() const = 0;
    virtual DisplayLength GetSize() const = 0;
    virtual DisplayLength GetMarginBefore() const { return 0; }
    virtual DisplayLength GetMarginAfter() const { return 0; }
  };

  class SimpleContext : public Context {
  public:
    explicit SimpleContext(ArrangementType arrangement, DisplayLength size, DisplayLength marginBefore = 0,
                           DisplayLength marginAfter = 0)
        : Arrangement(arrangement), Size(size), MarginBefore(marginBefore), MarginAfter(marginAfter) {}
    virtual ~SimpleContext() = default;

    virtual ArrangementType GetArrangement() const { return Arrangement; }
    virtual DisplayLength GetSize() const { return Size; }
    virtual DisplayLength GetMarginBefore() const { return MarginBefore; }
    virtual DisplayLength GetMarginAfter() const { return MarginAfter; }

  private:
    ArrangementType Arrangement;
    DisplayLength Size;
    DisplayLength MarginBefore;
    DisplayLength MarginAfter;
  };

  explicit Container(GrowthType growth) : View(), Growth(growth) {}
  ~Container() override {}

  bool SetLayout(Layout offset, Layout layout) override;

  bool OnKey(TermKeyCode key) override;
  void DrawPrepare(const UpdateContext& attrs) override;
  void DrawContent(const UpdateContext& attrs) override;

  void AppendChild(std::shared_ptr<View> view, std::shared_ptr<const Context> context);
  void CalculateLayout();
  void MakeSimilarTo(const Container& other);

private:
  DisplayLength& ParrelGrowth(Layout& layout) { return Growth == GrowthType::TopDown ? layout.Height : layout.Width; }
  DisplayLength& PerpGrowth(Layout& layout) { return Growth == GrowthType::TopDown ? layout.Width : layout.Height; }

  void CalculateChildLayout(std::shared_ptr<View> view, std::shared_ptr<const Context> context);

  // Data members
public:
  const GrowthType Growth;

private:
  class Child {
  public:
    explicit Child(std::shared_ptr<View> view, std::shared_ptr<const Context> context)
        : _View(view), _Context(context) {}

    std::shared_ptr<View> _View;
    std::shared_ptr<const Context> _Context;
  };

  std::vector<Child> _Children;
  DisplayLength _AllcatedForward = 0;
  DisplayLength _AllcatedBackward = 0;
};

} // namespace frontend::curses
