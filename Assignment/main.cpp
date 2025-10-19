#include "Orderbook.cpp"
#include <thread>
#include <chrono>

int main()
{
    OrderBook ob;

    cout << "\n=== Adding orders ===\n";
    ob.addOrder(Order(1, true, 101.0, 100, get_current_timestamp()));
    ob.addOrder(Order(2, true, 100.0, 50, get_current_timestamp()));
    ob.addOrder(Order(3, false, 102.0, 70, get_current_timestamp()));
    ob.addOrder(Order(4, false, 103.0, 30, get_current_timestamp()));
    ob.addOrder(Order(5, true, 101.0, 150, get_current_timestamp()));

    ob.printOrderBook(5);

    cout << "\n=== Amending order #2 (quantity 200) ===\n";
    ob.amendOrder(2, 100.0, 200);
    ob.printOrderBook(5);

    cout << "\n=== Canceling order #3 ===\n";
    ob.cancelOrder(3);
    ob.printOrderBook(5);

    cout << "\n=== Adding more orders ===\n";
    ob.addOrder(Order(6, true, 99.0, 80, get_current_timestamp()));
    ob.addOrder(Order(7, false, 104.0, 20, get_current_timestamp()));
    ob.addOrder(Order(8, false, 102.0, 10, get_current_timestamp()));
    ob.printOrderBook(5);

    cout << "\n=== Matching orders ===\n";
    ob.match();
    ob.printOrderBook(5);

    return 0;
}
