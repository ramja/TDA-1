#include <utilities/log.h>

#include <topology/simplex.h>
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/dynamic-persistence.h>
#include <topology/persistence-diagram.h>
#include <utilities/indirect.h>

#include <vector>
#include <map>
#include <numeric>


#if 1
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif



typedef         unsigned                                            Vertex;
typedef         Simplex<Vertex, double>                             Smplx;
typedef         Smplx::VertexContainer				    VertexCont;
typedef         std::vector<Vertex>                                 VertexVector;
typedef         Filtration<Smplx>                                   Fltr;
typedef         StaticPersistence<>                                 Persistence;
typedef         PersistenceDiagram<>                                PDgm;
typedef         OffsetBeginMap<Persistence, Fltr, 
                               Persistence::iterator, 
                               Fltr::Index>                         PersistenceFiltrationMap;
typedef         OffsetBeginMap<Fltr, Persistence,
                               Fltr::Index, 
                               Persistence::iterator>               FiltrationPersistenceMap;



// add a single edge to the filtration
template <typename RealVector>
void addEdge(Fltr& filtr, const RealVector& FUNvalues, 
            int vert01, int vert02) {
     VertexVector vertices(3);
     vertices[0] = vert01;
     vertices[1] = vert02;
     VertexVector::const_iterator bg = vertices.begin();

     double value = std::max(FUNvalues[vert01], FUNvalues[vert02]);
     filtr.push_back(Smplx(bg, bg + 2, value)); 
         // std::max(FUNvalues.at(vert03),FUNvalues.at(vert04))),
} // end function to add a single edge



// add a single triangle to the filtration
template <typename RealVector>
void addTri(Fltr& filtr, const RealVector& FUNvalues, 
            int vert01, int vert02, int vert03) {
     VertexVector vertices(3);
     vertices[0] = vert01;
     vertices[1] = vert02;
     vertices[2] = vert03;
     VertexVector::const_iterator bg = vertices.begin();

     double value = std::max(std::max(FUNvalues[vert01], FUNvalues[vert02]),
                    FUNvalues[vert03]);
     filtr.push_back(Smplx(bg, bg + 3, value)); 
         // std::max(FUNvalues.at(vert03),FUNvalues.at(vert04))),
} // end function to add a single triangle



// add a single tet to the filtration
template <typename RealVector>
void addTet(Fltr& filtr, const RealVector& FUNvalues, 
            int vert01, int vert02, int vert03, int vert04) {
     VertexVector vertices(3);
     vertices[0] = vert01;
     vertices[1] = vert02;
     vertices[2] = vert03;
     vertices[3] = vert04;
     VertexVector::const_iterator bg = vertices.begin();

     double value = std::max(std::max(FUNvalues[vert01], FUNvalues[vert02]),
                    std::max(FUNvalues[vert03], FUNvalues[vert04]));
     filtr.push_back(Smplx(bg, bg + 4, value)); 
         // std::max(FUNvalues.at(vert03),FUNvalues.at(vert04))),
} // end function to add a single tet



