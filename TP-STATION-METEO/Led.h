#include <iostream>
class Led {
  private:
   bool allumee;
   int pin;
  public:

    explicit Led(int pin): pin(pin), allumee(false) {}

    ~Led() {
      eteindre();
      std::cout << "[LED PIN " << pin << "] LIBERE" << std::endl
    }
  
    void allumer() {
      allumee = true;
      std::cout << "[LED PIN " << pin << "] ON" << std::endl;
    }

    void eteindre() {
      allumee = false;
      std::cout << "[LED PIN " << pin << "] OFF" << std::endl;
    }

    void basculer() {
      switch (allumee)
      {
      case false:
        allumer();
        break;
      case true:
        eteindre();
        break;
      }
    }

    bool estAllumee() const {
      return allumee;
    }
};