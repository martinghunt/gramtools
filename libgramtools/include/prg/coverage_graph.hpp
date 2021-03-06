/**
 * @file
 * Defines the `coverage_Graph`, a graph data structure containing:
 *  - Sequence nodes (`coverage_Node`). Each node has:
 *      - Nucleotide sequence
 *      - Outgoing edges (pointers) to other nodes
 *      - Coverage array to store per base coverage for each node
 *      - Site and allele ID
 *      - A position which refers to that in the original Multiple Sequence
 * Alignment
 *  - A bubble map (`coverage_Graph::bubble_map`), used to order the variant
 * sites for both coverage recording and genotyping
 *  - A parental map (`coverage_Graph::par_map`), used for recording grouped
 * allele counts coverage
 *  - A target map (`coverage_Graph::target_map), used to place new
 * `gram::SearchState`s at variant sites during quasimap.
 *  - A random access array (`coverage_Graph::random_access`) used to place a
 * mapped instance in the graph for per base coverage recording.
 */
#ifndef COV_GRAPH_HPP
#define COV_GRAPH_HPP

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/make_shared.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include "linearised_prg.hpp"
#include "prg/types.hpp"

using namespace gram;

/**
 * The building blocks of a `coverage_Graph`
 * Contain sequence, site & allele ID, coverage array
 */
class coverage_Node {
 public:
  coverage_Node();

  coverage_Node(std::size_t pos);

  coverage_Node(std::string const seq, int const pos, int const site_ID = 0,
                int const allele_ID = ALLELE_UNKNOWN);

  /**
   * Compare pointers to `coverage_Node`; used in topological ordering (lastmost
   * sequence position first) Equivalence in a set is defined using this, so we
   * also test whether the pointers are the same objects.
   */
  friend bool operator>(const covG_ptr& lhs, const covG_ptr& rhs);

  friend bool compare_nodes(coverage_Node const& f, coverage_Node const& s);

  friend bool operator==(coverage_Node const& f, coverage_Node const& s);

  friend std::ostream& operator<<(std::ostream& out, coverage_Node const& node);

  bool has_sequence() const { return sequence.size() != 0; }
  bool is_in_bubble() const {
    return allele_ID != ALLELE_UNKNOWN && site_ID != 0;
  }
  bool is_bubble_start() const {
    return next.size() > 1 && sequence.size() == 0;
  }
  bool is_bubble_end() const {
    return next.size() == 1 && sequence.size() == 0;
  }

  /*
   * Getters
   */
  std::size_t get_pos() const { return pos; }
  std::string get_sequence() const { return sequence; }
  std::size_t get_sequence_size() const { return sequence.size(); }
  int get_coverage_space() const { return coverage.size(); }
  PerBaseCoverage const& get_coverage() const { return coverage; }
  PerBaseCoverage& get_ref_to_coverage() { return coverage; }
  Marker get_site_ID() const { return site_ID; }
  AlleleId get_allele_ID() const { return allele_ID; }
  std::vector<covG_ptr> const& get_edges() const { return next; }
  std::size_t get_num_edges() const { return next.size(); }

  /*
   * Setters
   */
  void set_pos(std::size_t pos) { this->pos = pos; }
  void mark_as_boundary() { is_site_boundary = true; }
  void set_coverage(PerBaseCoverage const& new_cov) {
    assert(new_cov.size() == coverage.size() &&
           new_cov.size() == sequence.size());
    coverage = new_cov;
  }

  void add_sequence(std::string const& new_seq);
  void add_edge(covG_ptr const target) { next.emplace_back(target); }
  void clear_edges() { next.clear(); }

 private:
  std::string sequence;
  Marker site_ID;
  AlleleId allele_ID;
  std::size_t pos;
  PerBaseCoverage coverage;
  bool is_site_boundary;
  std::vector<covG_ptr> next;

  // Boost serialisation
  friend class boost::serialization::access;
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& sequence;
    ar& site_ID;
    ar& allele_ID;
    ar& pos;
    ar& coverage;
    ar& is_site_boundary;
    ar& next;  // Array of shared pointers, needs custom includes
  }
};

enum class marker_type { sequence, site_entry, allele_end, site_end };

struct node_access {
  covG_ptr node;        // The referred to node in the `coverage_Graph`
  std::size_t offset;   // The character's offset relative to the start of the
                        // `coverage_Node` it belongs to
  VariantLocus target;  // If the preceding character is a variant marker, gives
                        // what it is.
 private:
  // Boost serialisation
  friend class boost::serialization::access;
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& node;
    ar& offset;
    ar& target;
  }
};

struct targeted_marker {
  Marker ID{0};
  AlleleId direct_deletion_allele{
      ALLELE_UNKNOWN};  // ALLELE_UNKNOWN if not a direct deletion
  friend bool operator==(targeted_marker const& f, targeted_marker const& s);

 private:
  // Boost serialisation
  friend class boost::serialization::access;
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& ID;
    ar& direct_deletion_allele;
  }
};

