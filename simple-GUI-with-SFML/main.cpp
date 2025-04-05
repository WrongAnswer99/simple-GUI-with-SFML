#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <any>
#include <vector>
#include <stdarg.h>
#include <unordered_map>
#include <queue>
using namespace std;
namespace game {
	namespace Type {
		//statu = {"name1", value1, "name2", value2, ...};
		//statu["name"] = value;
		//T val = statu["name"].cast<T>();
		//statu["name"]["subname"];当"name"键也为Statu类型时可直接使用
		//自动将const char*转换为string,自动将const wchar_t*转换为wstring
		class Statu {
		protected:
			//方便转化的any类型
			//自动将const char*转换为string,自动将const wchar_t*转换为wstring
			class castable_any {
				friend class Statu;
				friend class Event;
				// private:
			private:
				any _data;
			public:
				castable_any() {}
				template <typename T>
				castable_any(T _val) {
					if constexpr (is_same_v<const char*, decay_t<T>>) {
						_data = string(_val);
					}
					else if constexpr (is_same_v<const wchar_t*, decay_t<T>>) {
						_data = wstring(_val);
					}
					else _data = _val;
				}
				//返回:转化为指定类型的值
				template <typename T>
				T& cast() {
					return any_cast<T&>(_data);
				}
				castable_any& operator[](string _name) {
					return any_cast<Statu&>(_data)[_name];
				}
			};
			unordered_map<string, castable_any>data;
		public:
			Statu() {}
			Statu(initializer_list<castable_any> _list) {
				for (auto i = _list.begin(); i != _list.end(); i++) {
					string _name = any_cast<string>(i->_data);
					i++;
					data[_name] = *i;
				}
			}
			//可变参数递归调用终止条件:处理无参数或参数满足递归结束的情况
			bool contain() {
				return true;
			}
			//可变参数递归调用异常:处理参数个数为奇数的非法情况
			template <typename T>
			bool contain(T) {
				throw runtime_error("Unpaired argument");
			}
			//用法:statu.contain("name1", value1, "name2", value2, ...);
			//需要保证参数个数为偶数(因为是键值对),否则抛出runtime_error
			//返回:是否包含键值对
			template<typename T1, typename T2, typename ...Args>
			bool contain(T1 _name, T2 _val, Args ...args) {
				try {
					if constexpr (is_same_v<const char*, decay_t<T2>>) {
						if (any_cast<string>(data[static_cast<string>(_name)]._data) != string(_val))
							return false;
					}
					else if constexpr (is_same_v<const wchar_t*, decay_t<T2>>) {
						if (any_cast<wstring>(data[static_cast<string>(_name)]._data) != wstring(_val))
							return false;
					}
					else {
						if (any_cast<T2>(data[static_cast<string>(_name)]._data) != _val)
							return false;
					}
				}
				catch (bad_any_cast) {
					return false;
				}
				return contain(args...);
			}
			//可变参数递归调用终止条件:处理无参数或参数满足递归结束的情况
			bool count() {
				return true;
			}
			//用法:statu.count("name1", "name2", ...);
			//返回:是否包含键
			template<typename T, typename ...Args>
			bool count(T _name, Args ...args) {
				if (!data.count(_name))
					return false;
				else return count(args...);
			}
			//返回:指定键的值
			castable_any& operator[](string _name) {
				return data[_name];
			}
			//批量添加键值对
			void operator+=(Statu _statu) {
				for (auto& elem : _statu.data) {
					data[elem.first] = elem.second;
				}
			}
			//批量删除指定的键
			void operator-=(initializer_list<string> _name_list) {
				for (auto& name_elem : _name_list) {
					data.erase(name_elem);
				}
			}
			//返回:是否为空
			bool empty() const {
				return data.empty();
			}
			//返回:键值对个数
			size_t size() const {
				return data.size();
			}
		};
		//event(eventEnum, {"name1", value1, "name2", value2});
		class Event :public Statu {
		public:
			//事件编号
			enum eventEnum {
				//空
				null,
				guiButtonpressed
			};
			eventEnum eventId = null;
			Event() {}
			Event(eventEnum _eventId, initializer_list<castable_any> _list) {
				eventId = _eventId;
				for (auto i = _list.begin(); i != _list.end(); i++) {
					string _name = any_cast<string>(i->_data);
					i++;
					data[_name] = *i;
				}
			}
		};
	}
	sf::RenderWindow window;
	//快速绘制简单图形
	namespace Draw {
		static void rect(sf::RenderTarget& drawTarget, sf::Vector2f _point1, sf::Vector2f _point2, sf::Color _fillcolor, sf::Color _linecolor = sf::Color(0, 0, 0, 0), float _thickness = 0) {
			sf::RectangleShape _rectShape;
			_rectShape.setOrigin({ (_point2.x - _point1.x) / 2.0f, (_point2.y - _point1.y) / 2.0f });
			_rectShape.setPosition({ (_point2.x + _point1.x) / 2.0f, (_point2.y + _point1.y) / 2.0f });
			_rectShape.setSize({ _point2.x - _point1.x, _point2.y - _point1.y });
			_rectShape.setFillColor(_fillcolor);
			_rectShape.setOutlineColor(_linecolor);
			_rectShape.setOutlineThickness(_thickness);
			drawTarget.draw(_rectShape);
		}
		static void line(sf::RenderTarget& drawTarget, sf::Vector2f _point1, sf::Vector2f _point2, sf::Color _color, float _thickness) {
			sf::RectangleShape _rectShape;
			sf::CircleShape _circleShape;
			float _len = (_point2 - _point1).length();
			_rectShape.setPosition({ (_point2.x + _point1.x) / 2.0f, (_point2.y + _point1.y) / 2.0f });
			_rectShape.setOrigin({ _len / 2.0f, _thickness / 2.0f });
			_rectShape.setSize({ _len, _thickness });
			_rectShape.setFillColor(_color);
			_rectShape.setRotation((_point2 - _point1).angle());
			drawTarget.draw(_rectShape);
			_circleShape.setOrigin({ _thickness / 2.0f, _thickness / 2.0f });
			_circleShape.setRadius(_thickness / 2.0f);
			_circleShape.setFillColor(_color);
			_circleShape.setPosition(_point1);
			drawTarget.draw(_circleShape);
			_circleShape.setPosition(_point2);
			drawTarget.draw(_circleShape);
		}
	}

