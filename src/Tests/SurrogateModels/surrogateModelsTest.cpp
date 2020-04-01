#include "ceinms/Curve.h"
#include "rtb/plsr/plsr.h"
#include "mcbs/Spline.h"
#include <string>
using std::string;
using std::vector;
#include <fstream>
using std::ifstream;
#include <iostream>
using std::cout;
using std::endl;

SimTK::Matrix parseCSV(const std::string& filename) {

	std::ifstream data(filename);
	std::string line;
	std::vector<std::vector<double>> parsedCsv;
	while (std::getline(data, line))
	{
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<double> parsedRow;
		while (std::getline(lineStream, cell, ','))
		{
			parsedRow.push_back(std::stod(cell));
		}
		parsedCsv.push_back(parsedRow);
	}
	SimTK::Matrix m(parsedCsv.size(), parsedCsv.front().size());
	for (int r(0); r < parsedCsv.size(); ++r) {
		for (int c(0); c < parsedCsv.front().size(); ++c) {
			m(r, c) = parsedCsv[r][c];
		}
	}

	return m;
};

void printCSV(const SimTK::Matrix& m, const std::string& outputFilename) {

	std::string sp(",");
	std::ofstream oF(outputFilename);
	for (int r(0); r < m.nrow(); ++r) {
		for (int c(0); c < m.ncol(); ++c) {
			oF << m(r, c);
			if (c != m.ncol() - 1)
				oF << sp;
		}
		if (r != m.nrow() - 1)
			oF << "\n";
	}
	oF.close();
}

