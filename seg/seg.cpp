#include <map>
#include <queue>
#include <set>

#include "seg.h"

// Change from RGB colour space to LAB colour space
namespace ljj {

static Vec3d RGB2XYZ(const Vec3b &rgb)
{
	const double R = rgb(0);
	const double G = rgb(1);
	const double B = rgb(2);

	const double r = R <= 0.04045 ? R / 12.92 : pow((R + 0.055) / 1.055, 2.4);
	const double g = G <= 0.04045 ? G / 12.92 : pow((G + 0.055) / 1.055, 2.4);
	const double b = B <= 0.04045 ? B / 12.92 : pow((B + 0.055) / 1.055, 2.4);

	Vec3d ret;

	ret(0) = r * 0.412453 + g * 0.357580 + b * 0.180423;
	ret(1) = r * 0.212671 + g * 0.715160 + b * 0.072169;
	ret(2) = r * 0.019334 + g * 0.119193 + b * 0.950227;

	return ret;
}

static Vec3b RGB2LAB(const Vec3b &rgb)
{
	const Vec3d XYZ = RGB2XYZ(rgb);

	const double epsilon = 0.008856;	// actual CIE standard
	const double kappa   = 903.3;		// actual CIE standard

	const double Xr = 0.950456;	// reference white
	const double Yr = 1.0;		// reference white
	const double Zr = 1.088754;	// reference white

	const double xr = XYZ(0) / Xr;
	const double yr = XYZ(1) / Yr;
	const double zr = XYZ(2) / Zr;

	const double fx = xr > epsilon ? pow(xr, 1.0 / 3.0) : (kappa * xr + 16.0) / 116.0;
	const double fy = yr > epsilon ? pow(yr, 1.0 / 3.0) : (kappa * yr + 16.0) / 116.0;
	const double fz = zr > epsilon ? pow(zr, 1.0 / 3.0) : (kappa * zr + 16.0) / 116.0;

	Vec3b ret;

	ret(0) = (unsigned char)((116.0 * fy - 16.0) / 100 * 255 + 0.5);
	ret(1) = (unsigned char)(500.0 * (fx - fy) + 128 + 0.5);
	ret(2) = (unsigned char)(200.0 * (fy - fz) + 128 + 0.5);

	return ret;
}

static Mat_8UC3 myrgb2lab(const Mat_8UC3 &rgb)
{
	Mat_8UC3 ret(rgb.rows(), rgb.cols());

	for (size_t x = 0; x < rgb.cols(); x++) {
		for (size_t y = 0; y < rgb.rows(); y++) {
			ret(y, x) = RGB2LAB(rgb(y, x));
		}
	}

	return ret;
}

class point
{
public:
	int x, y;
	point(const int X=0, const int Y=0): x(X), y(Y)
	{
	}
};

//Initialize the seeds

static int Seeds(const int rows, const int cols,
                 const int Row_num, const int Col_num,
                 const int Row_step, const int Col_step,
                 std::vector<point> &point_array)
{
	const int Row_remain = rows - Row_step * Row_num;
	const int Col_remain = cols - Col_step * Col_num;

	int cnt = 0;

	for (int i = 0, t1 = 1; i < Row_num; i++) {

		int t2 = 1;

		for (int j = 0; j < Col_num; j++) {

			const int center_x = std::min(rows - 1, i * Row_step + Row_step / 2 + t1);
			const int center_y = std::min(cols - 1, j * Col_step + Col_step / 2 + t2);

			if (t2 < Col_remain) {
				t2++;
			}

			point_array[cnt] = point(center_x, center_y);
			cnt++;
		}

		if (t1 < Row_remain) {
			t1++;
		}
	}

	return cnt;
}

//map pixels into ten dimensional feature space

static void Initialize(const Mat_8UC3 &lab,
                       Mat_64FC5 &labxy1, Mat_64FC5 &labxy2, Mat_64FC1 &W,
                       const int StepX, const int StepY,
                       const float color, const float distance)
{
	for (int x = 0; x < lab.cols(); x++) {
		for (int y = 0; y < lab.rows(); y++) {

			const double theta_l = ((double)lab(y, x)(0) / 255.0) * M_PI / 2;
			const double theta_a = ((double)lab(y, x)(1) / 255.0) * M_PI / 2;
			const double theta_b = ((double)lab(y, x)(2) / 255.0) * M_PI / 2;
			const double theta_x = ((double)y / (double)StepX) * M_PI / 2;
			const double theta_y = ((double)x / (double)StepY) * M_PI / 2;

			labxy1(y, x)(0) = color * std::cos(theta_l);
			labxy2(y, x)(0) = color * std::sin(theta_l);
			labxy1(y, x)(1) = color * std::cos(theta_a) * 2.55f;
			labxy2(y, x)(1) = color * std::sin(theta_a) * 2.55f;
			labxy1(y, x)(2) = color * std::cos(theta_b) * 2.55f;
			labxy2(y, x)(2) = color * std::sin(theta_b) * 2.55f;
			labxy1(y, x)(3) = distance * std::cos(theta_x);
			labxy2(y, x)(3) = distance * std::sin(theta_x);
			labxy1(y, x)(4) = distance * std::cos(theta_y);
			labxy2(y, x)(4) = distance * std::sin(theta_y);
		}
	}

	Vec5d sigma1 = Vec5d::Zero(), sigma2 = Vec5d::Zero();

	for (size_t x = 0; x < lab.cols(); x++) {
		for (size_t y = 0; y < lab.rows(); y++) {
			sigma1 += labxy1(y, x);
			sigma2 += labxy2(y, x);
		}
	}

	sigma1 /= lab.size();
	sigma2 /= lab.size();

	for (size_t x = 0; x < lab.cols(); x++) {
		for (size_t y = 0; y < lab.rows(); y++) {
			W(y, x) = labxy1(y, x).dot(sigma1) + labxy2(y, x).dot(sigma2);
			labxy1(y, x) /= W(y, x);
			labxy2(y, x) /= W(y, x);
		}
	}
}

// Enforce Connectivity by merging very small superpixels with their neighbors

static void preEnforceConnectivity(Mat_16UC1 &label)
{
	static const std::vector<int> dx8 = {-1, -1,  0,  1, 1, 1, 0, -1};
	static const std::vector<int> dy8 = { 0, -1, -1, -1, 0, 1, 1,  1};

	const int rows = label.rows();
	const int cols = label.cols();
	const int Bond = 20;
	int adj = 0;
	Mat_bool mask = Mat_bool::Constant(rows, cols, false);

	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			if (!mask(y, x)) {

				const uint16_t L = label(y, x);

				for (int k = 0; k < 8; k++) {

					const int x2 = x + dx8[k];
					const int y2 = y + dy8[k];

					if (0 <= x2 && x2 <= cols - 1 && 0 <= y2 && y2 <= rows - 1) {
						if (mask(y2, x2) && label(y2, x2) != L) {
							adj = label(y2, x2);
							break;
						}
					}
				}

				mask(y, x) = true;

				std::vector<std::pair<uint16_t, uint16_t>> locs;

				locs.push_back(std::make_pair(x, y));

				int indexMarker = 0;

				while (indexMarker < locs.size()) {

					const int x = locs[indexMarker].first;
					const int y = locs[indexMarker].second;
					const int minX = std::max(0, x - 1);
					const int minY = std::max(0, y - 1);
					const int maxX = std::min(cols - 1, x + 1);
					const int maxY = std::min(rows - 1, y + 1);

					for (size_t x = minX; x <= maxX; x++) {
						for (size_t y = minY; y <= maxY; y++) {
							if (!mask(y, x) && label(y, x) == L) {
								mask(y, x) = true;
								locs.push_back(std::make_pair(x, y));
							}
						}
					}

					indexMarker++;
				}

				if (indexMarker < Bond) {
					for (auto &loc : locs) {
						const uint16_t x = loc.first;
						const uint16_t y = loc.second;
						label(y, x) = adj;
					}
				}
			}
		}
	}
}

