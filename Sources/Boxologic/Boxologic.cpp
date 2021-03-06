// Boxologic.cpp : Defines the entry point for the console application.
//
//----------------------------------------------------------------------------
// INCLUDED HEADER FILES
//----------------------------------------------------------------------------
#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

#define TRUE 1
#define FALSE 0

void print_help(void);

#pragma region Helper classes
struct boxinfo
{
	bool is_packed;
	int dim1, dim2, dim3, n, cox, coy, coz, packx, packy, packz;

	int volume() { return dim1 * dim2 * dim3; }
	void setPacked(int cboxx, int cboxy, int cboxz)
	{
		is_packed = true;
		packx = cboxx;
		packy = cboxy;
		packz = cboxz;
	}
	void write(FILE *f)
	{
		if (is_packed)
			fprintf(f, "%5d%5d%5d%5d%5d%5d\n",	cox, coy, coz, packx, packy, packz);
		else
			fprintf(f, "%5d%5d%5d\n", dim1, dim2, dim3);
	}

	//----------------------------------------------------------------------------
	// TRANSFORMS THE FOUND COORDINATE SYSTEM TO THE ONE ENTERED BY THE USER AND
	// WRITES THEM TO THE REPORT FILE
	//----------------------------------------------------------------------------
	void write_boxlist_file(FILE *f, int variant, int index)
	{
		char strx[5];
		char strpackst[5];
		char strdim1[5], strdim2[5], strdim3[5];
		char strcox[5], strcoy[5], strcoz[5];
		char strpackx[5], strpacky[5], strpackz[5];

		int x, y, z, bx, by, bz;

		switch (variant)
		{
		case 1:
			x = cox;
			y = coy;
			z = coz;
			bx = packx;
			by = packy;
			bz = packz;
			break;
		case 2:
			x = coz;
			y = coy;
			z = cox;
			bx = packz;
			by = packy;
			bz = packx;
			break;
		case 3:
			x = coy;
			y = coz;
			z = cox;
			bx = packy;
			by = packz;
			bz = packx;
			break;
		case 4:
			x = coy;
			y = cox;
			z = coz;
			bx = packy;
			by = packx;
			bz = packz;
			break;
		case 5:
			x = cox;
			y = coz;
			z = coy;
			bx = packx;
			by = packz;
			bz = packy;
			break;
		case 6:
			x = coz;
			y = cox;
			z = coy;
			bx = packz;
			by = packx;
			bz = packy;
			break;
		}

		sprintf(strx, "%d", index);
		sprintf(strpackst, "%d", is_packed ? 1 : 0);
		sprintf(strdim1, "%d", dim1);
		sprintf(strdim2, "%d", dim2);
		sprintf(strdim3, "%d", dim3);
		sprintf(strcox, "%d", x);
		sprintf(strcoy, "%d", y);
		sprintf(strcoz, "%d", z);
		sprintf(strpackx, "%d", bx);
		sprintf(strpacky, "%d", by);
		sprintf(strpackz, "%d", bz);

		cox = x;
		coy = y;
		coz = z;
		packx = bx;
		packy = by;
		packz = bz;
		fprintf(f, "%5s%5s%9s%9s%9s%9s%9s%9s%9s%9s%9s\n", strx, strpackst, strdim1, strdim2, strdim3, strcox, strcoy, strcoz, strpackx, strpacky, strpackz);
	}
};

struct palletinfo
{
	long xx, yy, zz;
	int variant;
	double total_pallet_volume()
	{
		return double(xx) * double(yy) * double(zz); 
	};
	long pallet_x()
	{
		switch (variant)
		{
		case 1:	return xx;
		case 2:	return zz;
		case 3:	return zz;
		case 4:	return yy;
		case 5:	return xx;
		case 6:	return yy;
		default: return 0;
		}
	}
	long pallet_y()
	{
		switch (variant)
		{
		case 1: return yy;
		case 2: return yy;
		case 3: return xx;
		case 4: return xx;
		case 5: return zz;
		case 6: return zz;
		default: return 0;
		}
	}
	long pallet_z()
	{
		switch (variant)
		{
		case 1:	return zz;
		case 2: return xx;
		case 3: return yy;
		case 4: return zz;
		case 5: return yy;
		case 6: return xx;
		default: return 0;
		}	
	}
};


struct layerlist
{
	layerlist(long int leval, int ldim)
	{
		layereval = leval;
		layerdim = ldim;
	}
	bool operator<(const layerlist &l) const
	{
		return layereval < l.layereval;
	}
	std::string ToString() const
	{
		std::stringstream ss;
		ss << "ldim =" << layerdim << ", eval = " << layereval;
		return ss.str();
	}
	long int layereval;
	int layerdim;
};

struct scrappad
{
	scrappad() : prev(0), next(0), cumx(0), cumz(0) {};
	struct scrappad *prev, *next;
	int cumx, cumz;
};
#pragma endregion
char version[] = "0.01";

class Boxologic
{
public:
	FILE *report_output_file, *visualizer_file;
	time_t start, finish;

