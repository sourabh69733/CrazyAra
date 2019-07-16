/*
 * CrazyAra, a deep learning chess variant engine
 * Copyright (C) 2018 Johannes Czech, Moritz Willig, Alena Beyer
 * Copyright (C) 2019 Johannes Czech
 *
 * CrazyAra is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * @file: evalinfo.cpp
 * Created on 13.05.2019
 * @author: queensgambit
 */

#include "evalinfo.h"
#include "uci.h"

EvalInfo::EvalInfo()
{

}

std::ostream& operator<<(std::ostream& os, const EvalInfo& evalInfo)
{
    os << "info score cp " << evalInfo.centipawns
       << " depth " << evalInfo.depth
       << " nodes " << evalInfo.nodes
       << " time " << evalInfo.elapsedTimeMS
          // + 0.5 and int() is a simple way for rounding to the first decimal
       << " nps " << int(((evalInfo.nodes-evalInfo.nodesPreSearch) / (evalInfo.elapsedTimeMS / 1000.0f)) + 0.5f)
       << " pv";
    for (Move move: evalInfo.pv) {
        os << " " << UCI::move(move, evalInfo.is_chess960);
    }
    return os;
}