template <typename RealVector>
void addAllEdges(Fltr& filtr, const RealVector& FUNvalues, 
              const int ncols, const int nrows, int i, int j, int k) {     
     int curidx = i + ncols*j + ncols*nrows*k;

     // ... add edge (i-1,j,k) <--> (i,j,k)
     if (i > 0)
     {
        addEdge(filtr, FUNvalues, curidx, curidx -1);
     }
     
     // ... add edge (i,j-1,k) <--> (i,j,k)
     if (j > 0)
     {
        addEdge(filtr, FUNvalues, curidx, curidx - ncols);
     }
   
     // ... add edge (i,j,k-1) <--> (i,j,k)
     if (k > 0)
     {
        addEdge(filtr, FUNvalues, curidx, curidx - nrows*ncols);
     }


     // TODO: add the rest of the code for creating edges to here
     //       from fcn simplicesFromGrid 

     // ... consider two cases for the cubical decomposition:
     if ((i+j+k)%2 == 0)
     {
	// ... EVEN BOX 
        if (i > 0 && j > 0) // top
        { 
           addEdge(filtr, FUNvalues, curidx, curidx - ncols -1);
        }
        if (i > 0 && k > 0) // back
        {
           addEdge(filtr, FUNvalues, curidx, curidx - nrows*ncols -1);
        }
        if (j > 0 && k > 0) // right
        {
           addEdge(filtr, FUNvalues, curidx, curidx - nrows*ncols - ncols);
        }
     }
     else
     {
     	// ... ODD BOX
        if (i > 0 && j > 0) // top
        {
           addEdge(filtr, FUNvalues, curidx - 1, curidx - ncols);
        }
        if (i > 0 && k > 0) // back
        {
           addEdge(filtr, FUNvalues, curidx - 1, curidx - nrows*ncols);
        }
        if (j > 0 && k > 0) // right
        {
           addEdge(filtr, FUNvalues, curidx - ncols, curidx - nrows*ncols);
        }
     }


     return;
} // end function addEdges



template <typename RealVector>
void addEvenTets(Fltr& filtr, const RealVector& FUNvalues, 
                  const int ncols, const int nrows, int i, int j, int k) {
     assert(i > 0 && j > 0 && k > 0);
     int curidx = i + ncols*j + ncols*nrows*k;
     
     // top vertex (i, j-1, k)
     addTet(filtr, FUNvalues, curidx, curidx - 1 - ncols, curidx - ncols - nrows*ncols, curidx - ncols);
     
     // top vertex (i-1, j, k)
     addTet(filtr, FUNvalues, curidx, curidx - 1, curidx - nrows*ncols - 1, curidx -1 - ncols);

     // top vertex (i, j, k-1)
     addTet(filtr, FUNvalues, curidx, curidx - 1 - nrows*ncols, curidx - ncols - nrows*ncols, curidx - nrows*ncols);

     // top vertex (i-1, j-1, k-1)
     addTet(filtr, FUNvalues, curidx - 1 - nrows*ncols, curidx - ncols - nrows*ncols, curidx - 1 - ncols, curidx - 1 - ncols - nrows*ncols);
     
     return;
} // end fcn to add four EVEN tets



template <typename RealVector>
void addOddTets(Fltr& filtr, const RealVector& FUNvalues, 
                  const int ncols, const int nrows, int i, int j, int k) {
     assert(i > 0 && j > 0 && k > 0);
     int curidx = i + ncols*j + ncols*nrows*k;
      
     VertexVector vertices(4);
     vertices[0] = curidx;  vertices[3] = curidx;
     vertices[1] = -1; vertices[2] = -1; 
     VertexVector::const_iterator bg = vertices.begin();
//   VertexVector::const_iterator end = vertices.end();
    
     int v1, v2, v3, v4;
     double value, value2;  // max of value and value 2 is the fcn value. 

     // top vertex (i, j, k)
     v1 = curidx -1;   vertices[0] = v1;
     v2 = curidx - ncols;  vertices[1] = v2;  
     value = std::max(FUNvalues[v1],FUNvalues[v2]);
     
     v3 = curidx - nrows*ncols;  vertices[2] = v3;
     v4 = curidx; vertices[3] = v4;
     value2 = std::max(FUNvalues[v3],FUNvalues[v4]);
     
     filtr.push_back(Smplx(bg, bg + 4, std::max(value,value2)));
     
     // top vertex (i-1, j-1, k)
     v3 = curidx - 1 - ncols - nrows*ncols;  vertices[2] = v3;
     v4 = curidx - 1 -ncols; vertices[3] = v4;
     value2 = std::max(FUNvalues[v3],FUNvalues[v4]);
     
     filtr.push_back(Smplx(bg, bg + 4, std::max(value,value2)));

     // top vertex (i, j-1, k-1)
     v1 = curidx - nrows*ncols;   vertices[0] = v1;
     v2 = curidx - 1 - ncols - nrows*ncols;  vertices[1] = v2;  
     value = std::max(FUNvalues[v1],FUNvalues[v2]);
     
     v3 = curidx - ncols;  vertices[2] = v3;
     v4 = curidx -ncols - nrows*ncols; vertices[3] = v4;
     value2 = std::max(FUNvalues[v3],FUNvalues[v4]);
     
     filtr.push_back(Smplx(bg, bg + 4, std::max(value,value2)));

     // top vertex (i-1, j, k-1)
     v3 = curidx - 1;  vertices[2] = v3;
     v4 = curidx -1 - nrows * ncols; vertices[3] = v4;
     value2 = std::max(FUNvalues[v3],FUNvalues[v4]);
    
     filtr.push_back(Smplx(bg, bg + 4, std::max(value,value2)));
      

     return;
} // end fcn addEvenTets



