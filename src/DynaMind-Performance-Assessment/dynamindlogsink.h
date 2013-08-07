/**
 * CityDrain3 is an open source software for modelling and simulating integrated 
 * urban drainage systems.
 * 
 * Copyright (C) 2012 Michael Mair
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin 
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 **/

#ifndef DYNAMINDLOGSINK_H
#define DYNAMINDLOGSINK_H

#include <string>
#include <ostream>
#include <logsink.h>
#include <dmlogger.h>

#include <cd3globals.h>

class CD3_PUBLIC DynaMindStreamLogSink : public LogSink {
public:
    DynaMindStreamLogSink();

	void close() {}
    LogSink &operator<<(const std::string &string);
    LogSink &operator<<(const char *string);
    LogSink &operator<<(int i);
    LogSink &operator<<(LSEndl e);
};

#endif // DYNAMINDLOGSINK_H
