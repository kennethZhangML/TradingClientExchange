#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QWidget>
#include <QTimer>
#include <QTableWidgetItem>
#include <random>
#include "EngineRunner.hpp"

class TradingUI : public QMainWindow {
    Q_OBJECT
public:
    TradingUI(QWidget* parent = nullptr)
        : QMainWindow(parent), runner_(), rng_(std::random_device{}()) {
        auto* central = new QWidget;
        auto* mainLayout = new QVBoxLayout;

        auto* inputRow = new QHBoxLayout;
        symbolInput_ = new QLineEdit; symbolInput_->setPlaceholderText("Symbol");
        sideInput_   = new QLineEdit; sideInput_->setPlaceholderText("0=Buy,1=Sell");
        typeInput_   = new QLineEdit; typeInput_->setPlaceholderText("0=Limit,1=Market");
        priceInput_  = new QLineEdit; priceInput_->setPlaceholderText("Price");
        qtyInput_    = new QLineEdit; qtyInput_->setPlaceholderText("Qty");
        inputRow->addWidget(symbolInput_);
        inputRow->addWidget(sideInput_);
        inputRow->addWidget(typeInput_);
        inputRow->addWidget(priceInput_);
        inputRow->addWidget(qtyInput_);

        auto* btnRow = new QHBoxLayout;
        auto* newBtn    = new QPushButton("New Order");
        auto* cancelBtn = new QPushButton("Cancel Order");
        auto* modBtn    = new QPushButton("Modify Order");
        orderIdInput_   = new QLineEdit; orderIdInput_->setPlaceholderText("Order ID");
        btnRow->addWidget(newBtn);
        btnRow->addWidget(cancelBtn);
        btnRow->addWidget(modBtn);
        btnRow->addWidget(orderIdInput_);

        tabs_ = new QTabWidget;
        auto* tobWidget = new QWidget;
        auto* tobLayout = new QVBoxLayout;
        tobTable_ = new QTableWidget(0,5);
        tobTable_->setHorizontalHeaderLabels({"Symbol","BidQty","BidPx","AskQty","AskPx"});
        tobTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tobLayout->addWidget(tobTable_);
        tobWidget->setLayout(tobLayout);
        tabs_->addTab(tobWidget, "Top of Book");

        auto* histWidget = new QWidget;
        auto* histLayout = new QVBoxLayout;
        historyLog_ = new QTextEdit;
        historyLog_->setReadOnly(true);
        QFont monoFont("Courier");
        historyLog_->setFont(monoFont);
        histLayout->addWidget(historyLog_);
        histWidget->setLayout(histLayout);
        tabs_->addTab(histWidget, "Trade History");

        mainLayout->addLayout(inputRow);
        mainLayout->addLayout(btnRow);
        mainLayout->addWidget(tabs_);
        central->setLayout(mainLayout);
        setCentralWidget(central);
        setWindowTitle("TradingClientExchange UI");
        resize(900, 600);

        connect(newBtn, &QPushButton::clicked, this, &TradingUI::onNewOrder);
        connect(cancelBtn, &QPushButton::clicked, this, &TradingUI::onCancelOrder);
        connect(modBtn, &QPushButton::clicked, this, &TradingUI::onModifyOrder);
        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TradingUI::processEvents);
        timer->start(100);

        simulateRandomHistory(100);
    }

    ~TradingUI() { runner_.stop(); }

private slots:
    void onNewOrder() {
        auto sym = symbolInput_->text().toStdString();
        bool ok;
        int side = sideInput_->text().toInt(&ok);
        int type = typeInput_->text().toInt(&ok);
        double px = priceInput_->text().toDouble(&ok);
        int qty   = qtyInput_->text().toInt(&ok);
        runner_.push(NewOrderMsg{ std::make_shared<Order>(sym,
            side==0?OrderSide::BUY:OrderSide::SELL,
            type==0?OrderType::LIMIT:OrderType::MARKET,
            px, qty) });
    }

    void onCancelOrder() {
        int id = orderIdInput_->text().toInt();
        runner_.push(CancelMsg{id});
    }

    void onModifyOrder() {
        int id = orderIdInput_->text().toInt();
        std::optional<double> px;
        std::optional<int> qty;
        bool ok;
        if (!priceInput_->text().isEmpty()) px = priceInput_->text().toDouble(&ok);
        if (!qtyInput_->text().isEmpty())   qty = qtyInput_->text().toInt(&ok);
        runner_.push(ModifyMsg{id, px, qty});
    }

    void processEvents() {
        OutboundMsg evt;
        while (runner_.poll(evt)) {
            std::visit([&](auto&& e){
                using T = std::decay_t<decltype(e)>;
                if constexpr (std::is_same_v<T, TradeEvent>) {
                    historyLog_->append(QString::fromStdString(
                        e.fill.symbol + " " + std::to_string(e.fill.qty) +
                        " @ " + std::to_string(e.fill.price)));
                } else if constexpr (std::is_same_v<T, TopOfBookEvt>) {
                    int row = findOrAddRow(e.symbol);
                    tobTable_->setItem(row, 1, new QTableWidgetItem(QString::number(e.bidQty)));
                    tobTable_->setItem(row, 2, new QTableWidgetItem(QString::number(e.bidPx)));
                    tobTable_->setItem(row, 3, new QTableWidgetItem(QString::number(e.askQty)));
                    tobTable_->setItem(row, 4, new QTableWidgetItem(QString::number(e.askPx)));
                }
            }, evt);
        }
    }

private:
    int findOrAddRow(const std::string& sym) {
        for (int r=0; r<tobTable_->rowCount(); ++r) {
            if (tobTable_->item(r,0)->text().toStdString() == sym)
                return r;
        }
        int newRow = tobTable_->rowCount();
        tobTable_->insertRow(newRow);
        tobTable_->setItem(newRow,0, new QTableWidgetItem(QString::fromStdString(sym)));
        return newRow;
    }

    void simulateRandomHistory(int n) {
        std::vector<std::string> syms = {"AAPL","MSFT","GOOGL","TSLA","AMZN"};
        std::uniform_int_distribution<int> symd(0, syms.size()-1);
        std::uniform_int_distribution<int> qtd(1, 100);
        std::uniform_real_distribution<double> prd(10.0, 3000.0);
        for (int i = 0; i < n; ++i) {
            auto s = syms[symd(rng_)];
            int q = qtd(rng_);
            double p = std::round(prd(rng_)*100)/100;
            historyLog_->append(QString::fromStdString(s + " " + std::to_string(q) + " @ " + std::to_string(p)));
        }
    }

    EngineRunner runner_;
    QLineEdit* symbolInput_;
    QLineEdit* sideInput_;
    QLineEdit* typeInput_;
    QLineEdit* priceInput_;
    QLineEdit* qtyInput_;
    QLineEdit* orderIdInput_;
    QTabWidget* tabs_;
    QTableWidget* tobTable_;
    QTextEdit* historyLog_;
    std::mt19937 rng_;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    TradingUI win;
    win.show();
    return app.exec();
}

#include "main.moc" // ensure automoc generates this file
