/**********************************************************************
*
*  FILE NAME	: functions.c
*
*  DESCRIPTION  : Modular functions used to implement 
*				  triangle rendering for a PLY - Polygon File Format.
* 
*  PLATFORM		: Win32
*
*  DATE                 NAME                     REASON
*  27th Feb,2018        Shashi Shivaraju         ECE_6680_Lab_05
*                       [C88650674]
***********************************************************************/

/*Header file inclusions*/
#include "header.h" 

void RenderTriangles(FILE * PLY_fpt,float X_Degree,float Y_Degree,float Z_Degree)
{
	FILE *fpt = NULL;
	unsigned char *image = NULL;
	float *zbuffer_depth = NULL;
	vector	*image_vectors = NULL,*curr_vector = NULL;
	vertex_data **vertex_pointers = NULL,*curr_vert = NULL;
	face_data	**face_pointers = NULL,*curr_face = NULL;
	vertex_data	*v0_vector = NULL,*v1_vector = NULL,*v2_vector = NULL;
	vertex_data min_vertex = {0},max_vertex = {0},center_vertex = {0},Box_Extent = {0};
	vector cam_pos = {0},up_pos = {0},left_pos = {0},right_pos = {0},
		top_pos = {0},bottom_pos = {0},top_left = {0},temp = {0},abc_pos = {0},temp_2 = {0},
		intersect = {0},temp_3 = {0},temp_4 = {0},temp_5 = {0};
	float rotation_x[3][3] = {0},rotation_y[3][3] = {0},rotation_z[3][3] = {0};
	float E = 0; /*maximum extent of the bounding box on the vertices*/
	float a = 0; /*magnitude of the left vector*/
	float D = 0; /*Component of <A,B,C,D> plane*/
	float n = 0,d = 0,nd = 0,dabs = 0; /*find the distance along image pixel ray to triangle*/
	float dot1 = 0,dot2 = 0,dot3 = 0;
	int ret = 0,i = 0,k = 0,j = 0,col = 0,row = 0;
	int total_vertices = 0,total_faces = 0; /*Total number of vetices and faces in the PLY file*/
	int ROWS = 256,COLS = 256; /*default image size in pixels*/
	
	/*Parse the PLY header to obtain number of vertices and number of faces*/
	ParsePLYHeader(PLY_fpt,&total_vertices,&total_faces);

	/*Allocate memory to store vertex data of triangles*/
	vertex_pointers = (vertex_data **)calloc(total_vertices,sizeof(vertex_data*));
	if(!vertex_pointers)
	{
		printf("memeory allocation failed\n");/*calloc operation failed*/
		return; /*return*/
	}

	/*Allocate memory to store face data of triangles*/
	face_pointers = (face_data **)calloc(total_faces,sizeof(face_data*));
	if(!face_pointers)
	{
		printf("memeory allocation failed\n");/*calloc operation failed*/
		return; /*return*/
	}

	/*memory allocation for the image*/
	image = (unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char)); /*Default image color = black (greyscale 0)*/
	if(!image)
	{
		printf("memeory allocation failed\n");/*calloc operation failed*/
		return; /*return*/
	}
 
	zbuffer_depth = (float *)calloc(ROWS*COLS,sizeof(float)); /*Default image color = black (greyscale 0)*/
	if(!image)
	{
		printf("memeory allocation failed\n");/*calloc operation failed*/
		return; /*return*/
	}

	for(i = 0;i<ROWS*COLS;i++)
	{
		zbuffer_depth[i] = DEFAULT_ZBUFFER_DEPTH;
	}

	image_vectors = (vector	*)calloc(ROWS*COLS,sizeof(vector));
	if(!image_vectors)
	{
		printf("memeory allocation failed\n");/*calloc operation failed*/
		return; /*return*/
	}

	/*Function call to read the data of vertices and faces form the PLY file*/
	ReadPLY_Vertices_Faces(PLY_fpt,
							total_vertices,vertex_pointers,
							total_faces,face_pointers,
							&min_vertex,&max_vertex);

	/*center point of the vertices*/
	center_vertex.x_pos =  (min_vertex.x_pos + max_vertex.x_pos)/2;
	center_vertex.y_pos =  (min_vertex.y_pos + max_vertex.y_pos)/2;
	center_vertex.z_pos =  (min_vertex.z_pos + max_vertex.z_pos)/2;

	/*Extent of the bounding box*/
	Box_Extent.x_pos = max_vertex.x_pos-min_vertex.x_pos;
	Box_Extent.y_pos = max_vertex.y_pos-min_vertex.y_pos;
	Box_Extent.z_pos = max_vertex.z_pos-min_vertex.z_pos;

	/*maximum extent of the bounding box on the vertices*/
	if(Box_Extent.x_pos > Box_Extent.y_pos)
	{
		if(Box_Extent.x_pos > Box_Extent.z_pos)
			E = Box_Extent.x_pos;
		else
			E = Box_Extent.z_pos;
	}
	else
	{
		if(Box_Extent.y_pos > Box_Extent.z_pos)
			E = Box_Extent.y_pos;
		else
			E = Box_Extent.z_pos;
	}


	/*Initial camera position and up vector*/
	cam_pos.x_pos = 1;
	cam_pos.y_pos = 0;
	cam_pos.z_pos = 0;

	up_pos.x_pos = 0;
	up_pos.y_pos = 0;
	up_pos.z_pos = 1;

	/*Initialize rotation matrices*/
	initialize_rotation_matrices(rotation_x,rotation_y,rotation_z,
									X_Degree,Y_Degree,Z_Degree);

	/*Rotate cam vector by X_Degree,Y_Degree and Z_Degree*/
	matrix_multiply(&cam_pos,rotation_x,&temp);
	matrix_multiply(&temp,rotation_y,&cam_pos);
	matrix_multiply(&cam_pos,rotation_z,&temp);
	
	cam_pos.x_pos = temp.x_pos;
	cam_pos.y_pos = temp.y_pos;
	cam_pos.z_pos = temp.z_pos;

	/*Rotate up vectors by X_Degree,Y_Degree and Z_Degree*/
	matrix_multiply(&up_pos,rotation_x,&temp);
	matrix_multiply(&temp,rotation_y,&up_pos);
	matrix_multiply(&up_pos,rotation_z,&temp);
	
	up_pos.x_pos = temp.x_pos;
	up_pos.y_pos = temp.y_pos;
	up_pos.z_pos = temp.z_pos;

	/*move and scale the camera vector*/
	cam_pos.x_pos = 1.5*E*cam_pos.x_pos+center_vertex.x_pos;
	cam_pos.y_pos = 1.5*E*cam_pos.y_pos+center_vertex.y_pos;
	cam_pos.z_pos = 1.5*E*cam_pos.z_pos+center_vertex.z_pos;

	/*calculate the 3D coordinates bounding the image*/
	calculate_3D_bounding_box_coordinates(&cam_pos,&center_vertex,&up_pos,
										 &left_pos,&right_pos,&top_pos,
										 &bottom_pos,&top_left,&temp,
										 &a,E);

	/*calculate the vector coordinates <image> for each of the image pixel*/
	for(i = 0;i<ROWS*COLS;i++)
	{
		ret = convert_index2height_width(i,ROWS,COLS,&col,&row);
		if(ret < 0)
		{
			break;
		}

		image_vectors[i].x_pos = top_left.x_pos + ((float)col/(float)(COLS-1))*(right_pos.x_pos - left_pos.x_pos) + ((float)row/(float)(ROWS-1))*(bottom_pos.x_pos - top_pos.x_pos);
		image_vectors[i].y_pos = top_left.y_pos + ((float)col/(float)(COLS-1))*(right_pos.y_pos - left_pos.y_pos) + ((float)row/(float)(ROWS-1))*(bottom_pos.y_pos - top_pos.y_pos);
		image_vectors[i].z_pos = top_left.z_pos + ((float)col/(float)(COLS-1))*(right_pos.z_pos - left_pos.z_pos) + ((float)row/(float)(ROWS-1))*(bottom_pos.z_pos - top_pos.z_pos);
	} 

	/*Determine if the face has to be rendered or not*/
	for(i=0;i<total_faces;i++)
	{
		printf(".");
		if(0 == i % 100)
			printf("\n");

		curr_face = face_pointers[i];
		/*
		Find the plane equation <A,B,C,D>
		<A,B,C> = <v1 − v0> x <v2 − v0>
		D = - <A,B,C>.<v0>
		*/
		v0_vector = vertex_pointers[curr_face->vertex_one];
		v1_vector = vertex_pointers[curr_face->vertex_two];
		v2_vector = vertex_pointers[curr_face->vertex_three];
		
		/*<v1 − v0>*/
		temp.x_pos = v1_vector->x_pos - v0_vector->x_pos;
		temp.y_pos = v1_vector->y_pos - v0_vector->y_pos;
		temp.z_pos = v1_vector->z_pos - v0_vector->z_pos;
		/*<v2 − v0>*/
		temp_2.x_pos = v2_vector->x_pos - v0_vector->x_pos;
		temp_2.y_pos = v2_vector->y_pos - v0_vector->y_pos;
		temp_2.z_pos = v2_vector->z_pos - v0_vector->z_pos;
		
		/*<A,B,C> = <v1 − v0> x <v2 − v0>*/
		cross_product(&temp,&temp_2,&abc_pos);
		/*D = - <A,B,C>.<v0>*/
		temp.x_pos = -abc_pos.x_pos;
		temp.y_pos = -abc_pos.y_pos;
		temp.z_pos = -abc_pos.z_pos;
		D = scalar_product(&temp,v0_vector);

		/*Calculate the distance along the image pixel ray to the triangle = n/d*/
		/*n = −<A,B,C> .<camera> − D*/
		n  = scalar_product(&temp,&cam_pos);
		n  = n - D;
	
		/*check for each pixel*/
		for(k = 0;k<ROWS*COLS;k++)
		{
			temp.x_pos = image_vectors[k].x_pos - cam_pos.x_pos;
			temp.y_pos = image_vectors[k].y_pos - cam_pos.y_pos;
			temp.z_pos = image_vectors[k].z_pos - cam_pos.z_pos;
			/*d = <A,B,C> . <image − camera>*/
			d = scalar_product(&abc_pos,&temp);
			dabs = fabs(d);
			if(dabs < d_THRESHOLD)
				continue;

			/*Calculate 3D coordinates <intersect> of ray and plane*/
			intersect.x_pos = cam_pos.x_pos + (n/d)*(image_vectors[k].x_pos-cam_pos.x_pos);
			intersect.y_pos = cam_pos.y_pos + (n/d)*(image_vectors[k].y_pos-cam_pos.y_pos);
			intersect.z_pos = cam_pos.z_pos + (n/d)*(image_vectors[k].z_pos-cam_pos.z_pos);

			/*Determine if the intersection point lies within triangle 
			  by calculating the three dot products*/
/**********************************************************************************************/
			/*<v2-v0>*/
			temp.x_pos = v2_vector->x_pos - v0_vector->x_pos;
			temp.y_pos = v2_vector->y_pos - v0_vector->y_pos;
			temp.z_pos = v2_vector->z_pos - v0_vector->z_pos;

			/*<v1-v0>*/
			temp_2.x_pos = v1_vector->x_pos - v0_vector->x_pos;
			temp_2.y_pos = v1_vector->y_pos - v0_vector->y_pos;
			temp_2.z_pos = v1_vector->z_pos - v0_vector->z_pos;

			/*<v2 − v0> x <v1 − v0>*/
			cross_product(&temp,&temp_2,&temp_3);

			/*<intersect - v0>*/
			temp_4.x_pos = intersect.x_pos - v0_vector->x_pos;
			temp_4.y_pos = intersect.y_pos - v0_vector->y_pos;
			temp_4.z_pos = intersect.z_pos - v0_vector->z_pos;

			/*<intersect - v0> x <v1 − v0>*/
			cross_product(&temp_4,&temp_2,&temp_5);

			/*dot1 = <v2 − v0> x <v1 − v0> .<intersect − v0> x <v1 − v0> */
			dot1 = scalar_product(&temp_3,&temp_5);
/**************************************************************************************/
			/*<v0-v1>*/
			temp.x_pos = v0_vector->x_pos - v1_vector->x_pos;
			temp.y_pos = v0_vector->y_pos - v1_vector->y_pos;
			temp.z_pos = v0_vector->z_pos - v1_vector->z_pos;

			/*<v2-v1>*/
			temp_2.x_pos = v2_vector->x_pos - v1_vector->x_pos;
			temp_2.y_pos = v2_vector->y_pos - v1_vector->y_pos;
			temp_2.z_pos = v2_vector->z_pos - v1_vector->z_pos;

			/*<v0 − v1> x <v2 − v1>*/
			cross_product(&temp,&temp_2,&temp_3);

			/*<intersect - v1>*/
			temp_4.x_pos = intersect.x_pos - v1_vector->x_pos;
			temp_4.y_pos = intersect.y_pos - v1_vector->y_pos;
			temp_4.z_pos = intersect.z_pos - v1_vector->z_pos;

			/*<intersect - v1> x <v2 − v1>*/
			cross_product(&temp_4,&temp_2,&temp_5);

			/*dot2 = <v0 − v1> x <v2 − v1> . <intersect - v1> x <v2 − v1>*/
			dot2 = scalar_product(&temp_3,&temp_5);
/*********************************************************************************/
			/*<v1-v2>*/
			temp.x_pos = v1_vector->x_pos - v2_vector->x_pos;
			temp.y_pos = v1_vector->y_pos - v2_vector->y_pos;
			temp.z_pos = v1_vector->z_pos - v2_vector->z_pos;

			/*<v0-v2>*/
			temp_2.x_pos = v0_vector->x_pos - v2_vector->x_pos;
			temp_2.y_pos = v0_vector->y_pos - v2_vector->y_pos;
			temp_2.z_pos = v0_vector->z_pos - v2_vector->z_pos;

			/*<v1 − v2> x <v0 − v2>*/
			cross_product(&temp,&temp_2,&temp_3);

			/*<intersect - v2>*/
			temp_4.x_pos = intersect.x_pos - v2_vector->x_pos;
			temp_4.y_pos = intersect.y_pos - v2_vector->y_pos;
			temp_4.z_pos = intersect.z_pos - v2_vector->z_pos;

			/*<intersect - v2> x <v0 − v2>*/
			cross_product(&temp_4,&temp_2,&temp_5);

			/*dot3 = <v1 − v2> x <v0 − v2> . <intersect - v2> x <v0 − v2>*/
			dot3 = scalar_product(&temp_3,&temp_5);

			/*If any of the dot products is less than zero 
			(if dot1 < 0 or dot2 < 0 ordot3 < 0), 
			then the intersection point lies outside the triangle 
			and it can be skipped*/
			if(dot1 < 0 || dot2 < 0 || dot3 < 0)
			{
				continue;
			}
			else
			{
				/*If the distance to the triangle (n/d) is greater 
				than the current z-buffer value for this pixel,
				then the triangle lies behind a closer triangle 
				and it can beskipped.*/
				if(DEFAULT_ZBUFFER_DEPTH == zbuffer_depth[k]) /*will be only once for each pixel*/
				{
					zbuffer_depth[k] = n/d;
				}
				else if (zbuffer_depth[k] < (n/d))
				{
					continue;
				}
				else
				{
					zbuffer_depth[k] = n/d;
				}
				/*Each pixel inside triangle will have same greyscale value*/						
				image[k] = 155 + (i%100);
				//printf("pixel index %d ,value %d\n",k,image[k]);
				
			}
		}
	}

	/*store image as ppm image*/

	fpt = fopen("ply_image.ppm","wb+");/*open output image file*/
	if(!fpt)                     /*error handling*/
	{
		printf("fopen failed for %s\n","segmented_range_image.ppm");/*failure to open the output file*/
		return -1;          /*return error code*/
	}

	fprintf(fpt,"P5 %d %d 255 ",COLS,ROWS);/*Write the header as per PPM image specification to output image file*/
	fwrite(image,1,ROWS*COLS,fpt);/*write the output image data into file*/

	printf("\nImage file stored as ply_image.ppm\n");

	if(fpt)                      /*Close the output file handle*/
	{
		fclose(fpt);
		fpt = NULL;
	}

	/*deallocate the memory*/
	for(i = 0;i<total_vertices;i++)
	{
		curr_vert = vertex_pointers[i];
		if(curr_vert)
			free(curr_vert);
		curr_vert = NULL;
	}

	for(i = 0; i<total_faces;i++)
	{
		curr_face = face_pointers[i];
		if(curr_face)
			free(curr_face);
		curr_face = NULL;
	}

	if(image_vectors)
		free(image_vectors);
	image_vectors = NULL;
	
	if(face_pointers)
		free(face_pointers);
	face_pointers = NULL;

	if(vertex_pointers)
		free(vertex_pointers);
	vertex_pointers = NULL;

	if(image)
		free(image);
	image = NULL;

	if(zbuffer_depth)
		free(zbuffer_depth);
	zbuffer_depth = NULL;
}

