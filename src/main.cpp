#include <QApplication>
#include <memory>

#include "MainWindow.h"
#include "EncryptionManager.h"
#include "CaesarCipher.h"
#include "ROT13.h"
#include "DelimitedWeaverSimpleA.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create the encryption manager
    auto manager = std::make_shared<EncryptionManager>();

    // Register available encryption algorithms
    manager->registerAlgorithm(std::make_shared<CaesarCipher>());
    manager->registerAlgorithm(std::make_shared<ROT13>());
    manager->registerAlgorithm(std::make_shared<DelimitedWeaverSimpleA>());

    // Create and show the main window
    MainWindow window(manager);
    window.show();

    return app.exec();
}
