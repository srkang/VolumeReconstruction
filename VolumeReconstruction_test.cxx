#include <cv.h>
#include <highgui.h>
#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageExport.h>
#include <vtkVersion.h>
#include <iostream>
#include <fstream>
#include <string>

//#include <SFML/System.hpp>


double linearInterpolation( double A, double B, double loc_A, double loc_B, double new_loc)
{
	return (new_loc-loc_A)*(B-A)/(loc_B-loc_A) + A;
}


int main(int argc, char** argv)
{



	std::string path = "D:\\Projects\\3DUS\\3D_abdominal_phantom_11_13_2014"; 
	//std::string path = "C:\\Users\\Alan\\Dropbox\\CNMC_project\\Sono Table\\3DUS\\Abdominal Phantom";
	std::string filename = path+"\\US_201411131228.vti";
	//filename += "\\US_201411131228.dcm";
		
	std::string datafile = path + "\\US_201411131228.txt";
	
	vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkXMLImageDataReader> vtkReader = vtkSmartPointer<vtkXMLImageDataReader>::New();

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

	long int zPos[1000] = {0};  //mm
	std::string textLine;  
	
	for ( int i = 0; i<1000 ; i++) 
	{	
		std::getline(readText,textLine);
		zPos[i] = -1*atoi( textLine.c_str());
		
	}
	readText.close();

	/*
	std::vector<unsigned char> m_img_vol_vec3b; 

	

	for (int z =0 ; z <= extent[5]; z++)
	{
		//std::vector<cv::Mat> layers ( extent[3]+1, extent[1]+1,CV_8UC1);
		cv::Mat layers( extent[3]+1, extent[1]+1,CV_8UC1);

		
		clock_t startTime = clock();

		for (int width =0 ; width <= extent[1]; width++)
			for (int height = 0; height<=extent[3];height++)
			{				
				layers.at<unsigned char>(cv::Point(width,height)) = *static_cast<unsigned char*>( img->GetScalarPointer(width,height,z) );			
			}

		//cv::namedWindow( "2D US", CV_WINDOW_AUTOSIZE );
		//cv::imshow( "2D US", layers);

		if (cv::waitKey(1) >= 0) break;

		//cout << double( clock() - startTime ) << "ms." << endl;

		//std::cout << "z = " << z << std::endl;

		std::vector<unsigned char> a = cv::Mat_<unsigned char>(layers.reshape(1,(extent[3]+1)*(extent[1]+1)));   //gray image
		//std::vector<unsigned char> a = layers;
		m_img_vol_vec3b.insert(m_img_vol_vec3b.end(), a.begin(), a.end());

		//m_img_vol_vec3b.insert(m_img_vol_vec3b.end(), layers.begin(), layers.end());


	}
	std::cout << "Done! " << std::endl;
	// save m_img_vol_vec3b to file
	*/

	
	vtkSmartPointer<vtkImageExport> exporter = vtkSmartPointer<vtkImageExport>::New();
	unsigned char *cImage = new unsigned char[(extent[1]+1)*(extent[3]+1)*(extent[5]+1)];

#if VTK_MAJOR_VERSION <= 5
	exporter->SetInput(img);
#else
	exporter->SetInputData(imageData);
#endif
	exporter->ImageLowerLeftOn();
	exporter->Update();
	exporter->Export(cImage);
	
	std::vector<unsigned char> imageVector(cImage,cImage+(extent[1]+1)*(extent[3]+1)*(extent[5]+1));

	 
	// linear interpolation in C++ 
	
	
	cv::Mat interpolatedImg(extent[3]+1,extent[1]+1,extent[5]+1,CV_8UC1);

	//for (i = 0 ; i < extent[1] ; i++){
	//	for (j = 0 ; j < extent[3];j++){
	//
	//		
	//		linearInterpolation( double A, double B, double loc_A, double loc_B, double new_loc)
	//	}
	//}


	char test_input;
	std::cin >> test_input;
		
	return 0;
}