/* Function to parse the PLY header to obtain number of vertices and number of faces*/
void ParsePLYHeader(FILE * PLY_fpt,int *vertices,int *faces)
{
	char strng[MAX_FILENAME];
	int tvertices = 0,tfaces = 0; 
	while(0<fscanf(PLY_fpt,"%s",&strng))/*will exit at EOF or fscanf error*/
	{
		if(!strcmp(strng,"vertex"))
		{
			/*vertex field found*/
			fscanf(PLY_fpt,"%d",&tvertices);
		}
		else if(!strcmp(strng,"face"))
		{
			/*face field found*/
			fscanf(PLY_fpt,"%d",&tfaces);
		}
		else if(!strcmp(strng,"end_header"))
		{
			/*end of header found*/
			break;
		}
		memset(strng,0,sizeof(strng));
	}

	*vertices = tvertices;
	*faces	= tfaces;
}

/* Function to read the vertices and faces from the PLY file */
void ReadPLY_Vertices_Faces(FILE * PLY_fpt,
					   unsigned int total_vertices,vertex_data** vertex_pointers,
					   unsigned int total_faces,face_data** face_pointers,
					   vertex_data *min_vertex,vertex_data *max_vertex)
{
	vertex_data* curr_vertex = NULL;
	face_data*	 curr_face = NULL;
	int i = 0;
	unsigned int index = 0;
	float x_pos = 0,y_pos = 0,z_pos = 0;
	float x_min = 0,y_min = 0,z_min = 0,x_max = 0,y_max = 0,z_max = 0;
	unsigned int vertices_num = 3,vertex_one = 0,vertex_two = 0,vertex_three = 0;

	for(i = 0;i < total_vertices; i++)
	{
		fscanf(PLY_fpt,"%f %f %f",&x_pos,&y_pos,&z_pos);

		/*find vectors <min> and <max>*/
		if(x_min > x_pos)
			x_min = x_pos;
		if(y_min > y_pos)
			y_min = y_pos;
		if(z_min > z_pos)
			z_min = z_pos;

		if(x_max < x_pos)
			x_max = x_pos;
		if(y_max < y_pos)
			y_max = y_pos;
		if(z_max < z_pos)
			z_max = z_pos;
		
		curr_vertex = (vertex_data*)calloc(1,sizeof(vertex_data));
		if(!curr_vertex)
		{
			printf("memory allocation failed\n");
			return;
		}

		curr_vertex->index = i;
		curr_vertex->x_pos = x_pos;
		curr_vertex->y_pos = y_pos;
		curr_vertex->z_pos = z_pos;
		vertex_pointers[i] = curr_vertex;
	}
	
	min_vertex->x_pos = x_min; 
	min_vertex->y_pos = y_min;
	min_vertex->z_pos = z_min;

	max_vertex->x_pos =	x_max;
	max_vertex->y_pos = y_max;
	max_vertex->z_pos = z_max;

	i = 0;
	
	for(i = 0;i < total_faces; i++)
	{
		fscanf(PLY_fpt,"%d %d %d %d",&vertices_num,&vertex_one,&vertex_two,&vertex_three);
		
		curr_face = (face_data*)calloc(1,sizeof(face_data));
		if(!curr_face)
		{
			printf("memory allocation failed\n");
			return;
		}

		curr_face->index = i;
		curr_face->total_vertices = vertices_num;
		curr_face->vertex_one = vertex_one;
		curr_face->vertex_two = vertex_two;
		curr_face->vertex_three = vertex_three;
		face_pointers[i] = curr_face;
	}
}

