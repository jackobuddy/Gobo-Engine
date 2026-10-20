#include "AssetConfig.h"
#include "../Sprites/Animation.h"
#include "../Sprites/Tilemap.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <set>
#include <stdexcept>
#include <string_view>


namespace AssetConfig
{
	namespace
	{
		constexpr int MaxRows = 1024;

		std::string Lower(std::string value)
		{
			for (char& c : value)
				if (c >= 'A' && c <= 'Z')	c += 'a' - 'A';
			return value; 
		}

		void Keys(const Json& j, std::initializer_list<std::string_view> allowed)
		{
			if (not j.is_object())
				throw std::runtime_error("Expected a JSON object");
			for (const auto& [key, value] : j.items())
				if(std::find(allowed.begin(), allowed.end(), key) == allowed.end())
					throw std::runtime_error("Unknown field: " + key); 
		}

		int Integer(const Json& j, const char* key, int minimum = 1,
			int maximum = std::numeric_limits<int>::max())
		{
			const auto& value = j.at(key);
			if (!value.is_number_integer())
				throw std::runtime_error(std::string(key) + " must be an integer");

			// Compare before narrowing; JSON also represents unsigned integers.
			if (value < minimum || value > maximum)
				throw std::runtime_error(std::string(key) + " is out of range");
			return value.get<int>();
		}

		int ClampDimension(int requested, int available, const char* field)
		{
			if (available <= 0)
				throw std::runtime_error(std::string(field) +
					" cannot be generated from an image with no size.");

			return std::min(requested, available);
		}

		int ClampCount(int requested, int imageExtent, int cellSize,
			const char* field)
		{
			const int maximumCount = imageExtent / cellSize;

			if (maximumCount <= 0)
				throw std::runtime_error(std::string(field) +
					" cannot fit inside the image.");

			return std::min(requested, maximumCount);
		}

		float FPS(const Json& j)
		{
			const auto& value = j.at("defaultFPS");
			if (!value.is_number())
				throw std::runtime_error("defaultFPS must be a number");

			const double fps = value.get<double>();
			if (!std::isfinite(fps) || fps < 0 ||
				fps > std::numeric_limits<float>::max() ||
				(fps > 0 && static_cast<float>(fps) == 0))
				throw std::runtime_error(
					"defaultFPS must be finite and non-negative");

			return static_cast<float>(fps);
		}

		void Version(const Json& j)
		{
			if (j.contains("schemaVersion"))
				Integer(j, "schemaVersion", 1, 1);
		}

		void ValidateDefaults(const std::string& type, const Json& j)
		{
			if (type == "sprite")
				Keys(j, { "width", "height", "sourceX", "sourceY",
					"pivotX", "pivotY" });
			else if (type == "animation")
				Keys(j, { "width", "height", "originX", "originY",
					"pivotX", "pivotY", "defaultFPS",
					"defaultFrameCount", "rowCount" });
			else
				Keys(j, { "width", "height", "originX", "originY",
					"atlasColumns", "atlasRows" });

			Integer(j, "width");
			Integer(j, "height");

			if (type == "sprite")
			{
				Integer(j, "sourceX", 0);
				Integer(j, "sourceY", 0);
				Integer(j, "pivotX", 0);
				Integer(j, "pivotY", 0);
			}
			else if (type == "animation")
			{
				Integer(j, "originX", 0);
				Integer(j, "originY", 0);
				FPS(j);
				Integer(j, "defaultFrameCount");
				Integer(j, "rowCount", 1, MaxRows);
				Integer(j, "pivotX", 0);
				Integer(j, "pivotY", 0);
			}
			else
			{
				Integer(j, "originX", 0);
				Integer(j, "originY", 0);
				const auto columns = Integer(j, "atlasColumns");
				const auto rows = Integer(j, "atlasRows");
				if (static_cast<std::int64_t>(columns) * rows >
					std::numeric_limits<int>::max())
					throw std::runtime_error("Tile count exceeds int range");
			}
		}

