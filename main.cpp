#include <lua.hpp>
#include <QString>
#include <QDebug>

int functionExposedToLua(lua_State* luaVM)
{
	Q_UNUSED(luaVM)
	// Do stuff with Lua's registers
	return 0;
}

class luaQStringReader
{
private:
	QByteArray buffer;
public:
	const char* operator()(lua_State *VM,
				     void *string,
				     size_t *size)
	{
		Q_UNUSED(VM)
		QString *input{(QString*)string};
		if (!input || input->isEmpty())
		{
			*size = 0;
			return 0;
		}


		buffer = input->toLatin1();
		*size = buffer.size();

		return buffer.data(); // Totally not thread-safe.
	}
};

static QByteArray buffer;

int luaPrintBuffer(lua_State* VM)
{
	auto nargs = lua_gettop(VM) ;
	for (auto i = 1 ; i <=  nargs ; ++i)
	{
		if (lua_isstring(VM, i))
		{
			size_t stringlength;
			auto string = lua_tolstring(VM,i,&stringlength);
			buffer.append(string, stringlength);
		}
		else if (lua_isnil(VM,i))
		{
			buffer.append("\tnil");
		}
	}
	buffer.append("\n");
	return 0;
}

int main()
{
	lua_State *luaVM{lua_open()};
	luaopen_base(luaVM);
//	luaopen_table(luaVM);
//	luaopen_io(luaVM);
//	luaopen_string(luaVM);
//	luaopen_math(luaVM);

	if (!luaVM) return 1;

	lua_register(luaVM, "myFunction", functionExposedToLua);

	luaQStringReader reader;

//	if (lua_load(luaVM, reader, (void*) &script, "Testscript")) return 1;

	QString luaScript{"a = 5\nprint(a, b)"};
	QByteArray scriptString{luaScript.toLatin1()};

	lua_register(luaVM, "print", luaPrintBuffer);

	if (luaL_dostring(luaVM, scriptString.data()))
		return 3;

	lua_close(luaVM);

	qDebug() << "Content of buffer:" << buffer;
	return 0;
}
