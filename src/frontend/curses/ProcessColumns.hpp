#pragma once

#include <ftxui/dom/table.hpp>
#include <memory>
#include <string>

#include "ProcessTree.hpp"

namespace frontend::ftxui {

class ColumnCursor : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnPid : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnState : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnUser : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnCpu : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnMem : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnTime : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnDiskRead : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnDiskWrite : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnDiskAccumulated : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnIO : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnIOAccumulated : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnStart : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;
};

class ColumnCommand : public ProcessTree::Column {
public:
  std::string GetHeaderText() const override;
  void RegisterRow(ProcessTree::Row& row) const override;
  std::string GetDataText(bool isRowSelected, bool isColumnSelected, ProcessTree::Row& row) const override;
  void Decorate(::ftxui::TableSelection selection) const override;

private:
  static std::string TreeString(std::shared_ptr<Process> process);
  static std::string FormatCommand(const std::string& command);
};

} // namespace frontend::ftxui
