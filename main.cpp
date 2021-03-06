

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "ModelRun.h"
#include "acclimate.h"
#include "settingsnode.h"
#include "settingsnode/inner.h"
#include "settingsnode/yaml.h"
#include "version.h"

namespace acclimate {
extern const char* info;
}  // namespace acclimate

static void print_usage(const char* program_name) {
    std::cerr << "Acclimate model\n"
                 "Version: "
              << acclimate::version
              << "\n\n"
                 "Authors: Sven Willner <sven.willner@pik-potsdam.de>\n"
                 "         Christian Otto <christian.otto@pik-potsdam.de>\n"
                 "\n"
                 "Usage:   "
              << program_name
              << " (<option> | <settingsfile>)\n"
                 "Options:\n"
              << (acclimate::has_diff ? "  -d, --diff     Print git diff output from compilation\n" : "")
              << "  -h, --help     Print this help text\n"
                 "  -i, --info     Print further information\n"
                 "  -v, --version  Print version"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    const std::string arg = argv[1];
    if (arg.length() > 1 && arg[0] == '-') {
        if (arg == "--version" || arg == "-v") {
            std::cout << acclimate::version << std::endl;
        } else if (arg == "--info" || arg == "-i") {
            std::cout << "Version:                " << acclimate::version << "\n\n"
                      << acclimate::info
                      << "\n"
                         "Precision Time:         "
                      << acclimate::Time::precision_digits
                      << "\n"
                         "Precision Quantity:     "
                      << acclimate::Quantity::precision_digits
                      << "\n"
                         "Precision FlowQuantity: "
                      << acclimate::FlowQuantity::precision_digits
                      << "\n"
                         "Precision Price:        "
                      << acclimate::Price::precision_digits
                      << "\n"
                         "Options:                ";
            bool first = true;
            for (const auto& option : acclimate::options::options) {
                if (first) {
                    first = false;
                } else {
                    std::cout << "                        ";
                }
                std::cout << option.name << " = " << (option.value ? "true" : "false") << "\n";
            }
            std::cout << std::flush;
        } else if (acclimate::has_diff && (arg == "--diff" || arg == "-d")) {
            std::cout << acclimate::git_diff << std::flush;
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } else {
        try {
            if (arg == "-") {
                std::cin >> std::noskipws;
                acclimate::ModelRun acclimate(settings::SettingsNode(std::make_unique<settings::YAML>(std::cin)));
                acclimate.run();
            } else {
                std::ifstream settings_file(arg);
                if (!settings_file) {
                    throw std::runtime_error("Cannot open " + arg);
                }
                acclimate::ModelRun acclimate(settings::SettingsNode(std::make_unique<settings::YAML>(settings_file)));
                acclimate.run();
            }
        } catch (const acclimate::return_after_checkpoint&) {
            return 7;
        } catch (const std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return 255;
        }
    }
    return 0;
}
