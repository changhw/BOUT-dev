/**************************************************************************
 * Operators on vector objects
 * B.Dudson, October 2007
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

#include <globals.hxx>
#include <vecops.hxx>
#include <derivs.hxx>
#include <msg_stack.hxx>

/**************************************************************************
 * Gradient operators
 **************************************************************************/

const Vector2D Grad(const Field2D &f, CELL_LOC outloc)
{
  Vector2D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Grad( Field2D )");
#endif

  result.x = DDX(f);
  result.y = DDY(f);
  result.z = DDZ(f);

  result.covariant = true;

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D Grad(const Field3D &f, 
                    CELL_LOC outloc_x, CELL_LOC outloc_y, CELL_LOC outloc_z)
{
  Vector3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Grad( Field3D )");
#endif

  if(outloc_x == CELL_DEFAULT)
    outloc_x = f.getLocation();
  if(outloc_y == CELL_DEFAULT)
    outloc_y = f.getLocation();
  if(outloc_z == CELL_DEFAULT)
    outloc_z = f.getLocation();

  result.x = DDX(f, outloc_x);
  result.y = DDY(f, outloc_y);
  result.z = DDZ(f, outloc_z);

  result.covariant = true;
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D Grad(const Field3D &f, CELL_LOC outloc)
{
  if(outloc == CELL_VSHIFT)
    return Grad(f, CELL_XLOW, CELL_YLOW, CELL_ZLOW);
  
  return Grad(f, outloc, outloc, outloc);
}

const Vector3D Grad_perp(const Field3D &f, 
			 CELL_LOC outloc_x, CELL_LOC outloc_y, CELL_LOC outloc_z) {
  Vector3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Grad_perp( Field3D )");
#endif

  Coordinates *metric = mesh->coordinates();

  if(outloc_x == CELL_DEFAULT)
    outloc_x = f.getLocation();
  if(outloc_z == CELL_DEFAULT)
    outloc_z = f.getLocation();

  Field3D parcoef = 1./ (metric->J * metric->Bxy);
  parcoef *= parcoef;

  result.x = DDX(f, outloc_x) - parcoef*metric->g_12*DDY(f, outloc_x);
  result.y = 0.0;
  result.z = DDZ(f, outloc_z) - parcoef*metric->g_23*DDY(f, outloc_z);

  result.covariant = true;
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

/**************************************************************************
 * Divergence operators
 **************************************************************************/

const Field2D Div(const Vector2D &v, CELL_LOC outloc)
{
  Field2D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Div( Vector2D )");
#endif
  
  Coordinates *metric = mesh->coordinates();
  
  // get contravariant components of v
  Vector2D vcn = v;
  vcn.toContravariant();
  
  result = DDX(metric->J*vcn.x);
  result += DDY(metric->J*vcn.y);
  result += DDZ(metric->J*vcn.z);
  result /= metric->J;

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D Div(const Vector3D &v, CELL_LOC outloc)
{
  Field3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Div( Vector3D )");
#endif
  
  Coordinates *metric = mesh->coordinates();

  if(outloc == CELL_DEFAULT) 
    outloc = CELL_CENTRE;

  // get contravariant components of v
  Vector3D vcn = v;
  vcn.toContravariant();
  
  result = DDX(metric->J*vcn.x, outloc);
  result += DDY(metric->J*vcn.y, outloc);
  result += DDZ(metric->J*vcn.z, outloc);
  result /= metric->J;

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

/**************************************************************************
 * Divergence operators for flux methods
 **************************************************************************/

const Field2D Div(const Vector2D &v, const Field2D &f) {
  Field2D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Div( Vector2D, Field2D )");
#endif
  
  Coordinates *metric = mesh->coordinates();
  
  // get contravariant components of v
  Vector2D vcn = v;
  vcn.toContravariant();
  
  result = FDDX(metric->J*vcn.x, f);
  result += FDDY(metric->J*vcn.y, f);
  result += FDDZ(metric->J*vcn.z, f);
  result /= metric->J;

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D Div(const Vector3D &v, const Field3D &f, DIFF_METHOD method, CELL_LOC outloc)
{
    Field3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Div( Vector3D, Field3D )");
#endif
  
  Coordinates *metric = mesh->coordinates();
  
  if(outloc == CELL_DEFAULT) 
    outloc = CELL_CENTRE;

  // get contravariant components of v
  Vector3D vcn = v;
  vcn.toContravariant();
  
  result = FDDX(metric->J*vcn.x, f, method, outloc);
  result += FDDY(metric->J*vcn.y, f, method, outloc);
  result += FDDZ(metric->J*vcn.z, f, method, outloc);
  result /= metric->J;

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D Div(const Vector3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method)
{
  return Div(v, f, method, outloc);
}

const Field3D Div(const Vector3D &v, const Field3D &f)
{
  return Div(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

/**************************************************************************
 * Curl operators
 **************************************************************************/

const Vector2D Curl(const Vector2D &v, CELL_LOC outloc) {
  Vector2D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Curl( Vector2D )");
#endif
  
  Coordinates *metric = mesh->coordinates();
  
  // Get covariant components of v
  Vector2D vco = v;
  vco.toCovariant();

  // get components (curl(v))^j

  result.x = (DDY(vco.z) - DDZ(vco.y))/metric->J;
  result.y = (DDZ(vco.x) - DDX(vco.z))/metric->J;
  result.z = (DDX(vco.y) - DDY(vco.x))/metric->J;
  
  if(mesh->ShiftXderivs) {
    result.z -= metric->ShiftTorsion*vco.z / metric->J;
  }

  result.covariant = false; // result is contravariant
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif
  return result;
}

const Vector3D Curl(const Vector3D &v, 
                    CELL_LOC outloc_x, CELL_LOC outloc_y, CELL_LOC outloc_z) {
  Vector3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("Curl( Vector3D )");
#endif

  Coordinates *metric = mesh->coordinates();

  // Get covariant components of v
  Vector3D vco = v;
  vco.toCovariant();

  // get components (curl(v))^j

  result.x = (DDY(vco.z, outloc_x) - DDZ(vco.y, outloc_x))/metric->J;
  result.y = (DDZ(vco.x, outloc_y) - DDX(vco.z, outloc_y))/metric->J;
  result.z = (DDX(vco.y, outloc_z) - DDY(vco.x, outloc_z))/metric->J;

  if(mesh->ShiftXderivs) {
    result.z -= metric->ShiftTorsion*vco.z / metric->J;
  }

  result.covariant = false; // result is contravariant

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D Curl(const Vector3D &v, CELL_LOC outloc)
{
  if(outloc == CELL_VSHIFT)
    return Curl(v, CELL_XLOW, CELL_YLOW, CELL_ZLOW);
  
  return Curl(v, outloc, outloc, outloc);
}

/**************************************************************************
 * Upwinding operators
 **************************************************************************/

const Field2D V_dot_Grad(const Vector2D &v, const Field2D &f)
{
  Field2D result;
  
#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector2D , Field2D )");
#endif

  // Get contravariant components of v
  Vector2D vcn = v;
  vcn.toContravariant();

  result = VDDX(vcn.x, f) + VDDY(vcn.y, f) + VDDZ(vcn.z, f);

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D V_dot_Grad(const Vector2D &v, const Field3D &f)
{
  Field3D result;
  
#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector2D , Field3D )");
#endif

  // Get contravariant components of v
  Vector2D vcn = v;
  vcn.toContravariant();

  result = VDDX(vcn.x, f) + VDDY(vcn.y, f) + VDDZ(vcn.z, f);

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D V_dot_Grad(const Vector3D &v, const Field2D &f)
{
  Field3D result;
  
#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector3D , Field2D )");
#endif

  // Get contravariant components of v
  Vector3D vcn = v;
  vcn.toContravariant();

  result = VDDX(vcn.x, f) + VDDY(vcn.y, f) + VDDZ(vcn.z, f);

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Field3D V_dot_Grad(const Vector3D &v, const Field3D &f)
{
  Field3D result;
  
#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector3D , Field3D )");
#endif

  // Get contravariant components of v
  Vector3D vcn = v;
  vcn.toContravariant();
  
  result = VDDX(vcn.x, f) + VDDY(vcn.y, f) + VDDZ(vcn.z, f);

#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector2D V_dot_Grad(const Vector2D &v, const Vector2D &a) {
  Vector2D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector2D , Vector2D )");
#endif

  Coordinates *metric = mesh->coordinates();

  Vector2D vcn = v;
  vcn.toContravariant();

  if(a.covariant) {
    
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x -= vcn.x*(metric->G1_11*a.x + metric->G2_11*a.y + metric->G3_11*a.z);
    result.x -= vcn.y*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.x -= vcn.z*(metric->G1_13*a.x + metric->G3_13*a.z);

    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y -= vcn.x*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.y -= vcn.y*(metric->G1_22*a.x + metric->G2_22*a.y + metric->G3_22*a.z);
    result.y -= vcn.z*(metric->G2_23*a.y + metric->G3_23*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z -= vcn.x*(metric->G1_13*a.x + metric->G3_13*a.z);
    result.z -= vcn.y*(metric->G2_23*a.y + metric->G3_23*a.z);
    result.z -= vcn.z*(metric->G1_33*a.x + metric->G2_33*a.y + metric->G3_33*a.z);

    result.covariant = true;
  }else {
    
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x += vcn.x*(metric->G1_11*a.x + metric->G1_12*a.y + metric->G1_13*a.z);
    result.x += vcn.y*(metric->G1_12*a.x + metric->G1_22*a.y);
    result.x += vcn.z*(metric->G1_13*a.x + metric->G1_33*a.z);
    
    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y += vcn.x*(metric->G2_11*a.x + metric->G2_12*a.y);
    result.y += vcn.y*(metric->G2_12*a.x + metric->G2_22*a.y + metric->G2_23*a.z);
    result.y += vcn.z*(metric->G2_23*a.y + metric->G2_33*a.z);
    
    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z += vcn.x*(metric->G3_11*a.x + metric->G3_13*a.z);
    result.z += vcn.y*(metric->G3_22*a.y + metric->G3_23*a.z);
    result.z += vcn.z*(metric->G3_13*a.x + metric->G3_23*a.y + metric->G3_33*a.z);

    result.covariant = false;
  }
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D V_dot_Grad(const Vector2D &v, const Vector3D &a) {
  Vector3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector2D , Vector3D )");
#endif

  Coordinates *metric = mesh->coordinates();

  Vector2D vcn = v;
  vcn.toContravariant();

  if(a.covariant) {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x -= vcn.x*(metric->G1_11*a.x + metric->G2_11*a.y + metric->G3_11*a.z);
    result.x -= vcn.y*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.x -= vcn.z*(metric->G1_13*a.x + metric->G3_13*a.z);

    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y -= vcn.x*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.y -= vcn.y*(metric->G1_22*a.x + metric->G2_22*a.y + metric->G3_22*a.z);
    result.y -= vcn.z*(metric->G2_23*a.y + metric->G3_23*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z -= vcn.x*(metric->G1_13*a.x + metric->G3_13*a.z);
    result.z -= vcn.y*(metric->G2_23*a.y + metric->G3_23*a.z);
    result.z -= vcn.z*(metric->G1_33*a.x + metric->G2_33*a.y + metric->G3_33*a.z);

    result.covariant = true;
  }else {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x += vcn.x*(metric->G1_11*a.x + metric->G1_12*a.y + metric->G1_13*a.z);
    result.x += vcn.y*(metric->G1_12*a.x + metric->G1_22*a.y);
    result.x += vcn.z*(metric->G1_13*a.x + metric->G1_33*a.z);

    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y += vcn.x*(metric->G2_11*a.x + metric->G2_12*a.y);
    result.y += vcn.y*(metric->G2_12*a.x + metric->G2_22*a.y + metric->G2_23*a.z);
    result.y += vcn.z*(metric->G2_23*a.y + metric->G2_33*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z += vcn.x*(metric->G3_11*a.x + metric->G3_13*a.z);
    result.z += vcn.y*(metric->G3_22*a.y + metric->G3_23*a.z);
    result.z += vcn.z*(metric->G3_13*a.x + metric->G3_23*a.y + metric->G3_33*a.z);

    result.covariant = false;
  }
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D V_dot_Grad(const Vector3D &v, const Vector2D &a) {
  Vector3D result;
  
#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector3D , Vector2D )");
#endif

  Coordinates *metric = mesh->coordinates();

  Vector3D vcn = v;
  vcn.toContravariant();

  if(a.covariant) {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x -= vcn.x*(metric->G1_11*a.x + metric->G2_11*a.y + metric->G3_11*a.z);
    result.x -= vcn.y*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.x -= vcn.z*(metric->G1_13*a.x + metric->G3_13*a.z);

    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y -= vcn.x*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.y -= vcn.y*(metric->G1_22*a.x + metric->G2_22*a.y + metric->G3_22*a.z);
    result.y -= vcn.z*(metric->G2_23*a.y + metric->G3_23*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z -= vcn.x*(metric->G1_13*a.x + metric->G3_13*a.z);
    result.z -= vcn.y*(metric->G2_23*a.y + metric->G3_23*a.z);
    result.z -= vcn.z*(metric->G1_33*a.x + metric->G2_33*a.y + metric->G3_33*a.z);

    result.covariant = true;
  }else {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x += vcn.x*(metric->G1_11*a.x + metric->G1_12*a.y + metric->G1_13*a.z);
    result.x += vcn.y*(metric->G1_12*a.x + metric->G1_22*a.y);
    result.x += vcn.z*(metric->G1_13*a.x + metric->G1_33*a.z);

    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y += vcn.x*(metric->G2_11*a.x + metric->G2_12*a.y);
    result.y += vcn.y*(metric->G2_12*a.x + metric->G2_22*a.y + metric->G2_23*a.z);
    result.y += vcn.z*(metric->G2_23*a.y + metric->G2_33*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z += vcn.x*(metric->G3_11*a.x + metric->G3_13*a.z);
    result.z += vcn.y*(metric->G3_22*a.y + metric->G3_23*a.z);
    result.z += vcn.z*(metric->G3_13*a.x + metric->G3_23*a.y + metric->G3_33*a.z);

    result.covariant = false;
  }
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}

const Vector3D V_dot_Grad(const Vector3D &v, const Vector3D &a)
{
  Vector3D result;

#ifdef CHECK
  int msg_pos = msg_stack.push("V_dot_Grad( Vector3D , Vector3D )");
#endif

  Coordinates *metric = mesh->coordinates();

  Vector3D vcn = v;
  vcn.toContravariant();

  if(a.covariant) {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x -= vcn.x*(metric->G1_11*a.x + metric->G2_11*a.y + metric->G3_11*a.z);
    result.x -= vcn.y*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.x -= vcn.z*(metric->G1_13*a.x + metric->G3_13*a.z);
    
    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y -= vcn.x*(metric->G1_12*a.x + metric->G2_12*a.y);
    result.y -= vcn.y*(metric->G1_22*a.x + metric->G2_22*a.y + metric->G3_22*a.z);
    result.y -= vcn.z*(metric->G2_23*a.y + metric->G3_23*a.z);

    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z -= vcn.x*(metric->G1_13*a.x + metric->G3_13*a.z);
    result.z -= vcn.y*(metric->G2_23*a.y + metric->G3_23*a.z);
    result.z -= vcn.z*(metric->G1_33*a.x + metric->G2_33*a.y + metric->G3_33*a.z);

    result.covariant = true;
  }else {
    result.x = VDDX(vcn.x, a.x) + VDDY(vcn.y, a.x) + VDDZ(vcn.z, a.x);
    result.x += vcn.x*(metric->G1_11*a.x + metric->G1_12*a.y + metric->G1_13*a.z);
    result.x += vcn.y*(metric->G1_12*a.x + metric->G1_22*a.y);
    result.x += vcn.z*(metric->G1_13*a.x + metric->G1_33*a.z);
    
    result.y = VDDX(vcn.x, a.y) + VDDY(vcn.y, a.y) + VDDZ(vcn.z, a.y);
    result.y += vcn.x*(metric->G2_11*a.x + metric->G2_12*a.y);
    result.y += vcn.y*(metric->G2_12*a.x + metric->G2_22*a.y + metric->G2_23*a.z);
    result.y += vcn.z*(metric->G2_23*a.y + metric->G2_33*a.z);
    
    result.z = VDDX(vcn.x, a.z) + VDDY(vcn.y, a.z) + VDDZ(vcn.z, a.z);
    result.z += vcn.x*(metric->G3_11*a.x + metric->G3_13*a.z);
    result.z += vcn.y*(metric->G3_22*a.y + metric->G3_23*a.z);
    result.z += vcn.z*(metric->G3_13*a.x + metric->G3_23*a.y + metric->G3_33*a.z);

    result.covariant = false;
  }
  
#ifdef CHECK
  msg_stack.pop(msg_pos);
#endif

  return result;
}


