#ifndef MECACELL_GRID_INSTANT_SIGNALING_PLUGIN_HPP
#define MECACELL_GRID_INSTANT_SIGNALING_PLUGIN_HPP
#include <array>
#include <mecacell/utility/grid.hpp>
#include <utility>
#include <vector>

/**
 * @brief simple grid based "molecule" signaling plugin.
 * No "real" diffusion is happening and "molecules" do not accumulate over time. This is
 * instant signaling from producing cells to sensing cells. It uses the mecacell sparse
 * grid implementation to compute averaged molecule production centers and interpolates it
 * for computing the averaged sense one. Worst case (grid cell size < cells size) gives
 * O(n^2) complexity (same as adding every cells productions for every sensing cell). With
 * grid cell sizes > cells size performances increase (but precision decreases)
 *
 * @tparam C the cell type. Must have a {getMoleculeProduction(int)->double} method
 * returning the production amount of the given molecule, as well as a
 * {setMoleculeSensing(unsigned int,double)->void} to set the sensed concentration of a
 * given molecule
 * @tparam NB_MOL the number of molecules type
 */
template <typename C, size_t NB_MOL> class GridInstantSignalingPlugin {
 private:
	using V = std::result_of_t<decltype (&C::getPosition)()>;  // Vec3D type
	MecaCell::Grid<Cell*> cellgrid;

 public:
	std::array<double, NB_MOL> moleculeRange{};  // ranges of each molecule
	std::vector<std::array<std::pair<V, double>, NB_MOL>>
	    molecules;  // used to store molecules quantities

	/**
	 * @brief Constructor
	 *
	 * @param gridSize the size of the grids cells. Larger is faster but less precise
	 */
	GridInstantSignalingPlugin(double gridSize) : cellgrid(gridSize) {}

	template <typename W> void beginUpdate(W* w) {
		cellgrid.clear();
		for (auto& c : w.cells)
			cellgrid.insertOnlyCenter(c);  // we only need the cells centers coordinates

		auto& gridcontent = cellgrid.getContent();

		for (auto& gridcell : gridcontent) {  // foreach grid cell
			std::array<std::pair<V, double>, NB_MOL> molCenters{};
			const double nbCells = static_cast<double>(gridcell.second.size());
			for (auto& c : gridcell.second) {  // foreach cell in a gridcell
				for (auto i = 0u; i < NB_MOL; ++i) {
					// we compute the sum of all productions in this gridcell as well as the
					// weighted center coordinate for increased precision
					const auto prod = c->getMoleculeProduction(i);
					molCenters[i].first += c->getPosition() * prod;
					molCenters[i].second += prod;
				}
				// centroid coords normalization:
				for (auto& c : gridcell.second)
					for (auto i = 0u; i < NB_MOL; ++i)
						if (molCenters[i].second > 0) molCenters[i].first /= molCenters[i].second;
			}
			molecules.push_back(morphoCenters);
		}
		for (auto& c : w.cells) {
			const auto P = c->getPosition();
			for (auto i = 0u; i < NB_MOL; ++i) {
				double sensed = 0.0;
				for (const auto& m : molecules) {
					auto sql = (m[i].first - P).sqlength() / moleculeRange[i];
					sensed += m[i].second / (sql + 1.0);
				}
				c->setMoleculeSensing(i, sensed);
			}
		}
	}
};