void initialize_rotation_matrices(float rotation_x[3][3],float rotation_y[3][3],float rotation_z[3][3],
								  float X_Degree,float Y_Degree,float Z_Degree)
{
	rotation_x[0][0]= 1;
	rotation_x[0][1]= 0;
	rotation_x[0][2]= 0;
	rotation_x[1][0]= 0;
	rotation_x[1][1]= cos(PI_VALUE*X_Degree/180);
	rotation_x[1][2]= -sin(PI_VALUE*X_Degree/180);
	rotation_x[2][0]= 0;
	rotation_x[2][1]= sin(PI_VALUE*X_Degree/180);
	rotation_x[2][2]= cos(PI_VALUE*X_Degree/180);

	rotation_y[0][0]= cos(PI_VALUE*Y_Degree/180);
	rotation_y[0][1]= 0;
	rotation_y[0][2]= sin(PI_VALUE*Y_Degree/180);
	rotation_y[1][0]= 0;
	rotation_y[1][1]= 1;
	rotation_y[1][2]= 0;
	rotation_y[2][0]= -sin(PI_VALUE*Y_Degree/180);
	rotation_y[2][1]= 0;
	rotation_y[2][2]= cos(PI_VALUE*Y_Degree/180);

	rotation_z[0][0]= cos(PI_VALUE*Z_Degree/180);
	rotation_z[0][1]= -sin(PI_VALUE*Z_Degree/180);
	rotation_z[0][2]= 0;
	rotation_z[1][0]= sin(PI_VALUE*Z_Degree/180);
	rotation_z[1][1]= cos(PI_VALUE*Z_Degree/180);
	rotation_z[1][2]= 0;
	rotation_z[2][0]= 0;
	rotation_z[2][1]= 0;
	rotation_z[2][2]= 1;
}

