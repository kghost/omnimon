# Curses Layout Module

This module provides a flexible, hierarchical UI layout system for terminal applications based on `ncurses`. It is designed to simplify the creation of complex terminal interfaces by providing a set of reusable components and automatic layout management.

## Core Concepts

### 1. View
The `View` class is the fundamental building block of the UI. Every UI element (text boxes, tables, containers) inherits from `View`.
- **Region**: Each view has a `Region` (offset and size) that defines its position and dimensions on the screen.
- **Visibility**: Views can be shown or hidden, which also affects their layout in containers.
- **Input Handling**: Views implement `InputHandler` to process key events via the `OnKey` method.

### 2. Container
`Container` is a special `View` that manages multiple child views. It acts like a simplified "Flexbox" for terminal interfaces.
- **Growth Type**: Containers can grow either **Vertically** or **Horizontally**.
- **Child Arrangement**: Children can be placed using different arrangement types:
    - `Forward`: Placed at the start of the container (top/left).
    - `Backward`: Placed at the end of the container (bottom/right).
    - `FillRest`: Expands to fill all remaining space between forward and backward children.
- **Layout Calculation**: The container automatically calculates the position and size of its children based on their requested size and margins.

### 3. Rendering Model
The module uses a **two-stage drawing process** to ensure clean updates:
1. **DrawPrepare**: In this stage, views clear their previous content or remove old elements. Containers propagate this call to all children, including those marked for deletion.
2. **DrawContent**: In this stage, the actual current content is rendered to the terminal.

### 4. Data Binding
The `ViewDataBinding` class allows views to be decoupled from the underlying data. A view can be bound to a data source, and it will use that binding to retrieve information to display.

## Key Components

- **`View`**: Base class for UI components.
- **`Container`**: Layout manager for child views.
- **`Region` & `Layout`**: Structs for managing position and size.
- **`TextView`**: A component for displaying text content.
- **`AttrView`**: A component for displaying key-value pairs or attributes.
- **`Table`**: A complex component for rendering tabular data.
- **`Curses`**: The main entry point that wraps `ncurses`, manages the root view, and handles window resize events.

## Usage Overview

1.  **Initialize Curses**: Create a `Curses` object.
2.  **Define Layout**: Build a tree of `Container` and `View` objects.
3.  **Set Root**: Assign the top-level container as the root of the `Curses` object.
4.  **Run Loop**: The `Curses` object integrates with an event loop to handle input and schedule redraws.

```cpp
auto root = std::make_shared<Container>(Container::GrowthType::Vertical);
auto header = std::make_shared<TextView>();
root->AppendChild(std::make_shared<FixedChild>(header, 1, ChildArrangement::ArrangementType::Forward));

curses.SetRoot(root);
```
