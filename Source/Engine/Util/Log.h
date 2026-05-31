//
// Clark Kromenaker
//
// "Log" and "Logf" are engine replacements for printf.
// In addition to printing to standard output, they also print to a shared log file.
//
// Logging is thread-safe. Logs from different threads at the same time should display correctly.
//
#pragma once

void Log(const char* message);
void Logf(const char* format, ...);
