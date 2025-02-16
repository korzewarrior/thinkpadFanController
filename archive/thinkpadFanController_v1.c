#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <cstdlib>

void setFanLevel(const char *level) {
    // Runs: echo "level X" | sudo tee /proc/acpi/ibm/fan
    std::string command = "echo 'level ";
    command += level;
    command += "' | sudo tee /proc/acpi/ibm/fan";
    system(command.c_str());
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QSystemTrayIcon trayIcon(QIcon::fromTheme("utilities-terminal"));
    QMenu menu;

    QAction autoFan("Auto", &menu);
    QAction level1("Low (1)", &menu);
    QAction level3("Mid (3)", &menu);
    QAction level5("High (5)", &menu);
    QAction level7("Full (7)", &menu);
    QAction disengaged("Unlimited (Max)", &menu);
    QAction exitAction("Exit", &menu);

    QObject::connect(&autoFan, &QAction::triggered, [](){ setFanLevel("auto"); });
    QObject::connect(&level1, &QAction::triggered, [](){ setFanLevel("1"); });
    QObject::connect(&level3, &QAction::triggered, [](){ setFanLevel("3"); });
    QObject::connect(&level5, &QAction::triggered, [](){ setFanLevel("5"); });
    QObject::connect(&level7, &QAction::triggered, [](){ setFanLevel("7"); });
    QObject::connect(&disengaged, &QAction::triggered, [](){ setFanLevel("disengaged"); });
    QObject::connect(&exitAction, &QAction::triggered, &app, &QApplication::quit);

    menu.addAction(&autoFan);
    menu.addAction(&level1);
    menu.addAction(&level3);
    menu.addAction(&level5);
    menu.addAction(&level7);
    menu.addAction(&disengaged);
    menu.addSeparator();
    menu.addAction(&exitAction);

    trayIcon.setContextMenu(&menu);
    trayIcon.show();

    return app.exec();
}
