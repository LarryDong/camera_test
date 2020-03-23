#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <opencv.hpp>
#include <iostream>
#include <sys/time.h>


#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))


#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 1

using namespace cv;
using namespace std;


int g_frameCounter = 0;
int g_thresh = 0;
int g_resolution = 0;
int g_windowNumber = 2;
int g_errorCounter = 0;

Mat *get_image(CAMERA_INSTANCE camera_instance, int width, int height) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_I420, 50};
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 20);
    if (!buffer) {
        //cout<<"No image found in 20ms"<<endl;
        g_errorCounter ++;
        if(g_errorCounter == 10){
            cout << "No image for a long time." << endl;
            cout << "Please reboot!" << endl;
        }
        return NULL;
    }
    g_errorCounter = 0;
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    width = VCOS_ALIGN_UP(width, 32);
    height = VCOS_ALIGN_UP(height, 16);
    Mat *image = new cv::Mat(cv::Size(width,(int)(height * 1.5)), CV_8UC1, buffer->data);
    cvtColor(*image, *image, cv::COLOR_YUV2BGR_I420);
    arducam_release_buffer(buffer);
    return image;
}

struct reg {
    uint16_t address;
    uint16_t value;
};


struct reg regs[] = {
    {0x4F00, 0x01},
    {0x3030, 0x04},
    {0x303F, 0x01},
    {0x302C, 0x00},
    {0x302F, 0x7F},
    {0x3823, 0x00},
    {0x0100, 0x00},
    {0X3501, 0X05},
};


static const int regs_size = sizeof(regs) / sizeof(regs[0]);

int write_regs(CAMERA_INSTANCE camera_instance, struct reg regs[], int length){
    int status = 0;
    for(int i = 0; i < length; i++){
        if (arducam_write_sensor_reg(camera_instance, regs[i].address, regs[i].value)) {
            LOG("Failed to write register: 0x%02X, 0x%02X.", regs[i].address, regs[i].value);
            status += 1;
        }
    }
    return status;
}

int main(int argc, char **argv) {
    namedWindow("Settings");        // toolbar
    namedWindow("video", WINDOW_KEEPRATIO);
    namedWindow("bin", WINDOW_KEEPRATIO);
    createTrackbar("thresh", "bin", &g_thresh, 255, NULL);
    createTrackbar("window", "Settings", &g_windowNumber, 2, NULL);
    createTrackbar("resolution", "Settings", &g_resolution, 1, NULL);
    
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    
    width = 640;
    height = 480;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    
	LOG("Change to external trigger....");
    write_regs(camera_instance, regs, regs_size);

	int c = 0;
	LOG("Start capturing...");
    
    // get time.
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    
    int oldRes = 0;
    
	while(c != 'q'){
		Mat *image = get_image(camera_instance, width, height);
        if(!image)
            continue;
        
        g_frameCounter++;
        gettimeofday(&te, NULL);
        int timeUsed = (te.tv_sec-tb.tv_sec)*1000+(te.tv_usec-tb.tv_usec)/1000;
        if(timeUsed > 1000){
            string res = (g_resolution==0? "640x480" : "1080x720");
            cout << res << ", FPS: " << g_frameCounter << endl;
            gettimeofday(&tb, NULL);        // reset begin time.
            g_frameCounter = 0;
        }
        
        // check resolution change
        if(oldRes != g_resolution){
            if(g_resolution == 1){
                width = 1080;
                height = 720;
                cout << "Resolution: 1080x720" << endl;
            }
            else{
                width =640;
                height = 480;
                //res = arducam_set_resolution(camera_instance, &width, &height);
                cout << "Resolution: 640x480" << endl;
            }
                            
            arducam_close_camera(camera_instance);
            arducam_init_camera(&camera_instance);
            arducam_set_resolution(camera_instance, &width, &height);
            write_regs(camera_instance, regs, regs_size);
            oldRes = g_resolution;
        }
        
        
        Mat src, binImg;
        switch(g_windowNumber){
        case 0:
            break;
        case 1:
            imshow("video", *image);
            break;
        case 2:
            src = *image;
            if(src.type()!=CV_8UC1)
                cvtColor(src, src, COLOR_BGR2GRAY);
            imshow("video", src);
            threshold(src, binImg, g_thresh, 255, THRESH_BINARY);
            imshow("bin", binImg);
            break;
        default:
            break;
        }
        
        c = waitKey(1);
        delete image;
	}
	
    LOG("Stop preview...");
    res = arducam_stop_preview(camera_instance);
    if (res) {
        LOG("stop preview status = %d", res);
    }

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}
