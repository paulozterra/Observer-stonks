#include <iostream>
#include <unordered_map>
#include <forward_list>
#include <string>
#include <memory>
#include <vector>
using namespace std;

class ObserverInterface;
class SubjectInterface;
class WeatherStation;
class DisplayDevice;

class ObserverInterface {
public:
    ObserverInterface() = default;
    virtual void update(weak_ptr<SubjectInterface> s) = 0;
};

class SubjectInterface : public std::enable_shared_from_this<SubjectInterface> {
protected:
    forward_list<shared_ptr<ObserverInterface>> observers;
public:
    virtual void registerObserver(shared_ptr<ObserverInterface> observer) {
        observers.push_front(observer);
    };
    virtual void registerMultipleObserver(const vector<shared_ptr<ObserverInterface>>& observers) {
        for (auto &obs : observers) {
            registerObserver(obs);
        }
    };
    virtual void removeObserver(shared_ptr<ObserverInterface> observer) {
        observers.remove(observer);
    };
    virtual void removeMultipleObserver(const vector<shared_ptr<ObserverInterface>>& observers) {
        for (auto &obs : observers) {
            removeObserver(obs);
        }
    };
    virtual void notifyObserver() {
        for (auto &obs : observers) {
            obs->update(shared_from_this()); //( shared_ptr<SubjectInterface>(this) ); // https://stackoverflow.com/questions/11711034/stdshared-ptr-of-this
        }
    };

};

class WeatherStation : public SubjectInterface {
private:
    double humidity = 0;
    double temperature = 0;
    double pressure = 0;
public:
    WeatherStation() = default;
    [[nodiscard]] double getHumidity() const {return humidity;}
    void setHumidity(double h) {
        humidity=h;
        notifyObserver();
    };
    [[nodiscard]] double getTemperature() const {return temperature;};
    void setTemperature(double t) {
        temperature=t;
        notifyObserver();
    };
    [[nodiscard]] double getPressure() const {return pressure;};
    void setPressure(double p) {
        pressure=p;
        notifyObserver();
    };
};

class StatisticsObserver : public ObserverInterface{
private:
    string statistics;
public:
    void update(weak_ptr<SubjectInterface> s) override {
      //auto ws = (WeatherStation*) (s);
      auto ws = dynamic_pointer_cast<WeatherStation>(shared_ptr<SubjectInterface>(s)); // Esto: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
        statistics = "Hum:"+to_string(ws->getHumidity())+"\nTemp:"+to_string(ws->getTemperature())+"\nPres:"+to_string(ws->getPressure());
    }
    string getStatistics() {return statistics;}
};

class CurrentObserver : public ObserverInterface{
private:
    string current;
public:
    void update(weak_ptr<SubjectInterface> s) override {
        auto ws = dynamic_pointer_cast<WeatherStation>(shared_ptr<SubjectInterface>(s));
        current = to_string(ws->getPressure()*0.8 + ws->getHumidity()*0.1 + ws->getTemperature()*0.1);
    }
    string getCurrent() {return current;}
};

class ForecastObserver : public ObserverInterface{
private:
    string forecast;
public:
    void update(weak_ptr<SubjectInterface> s) override {
        auto ws = dynamic_pointer_cast<WeatherStation>(shared_ptr<SubjectInterface>(s));
        if (ws->getHumidity()>0.95 && ws->getPressure()>1){
            forecast="Lluvia";
        }
        else {
            forecast="Libre";
        }
    }
    string getForecast() {return forecast;}
};

class DisplayDevice {
private:
    shared_ptr<StatisticsObserver> statistics_obs = make_shared<StatisticsObserver>();
    shared_ptr<CurrentObserver> current_obs = make_shared<CurrentObserver>();
    shared_ptr<ForecastObserver> forecast_obs = make_shared<ForecastObserver>();
public:
    DisplayDevice() = default;
    ~DisplayDevice() = default;

    void showDisplay() {
        cout << "STATISTICS:\n" << statistics_obs->getStatistics() << endl;
        cout << "CURRENT:\n" << current_obs->getCurrent() << endl;
        cout << "FORECAST:\n" << forecast_obs->getForecast() << endl;
    }

    vector<shared_ptr<ObserverInterface>> getObserverList() {
        vector<shared_ptr<ObserverInterface>> observers;
        observers.push_back( shared_ptr<ObserverInterface>(statistics_obs));
        observers.push_back( shared_ptr<ObserverInterface>(current_obs));
        observers.push_back( shared_ptr<ObserverInterface>(forecast_obs));
        return observers;
    }
};

int main() {
    auto ws = make_shared<WeatherStation>();
    auto  d1 = make_shared<DisplayDevice>();
    auto  d2 = make_shared<DisplayDevice>();
    cout << "TEST POINT 1" << endl;
    ws->registerMultipleObserver(d1->getObserverList());
    ws->registerMultipleObserver(d2->getObserverList());
    cout << "TEST POINT 2" << endl;

    ws->setHumidity(0.90);
    ws->setPressure(2);
    ws->setTemperature(10);
    cout << "TEST POINT 3" << endl;
    ws->removeMultipleObserver(d1->getObserverList()); // ACA SE ROMPE
    ws->setHumidity(0.99);
    cout << "DEVICE 1" << endl;
    d1->showDisplay();
    cout << "--------" << endl;
    cout << "DEVICE 2";
    d2->showDisplay();
    cout << "--------" << endl;
    return 0;
}