/**
 * This class implements a DAG of `coverage_Node`s.
 * It is used to record coverage & to perform genotyping in gramtools.
 **/
class coverage_Graph {
 public:
  covG_ptr root;

  coverage_Graph() = default;

  coverage_Graph(coverage_Graph&) = default;
  coverage_Graph(coverage_Graph&&) = default;
  coverage_Graph& operator=(coverage_Graph const&) = default;
  coverage_Graph& operator=(coverage_Graph&&) =
      default;  // Needs to be declared, because user-defined destructor exists.

  ~coverage_Graph();

  /**
   * Build a coverage graph from a PRG String int vector.
   */
  coverage_Graph(PRG_String const& vec_in);

  /** Maps the start of a local bubble, to its end.
   * Children nodes appear before parent nodes.
   * Use : genotyping
   */
  covG_ptr_map bubble_map;

  /**
   * Maps a site ID to a Locus which is its immediate parent in the graph.
   * Only populated for bubbles nested inside another bubble.
   * Use : equivalence class coverage recording
   */
  parental_map par_map;

  /**
   * A vector of the same size as the PRG string, giving access to the
   * corresponding node in the graph. Use : per base coverage recording
   */
  access_vec random_access;

  /**
   * Map from a variant marker to all variant markers it is directly linked to.
   * Note the following convention: the target marker is even if it signals a
   * site entry (in backward searching), and odd if it signals a site exit. Ie,
   * all allele markers that are not the last one of a site get marked as site
   * (odd) markers. Use: read mapping
   */
  target_m target_map;

  bool is_nested{false}; /**< Upon construction, gets set to true if graph has
                            nested bubbles */

  friend bool operator==(coverage_Graph const& f, coverage_Graph const& s);

 private:
  // Boost serialisation
  friend class boost::serialization::access;
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version) {
    /* Important: bubble_map should serialise before root.
     Indeed, `ar & root` recursively serialises all nodes from the root,
     which can overflow the stack and segfault for large graphs.
     Whereas bubble_map is ordered by 'rightmost' node first, so serialisation
     Is less recursive (note serialisation of the same pointer twice is ignored
     by boost)
     */
    ar& bubble_map;
    ar& root;
    ar& par_map;
    ar& random_access;
    ar& target_map;
    ar& is_nested;
  }
};

/**
 * A class in charge of mechanics of building coverage graph
 * It is designed for use by DEVELOPER only
 */
class cov_Graph_Builder {
 public:
  /*
   * All that's needed to make the object is under here
   */
  cov_Graph_Builder(PRG_String const& prg_string);

  cov_Graph_Builder() = default;

  // These will get transferred to the coverage_Graph
  covG_ptr root;
  covG_ptr_map bubble_map;
  parental_map par_map;
  access_vec random_access;
  target_m target_map;

  void make_root(); /**< Start state: set up the globals such as `cur_Node` &
                       `backWire` */
  void make_sink(); /**< End state: final wiring & pointers to null */
  /**
   * Function call dispatcher based on marker at position @param pos.
   * Called once per element in `PRG_String`.
   * @param pos index into the `PRG_String`
   */
  void process_marker(uint32_t const& pos);
  void setup_random_access(uint32_t const& pos);
  void add_sequence(Marker const& m);
  marker_type find_marker_type(uint32_t const& pos);
  void enter_site(Marker const& m);
  void end_allele(Marker const& m);
  void exit_site(Marker const& m);
  /**
   * A convenience function for reaching the end of an allele: called by both
   * `end_allele` & `exit_site`
   */
  covG_ptr reach_allele_end(Marker const& m);

  void wire(covG_ptr const& target); /** Build 1 or 2 edges depending on whether
                                        `cur_Node` contains sequence. */

  /**
   * Makes the map of marker targets used during mapping
   */
  void map_targets();
  void make_site_entry_target(marker_type prev_t, Marker prev_m, Marker cur_m);
  void make_site_exit_target(marker_type prev_t, Marker prev_m, Marker cur_m,
                             AlleleId cur_allele_ID);
  void make_allele_end_target(marker_type prev_t, Marker prev_m, Marker cur_m,
                              AlleleId cur_allele_ID);
  /**
   * Site exit points can point to different variant markers (site start & end)
   * Here we test for prior presence of the allele marker in the `target_map`
   * and insert or update accordingly
   */
  void add_exit_target(Marker cur_m, targeted_marker const new_t_m);

  /*
   * variables & data structures
   */
  marker_vec linear_prg;
  std::unordered_map<Marker, int> end_positions;

  covG_ptr backWire;  // Pointer to the most recent node needing edge building
  covG_ptr cur_Node;

  // For assigning position to nodes
  std::size_t cur_pos{0};
  bool first_allele{false};

  VariantLocus cur_Locus;  // For building parental map

  marker_to_node bubble_starts;
  marker_to_node bubble_ends;
};

#endif  // COV_GRAPH_HPP