	palletinfo pallet;
	std::vector<boxinfo> boxlist;
	std::vector<layerlist> layers;
	struct scrappad *scrapfirst, *smallestz, *trash;

	std::map<int, double> best_iterations;

	bool packing, layerdone, evened;
	int best_variant;

	int layerinlayer;
	int prelayer;
	int lilz;
	int number_of_iterations;
	int remainpy, remainpz;
	int packedy;
	int prepackedy;
	int layerthickness;
	int preremainpy;
	int best_iteration;
	int packednumbox;
	int number_packed_boxes;

	double packedvolume;
	double best_solution_volume;
	double total_box_volume;
	double pallet_volume_used_percentage;

	void initialize();
	void read_boxlist_input(const std::string &filename);
	void execute_iterations(void);
	void list_candidate_layers(bool show = false);

	int pack_layer(bool packingbest, bool &hundredpercent);
	int find_layer(int thickness);
	void find_box(int hmx, int hy, int hmy, int hz, int hmz
		, int &cboxi, int &cboxx, int &cboxy, int &cboxz);
	void analyze_box(int x, int hmx, int hy, int hmy, int hz, int hmz
		, int dim1, int dim2, int dim3
		, int &bfx, int &bfy, int &bfz
		, int &bbfx, int &bbfy, int &bbfz
		, int &boxi, int &boxx, int &boxy, int &boxz
		, int &bboxi, int &bboxx, int &bboxy, int &bboxz
	);
	void find_smallest_z(void);
	void Checkfound(int &cboxi, int &cboxx, int &cboxy, int &cboxz
		, int boxi, int boxx, int boxy, int boxz
		, int bboxi, int bboxx, int bboxy, int bboxz);
	void Volume_check(int cboxi, int cboxx, int cboxy, int cboxz, bool packingbest, bool &hundredPercent);
	void report_results(const std::string &filename);
};

//----------------------------------------------------------------------------
// MAIN PROGRAM
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	std::string filename;
	//Parse Command line options
	if (argc == 2 || argc == 3)
	{
		std::cout << "argc = " << argc << std::endl;
		if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--inputfile") == 0)
		{
			if (argc == 3)
			{
				filename = argv[2];
				std::cout << filename << std::endl;
			}
			else
			{
				printf("A filename is required.\n\n");
				print_help();
				exit(1);
			}
		}
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
		{
			printf("Boxologic version %s\n", version);
			return(0);
		}
		else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			print_help();
			return(0);
		}
		else
		{
			print_help();
			exit(1);
		}
	}
	else
	{
		print_help();
		exit(1);
	}
	Boxologic boxologic;
	boxologic.read_boxlist_input(filename);
	boxologic.initialize();
	boxologic.execute_iterations();
	boxologic.report_results(filename);
	return(0);
}
#pragma region Loading and initialization
//----------------------------------------------------------------------------
// PERFORMS INITIALIZATIONS
//----------------------------------------------------------------------------
void Boxologic::initialize()
{
	total_box_volume = 0.0;
	for (short x = 0; x < short(boxlist.size()); x++)
		total_box_volume += boxlist[x].volume();

	scrapfirst = new scrappad;
	best_solution_volume = 0.0;
	number_of_iterations = 0;
}
//----------------------------------------------------------------------------
// READS THE PALLET AND BOX SET DATA ENTERED BY THE USER FROM THE INPUT FILE
//----------------------------------------------------------------------------
void Boxologic::read_boxlist_input(const std::string &filename)
{
	FILE *boxlist_input_file;
	if ((boxlist_input_file = fopen(filename.c_str(), "r")) == NULL)
	{
		printf("Cannot open file %s\n", filename.c_str());
		exit(1);
	}

	char strxx[6], stryy[6], strzz[6];
	if (fscanf(boxlist_input_file, "%s %s %s", strxx, stryy, strzz) == EOF)
		throw std::exception("EOF reached?");

	pallet.xx = atoi(strxx);
	pallet.yy = atoi(stryy);
	pallet.zz = atoi(strzz);

	char lbl[6], dim1[6], dim2[6], dim3[6], boxn[6];
	while (fscanf(boxlist_input_file, "%s %s %s %s %s", lbl, dim1, dim2, dim3, boxn) != EOF)
	{
		boxinfo bi;
		bi.dim1 = atoi(dim1);
		bi.dim2 = atoi(dim2);
		bi.dim3 = atoi(dim3);

		int n = atoi(boxn);
		bi.n = n;
		for (int i = 0; i < n; ++i)
			boxlist.push_back(bi);
	}
	fclose(boxlist_input_file);
	return;
}
#pragma endregion

