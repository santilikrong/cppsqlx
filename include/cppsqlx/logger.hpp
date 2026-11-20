/*
MIT License

Copyright (c) 2025 Y-HRAH KRONG Santili

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logmanager.h> // Ajouté pour le reset de configuration
#include <string>
#include <mutex>
#include <filesystem>

namespace cppsqlx {

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& configFile = "log4cxx.properties") {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Évite de reconfigurer si cela a déjà été fait
        if (is_initialized_) {
            return;
        }

        // Nettoie toute configuration existante/par défaut avant d'appliquer la nouvelle
        log4cxx::LogManager::getLoggerRepository()->resetConfiguration();
        
        // Vérifie si le fichier de propriétés existe
        if (std::filesystem::exists(configFile)) {
            log4cxx::PropertyConfigurator::configure(configFile);
        } else {
            // Configuration de secours par défaut sur la console si le fichier est absent
            log4cxx::BasicConfigurator::configure();
        }

        is_initialized_ = true;
    }

    log4cxx::LoggerPtr getLogger(const std::string& name) {
        return log4cxx::Logger::getLogger(name);
    }

private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex mutex_;
    bool is_initialized_{false}; // Drapeau de suivi de l'initialisation
};

} // namespace cppsqlx

#endif // LOGGER_HPP
