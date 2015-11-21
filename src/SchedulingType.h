/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_SCHEDULING_TYPE_H
#define FEC_SCHEDULING_TYPE_H

namespace fec {

  /**
   *  Scheduling used in decoding.
   *  This defines the scheduling of extrinsic communication between code
   *    constituents.
   */
  enum SchedulingType {
    Serial,/**< Each constituent tries to decode and gives its extrinsic
            information to the next constituent in a serial behavior. */
    Parallel,/**< Each constituent tries to decode in parallel.
              The extrinsic information is then combined and shared to every
              constituents similar to the Belief Propagation algorithm used in ldpc. */
    Custom,/**< Lets the user define a scheduling function. Don't use that. */
  };
  
}

#endif
