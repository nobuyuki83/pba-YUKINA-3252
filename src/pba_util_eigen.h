//
// Created by Nobuyuki Umetani on 2023/05/15.
//

#ifndef PBA_UTIL_EIGEN_H_
#define PBA_UTIL_EIGEN_H_

#include <set>

namespace pba {

auto generate_mesh_annulus3(
    float r_small,
    float r_large,
    int ndiv_radius,
    int ndiv_theta) {
  Eigen::Matrix<int, Eigen::Dynamic, 3, Eigen::RowMajor> tri2vtx(ndiv_radius * ndiv_theta * 2, 3);
  Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor> vtx2xyz((ndiv_radius + 1) * ndiv_theta, 3);
  constexpr float pi = 3.1415926535;
  { // make coordinates
    float dr = (r_large - r_small) / static_cast<float>(ndiv_radius);
    float dth = 2.f * pi / static_cast<float>(ndiv_theta);
    for (int ir = 0; ir <= ndiv_radius; ir++) {
      for (int ith = 0; ith < ndiv_theta; ith++) {
        float rad = dr * static_cast<float>(ir) + r_small;
        float theta = static_cast<float>(ith +  (ir%2)*0.5 ) * dth;
        vtx2xyz.row(ir * ndiv_theta + ith) = Eigen::Vector3f(rad * cos(theta), 0.f, rad * sin(theta));
      }
    }
  }
  for (int ir = 0; ir < ndiv_radius; ir++) {
    for (int ith = 0; ith < ndiv_theta; ith++) {
      int i1 = (ir + 0) * ndiv_theta + (ith + 0) % ndiv_theta;
      int i2 = (ir + 0) * ndiv_theta + (ith + 1) % ndiv_theta;
      int i3 = (ir + 1) * ndiv_theta + (ith + 1) % ndiv_theta;
      int i4 = (ir + 1) * ndiv_theta + (ith + 0) % ndiv_theta;
      if (ir % 2 == 1) {
        tri2vtx.row((ir * ndiv_theta + ith) * 2 + 0) = Eigen::Vector3i(i3, i1, i2);
        tri2vtx.row((ir * ndiv_theta + ith) * 2 + 1) = Eigen::Vector3i(i4, i1, i3);
      }
      else {
        tri2vtx.row((ir * ndiv_theta + ith) * 2 + 0) = Eigen::Vector3i(i4, i2, i3);
        tri2vtx.row((ir * ndiv_theta + ith) * 2 + 1) = Eigen::Vector3i(i4, i1, i2);
      }
    }

  }
  return std::make_pair(tri2vtx, vtx2xyz);
}

auto vertex_to_elem(
    const Eigen::MatrixXi &elem2vtx,
    size_t num_vtx) {
  std::vector<unsigned int> vtx2idx, idx2elem;
  vtx2idx.assign(num_vtx + 1, 0);
  for (int i_elem = 0; i_elem < elem2vtx.rows(); i_elem++) {
    for (int i_node = 0; i_node < elem2vtx.cols(); i_node++) {
      const int i_vtx = elem2vtx(i_elem, i_node);
      vtx2idx[i_vtx + 1] += 1;
    }
  }
  for (unsigned int i_vtx = 0; i_vtx < num_vtx; ++i_vtx) {
    vtx2idx[i_vtx + 1] += vtx2idx[i_vtx];
  }
  unsigned int num_idx = vtx2idx[num_vtx];
  idx2elem.resize(num_idx);
  for (int i_elem = 0; i_elem < elem2vtx.rows(); i_elem++) {
    for (int i_node = 0; i_node < elem2vtx.cols(); i_node++) {
      const int i_vtx = elem2vtx(i_elem, i_node);
      const unsigned int ind1 = vtx2idx[i_vtx];
      idx2elem[ind1] = i_elem;
      vtx2idx[i_vtx] += 1;
    }
  }
  for (int ivtx = static_cast<int>(num_vtx); ivtx >= 1; --ivtx) {
    vtx2idx[ivtx] = vtx2idx[ivtx - 1];
  }
  vtx2idx[0] = 0;
  return std::make_pair(vtx2idx, idx2elem);
}

auto lines_of_mesh(
    const Eigen::MatrixXi &elem2vtx,
    int num_vtx) {
  const auto[vtx2idx, idx2tri] = vertex_to_elem(elem2vtx, num_vtx);
  std::vector<int> _line2vtx;
  for (int i_vtx = 0; i_vtx < num_vtx; ++i_vtx) {
    std::set<int> set_connected_points;
    for (unsigned int idx = vtx2idx[i_vtx]; idx < vtx2idx[i_vtx + 1]; ++idx) {
      const unsigned int itri0 = idx2tri[idx];
      for (int inode = 0; inode < elem2vtx.cols(); ++inode) {
        const int j_vtx = elem2vtx(itri0, inode);
        if (j_vtx <= i_vtx) continue;
        set_connected_points.insert(j_vtx);
      }
    }
    for (int j_vtx: set_connected_points) {
      _line2vtx.push_back(i_vtx);
      _line2vtx.push_back(j_vtx);
    }
  }
  auto map = Eigen::Map<Eigen::Matrix<int, Eigen::Dynamic, 2, Eigen::RowMajor> >(
      _line2vtx.data(), static_cast<int>(_line2vtx.size() / 2), 2);
  return Eigen::Matrix<int, Eigen::Dynamic, 2, Eigen::RowMajor>(map);
}

Eigen::Vector3f unit_normal_of_triangle(
    const Eigen::Vector3f& v1,
    const Eigen::Vector3f& v2,
    const Eigen::Vector3f& v3){
  return (v2-v1).cross(v3-v1).normalized();
}

}

#endif //PBA_UTIL_EIGEN_H_
