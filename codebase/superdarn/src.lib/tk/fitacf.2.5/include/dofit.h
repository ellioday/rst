/* dofit.h
   ========
   Author: R.J.Barnes
Copyright (C) <year>  <name of author>

This file is part of the Radar Software Toolkit (RST).

RST is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Modifications:

*/

#ifndef _DOFIT_H
#define _DOFIT_H

int do_fit(struct FitBlock *iptr, int lag_lim, int goose,
           struct FitRange *ptr, struct FitRange *xptr,
           struct FitElv *elv, struct FitNoise *nptr);
#endif