class CubicSplineSurrogate {
public:
	CubicSplineSurrogate(SimTK::Matrix X, SimTK::Matrix Y) {

		if (X.ncol() != 1) {
			std::cout << "CubicSplineSurrogate: only 1 independent variable allowed, but " << X.ncol() << " provided" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		if (X.nrow() != Y.nrow()) {
			std::cout << "CubicSplineSurrogate: number or rows of X matrix (" << X.nrow() << ") is"
				"different than number of rows of Y matrix (" << Y.nrow() << std::endl;
			std::exit(EXIT_FAILURE);
		}

		auto convertToStd([](SimTK::Vector vec) {
			vector<double> outV;
			//Currently, the data from the FE model does not have a zero strain,
			//so we add a zero to get the spline to start from 0
			outV.emplace_back(0);
			for (auto e : vec) {
				outV.emplace_back(e);
			}
			return outV;
		});

		nDataPoints_ = Y.nrow() + 1;
		nNodes_ = Y.ncol();
		vector<double> xvals = convertToStd(X.col(0));
		splines_.resize(nNodes_);
		for (int i(0); i < nNodes_; ++i) {
			vector<double> yvals = convertToStd(Y.col(i));
			splines_.at(i).resetPointsWith(xvals, yvals);
		}
	}


	SimTK::Matrix project(const SimTK::Matrix& X) const {
		SimTK::Matrix out(X.nrow(), nNodes_);
		for (int iRow(0); iRow < X.nrow(); ++iRow) {
			double xval = X(iRow, 0);
			for (unsigned i(0); i < nNodes_; ++i)
				out(iRow, i) = splines_.at(i).getValue(xval);
		}
		return out;
	}

private:
	unsigned nDataPoints_, nNodes_;
	std::vector<ceinms::Curve<ceinms::CurveMode::Offline>> splines_;
};


class BasisSplineSurrogate {
public:
	BasisSplineSurrogate(SimTK::Matrix X, SimTK::Matrix Y) {

		if (X.ncol() != 1) {
			std::cout << "CubicSplineSurrogate: only 1 independent variable allowed, but " << X.ncol() << " provided" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		if (X.nrow() != Y.nrow()) {
			std::cout << "CubicSplineSurrogate: number or rows of X matrix (" << X.nrow() << ") is"
				"different than number of rows of Y matrix (" << Y.nrow() << std::endl;
			std::exit(EXIT_FAILURE);
		}

		auto convertToStd([](SimTK::Vector vec) {
			vector<double> outV;
			//Currently, the data from the FE model does not have a zero strain,
			//so we add a zero to get the spline to start from 0
			outV.emplace_back(0);
			for (auto e : vec) {
				outV.emplace_back(e);
			}
			return outV;
		});

		nDataPoints_ = Y.nrow() + 1;
		nNodes_ = Y.ncol();

		vector<double> xvals = convertToStd(X.col(0));
		a_ = xvals.front();
		b_ = xvals.back();
		n_ = xvals.size();
		for (int i(0); i < nNodes_; ++i) {
			mcbs::Spline<1> spline(a_, b_, n_);
			vector<double> yvals = convertToStd(Y.col(i));
			spline.computeCoefficients(yvals, yvals.begin());
			splines_.emplace_back(spline);
		}
	}


	SimTK::Matrix project(const SimTK::Matrix& X) const {
		SimTK::Matrix out(X.nrow(), nNodes_);
		for (int iRow(0); iRow < X.nrow(); ++iRow) {
			double xval = X(iRow, 0);
			for (unsigned i(0); i < nNodes_; ++i) {
				if (xval < a_)
					xval = a_;
				if (xval > b_)
					xval = b_;
				out(iRow, i) = splines_.at(i).getValue(xval);

			}
		}
		return out;
	}

private:
	unsigned nDataPoints_, nNodes_;
	double a_, b_;
	int n_;
	std::vector<mcbs::Spline<1>> splines_;
};



void testNodes(const SimTK::Matrix& X, const SimTK::Matrix& Y) {
	rtb::Plsr plsr;
	plsr.train(X, Y, 0.001);
	CubicSplineSurrogate cubic(X, Y);
	SimTK::Matrix YP_plsr = plsr.project(X.getAsMatrixView());
	SimTK::Matrix YP_cubic = cubic.project(X);
	printCSV(YP_plsr, string(BASE_DIR) + "/data/node_prediction_YP_plsr.csv");
	printCSV(YP_cubic, string(BASE_DIR) + "/data/node_prediction_YP_cubic.csv");
}

void testFullRange(const SimTK::Matrix& X, const SimTK::Matrix& Y) {
	rtb::Plsr plsr;
	plsr.train(X, Y, 0.001);
	CubicSplineSurrogate cubic(X, Y);
	int n = 101;
	SimTK::Matrix xnew(n, 1);
	double xmin = 0;
	double xmax = X(X.nrow() - 1, 0);
	for (int i(0); i < n; ++i) {
		xnew(i, 0) = xmin + (xmax - xmin) / n * i;
	}

	SimTK::Matrix YP_plsr = plsr.project(xnew.getAsMatrixView());
	SimTK::Matrix YP_cubic = cubic.project(xnew);
	printCSV(YP_plsr, string(BASE_DIR) + "/data/full_prediction_YP_plsr.csv");
	printCSV(YP_cubic, string(BASE_DIR) + "/data/full_prediction_YP_cubic.csv");
}



void leaveOneOut(const SimTK::Matrix& X, const SimTK::Matrix& Y) {
	SimTK::Matrix X_reduced(X.nrow() - 1, X.ncol());
	SimTK::Matrix Y_reduced(Y.nrow() - 1, Y.ncol());
	SimTK::Matrix YP_plsr(Y.nrow(), Y.ncol()), YP_cubic(Y.nrow(), Y.ncol());
	for (unsigned toSkip(0); toSkip < X.nrow(); ++toSkip) {
		unsigned k(0);
		for (unsigned i(0); i < X.nrow(); ++i) {
			if (i != toSkip) {
				for (int l(0); l < X.ncol(); ++l) {
					X_reduced(k,l) = X(i,l);
				}
				for (int l(0); l < Y.ncol(); ++l) {
					Y_reduced(k, l) = Y(i, l);
				}
				++k;
			}
		}
		std::cout << "LOO: " << toSkip << endl;
		rtb::Plsr plsr;

		plsr.train(X_reduced, Y_reduced, 0.001);
		CubicSplineSurrogate cubic(X_reduced, Y_reduced);
		SimTK::Matrix toEval(1, 1);
		toEval(0, 0) = X.get(toSkip, 0);
		SimTK::Matrix yy_plsr = plsr.project(toEval.getAsMatrixView());
		SimTK::Matrix yy_cubic = cubic.project(toEval.getAsMatrixView());
		for (int l(0); l < yy_plsr.ncol(); ++l) {
			YP_plsr(toSkip, l) = yy_plsr(0, l);
			YP_cubic(toSkip, l) = yy_cubic(0, l);
		}
	}
	printCSV(YP_plsr, string(BASE_DIR) + "/data/leave1out_prediction_YP_plsr.csv");
	printCSV(YP_cubic, string(BASE_DIR) + "/data/leave1out_prediction_YP_cubic.csv");
}

int main() {
	string dataDir(string(BASE_DIR) + "/data");
	string dataXfile{ dataDir + "/xvalues.csv" };
	string dataYfile{ dataDir + "/yvalues.csv" };
	SimTK::Matrix X(parseCSV(dataXfile));
	SimTK::Matrix Y(parseCSV(dataYfile));
	testNodes(X, Y);
	leaveOneOut(X, Y);
	testFullRange(X, Y);
	return 0;
}