		const Json& ForType(const std::string& type, const Defaults& d)
		{
			if (type == "sprite") return d.sprite;
			if (type == "animation") return d.animation;
			if (type == "tilemap") return d.tilemap;
			throw std::runtime_error("Unknown asset type: " + type);
		}

		Json Read(const std::filesystem::path& path)
		{
			std::ifstream input(path, std::ios::binary);
			if (!input) throw std::runtime_error("Cannot open config");
			// parse consumes the entire stream, including trailing-data checks.
			Json result = Json::parse(input);
			if (input.bad()) throw std::runtime_error("Cannot read config");
			return result;
		}
	}

	std::string PathText(const std::filesystem::path& path)
	{
		const auto text = path.generic_u8string();
		return std::string(text.begin(), text.end());
	}

	Defaults EngineDefaults()
	{
		return {
			{{"width", 32}, {"height", 32},
			 {"sourceX", 0}, {"sourceY", 0},
			 {"pivotX", 0}, {"pivotY", 0}},
			{{"width", 32}, {"height", 32},
			 {"originX", 0}, {"originY", 0},
			 {"pivotX", 0}, {"pivotY", 0},
			 {"defaultFPS", 10},
			 {"defaultFrameCount", 1}, {"rowCount", 1}},
			{{"width", 32}, {"height", 32},
			 {"originX", 0}, {"originY", 0},
			 {"atlasColumns", 1}, {"atlasRows", 1}}
		};
	}

	Defaults ParseProject(const Json& document)
	{
		Keys(document, { "schemaVersion", "sprite", "animation", "tilemap" });
		Version(document);
		Defaults result = EngineDefaults();
		for (const std::string type : {"sprite", "animation", "tilemap"})
		{
			Json* destination = type == "sprite" ? &result.sprite :
				type == "animation" ? &result.animation : &result.tilemap;
			if (document.contains(type))
			{
				if (!document.at(type).is_object())
					throw std::runtime_error(type + " must be an object");
				destination->update(document.at(type));
			}
			try { ValidateDefaults(type, *destination); }
			catch (const std::exception& error)
			{
				throw std::runtime_error(type + "." + error.what());
			}
		}
		return result;
	}

	Defaults LoadProject(const std::filesystem::path& path, Errors& errors)
	{
		try
		{
			if (std::filesystem::exists(path)) return ParseProject(Read(path));
		}
		catch (const std::exception& error)
		{
			errors.push_back(PathText(path) + ": " + error.what() +
				"; using Engine defaults");
		}
		return EngineDefaults();
	}

	std::string DetectType(const std::filesystem::path& image)
	{
		const auto name = Lower(PathText(image.filename()));
		for (const std::string prefix : {"spr", "s", "sprite"})
			if (name.starts_with(prefix + "_") || name.starts_with(prefix + "-"))
				return "sprite";
		for (const std::string prefix : {"anim", "a", "animation"})
			if (name.starts_with(prefix + "_") || name.starts_with(prefix + "-"))
				return "animation";
		for (const std::string prefix : {"tile", "t", "tm", "tilemap"})
			if (name.starts_with(prefix + "_") || name.starts_with(prefix + "-"))
				return "tilemap";
		return {};
	}