/*Function to multiply vector with a 3x3 rotation matrix*/
void matrix_multiply(vector *in,float rotation_mat[3][3],vector *out)
{
	out->x_pos = rotation_mat[0][0]*in->x_pos+rotation_mat[0][1]*in->y_pos+rotation_mat[0][2]*in->z_pos;
	out->y_pos = rotation_mat[1][0]*in->x_pos+rotation_mat[1][1]*in->y_pos+rotation_mat[1][2]*in->z_pos;
	out->z_pos = rotation_mat[2][0]*in->x_pos+rotation_mat[2][1]*in->y_pos+rotation_mat[2][2]*in->z_pos;
}

/*Function to calculate cross product of two vectors*/
void cross_product(vector *u,vector *v,vector *uxv)
{
	uxv->x_pos = (u->y_pos*v->z_pos)-(u->z_pos*v->y_pos);
	uxv->y_pos = (u->z_pos*v->x_pos)-(u->x_pos*v->z_pos);
	uxv->z_pos = (u->x_pos*v->y_pos)-(u->y_pos*v->x_pos);
}

/*Function to calculate scalar product of two vectors*/
float scalar_product(vector *u,vector *v)
{
	return (u->x_pos*v->x_pos + u->y_pos*v->y_pos +u->z_pos*v->z_pos);

}

