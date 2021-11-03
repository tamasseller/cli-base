/*******************************************************************************
 *
 * Copyright (c) 2021 Tamás Seller. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************/

#include "Levenshtein.h"

#include <string>
#include <memory>
#include <algorithm>

size_t levenshteinDistance(std::string_view a, std::string_view b)
{
	std::unique_ptr<size_t[]> prev(new size_t[b.length() + 1]), next(new size_t[b.length() + 1]);

	for(auto j = 0u; j <= b.length(); j++)
	{
		prev[j] = j;
	}

	for(auto i = 0u; i < a.length(); i++)
	{
		next[0] = i;

		for(auto j = 0u; j < b.length(); j++)
		{
			next[j + 1] = (a[i] == b[j]) ? prev[j] : 1 + std::min({prev[j], prev[j + 1], next[j]});
		}

		std::swap(prev, next);
	}

	return prev[b.length()];
}