	namespace gui {
		class windowManager {
		private:
			queue<Type::Event>Event;
			Type::Statu focus, pressFocus;
			unsigned int tick = 0;
			windowManager& operator=(const windowManager& _w)const {}
		public:
			void clear() {
				while (!Event.empty())
					Event.pop();
				focus = pressFocus = Type::Event();
				_window.clear();
				_windowId.clear();
			}
			class style {
				friend class windowManager;
			private:
				bool _isEmpty = true;
				sf::Color backgroundColor = sf::Color(0, 0, 0, 0), outlineColor = sf::Color(0, 0, 0, 0);
				float outlineThickness = 1;
				sf::Color textColor = sf::Color(0, 0, 0, 0);
				sf::Font font;
				unsigned int characterSize = 30;
				float letterSpacing = 1, lineSpacing = 1;
			public:
				style& setStyle(sf::Color _backgroundColor = sf::Color(0, 0, 0, 0), sf::Color _outlineColor = sf::Color(0, 0, 0, 0), float _outlineThickness = -1) {
					if (_backgroundColor != sf::Color(0, 0, 0, 0)) {
						backgroundColor = _backgroundColor; _isEmpty = false;
					}
					if (_outlineColor != sf::Color(0, 0, 0, 0)) {
						outlineColor = _outlineColor; _isEmpty = false;
					}
					if (_outlineThickness != -1) {
						outlineThickness = _outlineThickness; _isEmpty = false;
					}
					return *this;
				}
				style& setFont(sf::Font _font) {
					font = _font; _isEmpty = false;
					return *this;
				}
				style& setTextStyle(sf::Color _textColor = sf::Color(0, 0, 0, 0), unsigned int _characterSize = 0, float _letterSpacing = -1, float _lineSpacing = -1) {
					if (_textColor != sf::Color(0, 0, 0, 0)) {
						textColor = _textColor; _isEmpty = false;
					}
					if (_characterSize != 0) {
						characterSize = _characterSize; _isEmpty = false;
					}
					if (_letterSpacing != -1) {
						letterSpacing = _letterSpacing; _isEmpty = false;
					}
					if (_lineSpacing != -1) {
						lineSpacing = _lineSpacing; _isEmpty = false;
					}
					return *this;
				}
				style asCharacterSize(unsigned int _characterSize) const {
					style _style = *this;
					_style.characterSize = _characterSize;
					return _style;
				}
				bool empty() const {
					return _isEmpty;
				}
			};
		private:
			//CRTP模式基类
			template <typename T>
			class Obj {
			protected:
				string id;
				float x = 0, y = 0;
				float width = 0, height = 0;
				windowManager::style normalStyle;
				bool isCenter = false;
				sf::FloatRect rect;
				void updateRect(sf::Vector2f _origin) {
					rect.position.x = _origin.x + x - isCenter * width / 2;
					rect.position.y = _origin.y + y - isCenter * height / 2;
					rect.size.x = width;
					rect.size.y = height;
				}
				void draw(sf::RenderTarget& r, style& _style) {
					Draw::rect(
						r,
						rect.position,
						{ rect.position.x + rect.size.x, rect.position.y + rect.size.y },
						_style.backgroundColor,
						_style.outlineColor,
						_style.outlineThickness
					);
				}
			public:
				T& setPosition(float _x, float _y, bool _isCenter = false) {
					x = _x;
					y = _y;
					isCenter = _isCenter;
					return static_cast<T&>(*this);
				}
				template<typename _T>
				T& setPositionCenterOf(_T& _Obj) {
					x = _Obj.x + (!_Obj.isCenter) * _Obj.width / 2;
					y = _Obj.y + (!_Obj.isCenter) * _Obj.height / 2;
					isCenter = true;
					return static_cast<T&>(*this);
				}
				T& setSize(float _width, float _height) {
					width = _width;
					height = _height;
					return static_cast<T&>(*this);
				}
				T& setStyle(string _normalStyleTitle);
				T& setStyle(style _normalStyle) {
					if (!_normalStyle.empty())normalStyle = _normalStyle;
					return static_cast<T&>(*this);
				}
			};
		public:
			class buttonObj :public Obj<buttonObj> {
			protected:
				friend class windowManager;
				style overStyle, pressedStyle;
			public:
				buttonObj& setStyle(string _normalStyleTitle, string _overStyleTitle, string _pressedStyleTitle);
				buttonObj& setStyle(style _normalStyle, style _overStyle, style _pressedStyle) {
					if (!_normalStyle.empty())normalStyle = _normalStyle;
					if (!_overStyle.empty())overStyle = _overStyle;
					if (!_pressedStyle.empty())pressedStyle = _pressedStyle;
					return *this;
				}
			};
			class textObj :public Obj<textObj> {
			protected:
				friend class windowManager;
				sf::String text;
				sf::FloatRect textRect;
				float textDeltaX = 0, textDeltaY = 0;
				sf::Text updateTextRect(sf::Vector2f _origin,style& _style) {
					sf::Text _text(_style.font);
					_text.setString(text);
					_text.setCharacterSize(_style.characterSize);
					_text.setLineSpacing(_style.lineSpacing);
					_text.setLetterSpacing(_style.letterSpacing);
					_text.setFillColor(_style.textColor);
					_text.setPosition({ 0, 0 });
					textDeltaX = _text.getGlobalBounds().position.x;
					textDeltaY = _text.getGlobalBounds().position.y;
					_text.setPosition({
						_origin.x + x - isCenter * _text.getGlobalBounds().size.x / 2.0f - textDeltaX,
						_origin.y + y - isCenter * _text.getGlobalBounds().size.y / 2.0f - textDeltaY
						});
					textRect = _text.getGlobalBounds();
					return _text;
				}
				void drawText(sf::RenderTarget& r, style& _style, sf::Text& _text) const {
					Draw::rect(
						r,
						textRect.position,
						{ textRect.position.x + textRect.size.x, textRect.position.y + textRect.size.y },
						_style.backgroundColor,
						_style.outlineColor,
						_style.outlineThickness
					);
					r.draw(_text);
				}
			public:
				textObj& setText(sf::String _text) {
					text = _text;
					return *this;
				}
			};
			class textInputObj :public Obj<textInputObj>{
			protected:
				friend class windowManager;
				style overStyle, focusStyle;
				bool isTextXCenter = true, isTextYCenter = true;
				bool oneLineLimit=false;
				size_t sizeLimit=-1;
				size_t cursor = 0;
				sf::String text;
				sf::FloatRect textRect;
				float textDeltaX = 0, textDeltaY = 0;
				sf::Vector2f cursorPos;
				sf::Text updateTextRect(sf::Vector2f _origin, style& _style) {
					sf::Text _text(_style.font);
					_text.setString(text);
					_text.setCharacterSize(_style.characterSize);
					_text.setLineSpacing(_style.lineSpacing);
					_text.setLetterSpacing(_style.letterSpacing);
					_text.setFillColor(_style.textColor);
					_text.setPosition({ 0, 0 });
					textDeltaX = _text.getGlobalBounds().position.x;
					textDeltaY = _text.getGlobalBounds().position.y;
					_text.setPosition({
						_origin.x + x - isCenter * width / 2 + isTextXCenter * (width / 2 - _text.getGlobalBounds().size.x / 2) - textDeltaX,
						_origin.y + y - isCenter * height / 2 + isTextYCenter * (height / 2 - _text.getGlobalBounds().size.y / 2) - textDeltaY
						});
					textRect = _text.getGlobalBounds();
					cursorPos = _text.findCharacterPos(cursor) + sf::Vector2f(0, textDeltaY);
					return _text;
				}
				void drawText(sf::RenderTarget& r, style& _style, sf::Text& _text) const {
					Draw::rect(
						r,
						textRect.position,
						{ textRect.position.x + textRect.size.x, textRect.position.y + textRect.size.y },
						_style.backgroundColor
					);
					r.draw(_text);
				}
				void drawCursor(sf::RenderTarget& r, style& _style) const {
					Draw::line(
						r,
						cursorPos,
						cursorPos + sf::Vector2f(0, (float)_style.characterSize),
						_style.textColor,
						2.0f
					);
				}
			public:
				textInputObj& setStyle(string _normalStyleTitle,string _overStyleTitle, string _focusStyleTitle);
				textInputObj& setStyle(style _normalStyle, style _overStyle, style _focusStyle) {
					if (!_normalStyle.empty())normalStyle = _normalStyle;
					if (!_overStyle.empty())overStyle = _overStyle;
					if (!_focusStyle.empty())focusStyle = _focusStyle;
					return *this;
				}
				textInputObj& setLimit(bool _oneLineLimit=false,int _sizeLimit=-1) {
					oneLineLimit = _oneLineLimit;
					sizeLimit = _sizeLimit;
					return *this;
				}
				textInputObj& setText(sf::String _text) {
					text = _text;
					cursor = (int)text.getSize();
					return *this;
				}
				textInputObj& setTextCenter(bool _isTextXCenter, bool _isTextYCenter) {
					isTextXCenter = _isTextXCenter;
					isTextYCenter = _isTextYCenter;
					return *this;
				}
				sf::String getText() {
					return text;
				}
			};
			class areaObj :public Obj<areaObj> {
				friend class windowManager;
			protected:
				int scrollX = 0, scrollY = 0;
				sf::Vector2f origin;
				unordered_map<string, int>_textId, _buttonId, _textInputId;
				vector<textObj>_text;
				vector<buttonObj>_button;
				vector<textInputObj>_textInput;
				void updateRect(sf::Vector2f _origin) {
					rect.position.x = _origin.x + x;
					rect.position.y = _origin.y + y;
					rect.size.x = width;
					rect.size.y = height;
					origin.x = _origin.x + x + scrollX;
					origin.y = _origin.y + y + scrollY;
				}
			public:
				textObj& text(string _id) {
					if (!_textId.count(_id)) {
						textObj temp;
						temp.id = _id;
						_textId[_id] = (int)_text.size();
						_text.push_back(temp);
					}
					return _text[_textId[_id]];
				}
				buttonObj& button(string _id) {
					if (!_buttonId.count(_id)) {
						buttonObj temp;
						temp.id = _id;
						_buttonId[_id] = (int)_button.size();
						_button.push_back(temp);
					}
					return _button[_buttonId[_id]];
				}
				textInputObj& textInput(string _id) {
					if (!_textInputId.count(_id)) {
						textInputObj temp;
						temp.id = _id;
						_textInputId[_id] = (int)_textInput.size();
						_textInput.push_back(temp);
					}
					return _textInput[_textInputId[_id]];
				}
			};
			class windowObj :public Obj<windowObj> {
				friend class windowManager;
			protected:
				int scrollX = 0, scrollY = 0;
				sf::Vector2f origin;
				void updateRect() {
					rect.position.x = x;
					rect.position.y = y;
					rect.size.x = width;
					rect.size.y = height;
					origin.x = x + scrollX;
					origin.y = y + scrollY;
				}
				unordered_map<string, int>_areaId = { {"",0} };
				vector<areaObj>_area = {areaObj()};
			public:
				windowObj& setPosition(float _x, float _y, bool _isCenter = false) {
					x = _x;
					y = _y;
					isCenter = _isCenter;
					_area[0].setPosition(0, 0, false);
					return *this;
				}
				template<typename _T>
				windowObj& setPositionCenterOf(_T& _Obj) {
					x = _Obj.x + (!_Obj.isCenter) * _Obj.width / 2;
					y = _Obj.y + (!_Obj.isCenter) * _Obj.height / 2;
					isCenter = true;
					_area[0].setPosition(x,y,isCenter);
					return *this;
				}
				windowObj& setSize(float _width, float _height) {
					width = _width;
					height = _height;
					_area[0].setSize(width, height);
					return *this;
				}
				areaObj& areaDefault() {
					return _area[0];
				}
				areaObj& area(string _id) {
					if (!_areaId.count(_id)) {
						areaObj temp;
						temp.id = _id;
						_areaId[_id] = (int)_area.size();
						_area.push_back(temp);
					}
					return _area[_areaId[_id]];
				}
			};
		private:
			unordered_map<string, int>_windowId;
			vector <windowObj>_window;
		public:
			bool pollEvent(Type::Event& _e) {
				if (Event.empty()) {
					return false;
				}
				_e = Event.front();
				Event.pop();
				return true;
			}
			bool newWindow(string _id) {
				if (_windowId.count(_id)) {
					//窗口名称重复
					return false;
				}
				windowObj temp;
				temp.id = _id;
				_windowId[_id] = (int)_window.size();
				_window.push_back(temp);
			}
			bool newWindow(string _id, windowObj _windowTemplate) {
				if (_windowId.count(_id)) {
					//窗口名称重复
					return false;
				}
				_windowId[_id] = (int)_window.size();
				_window.push_back(_windowTemplate);
				_window[_window.size() - 1].id = _id;
			}
			bool closeTopWindow() {
				if (_window.size() > 0) {
					_windowId.erase(_window[_window.size() - 1].id);
					_window.pop_back();
					return true;
				}
				return false;
			}
			bool newWindow(string _id, string _templateTitle);
			windowObj& window(int& _pos) {
				return _window[_pos];
			}
			windowObj& window(string _id) {
				return _window[_windowId[_id]];
			}
			bool update(const optional<sf::Event> & _windowEvent) {
				if (_window.size() == 0)return false;
				windowObj& windowTemp = _window[_window.size() - 1];
				windowTemp.updateRect();
				if (_windowEvent->is<sf::Event::MouseButtonPressed>()) {
					sf::Vector2f mousePos = sf::Vector2f(_windowEvent->getIf<sf::Event::MouseButtonPressed>()->position);
					int mouseButton=static_cast<int>(_windowEvent->getIf<sf::Event::MouseButtonPressed>()->button);
					for (auto& areaTemp : windowTemp._area) {
						areaTemp.updateRect(windowTemp.origin);
						for (auto& buttonTemp : areaTemp._button) {
							buttonTemp.updateRect(areaTemp.origin);
							if (buttonTemp.rect.contains(mousePos) &&
								windowTemp.rect.contains(mousePos) &&
								areaTemp.rect.contains(mousePos)
								) {
								pressFocus = {
									"window", windowTemp.id,
									"area", areaTemp.id,
									"button", buttonTemp.id,
									"mouseButton", mouseButton
								};
								focus = {
									"window", windowTemp.id,
									"area", areaTemp.id,
									"button", buttonTemp.id
								};
								return true;
							}
						}
						for (auto& textInputTemp : areaTemp._textInput) {
							textInputTemp.updateRect(areaTemp.origin);
							if (textInputTemp.rect.contains(mousePos) &&
								windowTemp.rect.contains(mousePos) &&
								areaTemp.rect.contains(mousePos)
								) {
								tick = 0;
								pressFocus = {
									"window", windowTemp.id,
									"area", areaTemp.id,
									"textInput", textInputTemp.id,
									"mouseButton", mouseButton
								};
								focus = {
									"window", windowTemp.id,
									"area", areaTemp.id,
									"textInput", textInputTemp.id,
								};
								return true;
							}
						}
					}
					pressFocus = {};
					focus = {};
					return true;
				}
				if (_windowEvent->is<sf::Event::MouseButtonReleased>()) {
					sf::Vector2f mousePos = sf::Vector2f(_windowEvent->getIf<sf::Event::MouseButtonReleased>()->position);
					int mouseButton = static_cast<int>(_windowEvent->getIf<sf::Event::MouseButtonReleased>()->button);
					for (auto& areaTemp : windowTemp._area) {
						areaTemp.updateRect(windowTemp.origin);
						for (auto& buttonTemp : areaTemp._button) {
							buttonTemp.updateRect(areaTemp.origin);
							if (buttonTemp.rect.contains(mousePos) &&
								windowTemp.rect.contains(mousePos) &&
								areaTemp.rect.contains(mousePos)
								) {
								if (pressFocus.contain("window", windowTemp.id, "area", areaTemp.id, "button", buttonTemp.id, "mouseButton", mouseButton)) {
									Event.push(game::Type::Event(game::Type::Event::guiButtonpressed, { "window", windowTemp.id, "area", areaTemp.id, "button", buttonTemp.id, "mouseButton", mouseButton}));
									pressFocus = {};
								}
								return true;
							}
						}
					}
					pressFocus = {};
					return true;
				}
				if (_windowEvent->is<sf::Event::TextEntered>()) {
					if (!focus.empty()) {
						char32_t ch = _windowEvent->getIf<sf::Event::TextEntered>()->unicode;
						if (focus.count("textInput")) {
							textInputObj& textInputTemp = _window[_windowId[focus["window"].cast<string>()]].area(focus["area"].cast<string>()).textInput(focus["textInput"].cast<string>());
							sf::String& text = textInputTemp.text;
							if (ch == 8) {
								if (text.getSize() > 0&&textInputTemp.cursor>0) {
									text.erase(textInputTemp.cursor - 1);
									textInputTemp.cursor--;
								}
							}
							else if (ch == 22) {
								sf::String clipboardTemp = sf::Clipboard::getString();
								if (textInputTemp.sizeLimit - text.getSize() > 0) {
									text.insert(textInputTemp.cursor, clipboardTemp.substring(0, textInputTemp.sizeLimit - text.getSize()));
									textInputTemp.cursor += min(textInputTemp.sizeLimit - text.getSize(),clipboardTemp.getSize());
								}
							}
							else if (ch == 13) {
								if (!textInputTemp.oneLineLimit) {
									text.insert(textInputTemp.cursor, '\n');
									textInputTemp.cursor++;
								}
							}
							else {
								if (text.getSize() < textInputTemp.sizeLimit) {
									text.insert(textInputTemp.cursor, ch);
									textInputTemp.cursor++;
								}
							}
							tick = 0;
						}
					}
					return true;
				}
				if (_windowEvent->is<sf::Event::KeyPressed>()) {
					if (_windowEvent->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Left) {
						textInputObj& textInputTemp = _window[_windowId[focus["window"].cast<string>()]].area(focus["area"].cast<string>()).textInput(focus["textInput"].cast<string>());
						if (textInputTemp.cursor > 0) {
							textInputTemp.cursor--;
						}
					}
					if (_windowEvent->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Right) {
						textInputObj& textInputTemp = _window[_windowId[focus["window"].cast<string>()]].area(focus["area"].cast<string>()).textInput(focus["textInput"].cast<string>());
						sf::String& text = textInputTemp.text;
						if (textInputTemp.cursor <text.getSize()) {
							textInputTemp.cursor++;
						}
					}
					if (_windowEvent->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Delete) {
						textInputObj& textInputTemp = _window[_windowId[focus["window"].cast<string>()]].area(focus["area"].cast<string>()).textInput(focus["textInput"].cast<string>());
						sf::String& text = textInputTemp.text;
						if (text.getSize() > 0 && textInputTemp.cursor < text.getSize()) {
							text.erase(textInputTemp.cursor);
						}
					}
					tick = 0;
					return true;
				}
				return false;
			}
			void draw(sf::RenderTarget& drawTarget,sf::RenderWindow& getMousePosFrom) {
				if (focus.count("textInput")) {
					tick++;
					tick %= 60;
				}
				if (_window.size() == 0)return;
				sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(getMousePosFrom));
				for (int i = 0; i < _window.size(); i++) {
					windowObj& windowTemp = _window[i];
					windowTemp.updateRect();
					windowTemp.draw(drawTarget, windowTemp.normalStyle);
					for (auto& areaTemp : windowTemp._area) {
						areaTemp.updateRect(windowTemp.origin);
						if (windowTemp.rect.findIntersection(areaTemp.rect)) {
							for (auto& buttonTemp : areaTemp._button) {
								buttonTemp.updateRect(areaTemp.origin);
								if (areaTemp.rect.findIntersection(buttonTemp.rect)) {
									if (
										pressFocus.contain("window", windowTemp.id, "area", areaTemp.id, "button", buttonTemp.id) ||
										(
											buttonTemp.rect.contains(mouse) &&
											windowTemp.rect.contains(mouse) &&
											areaTemp.rect.contains(mouse)
											) &&
										i == _window.size() - 1
										) {
										if (
											buttonTemp.rect.contains(mouse) &&
											windowTemp.rect.contains(mouse) &&
											areaTemp.rect.contains(mouse) &&
											pressFocus.contain("window", windowTemp.id, "area", areaTemp.id, "button", buttonTemp.id)
											) {
											buttonTemp.draw(drawTarget, buttonTemp.pressedStyle);
										}
										else {
											buttonTemp.draw(drawTarget, buttonTemp.overStyle);
										}
									}
									else {
										buttonTemp.draw(drawTarget, buttonTemp.normalStyle);
									}
								}
							}
							for (auto& textTemp : areaTemp._text) {
								sf::Text _text(textTemp.updateTextRect(areaTemp.origin, textTemp.normalStyle));
								//if (areaTemp.rect.findIntersection(textTemp.textRect)) {
									textTemp.drawText(drawTarget, textTemp.normalStyle, _text);
								//}
							}
							for (auto& textInputTemp : areaTemp._textInput) {
								textInputTemp.updateRect(areaTemp.origin);
								if (areaTemp.rect.findIntersection(textInputTemp.rect)) {
									if (focus.contain("window", windowTemp.id, "textInput", textInputTemp.id)) {
										sf::Text _text(textInputTemp.updateTextRect(areaTemp.origin, textInputTemp.focusStyle));
										textInputTemp.draw(drawTarget, textInputTemp.focusStyle);
										textInputTemp.drawText(drawTarget, textInputTemp.focusStyle, _text);
										if (tick < 30)
											textInputTemp.drawCursor(drawTarget, textInputTemp.focusStyle);
									}
									else
										if (
											(
												textInputTemp.rect.contains(mouse) &&
												windowTemp.rect.contains(mouse)
												) &&
											i == _window.size() - 1
											) {
											sf::Text _text(textInputTemp.updateTextRect(areaTemp.origin, textInputTemp.overStyle));
											textInputTemp.draw(drawTarget, textInputTemp.overStyle);
											textInputTemp.drawText(drawTarget, textInputTemp.overStyle, _text);
											textInputTemp.drawCursor(drawTarget, textInputTemp.overStyle);
										}
									else {
										sf::Text _text(textInputTemp.updateTextRect(areaTemp.origin, textInputTemp.normalStyle));
										textInputTemp.draw(drawTarget, textInputTemp.normalStyle);
										textInputTemp.drawText(drawTarget, textInputTemp.normalStyle, _text);
									}
								}
							}
						}
					}
				}
			}
		};
		
