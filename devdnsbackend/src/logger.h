#pragma once
#include <iostream>
#include <fmt/core.h>
#include "pdns/logger.hh"
#include <string>

static void log(Logger::Urgency level, const std::string &message) {
#ifdef PDNS_BACKEND
    g_log << level << "[DEVDNSBackend]" << " " << message << std::endl;
#else
    cout << level << "[DEVDNSBackend]" << " " << message << std::endl;
#endif
}

template<typename... T>
static void log(Logger::Urgency level, std::string str, T &&... args) {
    log(level, fmt::format(str, args...));
}

static void log_debug(const std::string &message) {
    log(Logger::Debug, message);
}
template<typename... T>
static void log_debug(std::string str, T &&... args) {
    log_debug(fmt::format(str, args...));
}

static void log_info(const std::string &message) {
    log(Logger::Info, message);
}
template<typename... T>
static void log_info(std::string str, T &&... args) {
    log_info(fmt::format(str, args...));
}

static void log_error(const std::string &message) {
    log(Logger::Error, message);
}
template<typename... T>
static void log_error(std::string str, T &&... args) {
    log_error(fmt::format(str, args...));
}
