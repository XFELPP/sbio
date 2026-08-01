/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SBIO_EXPORT_MACRO_HH
#define SBIO_EXPORT_MACRO_HH

#ifdef _WIN32

#ifdef SBIO_BUILD_API
#define SBIO_API __declspec(dllexport)
#else
#define SBIO_API __declspec(dllimport)
#endif // SBIO_BUILD_API

#else

#define SBIO_API

#endif // _WIN32

#endif // SBIO_EXPORT_MACRO_HH