	OrderedJson MakeConfig(const std::string& type, const Defaults& defaults, int imageWidth, 
		int imageHeight)
	{
		const Json& values = ForType(type, defaults);
		ValidateDefaults(type, values);

		OrderedJson result = OrderedJson::object();
		result["schemaVersion"] = 1;
		result["type"] = type;

		if (type == "sprite")
		{
			const int sourceX = Integer(values, "sourceX", 0);
			const int sourceY = Integer(values, "sourceY", 0);
			const int pivotX = Integer(values, "pivotX", 0);
			const int pivotY = Integer(values, "pivotY", 0);

			if (sourceX >= imageWidth || sourceY >= imageHeight)
				throw std::runtime_error(
					"Sprite source must be inside the image.");

			result["width"] = ClampDimension(
				Integer(values, "width"), imageWidth - sourceX, "width");
			result["height"] = ClampDimension(
				Integer(values, "height"), imageHeight - sourceY, "height");
			result["sourceX"] = sourceX;
			result["sourceY"] = sourceY;
			result["pivotX"] = pivotX;
			result["pivotY"] = pivotY;
		}
		else if (type == "animation")
		{
			const int originX = Integer(values, "originX", 0);
			const int originY = Integer(values, "originY", 0);
			if (originX >= imageWidth || originY >= imageHeight)
				throw std::runtime_error(
					"Animation origin must be inside the image.");

			const int availableWidth = imageWidth - originX;
			const int availableHeight = imageHeight - originY;
			const int width = ClampDimension(
				Integer(values, "width"), availableWidth, "width");
			const int height = ClampDimension(
				Integer(values, "height"), availableHeight, "height");
			const int pivotX = Integer(values, "pivotX", 0);
			const int pivotY = Integer(values, "pivotY", 0);
			const int defaultFrames = ClampCount(
				Integer(values, "defaultFrameCount"),
				availableWidth, width, "defaultFrameCount");
			const int rowCount = ClampCount(
				Integer(values, "rowCount", 1, MaxRows),
				availableHeight, height, "rowCount");

			result["width"] = width;
			result["height"] = height;
			result["originX"] = originX;
			result["originY"] = originY;
			result["pivotX"] = pivotX;
			result["pivotY"] = pivotY;
			result["defaultFPS"] = values.at("defaultFPS");
			result["defaultFrameCount"] = defaultFrames;
			result["rowCount"] = rowCount;
			result["rows"] = OrderedJson::array();

			for (int i = 0; i < rowCount; ++i)
			{
				OrderedJson row = OrderedJson::object();
				row["id"] = "row-" + std::to_string(i);
				row["frameCount"] = defaultFrames;
				result["rows"].push_back(std::move(row));
			}
		}
		else if (type == "tilemap")
		{
			const int originX = Integer(values, "originX", 0);
			const int originY = Integer(values, "originY", 0);
			if (originX >= imageWidth || originY >= imageHeight)
				throw std::runtime_error(
					"Tilemap origin must be inside the image.");

			const int availableWidth = imageWidth - originX;
			const int availableHeight = imageHeight - originY;
			const int width = ClampDimension(
				Integer(values, "width"), availableWidth, "width");
			const int height = ClampDimension(
				Integer(values, "height"), availableHeight, "height");

			result["width"] = width;
			result["height"] = height;
			result["originX"] = originX;
			result["originY"] = originY;
			result["atlasColumns"] = ClampCount(
				Integer(values, "atlasColumns"),
				availableWidth, width, "atlasColumns");
			result["atlasRows"] = ClampCount(
				Integer(values, "atlasRows"),
				availableHeight, height, "atlasRows");
		}

		return result;
	}