/*calculate the 3D coordinates bounding the image*/
void calculate_3D_bounding_box_coordinates(vector *cam_pos,vertex_data *center_vertex,vector *up_pos,
										   vector *left_pos,vector *right_pos,vector *top_pos,
										   vector *bottom_pos,vector *top_left,vector *temp,
										   float *a,float E)
{
	float a_temp = 0;
	/*To calculate <temp> = <center-camera> */
	temp->x_pos = center_vertex->x_pos - cam_pos->x_pos;
	temp->y_pos = center_vertex->y_pos - cam_pos->y_pos;
	temp->z_pos = center_vertex->z_pos - cam_pos->z_pos;

	/*To calculate <left> = <up>x<center-camera>*/
	cross_product(up_pos,temp,left_pos);

	/*To calculate a = ||<left>||*/
	a_temp = sqrt(SQR(left_pos->x_pos)+SQR(left_pos->y_pos)+SQR(left_pos->z_pos));

	/*To calculate <left> = (E/2a)<left>+<center>*/
	left_pos->x_pos = ((E/(2*a_temp)) * (left_pos->x_pos)) + center_vertex->x_pos;
	left_pos->y_pos = ((E/(2*a_temp)) * (left_pos->y_pos)) + center_vertex->y_pos;
	left_pos->z_pos = ((E/(2*a_temp)) * (left_pos->z_pos)) + center_vertex->z_pos;

	/*To calculate <right> = <center-camera> x <up>*/
	cross_product(temp,up_pos,right_pos);

	/*To calculate <right> = (E/2a)<right>+<center>*/
	right_pos->x_pos = ((E/(2*a_temp)) * right_pos->x_pos) + center_vertex->x_pos;
	right_pos->y_pos = ((E/(2*a_temp)) * right_pos->y_pos) + center_vertex->y_pos;
	right_pos->z_pos = ((E/(2*a_temp)) * right_pos->z_pos) + center_vertex->z_pos;

	/*To calculte <top> = (E/2)<up>+<center>*/
	top_pos->x_pos = ((E/2) * up_pos->x_pos) + center_vertex->x_pos;
	top_pos->y_pos = ((E/2) * up_pos->y_pos) + center_vertex->y_pos;
	top_pos->z_pos = ((E/2) * up_pos->z_pos) + center_vertex->z_pos;

	/*To calculte <bottom> = (-E/2)<up>+<center>*/
	bottom_pos->x_pos = ((-E/2) * up_pos->x_pos) + center_vertex->x_pos;
	bottom_pos->y_pos = ((-E/2) * up_pos->y_pos) + center_vertex->y_pos;
	bottom_pos->z_pos = ((-E/2) * up_pos->z_pos) + center_vertex->z_pos;

	/*To calculte <top_left> = (E/2)<up>+<left>*/
	top_left->x_pos = ((E/2) * up_pos->x_pos) + left_pos->x_pos;
	top_left->y_pos = ((E/2) * up_pos->y_pos) + left_pos->y_pos;
	top_left->z_pos = ((E/2) * up_pos->z_pos) + left_pos->z_pos;

	*a = a_temp;

}

int convert_index2height_width(int index,
								int ROW,int COL,/* size of image */
								int *x_pos,int *y_pos)/* pixel position */
{
	if(index>ROW*COL || !ROW || !COL)
	{
		/*printf("invalid -index out of range or invalid image size\
			   index %d Image Size: ROW %d COL %d",index,ROW,COL);*/
		return -1;
	}

	*y_pos = index/COL;
	*x_pos = index -(*y_pos*COL);
	return 0;

}