static void EnforceConnectivity(const Mat_64FC5 &labxy1, const Mat_64FC5 &labxy2, const Mat_64FC1 &W,
                                Mat_16UC1 &label,
                                const int threshold)
{
	assert(labxy1.rows() == W.rows());
	assert(labxy2.rows() == W.rows());
	assert(labxy1.cols() == W.cols());
	assert(labxy2.cols() == W.cols());

	class Superpixel
	{
	public:
		int Label;
		int Size;
		std::set<uint16_t> Neighbor;
		Superpixel(const int L=0, const int S=0) : Label(L), Size(S)
		{
		}
		std::vector<std::pair<int, int>> loc;
		bool operator==(const int L)
		{
			return this->Label == L;
		}
	};

	const int rows = W.rows();
	const int cols = W.cols();

	Mat_bool mask = Mat_bool::Constant(rows, cols, false);

	std::vector<std::pair<uint16_t, uint16_t>> stray;
	std::vector<unsigned short> Size;
	std::vector<Vec5d> center1;
	std::vector<Vec5d> center2;
	std::vector<double> centerW;

	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			if (!mask(y, x)) {

				int cnt = 1;

				center1.push_back(labxy1(y, x) * W(y, x));
				center2.push_back(labxy2(y, x) * W(y, x));
				centerW.push_back(W(y, x));
				stray.push_back(std::make_pair(x, y));

				const int L = label(y, x);

				label(y, x) = center1.size() - 1;
				mask(y, x) = true;

				std::queue<std::pair<uint16_t, uint16_t>> loc;

				loc.push(std::make_pair(x, y));

				while (!loc.empty()) {

					const int x = loc.front().first;
					const int y = loc.front().second;
					loc.pop();

					const int minX = std::max(0, x - 1);
					const int minY = std::max(0, y - 1);
					const int maxX = std::min(cols - 1, x + 1);
					const int maxY = std::min(rows - 1, y + 1);

					for (int x = minX; x <= maxX; x++) {
						for (int y = minY; y <= maxY; y++) {
							if (!mask(y, x) && label(y, x) == L) {
								cnt++;
								loc.push(std::make_pair(x, y));
								mask(y, x) = true;
								label(y, x) = center1.size() - 1;
								center1.back() += labxy1(y, x) * W(y, x);
								center2.back() += labxy2(y, x) * W(y, x);
								centerW.back() += W(y, x);
							}
						}
					}
				}

				Size.push_back(cnt);
				center1.back() /= centerW.back();
				center2.back() /= centerW.back();
			}
		}
	}

	std::map<uint16_t, Superpixel> Sarray;

	for (int i = 0; i < center1.size(); i++) {
		if (Size[i] < threshold) {

			const int x = stray[i].first;
			const int y = stray[i].second;
			const uint16_t L = label(y, x);
			Superpixel S(L, Size[i]);

			mask(y, x) = false;

			S.loc.push_back(std::make_pair(x, y));

			for (size_t indexMark = 0; indexMark < S.loc.size(); indexMark++) {

				const int x = S.loc[indexMark].first;
				const int y = S.loc[indexMark].second;
				const int minX = std::max(0, x - 1);
				const int minY = std::max(0, y - 1);
				const int maxX = std::min(cols - 1, x + 1);
				const int maxY = std::min(rows - 1, y + 1);

				for (int x = minX; x <= maxX; x++) {
					for (int y = minY; y <= maxY; y++) {
						if (label(y, x) != L) {

							const uint16_t NewLabel = label(y, x);

							S.Neighbor.insert(NewLabel);
						} else if (mask(y, x)) {
							mask(y, x) = false;
							S.loc.push_back(std::make_pair(x, y));
						}
					}
				}
			}

			Sarray[L] = std::move(S);
		}
	}

	while (!Sarray.empty()) {

		std::map<uint16_t, Superpixel>::iterator S = Sarray.begin();

		double MinDist = std::numeric_limits<double>::max();
		int Label1 = S->second.Label;
		int Label2 = -1;

		for (const int i : S->second.Neighbor) {
			const double D = (center1[Label1] - center1[i]).squaredNorm() + (center2[Label1] - center2[i]).squaredNorm();
			if (D < MinDist) {
				MinDist = D;
				Label2 = i;
			}
		}

		const double W1 = centerW[Label1];
		const double W2 = centerW[Label2];
		const double W = W1 + W2;

		center1[Label2] = (W2 * center1[Label2] + W1 * center1[Label1]) / W;
		center2[Label2] = (W2 * center2[Label2] + W1 * center2[Label1]) / W;
		centerW[Label2] = W;

		for (auto &loc : S->second.loc) {

			const int x = loc.first;
			const int y = loc.second;

			label(y, x) = Label2;
		}

		std::map<uint16_t, Superpixel>::iterator Stmp = Sarray.find(Label2);

		if (Stmp != Sarray.end()) {

			Size[Label2] += Size[Label1];

			if (Size[Label2] >= threshold) {
				Sarray.erase(Stmp);
			} else {
				Stmp->second.loc.insert(Stmp->second.loc.end(), S->second.loc.cbegin(), S->second.loc.cend());
				Stmp->second.Neighbor.insert(S->second.Neighbor.cbegin(), S->second.Neighbor.cend());
				Stmp->second.Neighbor.erase(Label1);
				Stmp->second.Neighbor.erase(Label2);
			}
		}

		Sarray.erase(S);

		for (auto &S : Sarray) {
			if (S.second.Neighbor.find(Label1) != S.second.Neighbor.end()) {
				S.second.Neighbor.erase(Label1);
				S.second.Neighbor.insert(Label2);
			}
		}
	}
}

