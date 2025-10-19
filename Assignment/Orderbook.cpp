#include <cstdint>
#include <unordered_map>
#include <set>
#include <chrono>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

struct Order
{
    uint64_t id;
    bool isBuy;
    double price;
    uint64_t quantity;
    uint64_t timestampInNanoSeconds;

    Order(uint64_t id_, bool isBuy_, double price_, uint64_t quantity_, uint64_t ts_)
        : id(id_), isBuy(isBuy_), price(price_), quantity(quantity_), timestampInNanoSeconds(ts_) {}
};

struct PriceLevel
{
    double price;
    uint64_t totalQuantity;
};

struct BuyComparator
{
    bool operator()(const Order *a, const Order *b) const
    {
        if (a->price != b->price)
            return a->price > b->price;
        return a->timestampInNanoSeconds < b->timestampInNanoSeconds;
    }
};

struct SellComparator
{
    bool operator()(const Order *a, const Order *b) const
    {
        if (a->price != b->price)
            return a->price < b->price;
        return a->timestampInNanoSeconds < b->timestampInNanoSeconds;
    }
};

static uint64_t get_current_timestamp()
{
    return static_cast<uint64_t>(
        chrono::duration_cast<chrono::nanoseconds>(
            chrono::steady_clock::now().time_since_epoch())
            .count());
}

struct OrderBook
{
private:
    unordered_map<uint64_t, Order *> orders;
    set<Order *, BuyComparator> buyOrders;
    set<Order *, SellComparator> sellOrders;

public:
    ~OrderBook()
    {
        for (auto &[id, order] : orders)
            delete order;
    }

    void addOrder(const Order &order)
    {
        Order *o = new Order(order);
        orders[order.id] = o;
        if (order.isBuy)
            buyOrders.insert(o);
        else
            sellOrders.insert(o);
        match();
    }

    bool cancelOrder(uint64_t orderId)
    {
        auto it = orders.find(orderId);
        if (it == orders.end())
        {
            return false;
        }

        Order *order = it->second;
        if (order->isBuy)
            buyOrders.erase(order);
        else
            sellOrders.erase(order);
        delete order;
        orders.erase(it);
        return true;
    }

    bool amendOrder(uint64_t orderId, double newPrice, uint64_t newQuantity)
    {
        auto it = orders.find(orderId);
        if (it == orders.end())
        {
            return false;
        }

        Order *order = it->second;

        if (order->price != newPrice)
        {
            bool isBuy = order->isBuy;
            cancelOrder(orderId);

            Order newOrder(orderId, isBuy, newPrice, newQuantity, get_current_timestamp());
            addOrder(newOrder);
            return true;
        }

        order->quantity = newQuantity;
        match();
        return true;
    }

    template <typename OrderSet>
    void aggregateLevels(const OrderSet &orderSet, size_t depth, vector<PriceLevel> &levels) const
    {
        levels.clear();

        double currentPrice = -1.0;
        uint64_t totalQty = 0;

        for (auto it = orderSet.begin(); it != orderSet.end(); ++it)
        {
            const Order *order = *it;

            if (currentPrice == -1.0)
                currentPrice = order->price;

            if (order->price != currentPrice)
            {
                levels.push_back({currentPrice, totalQty});
                if (levels.size() == depth)
                    return;

                currentPrice = order->price;
                totalQty = 0;
            }

            totalQty += order->quantity;
        }

        if (totalQty > 0 && levels.size() < depth)
            levels.push_back({currentPrice, totalQty});
    }

    void getSnapshot(size_t depth, vector<PriceLevel> &bids, vector<PriceLevel> &asks) const
    {
        aggregateLevels(buyOrders, depth, bids);
        aggregateLevels(sellOrders, depth, asks);
    }

    void printOrderBook(size_t depth = 10) const
    {
        cout << "\n===== ORDER BOOK SNAPSHOT (Top " << depth << " Levels) =====\n";

        // Aggregated levels
        vector<PriceLevel> bids, asks;
        getSnapshot(depth, bids, asks);

        cout << "\n--- SELL SIDE (lowest first) ---\n";
        if (asks.empty())
            cout << "(no sell orders)\n";
        else
        {
            cout << "AGGREGATED LEVELS:\n";
            for (const auto &level : asks)
                cout << "Price: " << level.price << " | Total Qty: " << level.totalQuantity << "\n";

            cout << "\nINDIVIDUAL ORDERS:\n";
            for (const auto &order : sellOrders)
            {
                cout << "ID: " << order->id
                     << " | Price: " << order->price
                     << " | Qty: " << order->quantity
                     << " | Timestamp: " << order->timestampInNanoSeconds << "\n";
            }
        }

        cout << "\n--- BUY SIDE (highest first) ---\n";
        if (bids.empty())
            cout << "(no buy orders)\n";
        else
        {
            cout << "AGGREGATED LEVELS:\n";
            for (const auto &level : bids)
                cout << "Price: " << level.price << " | Total Qty: " << level.totalQuantity << "\n";

            cout << "\nINDIVIDUAL ORDERS:\n";
            for (const auto &order : buyOrders)
            {
                cout << "ID: " << order->id
                    << " | Price: " << order->price
                    << " | Qty: " << order->quantity
                    << " | Timestamp: " << order->timestampInNanoSeconds << "\n";
            }
        }

        cout << "=================================\n";
    }

    void match()
    {
        while (!buyOrders.empty() && !sellOrders.empty())
        {
            Order *bestBuy = *buyOrders.begin();
            Order *bestSell = *sellOrders.begin();

            // No match possible
            if (bestBuy->price < bestSell->price)
                break;

            uint64_t tradedQty = min(bestBuy->quantity, bestSell->quantity);
            double tradePrice = bestSell->price; // Usually the maker's price

            cout << "TRADE: "
                 << "BuyID=" << bestBuy->id
                 << " SellID=" << bestSell->id
                 << " Price=" << tradePrice
                 << " Qty=" << tradedQty << "\n";

            // Reduce quantities
            bestBuy->quantity -= tradedQty;
            bestSell->quantity -= tradedQty;

            // Remove fully filled orders
            if (bestBuy->quantity == 0)
            {
                buyOrders.erase(buyOrders.begin());
                orders.erase(bestBuy->id);
                delete bestBuy;
            }

            if (bestSell->quantity == 0)
            {
                sellOrders.erase(sellOrders.begin());
                orders.erase(bestSell->id);
                delete bestSell;
            }
        }
    }
};
