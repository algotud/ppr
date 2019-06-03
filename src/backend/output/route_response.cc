#include "footrouting/backend/output/route_response.h"
#include "footrouting/output/json.h"
#include "footrouting/routing/route_steps.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace footrouting;
using namespace footrouting::routing;
using namespace footrouting::output;

namespace footrouting::backend::output {

std::string edge_type_str(edge_type const type) {
  switch (type) {
    case edge_type::CONNECTION: return "connection";
    case edge_type::STREET: return "street";
    case edge_type::FOOTWAY: return "footway";
    case edge_type::CROSSING: return "crossing";
    case edge_type::ELEVATOR: return "elevator";
  }
  throw std::runtime_error{"invalid edge type"};
}

std::string side_type_str(side_type const side) {
  switch (side) {
    case side_type::CENTER: return "center";
    case side_type::LEFT: return "left";
    case side_type::RIGHT: return "right";
  }
  throw std::runtime_error{"invalid side type"};
}

std::string crossing_type_str(crossing_type::crossing_type const crossing) {
  switch (crossing) {
    case crossing_type::NONE: return "none";
    case crossing_type::GENERATED: return "generated";
    case crossing_type::UNMARKED: return "unmarked";
    case crossing_type::MARKED: return "marked";
    case crossing_type::SIGNALS: return "signals";
    case crossing_type::ISLAND: return "island";
  }
  throw std::runtime_error{"invalid crossing type"};
}

std::string street_type_str(street_type const street) {
  switch (street) {
    case street_type::NONE: return "none";
    case street_type::PEDESTRIAN: return "pedestrian";
    case street_type::LIVING: return "living";
    case street_type::RESIDENTIAL: return "residential";
    case street_type::SERVICE: return "service";
    case street_type::UNCLASSIFIED: return "unclassified";
    case street_type::TERTIARY: return "tertiary";
    case street_type::SECONDARY: return "secondary";
    case street_type::PRIMARY: return "primary";
    case street_type::TRACK: return "track";
    case street_type::FOOTWAY: return "footway";
    case street_type::PATH: return "path";
    case street_type::CYCLEWAY: return "cycleway";
    case street_type::BRIDLEWAY: return "bridleway";
    case street_type::STAIRS: return "stairs";
    case street_type::ESCALATOR: return "escalator";
    case street_type::MOVING_WALKWAY: return "moving_walkway";
    case street_type::RAIL: return "rail";
    case street_type::TRAM: return "tram";
  }
  throw std::runtime_error{"invalid street type"};
}

std::string crossing_suffix(crossing_type::crossing_type const crossing) {
  switch (crossing) {
    case crossing_type::MARKED: return " (Zebrastreifen)";
    case crossing_type::SIGNALS: return " (Ampel)";
    case crossing_type::ISLAND: return " (Verkehrsinsel)";
    default: return "";
  }
}

std::string step_text(route_step const& step) {
  std::string text = step.street_name_;

  switch (step.step_type_) {
    case step_type::STREET:
      if (text.empty()) {
        text = "Straße";
      }
      break;
    case step_type::FOOTWAY:
      if (text.empty()) {
        text = "Fußweg";
      }
      if (step.street_type_ == street_type::STAIRS) {
        text += " (Treppe)";
      } else if (step.street_type_ == street_type::ESCALATOR) {
        text += " (Rolltreppe)";
      } else if (step.street_type_ == street_type::MOVING_WALKWAY) {
        text += " (Fahrsteig)";
      }
      break;
    case step_type::CROSSING:
      if (text.empty()) {
        if (step.street_type_ == street_type::RAIL) {
          text = "Gleise (Eisenbahn)";
        } else if (step.street_type_ == street_type::TRAM) {
          text = "Gleise (Straßenbahn)";
        } else {
          text = "Straße";
        }
      }
      text += " überqueren";
      text += crossing_suffix(step.crossing_);
      break;
    case step_type::ELEVATOR: text = "Aufzug"; break;
    case step_type::INVALID: text = "-"; break;
  }

  return text;
}

template <typename Writer>
void write_edge(Writer& writer, route::edge const& e) {
  writer.StartObject();

  writer.String("edge_type");
  writer.String(edge_type_str(e.edge_type_).c_str());

  writer.String("street_type");
  writer.String(street_type_str(e.street_type_).c_str());

  writer.String("crossing_type");
  writer.String(crossing_type_str(e.crossing_type_).c_str());

  writer.String("side");
  writer.String(side_type_str(e.side_).c_str());

  writer.String("osm_way_id");
  writer.Int64(e.osm_way_id_);

  writer.String("name");
  writer.String(e.name_.c_str());

  writer.String("marked_crossing_detour");
  writer.Int(e.marked_crossing_detour_);

  writer.String("distance");
  writer.Double(e.distance_);

  writer.String("duration");
  writer.Double(e.duration_);

  writer.String("accessibility");
  writer.Double(e.accessibility_);

  writer.String("path");
  writer.StartArray();
  std::for_each(begin(e.path_), end(e.path_),
                [&writer](location const& loc) { write_lon_lat(writer, loc); });
  writer.EndArray();  // path

  writer.String("elevation_up");
  writer.Int(e.elevation_up_);

  writer.String("elevation_down");
  writer.Int(e.elevation_down_);

  writer.String("incline_up");
  writer.Bool(e.incline_up_);

  writer.String("handrail");
  write_tri_state(writer, e.handrail_);

  writer.String("duration_penalty");
  writer.Double(e.duration_penalty_);

  writer.String("accessibility_penalty");
  writer.Double(e.accessibility_penalty_);

  writer.EndObject();
}

template <typename Writer>
void write_step(Writer& writer, route_step const& step, int index) {
  writer.StartObject();

  writer.String("index");
  writer.Int(index);

  writer.String("distance");
  writer.Double(step.distance_);

  writer.String("time");
  writer.Double(step.time_);

  writer.String("accessibility");
  writer.Double(step.accessibility_);

  writer.String("text");
  writer.String(step_text(step).c_str());

  writer.String("elevation_up");
  writer.Int(step.elevation_up_);

  writer.String("elevation_down");
  writer.Int(step.elevation_down_);

  writer.String("incline_up");
  writer.Bool(step.incline_up_);

  writer.String("handrail");
  write_tri_state(writer, step.handrail_);

  writer.String("duration_penalty");
  writer.Double(step.duration_penalty_);

  writer.String("accessibility_penalty");
  writer.Double(step.accessibility_penalty_);

  writer.EndObject();
}

template <typename Writer>
void write_route(Writer& writer, route const& r, bool full) {
  auto const steps = get_route_steps(r);
  auto const write_loc = [&writer](location const& loc) {
    write_lon_lat(writer, loc);
  };

  writer.StartObject();

  if (full) {
    writer.String("distance");
    writer.Double(r.distance_);
    writer.String("duration");
    writer.Double(r.duration_);
    writer.String("duration_exact");
    writer.Double(r.orig_duration_);
    writer.String("duration_division");
    writer.Double(r.disc_duration_);
    writer.String("accessibility");
    writer.Double(r.accessibility_);
    writer.String("accessibility_exact");
    writer.Double(r.orig_accessibility_);
    writer.String("accessibility_division");
    writer.Double(r.disc_accessibility_);
    writer.String("elevation_up");
    writer.Int(r.elevation_up_);
    writer.String("elevation_down");
    writer.Int(r.elevation_down_);
    writer.String("penalized_duration");
    writer.Double(r.penalized_duration_);
    writer.String("penalized_accessibility");
    writer.Double(r.penalized_accessibility_);
    writer.String("penalized_duration");
    writer.Double(r.penalized_duration_);
    writer.String("penalized_accessibility");
    writer.Double(r.penalized_accessibility_);
  }

  writer.String("coordinates");
  writer.StartArray();
  for (auto const& loc : get_route_path(r)) {
    write_loc(loc);
  }
  writer.EndArray();  // coordinates

  if (full) {
    writer.String("steps");
    writer.StartArray();
    {
      int index = -1;
      for (auto const& step : steps) {
        index += static_cast<int>(step.path_.size()) - 1;
        write_step(writer, step, index);
      }
    }
    writer.EndArray();  // steps

    writer.String("edge_count");
    writer.Uint64(r.edges_.size());

    writer.String("edges");
    writer.StartArray();
    for (auto const& e : r.edges_) {
      write_edge(writer, e);
    }
    writer.EndArray();  // edges
  }

  writer.EndObject();  // route
}

template <typename Writer>
void write_dijkstra_statistics(Writer& writer, dijkstra_statistics const& s) {
  writer.StartObject();
  writer.String("labels_created");
  writer.Uint64(s.labels_created_);
  writer.String("labels_popped");
  writer.Uint64(s.labels_popped_);
  writer.String("start_labels");
  writer.Uint64(s.start_labels_);
  writer.String("additional_nodes");
  writer.Uint64(s.additional_nodes_);
  writer.String("additional_edges");
  writer.Uint64(s.additional_edges_);
  writer.String("additional_areas");
  writer.Uint64(s.additional_areas_);
  writer.String("goals");
  writer.Uint64(s.goals_);
  writer.String("goals_reached");
  writer.Uint64(s.goals_reached_);
  writer.String("d_starts");
  writer.Double(s.d_starts_);
  writer.String("d_goals");
  writer.Double(s.d_goals_);
  writer.String("d_area_edges");
  writer.Double(s.d_area_edges_);
  writer.String("d_search");
  writer.Double(s.d_search_);
  writer.String("d_labels_to_route");
  writer.Double(s.d_labels_to_route_);
  writer.String("d_total");
  writer.Double(s.d_total_);
  writer.String("max_label_quit");
  writer.Bool(s.max_label_quit_);
  writer.EndObject();
}

template <typename Writer>
void write_routing_statistics(Writer& writer, routing_statistics const& s) {
  writer.StartObject();
  writer.String("attempts");
  writer.Int(s.attempts_);
  writer.String("start_pts_extended");
  writer.Int(s.start_pts_extended_);
  writer.String("destination_pts_extended");
  writer.Int(s.destination_pts_extended_);
  writer.String("d_start_pts");
  writer.Double(s.d_start_pts_);
  writer.String("d_start_pts_extended");
  writer.Double(s.d_start_pts_extended_);
  writer.String("d_destination_pts");
  writer.Double(s.d_destination_pts_);
  writer.String("d_destination_pts_extended");
  writer.Double(s.d_destination_pts_extended_);
  writer.String("d_postprocessing");
  writer.Double(s.d_postprocessing_);
  writer.String("d_total");
  writer.Double(s.d_total_);

  writer.String("dijkstra");
  writer.StartArray();
  for (auto const& ds : s.dijkstra_statistics_) {
    write_dijkstra_statistics(writer, ds);
  }
  writer.EndArray();

  writer.EndObject();
}

std::string routes_to_route_response(search_result const& result, bool full) {
  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

  writer.StartObject();
  writer.String("error");
  if (result.routes_.empty()) {
    writer.String("No route found.");
    writer.EndObject();
    return sb.GetString();
  }
  writer.String("");

  writer.String("routes");
  writer.StartArray();

  for (auto const& rs : result.routes_) {
    for (auto const& r : rs) {
      write_route(writer, r, full);
    }
  }

  writer.EndArray();  // routes

  writer.String("statistics");
  write_routing_statistics(writer, result.stats_);

  writer.EndObject();

  return sb.GetString();
}

}  // namespace footrouting::backend::output
