#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->number_of_image_label->setText("No image is selected");
    ui->histogram_image_name_label->setText("No image is Selected");
    is_image_list_modified = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_select_images_button_clicked()
{
    QString filter = "Image Files (*.bmp *.dib *.jpeg *.jpg *.jpe *.jp2 *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tiff)";
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Select images to open", QDir::currentPath(), filter);

    if (filenames.empty()) {
        return;
    }

    // This loop fills vector with selected images
    for (QStringList::iterator filenames_list_iterator = filenames.begin();
         filenames_list_iterator != filenames.end();
         ++filenames_list_iterator) {
        image_list.push_back(cv::imread(filenames_list_iterator->toStdString(), cv::IMREAD_GRAYSCALE));
    }

    is_image_list_modified = true;

    // This part is used to show how many image is selected
    if (image_list.empty()) {
        ui->number_of_image_label->setText("No image is selected");
    } else {
        ui->number_of_image_label->setText(QString(std::to_string(image_list.size()).c_str()) + " images are selected");
    }
}

void MainWindow::on_actionAbout_Me_triggered()
{
    QMessageBox::about(this, "About Me", "Muhammed Emin ÖMÜR\n150308007\nSoftware Engineering");
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_show_average_image_button_clicked()
{
    if (image_list.empty() || image_list.size() == 1) {
        QMessageBox::warning(this, "No Image Selected", "Before doing that you should select at least two images");
    } else {
        cv::namedWindow("Resultant Image");

        if (!is_image_list_modified) {
            // Then image is shown as it is
            cv::imshow("Resultant Image", resultant_average_image);
        } else {
            if (!resultant_average_image.empty()) {
                resultant_average_image.release();
            }

            if (!check_resolution_of_images()) {
                QMessageBox::warning(this, "Fatal Error", "To calculate average of images, all images must have same resolution,"
                                                          " cleaning up image list is recommended");
                cv::destroyAllWindows();
                return;
            }

            is_image_list_modified = false;
            calcualate_average();
            cv::imshow("Resultant Image", resultant_average_image);
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }
}

void MainWindow::calcualate_average()
{
    // In the next two lines
    // memory is allocated and initialized all values to 0.0, respectively
    cv::Mat image(image_list[0].rows, image_list[0].cols, CV_64FC1);
    image = cv::Scalar(0.0);

    for (unsigned int list_index = 0; list_index < image_list.size(); ++list_index) {
        int image_row = image_list[list_index].rows;
        int image_col = image_list[list_index].cols;

        for (int row = 0; row < image_row; ++row) {
            for (int col = 0; col < image_col; ++col) {
                image.at<double>(row, col) += static_cast<double>(image_list[list_index].at<uchar>(row, col));
            }
        }
    }

    for (int row = 0; row < image.rows; ++row) {
        for (int col = 0; col < image.cols; ++col) {
            image.at<double>(row, col) = cvRound(image.at<double>(row, col) / image_list.size());
        }
    }

    image.convertTo(image, CV_8UC1);
    resultant_average_image = image;
}

// In this function resolution of first image is taken as an inticator
// If all of the images in vector have the same resolution value
// then function returns true, false otherwise
bool MainWindow::check_resolution_of_images() const
{
    const int EXPECTED_HEIGHT = image_list.at(0).rows;
    const int EXPECTED_WIDTH = image_list.at(0).cols;

    for (std::vector<cv::Mat>::const_iterator image_list_iterator = image_list.begin();
         image_list_iterator != image_list.end();
         ++image_list_iterator) {
        if (image_list_iterator->rows != EXPECTED_HEIGHT || image_list_iterator->cols != EXPECTED_WIDTH) {
            return false;
        }
    }

    return true;
}

void MainWindow::on_clean_list_button_clicked()
{
    image_list.clear();
    ui->number_of_image_label->setText("No image is selected");

    // Since there is probabity of deletion of images in vector,
    // when clean list is clicked, is_image_list_modified is set to true
    is_image_list_modified = true;
}

void MainWindow::on_histogram_select_image_button_clicked()
{
    QString filter = "Image Files (*.bmp *.dib *.jpeg *.jpg *.jpe *.jp2 *.png *.pbm *.pgm *.ppm *.sr *.ras *.tiff *.tiff)";
    QString filename = QFileDialog::getOpenFileName(this, "Select images to open", QDir::currentPath(), filter);

    if (filename == "") {
        return;
    }

    histogram_selected_image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ui->histogram_image_name_label->setText(filename.toStdString().substr(filename.toStdString().find_last_of("/")).c_str() + 1);
}

void MainWindow::on_histogram_of_selected_image_button_clicked()
{
    if (histogram_selected_image.empty()) {
        QMessageBox::warning(this, "No Image", "Before doing that you must select an image first");
        return;
    }

    cv::namedWindow("Histogram of Selected Image");
    cv::imshow("Histogram of Selected Image", get_histogram_of_image(calculate_histogram(histogram_selected_image)));
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void MainWindow::on_histogram_equalization_button_clicked()
{
    if (histogram_selected_image.empty()) {
        QMessageBox::warning(this, "No Image", "Before doing that you must select an image first");
        return;
    }

    cv::Mat equalized_form;
    cv::equalizeHist(histogram_selected_image, equalized_form);

    cv::namedWindow("Histogram of Equalized Image");
    cv::imshow("Histogram of Equalized Image", get_histogram_of_image(calculate_histogram(equalized_form)));
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void MainWindow::on_local_histogram_equalization_button_clicked()
{
    if (histogram_selected_image.empty()) {
        QMessageBox::warning(this, "No Image", "Before doing that you must select an image first");
        return;
    }

    if (histogram_selected_image.rows < 100 || histogram_selected_image.cols < 100) {
        QMessageBox::warning(this, "Low Resolution", "Minimum image resolution must be 100x100");
        return;
    }

    std::vector<cv::Mat> result = local_histogram_equalization(resize_image(histogram_selected_image.clone()));

    cv::namedWindow("10x10");
    cv::namedWindow("25x25");
    cv::namedWindow("50x50");

    cv::imshow("10x10", result[0]);
    cv::imshow("25x25", result[1]);
    cv::imshow("50x50", result[2]);
    cv::waitKey(0);

    cv::destroyAllWindows();
}

cv::Mat MainWindow::calculate_histogram(const cv::Mat& input_image)
{
    cv::Mat histogram;
    int channels[] = {0};
    int histogram_size[] = {256};
    float range[] = {0, 256};
    const float *ranges[] = {range};

    cv::calcHist(&input_image, 1, channels, cv::Mat(), histogram, 1, histogram_size, ranges);

    return histogram;
}

cv::Mat MainWindow::get_histogram_of_image(const cv::Mat& histogram)
{
    const int PLOT_WIDTH = 1024;
    const int PLOT_HEIGTH = 400;
    const int BIN_WIDTH = PLOT_WIDTH / histogram.rows;
    cv::Mat histogram_plot(PLOT_HEIGTH, PLOT_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));

    cv::normalize(histogram, histogram, 0, PLOT_HEIGTH, cv::NORM_MINMAX);

    for (int row = 1; row < histogram.rows; ++row) {
        cv::line(histogram_plot,
                 cv::Point((BIN_WIDTH * (row - 1)), (PLOT_HEIGTH - cvRound(histogram.at<float>(row - 1, 0)))),
                 cv::Point(BIN_WIDTH * row, (PLOT_HEIGTH - cvRound(histogram.at<float>(row, 0)))),
                 cv::Scalar(255, 255, 255));
//        cv::rectangle(histogram_plot,
//                      cv::Point((BIN_WIDTH * (row - 1)), (PLOT_HEIGTH - cvRound(histogram.at<float>(row - 1, 0)))),
//                      cv::Point(BIN_WIDTH * row, PLOT_HEIGTH),
//                      cv::Scalar(255, 255, 255),
//                      cv::FILLED);
    }

    return histogram_plot;
}

// There will be 3 types of local histogram equalization (10, 25, 50)
// so resolution of image must be divided easily for these three divisor
// Minimum 100x100 image is expected
cv::Mat MainWindow::resize_image(const cv::Mat &input_image)
{
    cv::Mat result_image;
    int new_image_width, new_image_height;
    int division;
    const int input_image_width = input_image.cols;
    const int input_image_height = input_image.rows;

    division = input_image.rows / 100;
    if ((input_image_height / 10) % 10 >= 4 && (input_image_height / 10) % 10 <= 6) {
        new_image_height = 100 * division + 50;
    } else if (input_image_height % 100 < 50) {
        new_image_height = 100 * division;
    } else {
        new_image_height = 100 * (division + 1);
    }

    division = input_image.cols / 100;
    if ((input_image_width / 10) % 10 >= 4 && (input_image_width / 10) % 10 <= 6) {
        new_image_width = 100 * division + 50;
    } else if (input_image_width % 100 < 50) {
        new_image_width = 100 * division;
    } else {
        new_image_width = 100 * (division + 1);
    }

    if (new_image_height * new_image_width < input_image.rows * input_image.cols) {
        cv::resize(input_image, result_image, cv::Size(new_image_width, new_image_height), 0, 0, cv::INTER_CUBIC);
    } else {
        cv::resize(input_image, result_image, cv::Size(new_image_width, new_image_height), 0, 0, cv::INTER_AREA);
    }

    return result_image;
}

std::vector<cv::Mat> MainWindow::local_histogram_equalization(const cv::Mat& input_image)
{
    std::vector<cv::Mat> result;
    const int INPUT_IMAGE_ROWS = input_image.rows;
    const int INPUT_IMAGE_COLS = input_image.cols;
    cv::Mat ten_by_ten = input_image.clone();
    cv::Mat twentyfive_by_twentyfive = input_image.clone();
    cv::Mat fifty_by_fifty = input_image.clone();

    // 10x10 local histogram equalization
    for (int row = 0; row < INPUT_IMAGE_ROWS; row += 10) {
        for (int col = 0; col < INPUT_IMAGE_COLS; col += 10) {
            cv::equalizeHist(ten_by_ten(cv::Range(row, cv::min(row + 10, input_image.rows)), cv::Range(col, cv::min(col + 10, input_image.cols))),
                             ten_by_ten(cv::Range(row, cv::min(row + 10, input_image.rows)), cv::Range(col, cv::min(col + 10, input_image.cols))));
        }
    }

    // 25x25 local histogram equalization
    for (int row = 0; row < INPUT_IMAGE_ROWS; row += 25) {
        for (int col = 0; col < INPUT_IMAGE_COLS; col += 25) {
            cv::equalizeHist(twentyfive_by_twentyfive(cv::Range(row, cv::min(row + 25, input_image.rows)),
                                                      cv::Range(col, cv::min(col + 25, input_image.cols))),
                             twentyfive_by_twentyfive(cv::Range(row, cv::min(row + 25, input_image.rows)),
                                                      cv::Range(col, cv::min(col + 25, input_image.cols))));
        }
    }

    // 50x50 local histogram equalization
    for (int row = 0; row < INPUT_IMAGE_ROWS; row += 50) {
        for (int col = 0; col < INPUT_IMAGE_COLS; col += 50) {
            cv::equalizeHist(fifty_by_fifty(cv::Range(row, cv::min(row + 50, input_image.rows)), cv::Range(col, cv::min(col + 50, input_image.cols))),
                             fifty_by_fifty(cv::Range(row, cv::min(row + 50, input_image.rows)), cv::Range(col, cv::min(col + 50, input_image.cols))));
        }
    }

    result.push_back(ten_by_ten);
    result.push_back(twentyfive_by_twentyfive);
    result.push_back(fifty_by_fifty);

    return result;
}