//----------------------------------------------------------------------------
// ITERATIONS ARE DONE AND PARAMETERS OF THE BEST SOLUTION ARE FOUND
//----------------------------------------------------------------------------
void Boxologic::execute_iterations(void)
{
	bool hundredpercent = false;
	time(&start);
	for (int variant = 1; variant <= 6; variant++)
	{
		pallet.variant = variant;

		list_candidate_layers();

		for (int layersindex = 0; layersindex < short(layers.size()); layersindex++)
		{
			++number_of_iterations;
			time(&finish);
			double elapsed_time = difftime(finish, start);
			printf("VARIANT: %5d; ITERATION (TOTAL): %5d; BEST SO FAR: %.3f %%; TIME: %.0f\n"
				, variant, number_of_iterations, pallet_volume_used_percentage, elapsed_time);
			packedvolume = 0.0;
			packedy = 0;
			packing = true;
			layerthickness = layers[layersindex].layerdim;
			int itelayer = layersindex;
			remainpy = pallet.pallet_y();
			remainpz = pallet.pallet_z();
			packednumbox = 0;
			for (short x = 0; x < short(boxlist.size()); x++)
				boxlist[x].is_packed = false;

			//BEGIN DO-WHILE
			do
			{
				layerinlayer = 0;
				layerdone = false;
				pack_layer(false, hundredpercent);
				packedy = packedy + layerthickness;
				remainpy = pallet.pallet_y() - packedy;
				if (layerinlayer)
				{
					prepackedy = packedy;
					preremainpy = remainpy;
					remainpy = layerthickness - prelayer;
					packedy = packedy - layerthickness + prelayer;
					remainpz = lilz;
					layerthickness = layerinlayer;
					layerdone = false;
					pack_layer(false, hundredpercent);
					packedy = prepackedy;
					remainpy = preremainpy;
					remainpz = pallet.pallet_z();
				}
				find_layer(remainpy);
			} while (packing);
			// END DO-WHILE

			if (packedvolume >= best_solution_volume)
			{
				best_solution_volume = packedvolume;
				best_variant = variant;
				best_iteration = itelayer;
				number_packed_boxes = packednumbox;

				best_iterations[itelayer] = packedvolume;
			}

			if (hundredpercent) break;
			pallet_volume_used_percentage = best_solution_volume * 100 / pallet.total_pallet_volume();
		}
		if (hundredpercent) break;
		if ((pallet.xx == pallet.yy) && (pallet.yy == pallet.zz)) variant = 6;
	}
	time(&finish);
}
#pragma region list_candidate_layers
//----------------------------------------------------------------------------
// LISTS ALL POSSIBLE LAYER HEIGHTS BY GIVING A WEIGHT VALUE TO EACH OF THEM.
//----------------------------------------------------------------------------
void Boxologic::list_candidate_layers(bool show)
{
	for (short x = 0; x < short(boxlist.size()); x++)
	{
		int exdim, dimdif, dimen2, dimen3;
		for (int y = 1; y <= 3; y++)
		{
			switch (y)
			{
			case 1:
				exdim = boxlist[x].dim1;
				dimen2 = boxlist[x].dim2;
				dimen3 = boxlist[x].dim3;
				break;
			case 2:
				exdim = boxlist[x].dim2;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim3;
				break;
			case 3:
				exdim = boxlist[x].dim3;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim2;
				break;
			}
			if (
				(exdim > pallet.pallet_y())
				|| (
				((dimen2 > pallet.pallet_x()) || (dimen3 > pallet.pallet_z()))
					&& ((dimen3 > pallet.pallet_x()) || (dimen2 > pallet.pallet_z()))
					)
				) continue;

			bool same = false;
			for (int k = 0; k < short(layers.size()); k++)
			{
				if (exdim == layers[k].layerdim)
				{
					same = true;
					continue;
				}
			}
			if (same) continue;

			long int layereval = 0;
			for (int z = 0; z < short(boxlist.size()); z++)
			{
				if (!(x == z))
				{
					dimdif = abs(exdim - boxlist[z].dim1);
					if (abs(exdim - boxlist[z].dim2) < dimdif)
						dimdif = abs(exdim - boxlist[z].dim2);
					if (abs(exdim - boxlist[z].dim3) < dimdif)
						dimdif = abs(exdim - boxlist[z].dim3);
					layereval = layereval + dimdif;
				}
			}
			layers.push_back(layerlist(layereval, exdim));
		} // y (direction)
	} // x (boxes)
	std::sort(layers.begin(), layers.end());

	if (show)
	{
		for (std::vector<layerlist>::const_iterator itLayer = layers.begin(); itLayer != layers.end(); ++itLayer)
			std::cout << itLayer->ToString() << std::endl;
	}
}
#pragma endregion