template <typename RealVector>
void addAllTriangles(Fltr& filtr, const RealVector& FUNvalues, 
                  const int ncols, const int nrows, int i, int j, int k) {
     int curidx = i + ncols*j + ncols*nrows*k;
     
     // ... consider two cases for the cubical decomposition:
     if ((i+j+k)%2 == 0)
     {
	// ... EVEN BOX
        if (i > 0 && j > 0) // top
        {
     
           addTri(filtr, FUNvalues, curidx, curidx - ncols - 1, curidx - ncols);
           addTri(filtr, FUNvalues, curidx, curidx -1, curidx - ncols -1); 
        }
        if (i > 0 && k > 0) // back
        {
           addTri(filtr, FUNvalues, curidx, curidx - nrows*ncols -1, curidx - 1);
           addTri(filtr, FUNvalues, curidx, curidx - nrows*ncols, curidx - nrows*ncols -1);
        }
        
        if (j > 0 && k > 0) // right
        {
           addTri(filtr, FUNvalues, curidx, curidx - nrows*ncols - ncols, curidx - nrows*ncols);
           addTri(filtr, FUNvalues, curidx, curidx - ncols, curidx - nrows*ncols - ncols);
           
           if (i > 0) // middle
           {
              addTri(filtr, FUNvalues, curidx, curidx - ncols -1, curidx - ncols - nrows*ncols);
              addTri(filtr, FUNvalues, curidx, curidx -1 -nrows*ncols, curidx - ncols -1);
              addTri(filtr, FUNvalues, curidx -1 - nrows*ncols, curidx - ncols - nrows*ncols, curidx);
              addTri(filtr, FUNvalues, curidx -1 - nrows*ncols, curidx - 1 - ncols, curidx - ncols - nrows*ncols);
           }
        }
     } // end if for even case 
     else {
        // ... ODD CASE
        if (i > 0 && j > 0) // top
        {
           addTri(filtr, FUNvalues, curidx -1, curidx - ncols, curidx);
           addTri(filtr, FUNvalues, curidx -1, curidx - ncols -1, curidx - ncols);
        }

        if (i > 0 && k > 0) // back
        {
           addTri(filtr, FUNvalues, curidx -1, curidx - nrows*ncols, curidx - nrows*ncols - 1);
           addTri(filtr, FUNvalues, curidx -1, curidx, curidx - nrows*ncols);
        }  
        
        if (j > 0 && k > 0) // right
        {
           addTri(filtr, FUNvalues, curidx - ncols, curidx - nrows*ncols, curidx - ncols - nrows*ncols);
           addTri(filtr, FUNvalues, curidx - ncols, curidx, curidx - nrows*ncols);
           
           if (i > 0) // middle
           { 
              addTri(filtr, FUNvalues, curidx -1, curidx - ncols, curidx - nrows*ncols);
              addTri(filtr, FUNvalues, curidx -1, curidx - nrows*ncols - ncols - 1, curidx - ncols);
              addTri(filtr, FUNvalues, curidx - nrows*ncols, curidx - nrows*ncols - ncols -1, curidx - ncols);
              addTri(filtr, FUNvalues, curidx - nrows*ncols, curidx - 1, curidx - nrows*ncols - ncols -1);
           }
        } // end for through j k positive
     } // end else through odd case.

    return;
} // end function addAllTriangles



