// Example program showing passing of slots through an interface.
//
// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2009.
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// For more information, see http://www.boost.org

#include <iostream>
#include <boost/signals2/signal.hpp>

// a pretend GUI button
class Button
{
  typedef boost::signals2::signal<void (int x, int y)> OnClick;
public:
  typedef OnClick::slot_type OnClickSlotType;
  // forward slots through Button interface to its private signal
  boost::signals2::connection doOnClick(const OnClickSlotType & slot)
  {
    return onClick.connect(slot);
  }

  // simulate user clicking on GUI button at coordinates 52, 38
  void simulateClick()
  {
    onClick(52, 38);
  }
private:
  OnClick onClick;
};

void printCoordinates(long x, long y)
{
  std::cout << "(" << x << ", " << y << ")\n";
}

int main()
{
  Button button;
  button.doOnClick(&printCoordinates);
  button.simulateClick();
  return 0;
}