// Perform weighted kmeans iteratively in the ten dimensional feature space.

static void DoSuperpixel(const Mat_64FC5 &labxy1, const Mat_64FC5 &labxy2, const Mat_64FC1 &W,
                         Mat_16UC1 &label,
                         std::vector<point> &seedArray,
                         int seedNum,
                         int StepX, int StepY,
                         int iterationNum, int thresholdCoef)
{
	assert(labxy1.rows() == W.rows());
	assert(labxy2.rows() == W.rows());
	assert(labxy1.cols() == W.cols());
	assert(labxy2.cols() == W.cols());

	// Pre-treatment
	const int rows = W.rows();
	const int cols = W.cols();

	Eigen::Matrix<double, 5, Eigen::Dynamic> center1(5, seedNum);
	Eigen::Matrix<double, 5, Eigen::Dynamic> center2(5, seedNum);

	// Initialization
	for (size_t i = 0; i < seedNum; i++) {

		const int x = seedArray[i].x;
		const int y = seedArray[i].y;
		const int minX = std::max(0, x - StepX / 4);
		const int minY = std::max(0, y - StepY / 4);
		const int maxX = std::min(cols - 1, x + StepX / 4);
		const int maxY = std::min(rows - 1, y + StepY / 4);
		const size_t Count = (maxX - minX + 1) * (maxY - minY + 1);

		center1.col(i).fill(0);
		center2.col(i).fill(0);

		for (size_t x = minX; x <= maxX; x++) {
			for (size_t y = minY; y <= maxY; y++) {
				center1.col(i) += labxy1(y, x);
				center2.col(i) += labxy2(y, x);
			}
		}

		center1.col(i) /= Count;
		center2.col(i) /= Count;
	}

	// K-means
	for (size_t iteration = 0; iteration < iterationNum; iteration++) {

		Mat_64FC1 dist = Mat_64FC1::Constant(rows, cols, std::numeric_limits<double>::max());

		for (int i = 0; i < seedNum; i++) {

			const int seed_x = seedArray[i].x;
			const int seed_y = seedArray[i].y;
			const int minX = std::max(0, seed_x - StepX);
			const int minY = std::max(0, seed_y - StepY);
			const int maxX = std::min(cols - 1, seed_x + StepX);
			const int maxY = std::min(rows - 1, seed_y + StepY);

			for (int x = minX; x <= maxX; x++) {
				for (int y = minY; y <= maxY; y++) {

					const double D = (labxy1(y, x) - center1.col(i)).squaredNorm() + (labxy2(y, x) - center2.col(i)).squaredNorm();

					if (D < dist(y, x)) {
						label(y, x) = i;
						dist(y, x) = D;
					}
				}
			}

			center1.col(i).fill(0);
			center2.col(i).fill(0);
			seedArray[i].x = 0;
			seedArray[i].y = 0;
		}

		std::vector<double> WSum(seedNum, 0.0);
		std::vector<int> clusterSize(seedNum, 0);

		for (size_t x = 0; x < cols; x++) {
			for (size_t y = 0; y < rows; y++) {

				const uint16_t L = label(y, x);
				const double Weight = W(y, x);

				center1.col(L) += Weight * labxy1(y, x);
				center2.col(L) += Weight * labxy2(y, x);
				clusterSize[L]++;
				WSum[L] += Weight;
				seedArray[L].x += x;
				seedArray[L].y += y;
			}
		}

		for (size_t i = 0; i < seedNum; i++) {
			WSum[i] = WSum[i] == 0.0 ? 1.0 : WSum[i];
			clusterSize[i] = clusterSize[i] == 0 ? 1 : clusterSize[i];

			center1.col(i) /= WSum[i];
			center2.col(i) /= WSum[i];
			seedArray[i].x /= clusterSize[i];
			seedArray[i].y /= clusterSize[i];
		}
	}

	// EnforceConnection
	const int threshold = labxy1.size() / (seedNum * thresholdCoef);

	preEnforceConnectivity(label);
	EnforceConnectivity(labxy1, labxy2, W, label, threshold);
}

