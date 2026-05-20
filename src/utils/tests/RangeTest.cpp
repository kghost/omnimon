#include "../Range.hpp"

#include <cassert>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

struct Animal {
  virtual ~Animal() = default;
  virtual std::string Speak() const = 0;
};

struct Dog : public Animal {
  std::string Speak() const override { return "Woof"; }
};

struct Cat : public Animal {
  std::string Speak() const override { return "Meow"; }
};

void TestConcatRange() {
  std::vector<int> v1 = {1, 2, 3};
  std::vector<int> v2 = {4, 5};

  auto concatView = utils::concat<int&>(v1, v2);
  int sum = 0;
  for (int x : concatView) {
    sum += x;
  }
  assert(sum == 15);

  // Test empty ranges
  std::vector<int> empty;
  auto concatEmpty = utils::concat<int&>(empty, empty);
  assert(concatEmpty.begin() == concatEmpty.end());

  std::cout << "TestConcatRange passed." << std::endl;
}

void TestAnyViewBasic() {
  std::vector<int> nums = {10, 20, 30};
  utils::AnyView<int&> view(nums);

  auto it = view.begin();
  assert(*it == 10);
  ++it;
  assert(*it == 20);
  it++;
  assert(*it == 30);
  ++it;
  assert(it == view.end());

  std::cout << "TestAnyViewBasic passed." << std::endl;
}

void TestAnyViewCopyAndMove() {
  std::vector<int> nums = {1, 2, 3};
  utils::AnyView<int&> view1(nums);

  // Copy
  utils::AnyView<int&> view2 = view1;
  int sum1 = 0;
  for (int x : view2) {
    sum1 += x;
  }
  assert(sum1 == 6);

  // Move
  utils::AnyView<int&> view3 = std::move(view2);
  int sum2 = 0;
  for (int x : view3) {
    sum2 += x;
  }
  assert(sum2 == 6);

  std::cout << "TestAnyViewCopyAndMove passed." << std::endl;
}

void TestAnyViewPolymorphism() {
  std::vector<std::unique_ptr<Animal>> animals;
  animals.push_back(std::make_unique<Dog>());
  animals.push_back(std::make_unique<Cat>());

  utils::AnyView<std::unique_ptr<Animal>&> view(animals);

  auto it = view.begin();
  assert((*it)->Speak() == "Woof");
  ++it;
  assert((*it)->Speak() == "Meow");
  ++it;
  assert(it == view.end());

  std::cout << "TestAnyViewPolymorphism passed." << std::endl;
}

void TestAnyViewConstCorrectness() {
  const std::vector<int> nums = {100, 200};
  utils::AnyView<const int&> view(nums);

  int sum = 0;
  for (const int& x : view) {
    sum += x;
  }
  assert(sum == 300);

  std::cout << "TestAnyViewConstCorrectness passed." << std::endl;
}

void TestAnyViewPipelining() {
  std::vector<int> nums = {1, 2, 3, 4, 5, 6};
  utils::AnyView<int&> view(nums);

  auto filtered =
      view | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::transform([](int x) { return x * 10; });

  auto it = filtered.begin();
  assert(*it == 20);
  ++it;
  assert(*it == 40);
  ++it;
  assert(*it == 60);
  ++it;
  assert(it == filtered.end());

  std::cout << "TestAnyViewPipelining passed." << std::endl;
}

int main() {
  std::cout << "Running Range.hpp tests..." << std::endl;

  TestConcatRange();
  TestAnyViewBasic();
  TestAnyViewCopyAndMove();
  TestAnyViewPolymorphism();
  TestAnyViewConstCorrectness();
  TestAnyViewPipelining();

  std::cout << "All Range.hpp tests passed successfully!" << std::endl;
  return 0;
}
