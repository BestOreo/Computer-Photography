#ifndef HW3_GN_34804D67
#define HW3_GN_34804D67
#include "data.h"
#include <opencv2/opencv.hpp>
#include <math.h>
 
using namespace std;
using namespace cv;

#define POW(x) ((x)*(x))
#define POW_n3(x) (1.0/((x)*(x)*(x)))


struct GaussNewtonParams {
	GaussNewtonParams() :
		exact_line_search(false),
		gradient_tolerance(1e-5),
		residual_tolerance(1e-5),
		max_iter(1000),
		verbose(false)
	{}
	bool exact_line_search; // ʹ�þ�ȷ�����������ǽ�����������
	double gradient_tolerance; // �ݶ���ֵ����ǰ�ݶ�С�������ֵʱֹͣ����
	double residual_tolerance; // ������ֵ����ǰ����С�������ֵʱֹͣ����
	int max_iter; // ����������
	bool verbose; // �Ƿ��ӡÿ����������Ϣ
};

struct GaussNewtonReport {
	enum StopType {
		STOP_GRAD_TOL,       // �ݶȴﵽ��ֵ
		STOP_RESIDUAL_TOL,   // ����ﵽ��ֵ
		STOP_NO_CONVERGE,    // ������
		STOP_NUMERIC_FAILURE // ������ֵ����
	};
	StopType stop_type; // �Ż���ֹ��ԭ��
	double n_iter;      // ��������
};

class ResidualFunction {
public:
	virtual int nR() const = 0;
	virtual int nX() const = 0;
	virtual void eval(double *R, double *J, double *X) = 0;
};

class GaussNewtonSolver {
public:
	virtual double solve(
		ResidualFunction *f, // Ŀ�꺯��
		double *X,           // ������Ϊ��ֵ�������Ϊ���
		GaussNewtonParams param = GaussNewtonParams(), // �Ż�����
		GaussNewtonReport *report = nullptr // �Ż��������
	) = 0;
};

class EclipseFunction : public ResidualFunction {
public :
	double *x,*y,*z;
	int dimension = 0;
	int size;

	EclipseFunction() {
		size = sizeof(point) / (3 * sizeof(double));
		dimension = sizeof(point[0]) / sizeof(double);
		x = new double[size];
		y = new double[size];
		z = new double[size];
		for (int i = 0; i < size; i++) {
			x[i] = point[i][0];
			y[i] = point[i][1];
			z[i] = point[i][2];
		}
	}

	~EclipseFunction() {
		delete x;
		delete y;
		delete z;
	}

	double cal_value(double *coefficient, int i) {
		double A = coefficient[0];
		double B = coefficient[1];
		double C = coefficient[2];
		double result = 1 - 1.0 / POW(A)*POW(x[i]) - 1.0 / POW(B)*POW(y[i]) - 1.0 / POW(C)*POW(z[i]);
		return result;
	}


	virtual int nR() const {
		return size;
	}

	virtual int nX() const {
		return dimension;
	}

	virtual void eval(double *R, double *J, double *coefficient) {
		for (int i = 0; i < size; i++) {
			R[i] = cal_value(coefficient, i);
			J[i * 3 + 0] = -2 * POW(x[i])*POW_n3(coefficient[0]);
			J[i * 3 + 1] = -2 * POW(y[i])*POW_n3(coefficient[1]);
			J[i * 3 + 2] = -2 * POW(z[i])*POW_n3(coefficient[2]);
		}
	}
};


/**************
* My Solver to 
*/
class Sover2193 : public GaussNewtonSolver {
public:
	virtual double solve(
		ResidualFunction *f, // Ŀ�꺯��
		double *X,           // ������Ϊ��ֵ�������Ϊ���
		GaussNewtonParams param = GaussNewtonParams(), // �Ż�����
		GaussNewtonReport *report = nullptr // �Ż��������
	)
	{
		double *x = X;
		int n = 0;
		double step = 1;
		int nR = f->nR();
		int nX = f->nX();
		double *J = new double[nR*nX];
		double *R = new double[nR];
		double *delta_x = new double[nR];
		while (n < param.max_iter) {
			f->eval(R, J, x);
			Mat mat_R(nR, 1,CV_64FC1, R);
			Mat mat_J(nR, nX, CV_64FC1, J);
			Mat mat_delta_x(nX, 1, CV_64FC1);
			cv::solve(mat_J, mat_R, mat_delta_x, DECOMP_SVD);

			double max_R = -1;
			double max_mat_delta_x = -1;

			for (int i = 0; i < nR; i++) { // get linf of R
				if (abs(mat_R.at<double>(i, 0)) > max_R) {
					max_R = abs(mat_R.at<double>(i, 0));
				}
			}

			for (int i = 0; i < nX; i++) {
				if (abs(mat_delta_x.at<double>(i, 0)) > max_mat_delta_x) { // get linf of delta_x
					max_mat_delta_x = abs(mat_delta_x.at<double>(i, 0));
				}
			}

			if (max_R <= param.residual_tolerance) { // if linf of R is less than residual_tolerance, break
				report->stop_type = report->STOP_RESIDUAL_TOL;
				report->n_iter = n;
				return 0;
			}
			if (max_mat_delta_x <= param.gradient_tolerance) { // if linf of delta_x is less than gradient_tolerance, break
				report->stop_type = report->STOP_RESIDUAL_TOL;
				report->n_iter = n;
				return 0;
			}

			// update step

			for (int i = 0; i < nX; i++) {
				x[i] += step * mat_delta_x.at<double>(i, 0);
			}

			n++;
		}
		// case of NO_CONVERGE
		report->stop_type = report->STOP_NO_CONVERGE;
		report->n_iter = n;
		return 1;

	}
};

#endif /* HW3_GN_34804D67 */