template <typename RealVector>
void addAllTetrahedra(Fltr& filtr, const RealVector& FUNvalues, 
                  const int ncols, const int nrows, int i, int j, int k) {
     int curidx = i + ncols*j + ncols*nrows*k;
     
     // ... consider two cases for the cubical decomposition:
     if ((i+j+k)%2 == 0)
     {
	// ... EVEN BOX
        if (i > 0 && j > 0 && k > 0) // middle
        {
            // ... add center tets 
			addTet(filtr, FUNvalues, curidx -1 - nrows*ncols, curidx - 1 - ncols, curidx - ncols - nrows*ncols, curidx);
            // ... add remaining tets 
			addEvenTets(filtr, FUNvalues, ncols, nrows, i, j, k);
        }
     } // end if for even case 
     else {
        // ... ODD CASE
        if (i > 0 && j > 0 && k > 0) // middle
        {
			// ... add central tet
			addTet(filtr, FUNvalues, curidx -1, curidx - ncols, curidx - nrows*ncols, curidx - nrows*ncols -ncols -1);
			// ... add remaining tets
			addOddTets(filtr, FUNvalues, ncols, nrows, i, j, k);
        } // end for through j k positive
     } // end else through odd case.

    return;
} // end function addTriangles



template<typename RealVector, typename IntVector> void
simplicesFromGrid(Fltr & filtr, const RealVector & FUNvalues
                , const IntVector & gridDim, const int embedDim) {
	const unsigned gridProd = std::accumulate(
			gridDim.begin(), gridDim.end(), 1, std::multiplies< int >());
	int ncols, nrows;
	ncols = nrows = 1;
	int i = 0; // indexing the columns
	int j = 0; // indexing the rows
	int k = 0; // indexing the z dimension
	unsigned int curidx = 0; // curidx = i + ncols * j + nrows * ncols * k

	if (gridDim.size() > 0) {
		ncols = gridDim[0];
	}
	if (gridDim.size() > 1) {
		nrows = gridDim[1];
	}

  while(curidx < gridProd) {

    // .. add the vertex 
    std::vector<Vertex> vcont;
    vcont.push_back((Vertex)(curidx));
    filtr.push_back(Smplx(vcont, FUNvalues[curidx])); 

    // If dimension of embedded space >= 1, add the edges:
		if (embedDim >= 1) {
			addAllEdges(filtr, FUNvalues, ncols, nrows, i, j, k);
		}
		// If dimension of embedded space >= 2, add the triangles:
		if (embedDim >= 2) {
			addAllTriangles(filtr, FUNvalues, ncols, nrows, i, j, k);
		}
    // If dimension of embedded space >= 3, add the tetrahedra:
		if (embedDim >= 3) {
			addAllTetrahedra(filtr, FUNvalues, ncols, nrows, i, j, k);
		}

    ++i; // advance column
    // advance row
    if (i >= ncols) {
      i = 0;
      ++j;
    }
    // advance z value
    if (j >= nrows) {
      j = 0;
      ++k;
    }
    ++curidx; // advance curidx

  }
} // end simplicesFromGrid function



template <typename IntVector>
inline std::vector< unsigned char > isInternal(unsigned int argIdx, const IntVector& gridDim) {
    std::vector< unsigned char > resIsInt;
    resIsInt.reserve(gridDim.size());
    typename IntVector::const_iterator itrDim;
    for (itrDim = gridDim.begin(); itrDim != gridDim.end(); itrDim++)
    {
        resIsInt.push_back((unsigned char)(argIdx % (*itrDim) > 0));
        argIdx /= (*itrDim);
    }
    return resIsInt;
}



