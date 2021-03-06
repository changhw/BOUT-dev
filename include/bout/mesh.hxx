/*!************************************************************************
 * \file mesh.hxx
 *
 * Interface for mesh classes. Contains standard variables and useful
 * routines.
 * 
 * Changelog
 * =========
 *
 * 2014-12 Ben Dudson <bd512@york.ac.uk>
 *     * Removing coordinate system into separate
 *       Coordinates class
 *     * Adding index derivative functions from derivs.cxx
 * 
 * 2010-06 Ben Dudson, Sean Farley
 *     * Initial version, adapted from GridData class
 *     * Incorporates code from topology.cpp and Communicator
 *
 **************************************************************************
 * Copyright 2010 B.D.Dudson, S.Farley, M.V.Umansky, X.Q.Xu
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 * 
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

class Mesh;

#ifndef __MESH_H__
#define __MESH_H__

#include "mpi.h"

#include "field_data.hxx"
#include "bout_types.hxx"
#include "field2d.hxx"
#include "field3d.hxx"
#include "datafile.hxx"
#include "options.hxx"
#include "dcomplex.hxx" // For poloidal lowpass filter

#include "fieldgroup.hxx"

#include "boundary_region.hxx"
#include "parallel_boundary_region.hxx"

#include "sys/range.hxx" // RangeIterator

#include "bout/deprecated.hxx"

#include <bout/griddata.hxx>

#include "coordinates.hxx"    // Coordinates class

#include "paralleltransform.hxx" // ParallelTransform class

#include "unused.hxx"

#include <list>
#include <memory>

/// Type used to return pointers to handles
typedef void* comm_handle;

class Mesh {
 public:

  /// Constructor for a "bare", uninitialised Mesh
  /// Only useful for testing
  Mesh() : source(nullptr), coords(nullptr), options(nullptr) {}

  /// Constructor
  /// @param[in] s  The source to be used for loading variables
  /// @param[in] options  The options section for settings
  Mesh(GridDataSource *s, Options *options);

  /// Destructor
  virtual ~Mesh();

  /// Create a Mesh object
  ///
  /// @param[in] source  The data source to use for loading variables
  /// @param[in] opt     The option section. By default this is "mesh"
  static Mesh* create(GridDataSource *source, Options *opt = NULL);

  /// Create a Mesh object
  ///
  /// The source is determined by
  ///  1) If "file" is set in the options, read that
  ///  2) If "grid" is set in global options, read that
  ///  3) Use options as data source
  ///
  /// @param[in] opt  Input options. Default is "mesh" section
  static Mesh* create(Options *opt = NULL);
  
  /// Loads the mesh values
  /// 
  /// Currently need to create and load mesh in separate calls
  /// because creating Fields uses the global "mesh" pointer
  /// which isn't created until Mesh is constructed
  virtual int load() {return 1;}
  
  /// Add output variables to a data file
  /// These are used for post-processing
  virtual void outputVars(Datafile &UNUSED(file)) {} 
  
  // Get routines to request data from mesh file
  
  /// Get an integer from the input source
  /// 
  /// @param[out] ival  The value will be put into this variable
  /// @param[in] name   The name of the variable to read
  ///
  /// @returns zero if successful, non-zero on failure
  int get(int &ival, const string &name);

  /// Get a BoutReal from the input source
  /// 
  /// @param[out] rval  The value will be put into this variable
  /// @param[in] name   The name of the variable to read
  ///
  /// @returns zero if successful, non-zero on failure
  int get(BoutReal &rval, const string &name); 

  /// Get a Field2D from the input source
  /// including communicating guard cells
  ///
  /// @param[out] var   This will be set to the value. Will be allocated if needed
  /// @param[in] name   Name of the variable to read
  /// @param[in] def    The default value if not found
  ///
  /// @returns zero if successful, non-zero on failure
  int get(Field2D &var, const string &name, BoutReal def=0.0);

  /// Get a Field3D from the input source
  ///
  /// @param[out] var   This will be set to the value. Will be allocated if needed
  /// @param[in] name   Name of the variable to read
  /// @param[in] def    The default value if not found
  /// @param[in] communicate  Should the field be communicated to fill guard cells?
  ///
  /// @returns zero if successful, non-zero on failure
  int get(Field3D &var, const string &name, BoutReal def=0.0, bool communicate=true);

  /// Get a Vector2D from the input source.
  /// If \p var is covariant then this gets three
  /// Field2D variables with "_x", "_y", "_z" appended to \p name
  /// If \p var is contravariant, then "x", "y", "z" are appended to \p name
  ///
  /// By default all fields revert to zero
  ///
  /// @param[in] var  This will be set to the value read
  /// @param[in] name  The name of the vector. Individual fields are read based on this name by appending. See above
  ///
  /// @returns zero always. 
  int get(Vector2D &var, const string &name);

  /// Get a Vector3D from the input source.
  /// If \p var is covariant then this gets three
  /// Field3D variables with "_x", "_y", "_z" appended to \p name
  /// If \p var is contravariant, then "x", "y", "z" are appended to \p name
  ///
  /// By default all fields revert to zero
  ///
  /// @param[in] var  This will be set to the value read
  /// @param[in] name  The name of the vector. Individual fields are read based on this name by appending. See above
  ///
  /// @returns zero always. 
  int get(Vector3D &var, const string &name);

  /// Wrapper for GridDataSource::hasVar
  bool sourceHasVar(const string &name);
  
  // Communications
  /*!
   * Communicate a list of FieldData objects
   * Uses a variadic template (C++11) to pack all
   * arguments into a FieldGroup
   */
  template <typename... Ts>
  void communicate(Ts&... ts) {
    FieldGroup g(ts...);
    communicate(g);
  }

  template <typename... Ts>
  void communicateXZ(Ts&... ts) {
    FieldGroup g(ts...);
    communicateXZ(g);
  }

  /*!
   * Communicate a group of fields
   */
  void communicate(FieldGroup &g);

  /// Communcate guard cells in XZ only
  /// i.e. no Y communication
  ///
  /// @param g  The group of fields to communicate. Guard cells will be modified
  void communicateXZ(FieldGroup &g);

  /*!
   * Communicate an X-Z field
   */
  void communicate(FieldPerp &f); 

  /*!
   * Send a list of FieldData objects
   * Packs arguments into a FieldGroup and passes
   * to send(FieldGroup&).
   */
  template <typename... Ts>
  comm_handle send(Ts&... ts) {
    FieldGroup g(ts...);
    return send(g);
  }

  /// Perform communications without waiting for them
  /// to finish. Requires a call to wait() afterwards.
  ///
  /// \param g Group of fields to communicate
  /// \returns handle to be used as input to wait()
  virtual comm_handle send(FieldGroup &g) = 0;  
  virtual int wait(comm_handle handle) = 0; ///< Wait for the handle, return error code

  // non-local communications

  /// Low-level communication routine
  /// Send a buffer of data from this processor to another
  /// This must be matched by a corresponding call to
  /// receiveFromProc on the receiving processor
  ///
  /// @param[in] xproc  X index of processor to send to
  /// @param[in] yproc  Y index of processor to send to
  /// @param[in] buffer A buffer of data to send
  /// @param[in] size   The length of \p buffer
  /// @param[in] tag    A label, must be the same at receive
  virtual MPI_Request sendToProc(int xproc, int yproc, BoutReal *buffer, int size, int tag) = 0;

  /// Low-level communication routine
  /// Receive a buffer of data from another processor
  /// Must be matched by corresponding sendToProc call
  /// on the sending processor
  ///
  /// @param[in] xproc X index of sending processor
  /// @param[in] yproc Y index of sending processor
  /// @param[inout] buffer  The buffer to fill with data. Must already be allocated of length \p size
  /// @param[in] size  The length of \p buffer
  /// @param[in] tag   A label, must be the same as send
  virtual comm_handle receiveFromProc(int xproc, int yproc, BoutReal *buffer, int size, int tag) = 0;
  
  virtual int getNXPE() = 0; ///< The number of processors in the X direction
  virtual int getNYPE() = 0; ///< The number of processors in the Y direction
  virtual int getXProcIndex() = 0; ///< This processor's index in X direction
  virtual int getYProcIndex() = 0; ///< This processor's index in Y direction
  
  // X communications
  virtual bool firstX() = 0;  ///< Is this processor first in X? i.e. is there a boundary to the left in X?
  virtual bool lastX() = 0; ///< Is this processor last in X? i.e. is there a boundary to the right in X?
  bool periodicX; ///< Domain is periodic in X?

  int NXPE, PE_XIND; ///< Number of processors in X, and X processor index

  /// Send a buffer of data to processor at X index +1
  ///
  /// @param[in] buffer  The data to send. Must be at least length \p size
  /// @param[in] size    The number of BoutReals to send
  /// @param[in] tag     A label for the communication. Must be the same at receive
  virtual int sendXOut(BoutReal *buffer, int size, int tag) = 0;

  /// Send a buffer of data to processor at X index -1
  ///
  /// @param[in] buffer  The data to send. Must be at least length \p size
  /// @param[in] size    The number of BoutReals to send
  /// @param[in] tag     A label for the communication. Must be the same at receive
  virtual int sendXIn(BoutReal *buffer, int size, int tag) = 0;

  /// Receive a buffer of data from X index +1
  ///
  /// @param[in] buffer  A buffer to put the data in. Must already be allocated of length \p size
  /// @param[in] size    The number of BoutReals to receive and put in \p buffer
  /// @param[in] tag     A label for the communication. Must be the same as sent
  virtual comm_handle irecvXOut(BoutReal *buffer, int size, int tag) = 0;

  /// Receive a buffer of data from X index -1
  ///
  /// @param[in] buffer  A buffer to put the data in. Must already be allocated of length \p size
  /// @param[in] size    The number of BoutReals to receive and put in \p buffer
  /// @param[in] tag     A label for the communication. Must be the same as sent
  virtual comm_handle irecvXIn(BoutReal *buffer, int size, int tag) = 0;

  MPI_Comm getXcomm() {return getXcomm(0);} ///< Return communicator containing all processors in X
  virtual MPI_Comm getXcomm(int jy) const = 0; ///< Return X communicator
  virtual MPI_Comm getYcomm(int jx) const = 0; ///< Return Y communicator
  
  /// Is local X index \p jx periodic in Y?
  ///
  /// \param[in] jx   The local (on this processor) index in X
  virtual bool periodicY(int jx) const;

  /// Is local X index \p jx periodic in Y?
  ///
  /// \param[in] jx   The local (on this processor) index in X
  /// \param[out] ts  The Twist-Shift angle if periodic
  virtual bool periodicY(int jx, BoutReal &ts) const = 0;
  
  virtual int ySize(int jx) const; ///< The number of points in Y at fixed X index \p jx

  // Y communications
  virtual bool firstY() const = 0; ///< Is this processor first in Y? i.e. is there a boundary at lower Y?
  virtual bool lastY() const = 0; ///< Is this processor last in Y? i.e. is there a boundary at upper Y?
  virtual bool firstY(int xpos) const = 0; ///< Is this processor first in Y? i.e. is there a boundary at lower Y?
  virtual bool lastY(int xpos) const = 0; ///< Is this processor last in Y? i.e. is there a boundary at upper Y?
  virtual int UpXSplitIndex() = 0;  ///< If the upper Y guard cells are split in two, return the X index where the split occurs
  virtual int DownXSplitIndex() = 0; ///< If the lower Y guard cells are split in two, return the X index where the split occurs

  /// Send data
  virtual int sendYOutIndest(BoutReal *buffer, int size, int tag) = 0;

  /// 
  virtual int sendYOutOutdest(BoutReal *buffer, int size, int tag) = 0;

  ///
  virtual int sendYInIndest(BoutReal *buffer, int size, int tag) = 0;

  ///
  virtual int sendYInOutdest(BoutReal *buffer, int size, int tag) = 0;

  /// Non-blocking receive. Must be followed by a call to wait()
  ///
  /// @param[out] buffer  A buffer of length \p size which must already be allocated
  /// @param[in] size The number of BoutReals expected
  /// @param[in] tag  The tag number of the expected message
  virtual comm_handle irecvYOutIndest(BoutReal *buffer, int size, int tag) = 0;

  /// Non-blocking receive. Must be followed by a call to wait()
  ///
  /// @param[out] buffer  A buffer of length \p size which must already be allocated
  /// @param[in] size The number of BoutReals expected
  /// @param[in] tag  The tag number of the expected message
  virtual comm_handle irecvYOutOutdest(BoutReal *buffer, int size, int tag) = 0;

  /// Non-blocking receive. Must be followed by a call to wait()
  ///
  /// @param[out] buffer  A buffer of length \p size which must already be allocated
  /// @param[in] size The number of BoutReals expected
  /// @param[in] tag  The tag number of the expected message
  virtual comm_handle irecvYInIndest(BoutReal *buffer, int size, int tag) = 0;

  /// Non-blocking receive. Must be followed by a call to wait()
  ///
  /// @param[out] buffer  A buffer of length \p size which must already be allocated
  /// @param[in] size The number of BoutReals expected
  /// @param[in] tag  The tag number of the expected message
  virtual comm_handle irecvYInOutdest(BoutReal *buffer, int size, int tag) = 0;
  
  // Boundary region iteration

  /// Iterate over the lower Y boundary
  virtual const RangeIterator iterateBndryLowerY() const = 0;

  /// Iterate over the upper Y boundary
  virtual const RangeIterator iterateBndryUpperY() const = 0;
  virtual const RangeIterator iterateBndryLowerOuterY() const = 0;
  virtual const RangeIterator iterateBndryLowerInnerY() const = 0;
  virtual const RangeIterator iterateBndryUpperOuterY() const = 0;
  virtual const RangeIterator iterateBndryUpperInnerY() const = 0;
  
  bool hasBndryLowerY(); ///< Is there a boundary on the lower guard cells in Y?
  bool hasBndryUpperY(); ///< Is there a boundary on the upper guard cells in Y?

  // Boundary regions

  /// Return a vector containing all the boundary regions on this processor
  virtual vector<BoundaryRegion*> getBoundaries() = 0;

  /// Add a boundary region to this processor
  virtual void addBoundary(BoundaryRegion* UNUSED(bndry)) {}

  /// Get all the parallel (Y) boundaries on this processor 
  virtual vector<BoundaryRegionPar*> getBoundariesPar() = 0;

  /// Add a parallel(Y) boundary to this processor 
  virtual void addBoundaryPar(BoundaryRegionPar* UNUSED(bndry)) {}
  
  /// Branch-cut special handling (experimental)
  virtual const Field3D smoothSeparatrix(const Field3D &f) {return f;}
  
  virtual BoutReal GlobalX(int jx) const = 0; ///< Continuous X index between 0 and 1
  virtual BoutReal GlobalY(int jy) const = 0; ///< Continuous Y index (0 -> 1)
  virtual BoutReal GlobalX(BoutReal jx) const = 0; ///< Continuous X index between 0 and 1
  virtual BoutReal GlobalY(BoutReal jy) const = 0; ///< Continuous Y index (0 -> 1)
  
  //////////////////////////////////////////////////////////
  
  int GlobalNx, GlobalNy, GlobalNz; ///< Size of the global arrays. Note: can have holes
  int OffsetX, OffsetY, OffsetZ;    ///< Offset of this mesh within the global array
                                    ///< so startx on this processor is OffsetX in global
  
  /// Global locator functions
  virtual int XGLOBAL(int xloc) const = 0; ///< Continuous global X index
  virtual int YGLOBAL(int yloc) const = 0; ///< Continuous global Y index

  /// Not sure what this does. Probably should not be part of Mesh
  /// \todo Make this function standalone
  virtual void slice_r_y(const BoutReal *, BoutReal *, int , int)=0;

  /// Split a dcomplex array \p ayn of length \p n
  /// into real part \p r and imaginary part \p i
  /// All arrays are assumed to be allocated
  ///
  /// \todo Make this function standalone
  virtual void get_ri( dcomplex *ayn, int n, BoutReal *r, BoutReal *i)=0;

  /// Set a dcomplex array \p ayn of length \p n
  /// taking values from real array \p r and imaginary
  /// array \p i
  /// All arrays are assumed to be allocated
  ///
  /// \todo Make this function standalone
  virtual void set_ri( dcomplex *ayn, int n, BoutReal *r, BoutReal *i)=0;

  /// poloidal lowpass filter for n=0 mode
  /// \todo Make this function standalone
  virtual const Field2D lowPass_poloidal(const Field2D &,int)=0;
  
  /// volume integral

  /// Transpose Y and Z dimensions. Assumes that the
  /// size of the global Y and Z dimensions are the same
  ///
  /// \todo Make this function standalone
  virtual const Field3D Switch_YZ(const Field3D &var) = 0;

  /// Transpose X and Z dimensions. Assumes that the
  /// size of the global X and Z dimensions are the same
  ///
  /// \todo Make this function standalone
  virtual const Field3D Switch_XZ(const Field3D &var) = 0;

  /// Size of the mesh on this processor including guard/boundary cells
  int LocalNx, LocalNy, LocalNz;
  
  /// Local ranges of data (inclusive), excluding guard cells
  int xstart, xend, ystart, yend;
  
  bool StaggerGrids;    ///< Enable staggered grids (Centre, Lower). Otherwise all vars are cell centred (default).
  
  bool IncIntShear; ///< Include integrated shear (if shifting X)

  /// Coordinate system
  Coordinates *coordinates() {
    if (coords) { // True branch most common, returns immediately
      return coords;
    }
    // No coordinate system set. Create default
    // Note that this can't be allocated here due to incomplete type
    // (circular dependency between Mesh and Coordinates)
    coords = createDefaultCoordinates();
    return coords;
  }

  // First derivatives in index space
  // Implemented in src/mesh/index_derivs.hxx

  const Field3D indexDDX(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method); ///< First derivative in X direction, in index space
  const Field2D indexDDX(const Field2D &f); ///< First derivative in X direction, in index space
  
  const Field3D indexDDY(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method); ///< First derivative in Y direction in index space
  const Field2D indexDDY(const Field2D &f); ///< ///< First derivative in Y direction in index space

  const Field3D indexDDZ(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method, bool inc_xbndry); ///< First derivative in Z direction in index space
  const Field2D indexDDZ(const Field2D &f); ///< First derivative in Z direction in index space

  // Second derivatives in index space
  // Implemented in src/mesh/index_derivs.hxx
  
  /// Second derivative in X direction in index space
  ///
  /// @param[in] f  The field to be differentiated
  /// @param[in] outloc  The cell location where the result is desired
  /// @param[in] method  The differencing method to use, overriding default
  const Field3D indexD2DX2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field2D indexD2DX2(const Field2D &f); ///< Second derivative in X direction in index space

  /// Second derivative in Y direction in index space
  ///
  /// @param[in] f  The field to be differentiated
  /// @param[in] outloc  The cell location where the result is desired
  /// @param[in] method  The differencing method to use, overriding default
  const Field3D indexD2DY2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field2D indexD2DY2(const Field2D &f); ///< Second derivative in Y direction in index space

  /// Second derivative in Z direction in index space
  ///
  /// @param[in] f  The field to be differentiated
  /// @param[in] outloc  The cell location where the result is desired
  /// @param[in] method  The differencing method to use, overriding default
  const Field3D indexD2DZ2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method, bool inc_xbndry);
  
  // Fourth derivatives in index space
  const Field3D indexD4DX4(const Field3D &f); ///< Fourth derivative in X direction in index space
  const Field2D indexD4DX4(const Field2D &f); ///< Fourth derivative in X direction in index space
  const Field3D indexD4DY4(const Field3D &f); ///< Fourth derivative in Y direction in index space
  const Field2D indexD4DY4(const Field2D &f); ///< Fourth derivative in Y direction in index space
  const Field3D indexD4DZ4(const Field3D &f); ///< Fourth derivative in Z direction in index space
  
  // Advection schemes

  /// Advection operator in index space in X direction
  ///
  /// \f[
  ///   v \frac{d}{di} f
  /// \f]
  ///
  /// @param[in] The velocity in the X direction
  /// @param[in] f  The field being advected
  /// @param[in] outloc The cell location where the result is desired. The default is the same as \p f
  /// @param[in] method  The differencing method to use
  const Field2D indexVDDX(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field3D indexVDDX(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method);

  /// Advection operator in index space in Y direction
  ///
  /// \f[
  ///   v \frac{d}{di} f
  /// \f]
  ///
  /// @param[in] The velocity in the Y direction
  /// @param[in] f  The field being advected
  /// @param[in] outloc The cell location where the result is desired. The default is the same as \p f
  /// @param[in] method  The differencing method to use
  const Field2D indexVDDY(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field3D indexVDDY(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method);

  /// Advection operator in index space in Z direction
  ///
  /// \f[
  ///   v \frac{d}{di} f
  /// \f]
  ///
  /// @param[in] The velocity in the Z direction
  /// @param[in] f  The field being advected
  /// @param[in] outloc The cell location where the result is desired. The default is the same as \p f
  /// @param[in] method  The differencing method to use
  const Field3D indexVDDZ(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method);

  const Field2D indexFDDX(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field3D indexFDDX(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field2D indexFDDY(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field3D indexFDDY(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method);
  const Field3D indexFDDZ(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method);
  
  typedef BoutReal (*deriv_func)(stencil &); ///< Derivative functions of a single field stencil
  typedef BoutReal (*upwind_func)(BoutReal, stencil &); ///< Derivative functions of a BoutReal velocity, and field stencil
  typedef BoutReal (*flux_func)(stencil&, stencil &); ///< ///< Derivative functions of a velocity field, and field stencil v, f

  /// Transform a field into field-aligned coordinates
  const Field3D toFieldAligned(const Field3D &f) {
    return getParallelTransform().toFieldAligned(f);
  }
  /// Convert back into standard form
  const Field3D fromFieldAligned(const Field3D &f) {
    return getParallelTransform().fromFieldAligned(f);
  }

  /*!
   * Unique pointer to ParallelTransform object
   */
  typedef std::unique_ptr<ParallelTransform> PTptr;
  
  /*!
   * Set the parallel (y) transform for this mesh.
   * Unique pointer used so that ParallelTransform will be deleted
   */
  void setParallelTransform(PTptr pt) {
    transform = std::move(pt);
  }
  /*!
   * Set the parallel (y) transform from the options file
   */
  void setParallelTransform();
  
 protected:
  
  GridDataSource *source; ///< Source for grid data
  
  Coordinates *coords;    ///< Coordinate system. Initialised to Null

  Options *options; ///< Mesh options section
  
  /*!
   * Return the parallel transform, setting it if need be
   */
  ParallelTransform& getParallelTransform();

  PTptr transform; ///< Handles calculation of yup and ydown

  /// Read a 1D array of integers
  const vector<int> readInts(const string &name, int n);
  
  /// Calculates the size of a message for a given x and y range
  int msg_len(const vector<FieldData*> &var_list, int xge, int xlt, int yge, int ylt);
  
  // Initialise derivatives
  void derivs_init(Options* options);
  
  // Loop over mesh, applying a stencil in the X direction
  const Field2D applyXdiff(const Field2D &var, deriv_func func, CELL_LOC loc = CELL_DEFAULT, REGION region = RGN_NOX);
  const Field3D applyXdiff(const Field3D &var, deriv_func func, CELL_LOC loc = CELL_DEFAULT, REGION region = RGN_NOX);
  
  const Field2D applyYdiff(const Field2D &var, deriv_func func, CELL_LOC loc = CELL_DEFAULT, REGION region = RGN_NOBNDRY);
  const Field3D applyYdiff(const Field3D &var, deriv_func func, CELL_LOC loc = CELL_DEFAULT, REGION region = RGN_NOBNDRY);

  const Field3D applyZdiff(const Field3D &var, Mesh::deriv_func func, CELL_LOC loc = CELL_DEFAULT, REGION region = RGN_ALL);
  
private:
  /// Allocates a default Coordinates object
  Coordinates *createDefaultCoordinates();
};

#endif // __MESH_H__

