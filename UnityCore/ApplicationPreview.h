// -*- Mode: C++; indent-tabs-mode: nil; tab-width: 2 -*-
/*
 * Copyright (C) 2011 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Neil Jagdish Patel <neil.patel@canonical.com>
 */

#ifndef UNITY_APPLICATION_PREVIEW_H
#define UNITY_APPLICATION_PREVIEW_H

#include <memory>

#include <sigc++/trackable.h>

#include "Preview.h"

namespace unity
{
namespace dash
{

class ApplicationPreview : public Preview
{
public:
  typedef std::shared_ptr<ApplicationPreview> Ptr;
  
  ApplicationPreview(Preview::Properties& properties);

  std::string name;
  std::string version;
  std::string size;
  std::string license;
  std::string last_updated;
  float rating;
  uint n_ratings;
  std::string description;
  std::string icon_hint;
  std::string screenshot_icon_hint;
  std::string primary_action_name;
  std::string primary_action_icon_hint;
  std::string primary_action_uri;
};

}
}

#endif