static int countSuperpixel(Mat_16UC1 &label)
{
	const int cols = label.cols();
	const int rows = label.rows();

	int label_num = 0;
	Mat_bool mask = Mat_bool::Constant(rows, cols, false);

	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			if (!mask(y, x)) {

				const uint16_t L = label(y, x);

				mask(y, x) = true;
				label_num++;
				label(y, x) = label_num;

				std::queue<point> Q;
				Q.push(point(x, y));

				while (!Q.empty()) {

					const int x = Q.front().x;
					const int y = Q.front().y;
					Q.pop();

					const int minX = std::max(0, x - 1);
					const int minY = std::max(0, y - 1);
					const int maxX = std::min(cols - 1, x + 1);
					const int maxY = std::min(rows - 1, y + 1);

					for (size_t x = minX; x <= maxX; x++) {
						for (size_t y = minY; y <= maxY; y++) {
							if (label(y, x) == L && !mask(y, x)) {
								mask(y, x) = true;
								label(y, x) = label_num;
								Q.push(point(x, y));
							}
						}
					}
				}
			}
		}
	}

	return label_num;
}

#pragma mark - LSC superpixel segmentation algorithm

void segM(const Mat_8UC3 &rgb, const size_t superpixel_num, const double ratio, Mat_16UC1 &label)
{
	// Setting Parameter
	const size_t cols = rgb.cols();
	const size_t rows = rgb.rows();
	const float color_coefficient = 20;
	const float dist_coefficient = color_coefficient * ratio;
	const size_t seed_num = superpixel_num;
	const size_t iteration_times = 5;
	const int threshold_coef = 4;
	Mat_8UC3 lab = myrgb2lab(rgb);

	// Produce Seeds
	const int ColNum = std::sqrt(float(seed_num * cols / rows));
	const int RowNum = seed_num / ColNum;
	const int StepX = rows / RowNum;
	const int StepY = cols / ColNum;
	std::vector<point> seedArray(seed_num);
	const int newSeedNum = Seeds(rows, cols, RowNum, ColNum, StepX, StepY, seedArray);

	// Initialization
	Mat_64FC5 labxy1(rows, cols), labxy2(rows, cols);
	Mat_64FC1 W(rows, cols);

	Initialize(lab, labxy1, labxy2, W, StepX, StepY, color_coefficient, dist_coefficient);

	// Produce Superpixel
	DoSuperpixel(labxy1, labxy2, W, label, seedArray, newSeedNum, StepX, StepY, iteration_times, threshold_coef);

	int NUMBER = countSuperpixel(label);
}
}
