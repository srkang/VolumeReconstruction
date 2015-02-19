#include <cv.h>
#include <highgui.h>
#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <iostream>
#include <fstream>
#include <string>
//#include "Configuration.h"
//#include "cnmcUtils.h"

//#include "BKOEMDelegate.h"
//#include "cnmcStereoImage.h"

int main(int argc, char** argv)
{
	std::string path = "D:\\Projects\\3DUS\\3D_abdominal_phantom_11_13_2014"; 
	//std::string path = "C:\\Users\\Alan\\Dropbox\\CNMC_project\\Sono Table\\3DUS\\Abdominal Phantom";
	std::string filename = path+"\\US_201411131228.vti";
	//filename += "\\US_201411131228.dcm";
		
	std::string datafile = path + "\\US_201411131228.txt";
	
	vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkXMLImageDataReader> vtkReader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	//vtkSmartPointer< vtkDICOMImageReader > vtkReader = vtkSmartPointer< vtkDICOMImageReader >::New();

	vtkReader->SetFileName(filename.data());
	vtkReader->Update();
	vtkReader->UpdateWholeExtent();
		
	img->DeepCopy(vtkReader->GetOutput());
	std:: cout << "Scalar type " << img->GetScalarType() <<std::endl;
	std::cout << filename << " is loaded." << std::endl;

	int extent[6];
	img->GetExtent( extent);

	// // Visualize
	//vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	//imageViewer->SetInputConnection(img->GetProducerPort());
	//vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =  vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//imageViewer->SetupInteractor(renderWindowInteractor);
	//imageViewer->SetSlice(100);
	//imageViewer->Render();
	//imageViewer->GetRenderer()->ResetCamera();
	//renderWindowInteractor->Initialize();
	//imageViewer->Render();
	//renderWindowInteractor->Start();

	double z_rel = 0.2;  // unit : mm
	double cnt_mm = 13546;  // counts per mm
	double step_cnt = cnt_mm*z_rel;    // step in counts 
			
	// load data file 
	std::ifstream readText;
	readText.open(datafile);
	//readText.open("D:\\Projects\\3DUS\\3D_abdominal_phantom_11_13_2014\\US_201411131228.txt");

	int zPos[1000] = {0};  //mm
	std::string textLine;  

	
	for ( int i = 0; i<1000 ; i++) 
	{	
		std::getline(readText,textLine);
		zPos[i] = -1*atoi( textLine.c_str());
		
	}
	readText.close();

	// copy vtkImageData -> Vector
	//std::vector<cv::Mat> Volume;
	//cv::Mat Volume(extent[1]+1, extent[3]+1, extent[5]+1 , CV_8UC1);
	//unsigned char ***Volume = new unsigned char[extent[1]+1][extent[3]+1][extent[5]+1];
	//std::vector<unsigned char> Volume;
	

	std::vector<unsigned char> m_img_vol_vec3b; 


	for (int z =0 ; z <= extent[5]; z++)
	{
		//std::vector<cv::Mat> layers ( extent[1], extent[3],CV_8UC1);
		cv::Mat layers( extent[3]+1, extent[1]+1,CV_8UC1);


		for (int width =0 ; width <= extent[1]; width++)
		{
			//std::cout << "width = " << width << std::endl;
			for (int height = 0; height<=extent[3];height++)
			{
				//std::cout << "width= " << width << ",height= " << height << ",z= " << z << std::endl;
				//double temp = img->GetScalarComponentAsDouble(x,y,z,0);
				//static_cast<unsigned char*>( img->GetScalarPointer(x,y,z) );

				//unsigned char *temp = static_cast<unsigned char*>( img->GetScalarPointer(width,height,z) );
				//layers.at<unsigned char>(cv::Point(height,width)) = (int)*temp;

				layers.at<unsigned char>(cv::Point(width,height)) = *static_cast<unsigned char*>( img->GetScalarPointer(width,height,z) );
				//std::cout << layers << endl;
				//std::cout << (int)*temp << std::endl;
			}
		}
		cv::namedWindow( "2D US", CV_WINDOW_AUTOSIZE );
		cv::imshow( "2D US", layers);

		if (cv::waitKey(1) >= 0) break;
		m_img_vol_vec3b.insert(m_img_vol_vec3b.end(), a.begin(), a.end());

	}

	// 1D interpolation 







	return 0;
}



