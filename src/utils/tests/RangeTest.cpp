#include <ranges>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../Range.hpp"

namespace utils::tests {

namespace {

struct Animal {
  virtual ~Animal() = default;
  virtual std::string Speak() const = 0;
};

struct Dog : public Animal {
  std::string Speak() const override {
    return "Woof";
  }
};

struct Cat : public Animal {
  std::string Speak() const override {
    return "Meow";
  }
};

} // namespace

TEST(RangeTest, ConcatRange) {
  std::vector<int> v1 = {1, 2, 3};
  std::vector<int> v2 = {4, 5};

  auto concatView = utils::concat<int&>(v1, v2);
  int sum = 0;
  for (int x : concatView) {
    sum += x;
  }
  EXPECT_EQ(sum, 15);

  // Test empty ranges
  std::vector<int> empty;
  auto concatEmpty = utils::concat<int&>(empty, empty);
  EXPECT_EQ(concatEmpty.begin(), concatEmpty.end());
}

TEST(RangeTest, AnyViewBasic) {
  std::vector<int> nums = {10, 20, 30};
  utils::AnyView<int&> view(nums);

  auto it = view.begin();
  EXPECT_EQ(*it, 10);
  ++it;
  EXPECT_EQ(*it, 20);
  it++;
  EXPECT_EQ(*it, 30);
  ++it;
  EXPECT_EQ(it, view.end());
}

TEST(RangeTest, AnyViewCopyAndMove) {
  std::vector<int> nums = {1, 2, 3};
  utils::AnyView<int&> view1(nums);

  // Copy
  utils::AnyView<int&> view2 = view1;
  int sum1 = 0;
  for (int x : view2) {
    sum1 += x;
  }
  EXPECT_EQ(sum1, 6);

  // Move
  utils::AnyView<int&> view3 = std::move(view2);
  int sum2 = 0;
  for (int x : view3) {
    sum2 += x;
  }
  EXPECT_EQ(sum2, 6);
}

TEST(RangeTest, AnyViewPolymorphism) {
  std::vector<std::unique_ptr<Animal>> animals;
  animals.push_back(std::make_unique<Dog>());
  animals.push_back(std::make_unique<Cat>());

  utils::AnyView<std::unique_ptr<Animal>&> view(animals);

  auto it = view.begin();
  EXPECT_EQ((*it)->Speak(), "Woof");
  ++it;
  EXPECT_EQ((*it)->Speak(), "Meow");
  ++it;
  EXPECT_EQ(it, view.end());
}

TEST(RangeTest, AnyViewConstCorrectness) {
  const std::vector<int> nums = {100, 200};
  utils::AnyView<const int&> view(nums);

  int sum = 0;
  for (const int& x : view) {
    sum += x;
  }
  EXPECT_EQ(sum, 300);
}

TEST(RangeTest, AnyViewPipelining) {
  std::vector<int> nums = {1, 2, 3, 4, 5, 6};
  utils::AnyView<int&> view(nums);

  auto filtered =
      view | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::transform([](int x) { return x * 10; });

  auto it = filtered.begin();
  EXPECT_EQ(*it, 20);
  ++it;
  EXPECT_EQ(*it, 40);
  ++it;
  EXPECT_EQ(*it, 60);
  ++it;
  EXPECT_EQ(it, filtered.end());
}

} // namespace utils::tests