inline std::vector< std::vector< unsigned char > > verticesLessVertex(const std::vector< unsigned char > & argVtx, const bool argAlsoEqual) {
	std::vector< std::vector< unsigned char > > resCubeVertices;
    unsigned int idxVtx, vtxNum;
    std::vector< unsigned int > oneTwoVec;
	oneTwoVec.reserve(argVtx.size());
	std::vector< unsigned char >::const_iterator itrVtx;
	for (itrVtx = argVtx.begin(); itrVtx != argVtx.end(); ++itrVtx)
	{
		oneTwoVec.push_back(1+(unsigned int)(*itrVtx));
	}
	
    vtxNum = std::accumulate(oneTwoVec.begin(), oneTwoVec.end(), 1, std::multiplies< unsigned int >());
	if (!argAlsoEqual)
	{
		vtxNum -= 1;
	}
    resCubeVertices.reserve(vtxNum);
    for (idxVtx = 0; idxVtx < vtxNum; ++idxVtx)
    {
        resCubeVertices.push_back(isInternal(idxVtx, oneTwoVec));
    }
	return resCubeVertices;
}



std::vector< std::map< std::vector< unsigned char >, std::vector< std::vector< std::vector< unsigned char > > > > > triangulateHypercube(const int argDimEmbed, const unsigned char embedDim) {
    std::vector< std::map< std::vector< unsigned char >, std::vector< std::vector< std::vector< unsigned char > > > > > resTriedCube;
    resTriedCube.reserve(embedDim+1);

    // vertices of hypercube
	std::vector< unsigned char > rootVtx(argDimEmbed, 1);
    std::vector< std::vector< unsigned char > > cubeVertices = verticesLessVertex(rootVtx, true);

	std::map< std::vector< unsigned char >, std::vector< std::vector< std::vector< unsigned char > > > > mapDirSmpxVec;
	std::vector< std::vector< std::vector< unsigned char > > > dirSmpxVec;
	std::vector< std::vector< unsigned char > > dirSmpx;
	std::vector< std::vector< unsigned char > >::const_iterator itrVtx;

	// 0 dim
	mapDirSmpxVec.clear();
	for (itrVtx = cubeVertices.begin(); itrVtx != cubeVertices.end(); ++itrVtx)
	{
		dirSmpxVec.clear();
		dirSmpx.clear();
		dirSmpx.push_back(*itrVtx);
		dirSmpxVec.push_back(dirSmpx);
		mapDirSmpxVec[ *itrVtx ] = dirSmpxVec;
	}
	resTriedCube.push_back(mapDirSmpxVec);

	unsigned char idxDim;
	std::vector< std::vector< unsigned char > > vtxLessVtx;
	std::vector< std::vector< unsigned char > >::const_iterator itrLessVtx;
	std::vector< std::vector< std::vector< unsigned char > > > dirSmpxVecPrev;
	std::vector< std::vector< std::vector< unsigned char > > >::iterator itrSmpxVec;

	for (idxDim = 1; idxDim <= embedDim; ++idxDim)
	{
		mapDirSmpxVec.clear();
		for (itrVtx = cubeVertices.begin(); itrVtx != cubeVertices.end(); ++itrVtx)
		{
			dirSmpxVec.clear();
			vtxLessVtx = verticesLessVertex(*itrVtx, false);
			for (itrLessVtx = vtxLessVtx.begin(); itrLessVtx != vtxLessVtx.end(); ++itrLessVtx)
			{
				dirSmpxVecPrev = resTriedCube.at(idxDim-1).at(*itrLessVtx);
				for (itrSmpxVec = dirSmpxVecPrev.begin(); itrSmpxVec != dirSmpxVecPrev.end(); ++itrSmpxVec)
				{
					itrSmpxVec->push_back(*itrVtx); 
				}
				dirSmpxVec.insert(dirSmpxVec.end(), dirSmpxVecPrev.begin(), dirSmpxVecPrev.end());
			}
			mapDirSmpxVec[ *itrVtx ] = dirSmpxVec;
		}
		resTriedCube.push_back(mapDirSmpxVec);
	}

    return resTriedCube;
}



