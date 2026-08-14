#include "roster.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

using namespace rosterflow;

namespace {
void line() { std::cout << "\n------------------------------------------------------------\n"; }
void printPlayer(const Player& p) { std::cout << std::left << std::setw(6) << p.number << std::setw(24) << p.name << std::setw(6) << p.age << std::setw(18) << p.nationality << std::setw(14) << p.position << std::setw(8) << p.priority << "\n"; }
void printTable(const std::vector<Player>& players) {
    if (players.empty()) { std::cout << "لا توجد بيانات حالياً.\n"; return; }
    std::cout << std::left << std::setw(6) << "رقم" << std::setw(24) << "الاسم" << std::setw(6) << "العمر" << std::setw(18) << "الجنسية" << std::setw(14) << "المركز" << std::setw(8) << "الأولوية" << "\n";
    line(); for (const auto& p : players) printPlayer(p);
}
int readInt(const std::string& prompt) { int value; while (true) { std::cout << prompt; if (std::cin >> value) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return value; } std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "أدخل رقماً صحيحاً.\n"; } }
std::string readText(const std::string& prompt) { std::string value; std::cout << prompt; std::getline(std::cin, value); return trim(value); }
void menu() { line(); std::cout << "RosterFlow | نظام إدارة قائمة الفريق\n"; line(); std::cout << "1. عرض القائمة\n2. إضافة لاعب\n3. البحث برقم القميص\n4. حذف لاعب\n5. تحديث أولوية اللاعب\n6. عرض قائمة الأولوية\n7. عرض اللاعب التالي في الدور\n8. التراجع عن آخر إضافة\n9. حفظ البيانات\n10. تحميل البيانات\n0. خروج\n"; }
}

int main() {
    RosterService service; std::string error; service.loadCsv("data/players.csv", error);
    std::cout << "\nمرحباً بك في RosterFlow\nإدارة قائمة الفريق باستخدام هياكل بيانات عملية وآمنة.\n";
    while (true) {
        menu(); int choice = readInt("اختر العملية: ");
        if (choice == 0) break;
        if (choice == 1) printTable(service.players());
        else if (choice == 2) { Player p; p.number = readInt("رقم القميص: "); p.name = readText("اسم اللاعب: "); p.age = readInt("العمر: "); p.nationality = readText("الجنسية: "); p.position = readText("المركز: "); p.priority = readInt("الأولوية (1-10): "); if (service.addPlayer(p, error)) std::cout << "تمت الإضافة بنجاح.\n"; else std::cout << error << "\n"; }
        else if (choice == 3) { const auto* p = service.findPlayer(readInt("رقم القميص: ")); if (p) printTable({*p}); else std::cout << "لم يتم العثور على اللاعب.\n"; }
        else if (choice == 4) { if (service.removePlayer(readInt("رقم اللاعب المراد حذفه: "), error)) std::cout << "تم الحذف.\n"; else std::cout << error << "\n"; }
        else if (choice == 5) { int number = readInt("رقم اللاعب: "); int priority = readInt("الأولوية الجديدة (1-10): "); if (service.updatePriority(number, priority, error)) std::cout << "تم التحديث.\n"; else std::cout << error << "\n"; }
        else if (choice == 6) printTable(service.priorityQueue());
        else if (choice == 7) { const auto* p = service.nextPriorityPlayer(); if (p) printTable({*p}); else std::cout << "قائمة الأولوية فارغة.\n"; }
        else if (choice == 8) { if (service.undoLastAddition(error)) std::cout << "تم التراجع عن آخر إضافة.\n"; else std::cout << error << "\n"; }
        else if (choice == 9) { if (service.saveCsv("data/players.csv", error)) std::cout << "تم حفظ البيانات في data/players.csv.\n"; else std::cout << error << "\n"; }
        else if (choice == 10) { if (service.loadCsv("data/players.csv", error)) std::cout << "تم تحميل البيانات.\n"; else std::cout << error << "\n"; }
        else std::cout << "خيار غير معروف.\n";
    }
    service.saveCsv("data/players.csv", error); std::cout << "تم حفظ البيانات. إلى اللقاء.\n"; return 0;
}