		unordered_map<string, windowManager::windowObj>windowTemplate;
		unordered_map<string, windowManager::style>styleTemplate;
		bool windowManager::newWindow(string _id, string _templateTitle) {
			if (_windowId.count(_id)) {
				return false;
			}
			_windowId[_id] = (int)_window.size();
			_window.push_back(gui::windowTemplate[_templateTitle]);
			_window[_window.size() - 1].id = _id;
		}
		template <typename T>
		T& windowManager::Obj<T>::setStyle(string _normalStyleTitle) {
			if (gui::styleTemplate.count(_normalStyleTitle)) {
				normalStyle = gui::styleTemplate[_normalStyleTitle];
			}
			return static_cast<T&>(*this);
		}
		windowManager::buttonObj& windowManager::buttonObj::setStyle(string _normalStyleTitle, string _overStyleTitle, string _pressedStyleTitle) {
			if (gui::styleTemplate.count(_normalStyleTitle)) {
				normalStyle = gui::styleTemplate[_normalStyleTitle];
			}
			if (gui::styleTemplate.count(_overStyleTitle)) {
				overStyle = gui::styleTemplate[_overStyleTitle];
			}
			if (gui::styleTemplate.count(_pressedStyleTitle)) {
				pressedStyle = gui::styleTemplate[_pressedStyleTitle];
			}
			return *this;
		}
		windowManager::textInputObj& windowManager::textInputObj::setStyle(string _normalStyleTitle, string _overStyleTitle, string _focusStyleTitle) {
			if (gui::styleTemplate.count(_normalStyleTitle)) {
				normalStyle = gui::styleTemplate[_normalStyleTitle];
			}
			if (gui::styleTemplate.count(_overStyleTitle)) {
				overStyle = gui::styleTemplate[_overStyleTitle];
			}
			if (gui::styleTemplate.count(_focusStyleTitle)) {
				focusStyle = gui::styleTemplate[_focusStyleTitle];
			}
			return *this;
		}
	}
}
namespace createStyle {
	int windowWidth, windowHeight;
	sf::Font fontmsyh;
	static void newButtonTemplate(string _window,string _area, string _buttonId, sf::String _text, unsigned int _characterSize, float posX, float posY, float sizeX, float sizeY, bool _iscenter = false) {
		game::gui::windowTemplate[_window].area(_area).button(_buttonId).setStyle("stdbn", "stdbo", "stdbp").setPosition(posX, posY, _iscenter).setSize(sizeX, sizeY);
		game::gui::windowTemplate[_window].area(_area).text(_buttonId).setText(_text).setStyle(game::gui::styleTemplate["stdt"].asCharacterSize(_characterSize)).setPositionCenterOf(game::gui::windowTemplate[_window].area(_area).button(_buttonId));
	}
	static void newWindowTemplate(string _window, float posX = 0, float posY = 0, float sizeX = windowWidth, float sizeY = windowHeight, bool _iscenter = false) {
		game::gui::windowTemplate[_window].setStyle("stdw");
		game::gui::windowTemplate[_window].setPosition(posX, posY, _iscenter).setSize(sizeX, sizeY);
	}
	static void newAreaTemplate(string _window,string _area, float posX = 0, float posY = 0, float sizeX = windowWidth, float sizeY = windowHeight, bool _iscenter = false) {
		game::gui::windowTemplate[_window].area(_area).setStyle("stda").setPosition(posX, posY, _iscenter).setSize(sizeX, sizeY);
	}
	static void newTextTemplate(string _window, string _area, string _textId, sf::String _text, unsigned int _characterSize, float posX, float posY, bool _iscenter = false) {
		game::gui::windowTemplate[_window].area(_area).text(_textId).setText(_text).setStyle(game::gui::styleTemplate["stdt"].asCharacterSize(_characterSize)).setPosition(posX, posY, _iscenter);
	}
	static void newTextInputTemplate(string _window, string _area, string _textId, sf::String _text, unsigned int _characterSize, float posX, float posY, float sizeX, float sizeY, bool _iscenter = false, bool isTextXCenter = true, bool isTextYCenter = true, bool oneLineLimit = false, int sizeLimit = -1) {
		game::gui::windowTemplate[_window].area(_area).textInput(_textId).setText(_text).setTextCenter(isTextXCenter, isTextYCenter).setLimit(oneLineLimit, sizeLimit)
			.setStyle(game::gui::styleTemplate["stdin"].asCharacterSize(_characterSize), game::gui::styleTemplate["stdio"].asCharacterSize(_characterSize), game::gui::styleTemplate["stdif"].asCharacterSize(_characterSize))
			.setPosition(posX, posY, _iscenter).setSize(sizeX, sizeY);
	}
	static void newCopyWindowTemplate(string _window, string _fromWindow, sf::String _text) {
		game::gui::windowTemplate[_window] = game::gui::windowTemplate[_fromWindow];
		game::gui::windowTemplate[_window].areaDefault().text("head").setText(_text);
	}
	static void init(int _windowWidth, int _windowHeight) {
		windowWidth = _windowWidth;
		windowHeight = _windowHeight;
		bool ret = fontmsyh.openFromFile("C:\\Windows\\Fonts\\msyh.ttc");
		game::gui::styleTemplate["stdw"].setStyle(sf::Color::White, sf::Color(200, 200, 200), 2);
		game::gui::styleTemplate["stda"].setStyle(sf::Color::White, sf::Color(220, 220, 250), 2);
		game::gui::styleTemplate["stdbn"].setStyle(sf::Color::Color(250, 250, 250), sf::Color(200, 200, 200), 2);
		game::gui::styleTemplate["stdbo"].setStyle(sf::Color(220, 220, 220), sf::Color(200, 200, 200), 2);
		game::gui::styleTemplate["stdbp"].setStyle(sf::Color(200, 200, 200), sf::Color(150, 150, 150), 2);
		game::gui::styleTemplate["stdin"].setStyle(sf::Color(250, 250, 250), sf::Color(200, 200, 200), 2).setTextStyle(sf::Color::Black).setFont(fontmsyh);
		game::gui::styleTemplate["stdio"].setStyle(sf::Color(250, 250, 250), sf::Color(150, 150, 150), 2).setTextStyle(sf::Color::Black).setFont(fontmsyh);
		game::gui::styleTemplate["stdif"].setStyle(sf::Color(220, 220, 220), sf::Color(150, 150, 150), 2).setTextStyle(sf::Color::Black).setFont(fontmsyh);
		/*game::gui::styleTemplate["winbn"].setStyle(sf::Color::Color(225, 225, 225), sf::Color(173, 173, 173), 2);
		game::gui::styleTemplate["winbo"].setStyle(sf::Color(229, 241, 251), sf::Color(0, 120, 215), 2);
		game::gui::styleTemplate["winbp"].setStyle(sf::Color(204, 228, 247), sf::Color(0, 84, 153), 2);*/
		game::gui::styleTemplate["stdt"].setTextStyle(sf::Color::Black, 30).setFont(fontmsyh);

		newWindowTemplate("messageokcancelTemplate", windowWidth / 2.0f - 400, windowHeight / 2.0f - 150, 800, 275);
		newButtonTemplate("messageokcancelTemplate", "", "ok", L"确定", 50, 300, 225, 130, 50, true);
		newButtonTemplate("messageokcancelTemplate", "", "cancel", L"取消", 50, 500, 225, 130, 50, true);
		newTextTemplate("messageokcancelTemplate", "", "head", "", 50, 400, 100,true);

		newWindowTemplate("messageokTemplate", windowWidth / 2.0f - 400, windowHeight / 2.0f - 150, 800, 275);
		newButtonTemplate("messageokTemplate", "", "ok", L"确定", 50, 400, 225, 130, 50, true);
		newTextTemplate("messageokTemplate", "", "head", "", 50, 400, 100,true);
	}
	static bool checkClick(game::Type::Event& evt, string _window, string _area, string _button, int mouseButton = -1) {
		if (mouseButton == -1) {
			return evt.contain("window", _window, "area", _area, "button", _button);
		}
		else {
			return evt.contain("window", _window, "area", _area, "button", _button, "mouseButton", mouseButton);
		}
	}
}
ostream& operator<<(ostream &out, const sf::String sfU8string) {
	string converted_string;
	for (uint8_t& _char : sfU8string.toUtf8())
		converted_string.push_back(static_cast<char>(_char));
	out << converted_string;
	return out;
}


