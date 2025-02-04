/**
 * CityDrain3 is an open source software for modelling and simulating integrated
 * urban drainage systems.
 *
 * Copyright (C) 2014 Christian Urich
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

#include "sourcevector.h"
#include <flow.h>
#include <flowfuns.h>

CD3_DECLARE_NODE_NAME(SourceVector)

SourceVector::SourceVector() {
	out = Flow();




	addOutPort(ADD_PARAMETERS(out));


	addParameter(ADD_PARAMETERS(source));

	factor = 1;
	addParameter(ADD_PARAMETERS(factor));
}

SourceVector::~SourceVector() {
}

bool SourceVector::init(ptime start, ptime end, int dt) {
	counter = 0;
	return true;
}

int SourceVector::f(ptime time, int dt) {
	(void) time;

	out[0] = source[counter]*factor;

	counter++;
	return dt;
}