// add a single simplex to the filtration
template<typename RealVector>
inline void addSimplex(Fltr & argFltr, const RealVector& FUNvalues, VertexVector & argVtx) {
    VertexVector::const_iterator itrVtx = argVtx.begin();
    double maxFcnVal = FUNvalues[ *itrVtx ];
     for (; itrVtx != argVtx.end(); ++itrVtx)
     {
        maxFcnVal = std::max(maxFcnVal, FUNvalues[ *itrVtx ]);
     }
     argFltr.push_back(Smplx(argVtx.begin(), argVtx.end(), maxFcnVal)); 
}



template<typename RealVector, typename IntVector>
void addSimplices(Fltr& argFltr, const RealVector& FUNvalues, const int argIdxCur, const IntVector& gridDim, const unsigned char argIdxDim, std::vector< std::map< std::vector< unsigned char >, std::vector< std::vector< std::vector< unsigned char > > > > >& argTriedCube) {
    std::vector< unsigned char > isInt = isInternal(argIdxCur, gridDim);
    std::vector< std::vector< std::vector< unsigned char > > > dirSmpxVec = (argTriedCube.at(argIdxDim)).at(isInt);
    std::vector< std::vector< std::vector< unsigned char > > >::const_iterator itrDirSmpxVec;
    std::vector< std::vector< unsigned char > >::const_iterator itrDirVtxVec;
	std::vector< unsigned char > diffVtx(gridDim.size());
	std::vector< unsigned int > gridAccNum(gridDim.size(),1);
	std::partial_sum(gridDim.begin(), gridDim.end()-1, gridAccNum.begin()+1, std::multiplies< unsigned int >());

    VertexVector vtxVec;
    VertexVector::iterator itrVtxVec;
    vtxVec.resize(argIdxDim+1);
    for (itrDirSmpxVec = dirSmpxVec.begin(); itrDirSmpxVec != dirSmpxVec.end(); ++itrDirSmpxVec)
    {
        for (itrDirVtxVec = itrDirSmpxVec->begin(), itrVtxVec = vtxVec.begin();
             itrDirVtxVec != itrDirSmpxVec->end(); 
             ++itrDirVtxVec, ++itrVtxVec)
        {
			std::transform(isInt.begin(), isInt.end(), itrDirVtxVec->begin(), diffVtx.begin(), std::minus< char >());

            (*itrVtxVec) = (argIdxCur - std::inner_product(gridAccNum.begin(), gridAccNum.end(), diffVtx.begin(), 0));
        }
        addSimplex(argFltr, FUNvalues, vtxVec);
    }
}



template<typename RealVector, typename IntVector>
void simplicesFromGridBarycenter(Fltr& argFltr, const RealVector& FUNvalues, const IntVector& gridDim, const unsigned char embedDim) {
	const unsigned gridProd = std::accumulate(
			gridDim.begin(), gridDim.end(), 1, std::multiplies< int >());
	unsigned int idxCur; unsigned char idxDim;

   std::vector< std::map< std::vector< unsigned char >, std::vector< std::vector< std::vector< unsigned char > > > > >  triedCube = triangulateHypercube(gridDim.size(), embedDim);
  
    for (idxCur = 0; idxCur < gridProd ; ++idxCur) {
        for (idxDim = 0; idxDim <= embedDim; ++idxDim) {
		    addSimplices(argFltr, FUNvalues, idxCur, gridDim, idxDim, triedCube);
        }
    }
}
