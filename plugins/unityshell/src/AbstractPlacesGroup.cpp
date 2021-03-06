/*
 * Copyright (C) 2012 Canonical Ltd
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
 * Authored by: Andrea Azzarone <azzaronea@gmail.com>
 */


#include "AbstractPlacesGroup.h"

namespace unity
{
namespace dash
{

NUX_IMPLEMENT_OBJECT_TYPE(AbstractPlacesGroup);

AbstractPlacesGroup::AbstractPlacesGroup()
  : nux::View(NUX_TRACKER_LOCATION)
  , draw_separator(false)
{
}

void AbstractPlacesGroup::Draw(nux::GraphicsEngine&, bool)
{
}

void AbstractPlacesGroup::DrawContent(nux::GraphicsEngine&, bool)
{
}

} // namespace dash
} // namespace unity
