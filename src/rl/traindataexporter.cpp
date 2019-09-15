/*
  CrazyAra, a deep learning chess variant engine
  Copyright (C) 2018  Johannes Czech, Moritz Willig, Alena Beyer
  Copyright (C) 2019  Johannes Czech

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * @file: traindataexporter.cpp
 * Created on 12.09.2019
 * @author: queensgambit
 */

#include "traindataexporter.h"
#include <inttypes.h>

void TrainDataExporter::export_pos(const Board *pos, const EvalInfo& eval, size_t idxOffset)
{
    export_planes(pos, idxOffset);
    export_policy(eval.legalMoves, eval.policyProbSmall, pos->side_to_move(), idxOffset);
    // value will be set later in export_game_result()
}

void TrainDataExporter::export_game_result(const Result result, size_t idxOffset, size_t plys)
{
    // value
    // write array to roi
    z5::types::ShapeType offsetValue = { idxOffset };
    xt::xarray<int16_t>::shape_type shapeValue = { plys };
    xt::xarray<int16_t> array2(shapeValue, result);

    if (result != DRAW) {
    // invert the result on every second ply
        for (size_t idx = 1; idx < plys; idx+=2) {
            array2.data()[idx] = -result;
        }
    }

    z5::multiarray::writeSubarray<int16_t>(dValue, array2, offsetValue.begin());
}

TrainDataExporter::TrainDataExporter()
{
    const string fileName = "data.zr";
    // get handle to a File on the filesystem
    z5::filesystem::handle::File f(fileName);

    // create the file in zarr format
    const bool createAsZarr = true;
    z5::createFile(f, createAsZarr);

    z5::createGroup(f, "group");
    chunckSize = 128;

    // create a new zarr dataset
    const std::string dsName = "x";
    std::vector<size_t> shape = { chunckSize, NB_CHANNELS_TOTAL, BOARD_HEIGHT, BOARD_WIDTH };
    std::vector<size_t> chunks = { chunckSize, NB_CHANNELS_TOTAL, BOARD_HEIGHT, BOARD_WIDTH };
    dx = z5::createDataset(f, dsName, "int16", shape, chunks);
    dValue = z5::createDataset(f, "y_value", "int16", { chunckSize }, { chunckSize });
    dPolicy = z5::createDataset(f, "y_policy", "float32", { chunckSize, NB_LABELS }, { chunckSize, NB_LABELS });
}

void TrainDataExporter::export_planes(const Board *pos, size_t idxOffset)
{
    // x / plane representation
    float inputPlanes[NB_VALUES_TOTAL];
    board_to_planes(pos, 0, false, inputPlanes);
    // write array to roi
    z5::types::ShapeType offsetPlanes = { idxOffset, 0, 0, 0 };
    xt::xarray<int16_t>::shape_type shape1 = { 1, NB_CHANNELS_TOTAL, BOARD_HEIGHT, BOARD_WIDTH };
    xt::xarray<int16_t> array1(shape1);
    for (size_t idx = 0; idx < NB_VALUES_TOTAL; ++idx) {
        array1.data()[idx] = int16_t(inputPlanes[idx]);
    }
    z5::multiarray::writeSubarray<int16_t>(dx, array1, offsetPlanes.begin());
}

void TrainDataExporter::export_policy(const vector<Move>& legalMoves, const DynamicVector<float>& policyProbSmall, Color sideToMove, size_t idxOffset)
{
    assert(legalMoves.size() == policyProbSmall.size());

    // write array to roi
    z5::types::ShapeType offsetPolicy = { idxOffset, 0 };
    xt::xarray<float>::shape_type shapePolicy = { 1, NB_LABELS };
    xt::xarray<float> policy(shapePolicy, 0);

    for (size_t idx = 0; idx < legalMoves.size(); ++idx) {
        size_t policyIdx;
        if (sideToMove == WHITE) {
            policyIdx = MV_LOOKUP_CLASSIC[legalMoves[idx]];
        }
        else {
            policyIdx = MV_LOOKUP_MIRRORED_CLASSIC[legalMoves[idx]];
        }
        policy[policyIdx] = policyProbSmall[idx];
    }
    z5::multiarray::writeSubarray<float>(dPolicy, policy, offsetPolicy.begin());

}

void TrainDataExporter::export_positions(const std::vector<Node*>& nodes, Result result)
{
    size_t offset = 0;
    for (auto node : nodes) {
        DynamicVector<float> mctsPolicy(node->get_number_child_nodes());
        get_mcts_policy(node, 0.2f, 0.2f, mctsPolicy);

        vector<Move> legalMoves = retrieve_legal_moves(node->get_child_nodes());
    }
}
