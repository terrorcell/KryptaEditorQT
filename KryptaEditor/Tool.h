#ifndef TOOL_H
#define TOOL_H

#include <Graphics\Primitives.h>
#include <QObject>
#include <memory>

enum class ToolType
{
	POINTER,
	PAINT,
	WAYPOINT
};

struct PointerData
{
};

struct PaintData
{
	unsigned size = 1;
	Asset<kry::Graphics::Texture>* asset = nullptr;
};

struct Object;
struct CopyObjectData : PaintData
{
	Object* object = nullptr;
};

struct Waypoint
{
	kry::Util::Vector2f position;
	Object* owner;
};

struct WaypointData
{
	std::vector<Waypoint> waypoints;
	Object* object;
	bool looping = false;
};

template <typename DataType = PointerData>
class Tool
{
	public:
		~Tool();
		void setData(const DataType& data);
		DataType& getData();
		ToolType getType();

		static std::shared_ptr<Tool> getTool();
		static void switchTool(ToolType type);

	private:
		Tool(ToolType type);
		DataType* data;
		ToolType type;

		static std::shared_ptr<Tool> single;
};


template <typename DataType>
std::shared_ptr<Tool<DataType> > Tool<DataType>::single(new Tool<DataType>(ToolType::POINTER));

template <typename DataType>
Tool<DataType>::~Tool()
{
	delete data;
}

template <typename DataType>
void Tool<DataType>::setData(const DataType &data)
{
	*this->data = data;
}

template <typename DataType>
DataType& Tool<DataType>::getData()
{
	return *data;
}

template <typename DataType>
ToolType Tool<DataType>::getType()
{
	return type;
}

template <typename DataType>
std::shared_ptr<Tool<DataType> > Tool<DataType>::getTool()
{
	return single;
}

template <typename DataType>
void Tool<DataType>::switchTool(ToolType type)
{
	single.reset(new Tool(type));
}

template <typename DataType>
Tool<DataType>::Tool(ToolType type) : type(type), data(new DataType)
{
}

#endif // TOOL_H
