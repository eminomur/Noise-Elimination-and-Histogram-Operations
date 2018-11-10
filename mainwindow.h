#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core.hpp>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_select_images_button_clicked();

    void on_actionAbout_Me_triggered();

    void on_actionQuit_triggered();

    void on_show_average_image_button_clicked();

    void on_clean_list_button_clicked();

    void on_histogram_select_image_button_clicked();

    void on_histogram_of_selected_image_button_clicked();

    void on_histogram_equalization_button_clicked();

    void on_local_histogram_equalization_button_clicked();

private:
    Ui::MainWindow *ui;

    // This vector holds all the images selected by user
    std::vector<cv::Mat> image_list;

    // This is a flag to detect if image list is modified or not
    // if list is modified calculations are done again
    bool is_image_list_modified;

    // If select image is clicked this object will be
    // initialized automatically
    cv::Mat resultant_average_image;

    // This function is responsible for calculating average
    void calcualate_average();
    // This function is responsible for checking all images if all have same resolution
    bool check_resolution_of_images() const;

    // This object saves image to be used for histogram calculations
    cv::Mat histogram_selected_image;

    // Function calculates the histogram and returns it as a Mat object
    cv::Mat calculate_histogram(const cv::Mat& input_image);

    // This function plots histogram of image and returns it as a Mat object
    cv::Mat get_histogram_of_image(const cv::Mat& histogram);

    // This function resizes image to make it suitable for local histogram equalization
    cv::Mat resize_image(const cv::Mat& input_image);

    // Local histogram equalization part is done here
    std::vector<cv::Mat> local_histogram_equalization(const cv::Mat& input_image);
};

#endif // MAINWINDOW_H