int windowWidth = 1270, windowHeight = 720;
game::gui::windowManager windowManager;
game::Type::Event evt;
static void init() {
	createStyle::init(windowWidth, windowHeight);
	createStyle::newWindowTemplate("mainTemplate");
	createStyle::newButtonTemplate("mainTemplate", "", "button1", L"按钮1", 50, 100, 100, 300, 300, false);
	createStyle::newTextInputTemplate("mainTemplate", "", "textInput1", L"输入文字:", 50, 500, 100, 300, 300);
	createStyle::newCopyWindowTemplate("confirmTemplate", "messageokTemplate", L"确认窗口");
}
int main() {
	init();
	windowManager.newWindow("main", "mainTemplate");
	game::window.create(sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), L"", sf::Style::Close, sf::State::Windowed);
	game::window.setFramerateLimit(60);
	while (true) {
		while (const optional sfEvt=game::window.pollEvent()) {
			if (sfEvt->is<sf::Event::Closed>()) {
				exit(0);
			}
			else {
				windowManager.update(sfEvt);
			}
		}
		while (windowManager.pollEvent(evt)) {
			if (evt.eventId == game::Type::Event::guiButtonpressed) {
				printf("按钮 %s.%s 按下\n", evt["window"].cast<string>().data(), evt["button"].cast<string>().data());
				if (evt.contain("window", "main", "button", "button1")) {
					windowManager.newWindow("confirm","confirmTemplate");
				}
				if (evt.contain("window", "confirm", "button", "ok")) {
					windowManager.closeTopWindow();
				}
			}
		}
		game::window.clear();
		windowManager.draw(game::window,game::window);
		game::window.display();
	}
	return 0;
}