	SpriteRef ParseSprite(const Json& document, const std::string& id,
		int imageWidth, int imageHeight, const Defaults& defaults)
	{
		if (!document.is_object() || !document.contains("type") ||
			!document.at("type").is_string())
			throw std::runtime_error(
				"type must be sprite, animation or tilemap");

		const std::string type =
			Lower(document.at("type").get<std::string>());
		Json values = ForType(type, defaults);

		if (type == "sprite")
			Keys(document, { "schemaVersion", "type", "width",
				"height", "sourceX", "sourceY", "pivotX", "pivotY" });
		else if (type == "animation")
			Keys(document, { "schemaVersion", "type", "width",
				"height", "originX", "originY", "pivotX", "pivotY",
				"defaultFPS", "defaultFrameCount", "rowCount", "rows" });
		else
			Keys(document, { "schemaVersion", "type", "width",
				"height", "originX", "originY",
				"atlasColumns", "atlasRows" });

		Version(document);
		values.update(document);
		values.erase("schemaVersion");
		values.erase("type");
		values.erase("rows");

		if (type == "animation" && document.contains("rows"))
		{
			const auto& rows = document.at("rows");
			if (!rows.is_array() || rows.empty() ||
				rows.size() > MaxRows)
				throw std::runtime_error(
					"rows must contain 1 to 1024 entries");
			if (!document.contains("rowCount"))
				values["rowCount"] = static_cast<int>(rows.size());
			if (rows.size() > static_cast<std::size_t>(
				Integer(values, "rowCount", 1, MaxRows)))
				throw std::runtime_error(
					"rows contains more entries than rowCount");
		}

		ValidateDefaults(type, values);
		const int width = Integer(values, "width");
		const int height = Integer(values, "height");
		auto fits = [&](std::int64_t right, std::int64_t bottom)
			{
				if (imageWidth <= 0 || imageHeight <= 0 ||
					right > imageWidth || bottom > imageHeight)
					throw std::runtime_error(
						"Sprite regions exceed image dimensions");
			};

		if (type == "sprite")
		{
			const int sourceX = Integer(values, "sourceX", 0);
			const int sourceY = Integer(values, "sourceY", 0);
			const int pivotX = Integer(values, "pivotX", 0);
			const int pivotY = Integer(values, "pivotY", 0);
			fits(static_cast<std::int64_t>(sourceX) + width,
				static_cast<std::int64_t>(sourceY) + height);
			return Sprite(id, width, height,
				sourceX, sourceY, pivotX, pivotY).Clone();
		}

		const int originX = Integer(values, "originX", 0);
		const int originY = Integer(values, "originY", 0);

		if (type == "tilemap")
		{
			const int columns = Integer(values, "atlasColumns");
			const int rows = Integer(values, "atlasRows");
			fits(static_cast<std::int64_t>(originX) +
				static_cast<std::int64_t>(columns) * width,
				static_cast<std::int64_t>(originY) +
				static_cast<std::int64_t>(rows) * height);
			return Tilemap(id, width, height, columns, rows,
				originX, originY).Clone();
		}

		const int count = Integer(values, "rowCount", 1, MaxRows);
		const int frames = Integer(values, "defaultFrameCount");
		std::vector<Animation::Row> rows;
		std::set<std::string> names;

		for (int i = 0; i < count; ++i)
		{
			std::string name = "row-" + std::to_string(i);
			int rowFrames = frames;
			if (document.contains("rows") &&
				i < static_cast<int>(document.at("rows").size()))
			{
				const auto& row = document.at("rows").at(i);
				Keys(row, { "id", "frameCount" });
				if (row.contains("id"))
					name = row.at("id").get<std::string>();
				if (row.contains("frameCount"))
					rowFrames = Integer(row, "frameCount");
			}
			if (name.empty() || !names.insert(Lower(name)).second)
				throw std::runtime_error(
					"Empty or duplicate row ID: " + name);

			fits(static_cast<std::int64_t>(originX) +
				static_cast<std::int64_t>(rowFrames) * width,
				static_cast<std::int64_t>(originY) +
				static_cast<std::int64_t>(count) * height);
			rows.push_back({ name, rowFrames });
		}

		const int pivotX = Integer(values, "pivotX", 0);
		const int pivotY = Integer(values, "pivotY", 0);
		Animation animation(id, width, height, FPS(values),
			frames, count, pivotX, pivotY, originX, originY);

		// Avoid collisions with Animation's initial row-N names.
		std::string temporary = "__gobo_config_";
		while (std::any_of(rows.begin(), rows.end(),
			[&](const auto& row)
			{
				return Lower(row.id).starts_with(temporary);
			}))
			temporary += "_";

		for (int i = 0; i < count; ++i)
			animation.SetRow(
				i, temporary + std::to_string(i), rows[i].frameCount);
		for (int i = 0; i < count; ++i)
			animation.SetRow(i, rows[i].id, rows[i].frameCount);

		return animation.Clone();
	}

	SpriteRef LoadSprite(const std::filesystem::path& image, const std::string& id, int imageWidth, 
		int imageHeight, const Defaults& defaults, Errors& errors)
	{
		auto config = image;
		config.replace_extension(".config");
		try
		{
			if (std::filesystem::exists(config))
				return ParseSprite(Read(config), id, imageWidth, imageHeight, defaults);
		}
		catch (const std::exception& error)
		{
			errors.push_back(PathText(config) + ": " + error.what() + "; using full-image static Sprite");
		}
		return Sprite(id, imageWidth, imageHeight).Clone();
	}
}