#pragma region pack_layer
//----------------------------------------------------------------------------
// PACKS THE BOXES FOUND AND ARRANGES ALL VARIABLES AND RECORDS PROPERLY
//----------------------------------------------------------------------------
int Boxologic::pack_layer(bool packingbest, bool & hundredpercent)
{
	if (!layerthickness)
	{
		packing = false;
		return 0;
	}

	scrapfirst->cumx = pallet.pallet_x();
	scrapfirst->cumz = 0;
	int cboxi = 0, cboxx = 0, cboxy = 0, cboxz = 0;

	while (1)
	{
		find_smallest_z();

		if (!smallestz->prev && !smallestz->next)
		{
			//*** SITUATION-1: NO BOXES ON THE RIGHT AND LEFT SIDES ***
			int lenx = smallestz->cumx;
			int lpz = remainpz - smallestz->cumz;
			int boxi = 0, boxx = 0, boxy = 0, boxz = 0;
			int bboxi = 0, bboxx = 0, bboxy = 0, bboxz = 0;
			find_box(lenx, layerthickness, remainpy, lpz, lpz
				, cboxi, cboxx, cboxy, cboxz);

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].cox = 0;
			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx)
			{
				smallestz->cumz = smallestz->cumz + cboxz;
			}
			else
			{
				smallestz->next = new scrappad;
				smallestz->next->prev = smallestz;
				smallestz->next->cumx = smallestz->cumx;
				smallestz->next->cumz = smallestz->cumz;
				smallestz->cumx = cboxx;
				smallestz->cumz = smallestz->cumz + cboxz;
			}
			Volume_check(cboxi, cboxx, cboxy, cboxz, packingbest, hundredpercent);
		}
		else if (!smallestz->prev)
		{
			//*** SITUATION-2: NO BOXES ON THE LEFT SIDE ***
			int lenx = smallestz->cumx;
			int lenz = smallestz->next->cumz - smallestz->cumz;
			int lpz = remainpz - smallestz->cumz;
			int boxi = 0, boxx = 0, boxy = 0, boxz = 0;
			int bboxi = 0, bboxx = 0, bboxy = 0, bboxz = 0;
			find_box(lenx, layerthickness, remainpy, lenz, lpz
				, cboxi, cboxx, cboxy, cboxz);

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx)
			{
				boxlist[cboxi].cox = 0;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->cumz = smallestz->next->cumz;
					smallestz->cumx = smallestz->next->cumx;
					trash = smallestz->next;
					smallestz->next = smallestz->next->next;
					if (smallestz->next)
					{
						smallestz->next->prev = smallestz;
					}
					delete trash;
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				boxlist[cboxi].cox = smallestz->cumx - cboxx;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->cumx = smallestz->cumx - cboxx;
				}
				else
				{
					smallestz->next->prev = new scrappad;
					smallestz->next->prev->next = smallestz->next;
					smallestz->next->prev->prev = smallestz;
					smallestz->next = smallestz->next->prev;
					smallestz->next->cumx = smallestz->cumx;
					smallestz->cumx = smallestz->cumx - cboxx;
					smallestz->next->cumz = smallestz->cumz + cboxz;
				}
			}
			Volume_check(cboxi, cboxx, cboxy, cboxz, packingbest, hundredpercent);
		}
		else if (!smallestz->next)
		{
			//*** SITUATION-3: NO BOXES ON THE RIGHT SIDE ***
			int lenx = smallestz->cumx - smallestz->prev->cumx;
			int lenz = smallestz->prev->cumz - smallestz->cumz;
			int lpz = remainpz - (*smallestz).cumz;
			int boxi = 0, boxx = 0, boxy = 0, boxz = 0;
			int bboxi = 0, bboxx = 0, bboxy = 0, bboxz = 0;
			find_box(lenx, layerthickness, remainpy, lenz, lpz
				, cboxi, cboxx, cboxy, cboxz);

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			boxlist[cboxi].cox = smallestz->prev->cumx;

			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->cumx;
					smallestz->prev->next = NULL;
					delete smallestz;
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
				}
				else
				{
					smallestz->prev->next = new scrappad;
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			Volume_check(cboxi, cboxx, cboxy, cboxz, packingbest, hundredpercent);
		}
		else if (smallestz->prev->cumz == smallestz->next->cumz)
		{
			//*** SITUATION-4: THERE ARE BOXES ON BOTH OF THE SIDES ***
			//*** SUBSITUATION-4A: SIDES ARE EQUAL TO EACH OTHER ***
			int lenx = smallestz->cumx - smallestz->prev->cumx;
			int lenz = smallestz->prev->cumz - smallestz->cumz;
			int lpz = remainpz - smallestz->cumz;

			int boxi = 0, boxx = 0, boxy = 0, boxz = 0;
			int bboxi = 0, bboxx = 0, bboxy = 0, bboxz = 0;
			find_box(lenx, layerthickness, remainpy, lenz, lpz
				, cboxi, cboxx, cboxy, cboxz);

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				boxlist[cboxi].cox = smallestz->prev->cumx;
				if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					smallestz->prev->cumx = smallestz->next->cumx;
					if (smallestz->next->next)
					{
						smallestz->prev->next = smallestz->next->next;
						smallestz->next->next->prev = smallestz->prev;
						delete smallestz;
					}
					else
					{
						smallestz->prev->next = NULL;
						delete smallestz;
					}
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else if (smallestz->prev->cumx < pallet.pallet_x() - smallestz->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->cumx = smallestz->cumx - cboxx;
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
				}
				else
				{
					boxlist[cboxi].cox = smallestz->prev->cumx;
					smallestz->prev->next = new scrappad;
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
					boxlist[cboxi].cox = smallestz->prev->cumx;
				}
				else
				{
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
					smallestz->next->prev = new scrappad;
					smallestz->next->prev->next = smallestz->next;
					smallestz->next->prev->prev = smallestz;
					smallestz->next = smallestz->next->prev;
					smallestz->next->cumx = smallestz->cumx;
					smallestz->next->cumz = smallestz->cumz + cboxz;
					smallestz->cumx = smallestz->cumx - cboxx;
				}
			}
			Volume_check(cboxi, cboxx, cboxy, cboxz, packingbest, hundredpercent);
		}
		else
		{
			//*** SUBSITUATION-4B: SIDES ARE NOT EQUAL TO EACH OTHER ***
			int lenx = smallestz->cumx - smallestz->prev->cumx;
			int lenz = smallestz->prev->cumz - smallestz->cumz;
			int lpz = remainpz - smallestz->cumz;
			find_box(lenx, layerthickness, remainpy, lenz, lpz
				, cboxi, cboxx, cboxy, cboxz);

			if (layerdone) break;
			if (evened) continue;

			boxlist[cboxi].coy = packedy;
			boxlist[cboxi].coz = smallestz->cumz;
			boxlist[cboxi].cox = smallestz->prev->cumx;
			if (cboxx == smallestz->cumx - smallestz->prev->cumx)
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->cumx;
					smallestz->prev->next = smallestz->next;
					smallestz->next->prev = smallestz->prev;
					delete smallestz;
				}
				else
				{
					smallestz->cumz = smallestz->cumz + cboxz;
				}
			}
			else
			{
				if (smallestz->cumz + cboxz == smallestz->prev->cumz)
				{
					smallestz->prev->cumx = smallestz->prev->cumx + cboxx;
				}
				else if (smallestz->cumz + cboxz == smallestz->next->cumz)
				{
					boxlist[cboxi].cox = smallestz->cumx - cboxx;
					smallestz->cumx = smallestz->cumx - cboxx;
				}
				else
				{
					smallestz->prev->next = new scrappad;
					smallestz->prev->next->prev = smallestz->prev;
					smallestz->prev->next->next = smallestz;
					smallestz->prev = smallestz->prev->next;
					smallestz->prev->cumx = smallestz->prev->prev->cumx + cboxx;
					smallestz->prev->cumz = smallestz->cumz + cboxz;
				}
			}
			Volume_check(cboxi, cboxx, cboxy, cboxz, packingbest, hundredpercent);
		}
	}
	return 0;
}
#pragma endregion
#pragma region find_layer
//----------------------------------------------------------------------------
// FINDS THE MOST PROPER LAYER HIGHT BY LOOKING AT THE UNPACKED BOXES AND THE
// REMAINING EMPTY SPACE AVAILABLE
//----------------------------------------------------------------------------
int Boxologic::find_layer(int thickness)
{
	int exdim, dimdif, dimen2, dimen3;
	long int layereval, eval = 1000000;
	layerthickness = 0;
	for (int x = 0; x < short(boxlist.size()); x++)
	{
		if (boxlist[x].is_packed)
			continue;
		short y = 1;
		for (y = 1; y <= 3; y++)
		{
			switch (y)
			{
			case 1:
				exdim = boxlist[x].dim1;
				dimen2 = boxlist[x].dim2;
				dimen3 = boxlist[x].dim3;
				break;
			case 2:
				exdim = boxlist[x].dim2;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim3;
				break;
			case 3:
				exdim = boxlist[x].dim3;
				dimen2 = boxlist[x].dim1;
				dimen3 = boxlist[x].dim2;
				break;
			}
			layereval = 0;
			if ((exdim <= thickness)
				&& (((dimen2 <= pallet.pallet_x()) && (dimen3 <= pallet.pallet_z()))
					|| ((dimen3 <= pallet.pallet_x()) && (dimen2 <= pallet.pallet_z()))))
			{
				for (int z = 0; z < short(boxlist.size()); z++)
				{
					if (!(x == z) && !(boxlist[z].is_packed))
					{
						dimdif = abs(exdim - boxlist[z].dim1);
						if (abs(exdim - boxlist[z].dim2) < dimdif)
						{
							dimdif = abs(exdim - boxlist[z].dim2);
						}
						if (abs(exdim - boxlist[z].dim3) < dimdif)
						{
							dimdif = abs(exdim - boxlist[z].dim3);
						}
						layereval = layereval + dimdif;
					}
				}
				if (layereval < eval)
				{
					eval = layereval;
					layerthickness = exdim;
				}
			}
		}
	}
	if (layerthickness == 0 || layerthickness > remainpy)
		packing = false;
	return 0;
}
#pragma endregion
#pragma region find_box
//----------------------------------------------------------------------------
// FINDS THE MOST PROPER BOXES BY LOOKING AT ALL SIX POSSIBLE ORIENTATIONS,
// EMPTY SPACE GIVEN, ADJACENT BOXES, AND PALLET LIMITS
//----------------------------------------------------------------------------
void Boxologic::find_box(int hmx, int hy, int hmy, int hz, int hmz
	, int &cboxi, int &cboxx, int &cboxy, int &cboxz)
{
	int boxi = 0, boxx = 0, boxy = 0, boxz = 0;
	int bboxi = 0, bboxx = 0, bboxy = 0, bboxz = 0;
	int bfx = 32767, bfy = 32767, bfz = 32767;
	int bbfx = 32767, bbfy = 32767, bbfz = 32767;
	boxi = 0, bboxi = 0;
	for (int y = 0; y < short(boxlist.size()); y = y + boxlist[y].n)
	{
		short x = y;
		for (x = y; x < y + boxlist[y].n - 1; x++)
		{
			if (!boxlist[x].is_packed) break;
		}
		if (boxlist[x].is_packed) continue;
		if (x >= short(boxlist.size())) return;
		// 1 2 3
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim1, boxlist[x].dim2, boxlist[x].dim3
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
		if ((boxlist[x].dim1 == boxlist[x].dim3)
			&& (boxlist[x].dim3 == boxlist[x].dim2))
			continue;
		// 1 3 2
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim1, boxlist[x].dim3, boxlist[x].dim2
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
		// 2 1 3
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim2, boxlist[x].dim1, boxlist[x].dim3
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
		// 2 3 1
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim2, boxlist[x].dim3, boxlist[x].dim1
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
		// 3 1 2
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim3, boxlist[x].dim1, boxlist[x].dim2
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
		// 3 2 1
		analyze_box(x, hmx, hy, hmy, hz, hmz
			, boxlist[x].dim3, boxlist[x].dim2, boxlist[x].dim1
			, bfx, bfy, bfz
			, bbfx, bbfy, bbfz
			, boxi, boxx, boxy, boxz
			, bboxi, bboxx, bboxy, bboxz);
	}
	Checkfound(cboxi, cboxx, cboxy, cboxz
		, boxi, boxx, boxy, boxz
		, bboxi, bboxx, bboxy, bboxz);
}
#pragma endregion
#pragma region analyze_box
//----------------------------------------------------------------------------
// ANALYZES EACH UNPACKED BOX TO FIND THE BEST FITTING ONE TO THE EMPTY SPACE
// GIVEN
//----------------------------------------------------------------------------
void Boxologic::analyze_box(int x, int hmx, int hy, int hmy, int hz, int hmz
	, int dim1, int dim2, int dim3
	, int &bfx, int &bfy, int &bfz
	, int &bbfx, int &bbfy, int& bbfz
	, int &boxi, int &boxx, int &boxy, int &boxz
	, int &bboxi, int &bboxx, int &bboxy, int &bboxz)
{
	if (dim1 <= hmx && dim2 <= hmy && dim3 <= hmz)
	{
		if (dim2 <= hy)
		{
			if (hy - dim2 < bfy)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
			else if (hy - dim2 == bfy && hmx - dim1 < bfx)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
			else if (hy - dim2 == bfy && hmx - dim1 == bfx && abs(hz - dim3) < bfz)
			{
				boxx = dim1;
				boxy = dim2;
				boxz = dim3;
				bfx = hmx - dim1;
				bfy = hy - dim2;
				bfz = abs(hz - dim3);
				boxi = x;
			}
		}
		else
		{
			if (dim2 - hy < bbfy)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
			else if (dim2 - hy == bbfy && hmx - dim1 < bbfx)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
			else if (dim2 - hy == bbfy && hmx - dim1 == bbfx && abs(hz - dim3) < bbfz)
			{
				bboxx = dim1;
				bboxy = dim2;
				bboxz = dim3;
				bbfx = hmx - dim1;
				bbfy = dim2 - hy;
				bbfz = abs(hz - dim3);
				bboxi = x;
			}
		}
	}
}
#pragma endregion
#pragma region find_smallest_z
//----------------------------------------------------------------------------
// FINDS THE FIRST TO BE PACKED GAP IN THE LAYER EDGE
//----------------------------------------------------------------------------
void Boxologic::find_smallest_z(void)
{
	struct scrappad *scrapmemb = scrapfirst;
	smallestz = scrapmemb;
	while (!(scrapmemb->next == NULL))
	{
		if (scrapmemb->next->cumz < smallestz->cumz)
			smallestz = scrapmemb->next;
		scrapmemb = scrapmemb->next;
	}
}
#pragma endregion
#pragma region Checkfound
//----------------------------------------------------------------------------
// AFTER FINDING EACH BOX, THE CANDIDATE BOXES AND THE CONDITION OF THE LAYER
// ARE EXAMINED
//----------------------------------------------------------------------------
void Boxologic::Checkfound(
	int &cboxi, int &cboxx, int &cboxy, int &cboxz
	, int boxi, int boxx, int boxy, int boxz
	, int bboxi, int bboxx, int bboxy, int bboxz
)
{
	evened = false;
	if (boxi > 0)
	{
		cboxi = boxi;
		cboxx = boxx;
		cboxy = boxy;
		cboxz = boxz;
	}
	else
	{
		if ((bboxi > 0) && (layerinlayer || (!smallestz->prev && !smallestz->next)))
		{
			if (!layerinlayer)
			{
				prelayer = layerthickness;
				lilz = smallestz->cumz;
			}
			cboxi = bboxi;
			cboxx = bboxx;
			cboxy = bboxy;
			cboxz = bboxz;
			layerinlayer = layerinlayer + bboxy - layerthickness;
			layerthickness = bboxy;
		}
		else
		{
			if (!smallestz->prev && !smallestz->next)
			{
				layerdone = true;
			}
			else
			{
				evened = true;
				if (!smallestz->prev)
				{
					trash = smallestz->next;
					smallestz->cumx = smallestz->next->cumx;
					smallestz->cumz = smallestz->next->cumz;
					smallestz->next = smallestz->next->next;
					if (smallestz->next)
					{
						smallestz->next->prev = smallestz;
					}
					delete trash;
				}
				else if (!smallestz->next)
				{
					smallestz->prev->next = NULL;
					smallestz->prev->cumx = smallestz->cumx;
					delete smallestz;
				}
				else
				{
					if (smallestz->prev->cumz == smallestz->next->cumz)
					{
						smallestz->prev->next = smallestz->next->next;
						if (smallestz->next->next)
						{
							smallestz->next->next->prev = smallestz->prev;
						}
						smallestz->prev->cumx = smallestz->next->cumx;
						delete smallestz->next;
						delete smallestz;
					}
					else
					{
						smallestz->prev->next = smallestz->next;
						smallestz->next->prev = smallestz->prev;
						if (smallestz->prev->cumz < smallestz->next->cumz)
						{
							smallestz->prev->cumx = smallestz->cumx;
						}
						delete smallestz;
					}
				}
			}
		}
	}
}
#pragma endregion
#pragma region VolumeCheck
//----------------------------------------------------------------------------
// AFTER PACKING OF EACH BOX, 100% PACKING CONDITION IS CHECKED
//----------------------------------------------------------------------------
void Boxologic::Volume_check(int cboxi, int cboxx, int cboxy, int cboxz, bool packingbest
	, bool &hundredpercent)
{
	boxlist[cboxi].setPacked(cboxx, cboxy, cboxz);
	packedvolume += boxlist[cboxi].volume();
	packednumbox++;
	if (packingbest)
	{
		boxlist[cboxi].write(visualizer_file);
		boxlist[cboxi].write_boxlist_file(report_output_file, best_variant, cboxi);
	}
	else if (packedvolume == pallet.total_pallet_volume() || packedvolume == total_box_volume)
	{
		packing = false;
		hundredpercent = true;
	}
}
#pragma endregion
//----------------------------------------------------------------------------
// USING THE PARAMETERS FOUND, PACKS THE BEST SOLUTION FOUND AND REPORTS TO THE
// CONSOLE AND TO A TEXT FILE
//----------------------------------------------------------------------------
void Boxologic::report_results(const std::string &filename)
{
	pallet.variant = best_variant;

	char graphout[] = "visudat";
	if ((visualizer_file = fopen(graphout, "w")) == NULL)
	{
		printf("Cannot open file %s\n", graphout);
		exit(1);
	}

	fprintf(visualizer_file, "%5d%5d%5d\n", pallet.pallet_x(), pallet.pallet_y(), pallet.pallet_z());

	std::string filenameOut = filename + ".out";
	if ((report_output_file = fopen(filenameOut.c_str(), "w")) == NULL)
	{
		printf("Cannot open output file %s\n", filenameOut.c_str());
		exit(1);
	}

	double packed_box_percentage = best_solution_volume * 100 / total_box_volume;
	pallet_volume_used_percentage = best_solution_volume * 100 / pallet.total_pallet_volume();
	double elapsed_time = difftime(finish, start);

	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "                                       *** REPORT ***\n");
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "ELAPSED TIME                                          : Almost %.0f sec\n", elapsed_time);
	fprintf(report_output_file, "TOTAL NUMBER OF ITERATIONS DONE                       : %d\n", number_of_iterations);
	fprintf(report_output_file, "BEST SOLUTION FOUND AT ITERATION                      : %d OF VARIANT: %d\n", best_iteration, best_variant);
	fprintf(report_output_file, "TOTAL NUMBER OF BOXES                                 : %d\n", boxlist.size());
	fprintf(report_output_file, "PACKED NUMBER OF BOXES                                : %d\n", number_packed_boxes);
	fprintf(report_output_file, "TOTAL VOLUME OF ALL BOXES                             : %.0f\n", total_box_volume);
	fprintf(report_output_file, "PALLET VOLUME                                         : %.0f\n", pallet.total_pallet_volume());
	fprintf(report_output_file, "BEST SOLUTION'S VOLUME UTILIZATION                    : %.0f OUT OF %.0f\n", best_solution_volume, pallet.total_pallet_volume());
	fprintf(report_output_file, "PERCENTAGE OF PALLET VOLUME USED                      : %.6f %%\n", pallet_volume_used_percentage);
	fprintf(report_output_file, "PERCENTAGE OF PACKED BOXES (VOLUME)                   : %.6f %%\n", packed_box_percentage);
	fprintf(report_output_file, "WHILE PALLET ORIENTATION                              : X=%d; Y=%d; Z= %d\n", pallet.pallet_x(), pallet.pallet_y(), pallet.pallet_z());
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");
	fprintf(report_output_file, "  NO: PACKSTA DIMEN-1  DMEN-2  DIMEN-3   COOR-X   COOR-Y   COOR-Z   PACKEDX  PACKEDY  PACKEDZ\n");
	fprintf(report_output_file, "---------------------------------------------------------------------------------------------\n");

	list_candidate_layers();
	std::sort(layers.begin(), layers.end());
	packedvolume = 0.0;
	packedy = 0;
	packing = true;
	layerthickness = layers[best_iteration].layerdim;
	remainpy = pallet.pallet_y();
	remainpz = pallet.pallet_z();

	for (short x = 0; x < short(boxlist.size()); x++)
		boxlist[x].is_packed = false;

	bool hundredPercent = false;
	do
	{
		layerinlayer = 0;
		layerdone = false;
		pack_layer(true, hundredPercent);
		packedy = packedy + layerthickness;
		remainpy = pallet.pallet_y() - packedy;
		if (layerinlayer)
		{
			prepackedy = packedy;
			preremainpy = remainpy;
			remainpy = layerthickness - prelayer;
			packedy = packedy - layerthickness + prelayer;
			remainpz = lilz;
			layerthickness = layerinlayer;
			layerdone = false;
			pack_layer(true, hundredPercent);
			packedy = prepackedy;
			remainpy = preremainpy;
			remainpz = pallet.pallet_z();
		}
		find_layer(remainpy);
	} while (packing);

	fprintf(report_output_file, "\n\n *** LIST OF UNPACKED BOXES ***\n");
	for (int cboxi = 0; cboxi < short(boxlist.size()); cboxi++)
	{
		if (!boxlist[cboxi].is_packed)
		{
			fprintf(report_output_file, "%5d", cboxi);
			boxlist[cboxi].write(report_output_file);
		}
	}
	fclose(report_output_file);
	fclose(visualizer_file);
	printf("\n");
	for (short n = 0; n < short(boxlist.size()); n++)
	{
		if (boxlist[n].is_packed)
		{
			printf("%d %d %d %d %d %d %d %d %d %d\n", n, boxlist[n].dim1, boxlist[n].dim2, boxlist[n].dim3, boxlist[n].cox, boxlist[n].coy, boxlist[n].coz, boxlist[n].packx, boxlist[n].packy, boxlist[n].packz);
		}
	}
	printf("ELAPSED TIME                       : Almost %.0f sec\n", elapsed_time);
	printf("TOTAL NUMBER OF ITERATIONS DONE    : %d\n", number_of_iterations);
	printf("BEST SOLUTION FOUND AT             : ITERATION: %d OF VARIANT: %d\n", best_iteration, best_variant);
	printf("TOTAL NUMBER OF BOXES              : %d\n", boxlist.size());
	printf("PACKED NUMBER OF BOXES             : %d\n", number_packed_boxes);
	printf("TOTAL VOLUME OF ALL BOXES          : %.0f\n", total_box_volume);
	printf("PALLET VOLUME                      : %.0f\n", pallet.total_pallet_volume());
	printf("BEST SOLUTION'S VOLUME UTILIZATION : %.0f OUT OF %.0f\n", best_solution_volume, pallet.total_pallet_volume());
	printf("PERCENTAGE OF PALLET VOLUME USED   : %.6f %%\n", pallet_volume_used_percentage);
	printf("PERCENTAGE OF PACKEDBOXES (VOLUME) : %.6f%%\n", packed_box_percentage);
	printf("WHILE PALLET ORIENTATION           : X=%d; Y=%d; Z= %d\n\n\n", pallet.pallet_x(), pallet.pallet_y(), pallet.pallet_z());

	for (std::map<int, double>::iterator iter = best_iterations.begin(); iter != best_iterations.end(); iter++)
	{
		std::cout << "Best iterations : " << iter->first << " -> " << iter->second << std::endl;
	}
}

//----------------------------------------------------------------------------
// PRINT THE HELP SCREEN
//----------------------------------------------------------------------------

void print_help(void)
{
	printf("USAGE:\n");
	printf("\tboxologic <option>\n");
	printf("\nOPTIONS:\n");
	printf("\t[ -f|--inputfile ] <boxlist text file>   : Perform bin packing analysis\n");
	printf("\t[ -v|--version ]                         : Print software version\n");
	printf("\t[ -h|--help ]                            : Print this help screen\n\n");
}

