#pragma once

namespace sprintfmt
{
#if 0
	//
	// std::stringをstd::wstringに変換して返します。
	// https://nprogram.hatenablog.com/entry/2018/01/03/113206
	//
	inline std::wstring cp_to_wide(const std::string& s, UINT codepage)
	{
		auto in_length = (int)s.length();
		auto out_length = ::MultiByteToWideChar(codepage, DWORD(), s.c_str(), in_length, LPWSTR(), int()); 
		auto result = std::wstring(out_length, L'\0');
		if (out_length) ::MultiByteToWideChar(codepage, DWORD(), s.c_str(), in_length, result.data(), out_length);
		return result;
	}

	//
	// std::wstringをstd::stringに変換して返します。
	// https://nprogram.hatenablog.com/entry/2018/01/03/113206
	//
	inline std::string wide_to_cp(const std::wstring& s, UINT codepage)
	{
		auto in_length = (int)s.length();
		auto out_length = ::WideCharToMultiByte(codepage, DWORD(), s.c_str(), in_length, LPSTR(), int(), LPCCH(), LPBOOL()); 
		auto result = std::string(out_length, '\0');
		if (out_length) ::WideCharToMultiByte(codepage, DWORD(), s.c_str(), in_length, result.data(), out_length, LPCCH(), LPBOOL()); 
		return result;
	}

	//
	// マルチバイト文字列をワイド文字列に変換して返します。
	//
	inline std::wstring to_wide(const std::string& s)
	{
		return cp_to_wide(s, CP_ACP);
	}

	//
	// ワイド文字列をマルチバイト文字列に変換して返します。
	//
	inline std::string from_wide(const std::wstring& s)
	{
		return wide_to_cp(s, CP_ACP);
	}
#else
	//
	// マルチバイト文字列をワイド文字列に変換して返します。
	//
	inline std::wstring to_wide(const std::string& s)
	{
		auto s_begin = s.c_str();
		auto s_end = s.c_str() + s.length();
		auto mb_state = mbstate_t();
		auto result = std::wstring();
		result.reserve(s.length());

		for (auto p = s_begin; p < s_end;)
		{
			auto wc = wchar_t();
			auto char_length = (int)mbrtowc(&wc, p, MB_CUR_MAX, &mb_state);
			if (char_length < 1) break;
			result += wc;
			p += char_length;
		}

		return result;
	}

	//
	// ワイド文字列をマルチバイト文字列に変換して返します。
	//
	inline std::string from_wide(const std::wstring& s)
	{
		auto s_begin = s.c_str();
		auto s_end = s.c_str() + s.length();
		auto mb_state = mbstate_t();
		auto mc = std::make_unique<char[]>(MB_CUR_MAX);
		auto result = std::string();
		result.reserve(s.length() * 2);

		for (auto p = s_begin; p < s_end; p++)
		{
			auto char_length = size_t();
			wcrtomb_s(&char_length, mc.get(), MB_CUR_MAX, *p, &mb_state);
			if ((int)char_length < 1) break;
			result += { mc.get(), char_length };
		}

		return result;
	}
#endif
	//
	// C言語のsprintf()を使用して文字列をフォーマット化して返します。
	//
	template <size_t MaxSize = 64, typename... Args>
	_NODISCARD constexpr auto sprintf(const std::string& fmt, Args&&... args) -> std::string
	{
		auto s = std::string(MaxSize, '\0');
		sprintf_s(s.data(), s.size(), fmt.c_str(), args...);
		s.resize(strlen(s.data()));
		return s;
	}

	//
	// C言語のsprintf()を使用して文字列をフォーマット化して返します。
	//
	template <size_t MaxSize = 64, typename... Args>
	_NODISCARD constexpr auto sprintf(const std::wstring& fmt, Args&&... args) -> std::wstring
	{
		auto s = std::wstring(MaxSize, L'\0');
		swprintf_s(s.data(), s.size(), fmt.c_str(), args...);
		s.resize(wcslen(s.data()));
		return s;
	}

	//
	// 文字列化できない型の場合に呼び出されます。
	//
	template <typename T>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		static_assert(std::false_type::value, "この型に対応するfmt_value_to_string()が見つかりませんでした");
	}

	//
	// 符号あり32bit整数型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt + "d", args..., value);
	}

	//
	// 符号あり32bit整数型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt + L"d", args..., value);
	}

	//
	// 符号あり64bit整数型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt + "I64d", args..., value);
	}

	//
	// 符号あり64bit整数型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt + L"I64d", args..., value);
	}

	//
	// 符号なし32bit整数型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt + "u", args..., value);
	}

	//
	// 符号なし32bit整数型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt + L"u", args..., value);
	}

	//
	// 符号なし64bit整数型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt + "I64u", args..., value);
	}

	//
	// 符号なし64bit整数型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt + L"I64u", args..., value);
	}

	//
	// 浮動小数点型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_floating_point_v<T>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt + "f", args..., value);
	}

	//
	// 浮動小数点型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_floating_point_v<T>
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt + L"f", args..., value);
	}

	//
	// 32bitポインタ型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_pointer_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("0x%08X", value);
	}

	//
	// 32bitポインタ型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_pointer_v<T> && (sizeof(T) <= 4)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"0x%08X", value);
	}

	//
	// 64bitポインタ型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_pointer_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return sprintf("0x%016X", value);
	}

	//
	// 64bitポインタ型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_pointer_v<T> && (sizeof(T) == 8)
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return sprintf(L"0x%016X", value);
	}

	//
	// const char*型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_convertible_v<T, const char*>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return fmt.empty() ? value : sprintf<2048>("%" + fmt + "hs", args..., value);
	}

	//
	// const char*型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_convertible_v<T, const char*>
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return fmt.empty() ? to_wide(value) : sprintf<2048>(L"%" + fmt + L"hs", args..., value);
	}

	//
	// const wchar_t*型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_convertible_v<T, const wchar_t*>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return fmt.empty() ? from_wide(value) : sprintf<2048>("%" + fmt + "ws", args..., value);
	}

	//
	// const wchar_t*型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_convertible_v<T, const wchar_t*>
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return fmt.empty() ? value : sprintf<2048>(L"%" + fmt + L"ws", args..., value);
	}

	//
	// std::string型をstd::stringに変換します。
	//
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const std::string& value, auto... args) -> std::string
	{
		return fmt.empty() ? value : sprintf<2048>("%" + fmt + "hs", args..., value.c_str());
	}

	//
	// std::string型をstd::wstringに変換します。
	//
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const std::string& value, auto... args) -> std::wstring
	{
		return fmt.empty() ? to_wide(value) : sprintf<2048>(L"%" + fmt + L"hs", args..., value.c_str());
	}

	//
	// std::wstring型をstd::stringに変換します。
	//
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const std::wstring& value, auto... args) -> std::string
	{
		return fmt.empty() ? from_wide(value) : sprintf<2048>("%" + fmt + "ws", args..., value.c_str());
	}

	//
	// std::wstring型をstd::wstringに変換します。
	//
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const std::wstring& value, auto... args) -> std::wstring
	{
		return fmt.empty() ? value : sprintf<2048>(L"%" + fmt + L"ws", args..., value.c_str());
	}
#ifdef _FILESYSTEM_
	//
	// std::filesystem::path型をstd::stringに変換します。
	//
	template <typename T>
	requires std::is_same_v<T, std::filesystem::path>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const T& value, auto... args) -> std::string
	{
		return fmt_value_to_string(fmt, value.string(), args...);
	}

	//
	// std::filesystem::path型をstd::wstringに変換します。
	//
	template <typename T>
	requires std::is_same_v<T, std::filesystem::path>
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const T& value, auto... args) -> std::wstring
	{
		return fmt_value_to_string(fmt, value.wstring(), args...);
	}
#endif
	template <typename T>
	struct own { const T& value; };

	//
	// own<T>をstd::stringに変換します。
	//
	template <typename T>
	_NODISCARD constexpr auto fmt_value_to_string(const std::string& fmt, const own<T>& value, auto... args) -> std::string
	{
		return sprintf("%" + fmt, args..., value.value);
	}

	//
	// own<T>型をstd::wstringに変換します。
	//
	template <typename T>
	_NODISCARD constexpr auto fmt_value_to_string(const std::wstring& fmt, const own<T>& value, auto... args) -> std::wstring
	{
		return sprintf(L"%" + fmt, args..., value.value);
	}

	//
	// このクラスはユーティリティです。
	//
	template <typename String> struct Utils;

	//
	// このクラスはstd::string型に特殊化されたユーティリティです。
	//
	template <> struct Utils<std::string>
	{
		//
		// フォーマット用のキーワードです。
		//
		inline static constexpr struct
		{
			inline static const std::string
				begin = "{/",
				end = "}",
				escape = "/",
				separator = "$",
				invalid_arg_index = "invalid_arg_index";
		} keyword;

		//
		// 指定された文字列の出現位置を返します。
		//
		inline static const char* find_str(
			const char* str_begin, const char* str_end, const std::string& pattern)
		{
			// 文字列が無効の場合は失敗します。
			if (str_begin >= str_end) return nullptr;

			// パターンが無効の場合は失敗します。
			if (pattern.empty()) return nullptr;

			// 文字列の長さを取得します。
			auto str_length = (size_t)(str_end - str_begin);

			// 文字列がパターンより短い場合はnullptrを返します。
			if (str_length < pattern.length()) return nullptr;

			// mbrlen()が使用する変数です。
			mbstate_t mb_state = {};

			// 文字列の長さからパターンの長さを除いた数だけループします。
			for (auto p = str_begin; p <= str_end - pattern.length();)
			{
				// 文字の長さを取得します。
				auto char_length = (int)mbrlen(p, MB_CUR_MAX, &mb_state);

				// 文字の長さが取得できなかった場合はループを終了します。
				if (char_length < 1) break;

				// 文字列内の現在位置がパターンと一致した場合は
				if (memcmp(p, pattern.c_str(), pattern.length()) == 0)
					return p; // 一致した位置を返します。

				// 文字の長さの分だけ現在位置を進めます。
				p += char_length;
			}

			// パターンが見つからなかったのでnullptrを返します。
			return nullptr;
		}

		//
		// 指定された文字列を整数に変換して返します。
		//
		inline static int to_int(const std::string& s)
		{
			try { return std::stoi(s); } catch (...) { return 0; }
		}
	};

	//
	// このクラスはstd::wstring型に特殊化されたユーティリティです。
	//
	template <> struct Utils<std::wstring>
	{
		//
		// フォーマット用のキーワードです。
		//
		inline static constexpr struct
		{
			inline static const std::wstring
				begin = L"{/",
				end = L"}",
				escape = L"/",
				separator = L"$",
				invalid_arg_index = L"invalid_arg_index";
		} keyword;

		//
		// 指定された文字列の出現位置を返します。
		//
		inline static const wchar_t* find_str(
			const wchar_t* str_begin, const wchar_t* str_end, const std::wstring& pattern)
		{
			// 文字列が無効の場合は失敗します。
			if (str_begin >= str_end) return nullptr;

			// パターンが無効の場合は失敗します。
			if (pattern.empty()) return nullptr;

			// 文字列の長さを取得します。
			auto str_length = (size_t)(str_end - str_begin);

			// 文字列がパターンより短い場合はnullptrを返します。
			if (str_length < pattern.length()) return nullptr;

			// 文字列の長さからパターンの長さを除いた数だけループします。
			for (auto p = str_begin; p <= str_end - pattern.length(); p++)
			{
				// 文字列内の現在位置がパターンと一致した場合は
				if (memcmp(p, pattern.c_str(), pattern.length()) == 0)
					return p; // 一致した位置を返します。
			}

			// パターンが見つからなかったのでnullptrを返します。
			return nullptr;
		}

		//
		// 指定された文字列を整数に変換して返します。
		//
		inline static int to_int(const std::wstring& s)
		{
			try { return std::stoi(s); } catch (...) { return 0; }
		}
	};

	//
	// このクラスはstd::string型用のサブフォーマッタ生成関数オブジェクトです。
	//
	struct make_sub_formatter_s {
		template <typename T, typename... Args>
		constexpr auto operator()(const T& value, Args&&... args) const {
			return [&](const std::string& fmt) {
				return fmt_value_to_string(fmt, value, args...);
			};
		}
	};

	//
	// このクラスはstd::wstring型用のサブフォーマッタ生成関数オブジェクトです。
	//
	struct make_sub_formatter_w {
		template <typename T, typename... Args>
		constexpr auto operator()(const T& value, Args&&... args) const {
			return [&](const std::wstring& fmt) {
				return fmt_value_to_string(fmt, value, args...);
			};
		}
	};

	//
	// このクラスはフォーマッタです。
	//
	template <typename String, typename MakeSubFormatter>
	struct Formatter
	{
		//
		// フォーマット用のユーティリティです。
		//
		inline static const Utils<String> utils;

		//
		// 新規作成したサブフォーマッタを使用して文字列化を実行します。
		//
		template <typename T>
		inline static constexpr auto use_sub_formatter(const T& value, const String& fmt)
		{
			return use_sub_formatter(MakeSubFormatter()(value), fmt);
		}

		//
		// 既存のサブフォーマッタを使用して文字列化を実行します。
		//
		template <typename T>
		inline static constexpr auto use_sub_formatter(T&& sub_formatter, const String& fmt)
		requires requires { std::forward<T>(sub_formatter)(fmt); } // 引数がサブフォーマッタ(ラムダ)の場合は
		{
			return sub_formatter(fmt);
		}

		//
		// インデックスで指定された引数が存在しなかった場合に呼び出されます。
		//
		inline static constexpr auto sub_format(size_t index, const String& sub_fmt) -> String
		{
			return utils.keyword.invalid_arg_index;
		}

		//
		// インデックスで指定された引数をサブフォーマット化した文字列を返します。
		//
		template <typename Head, typename... Tail>
		inline static constexpr auto sub_format(size_t index, const String& sub_fmt, Head&& head, Tail&&... tail) -> String
		{
			// インデックスが無効の場合は
			if (index < 1)
			{
				// 引数が存在しなかった場合の処理を実行します。
				return sub_format(index, sub_fmt);
			}
			// インデックスが一致する場合は
			else if (index == 1)
			{
				// 引数をサブフォーマッタを使用して文字列化して返します。
				return use_sub_formatter(std::forward<Head>(head), sub_fmt);
			}
			// インデックスが一致しない場合は
			else
			{
				// インデックスをデクリメントし、次の引数を処理します。
				return sub_format(--index, sub_fmt, std::forward<Tail>(tail)...);
			}
		}

		//
		// フォーマット化された文字列を返します。
		//
		template <typename... Args>
		inline static constexpr auto format(const String& fmt, Args&&... args) -> String
		{
			//
			// このクラスはパターンを検索してその位置を保持します。
			//
			struct Finder {
				const String::value_type* head;
				const String::value_type* tail;
				Finder(const String::value_type* begin, const String::value_type* end, const String& pattern)
					: head(utils.find_str(begin, end, pattern))
					, tail(head ? head + pattern.length() : nullptr) {}
			};

			// フォーマットの開始位置です。
			auto fmt_begin = fmt.c_str();

			// フォーマットの終了位置です。
			auto fmt_end = fmt.c_str() + fmt.length();

			// 最終的に返す結果を格納する変数です。
			auto result = String {};

			// 現在の使用対象の引数のインデックスです。
			auto current_arg_index = size_t { 1 };

			// 現在の検索対象位置です。
			auto current_pos = fmt_begin;

			while (*current_pos)
			{
				// 開始位置を取得します。
				auto begin = Finder { current_pos, fmt_end, utils.keyword.begin };

				// 開始位置が取得できなかった場合はループを終了します。
				if (!begin.head)
					break;

				// 開始位置末尾が不正の場合はループを終了します。
				if (begin.tail >= fmt_end)
					break;

				// 現在の検索位置から開始位置までを結果に追加します。
				result += { current_pos, begin.head };

				// "{/...}"
				// エスケープされていない場合は
				if (begin.tail[0] != utils.keyword.escape[0])
				{
					// 終了位置を取得します。
					// 開始位置の末尾から検索します。
					auto end = Finder { begin.tail, fmt_end, utils.keyword.end };

					// 終了位置が取得できなかった場合はループを終了します。
					if (!end.head)
						break;

					// 文字列化に使用する引数のインデックスです。
					auto arg_index = current_arg_index;

					// 文字列化に使用するサブフォーマットです。
					auto sub_fmt = String {};

					// セパレータの位置を取得します。
					auto separator = Finder { begin.tail, end.head, utils.keyword.separator };

					// "{/arg_index$sub_fmt}"
					// セパレータが存在する場合は
					if (separator.head)
					{
						// 前半文字列から引数インデックスを取得します。
						arg_index = utils.to_int({ begin.tail, separator.head });

						// 後半文字列からサブフォーマットを取得します。
						sub_fmt = { separator.tail, end.head };
					}
					// "{/sub_fmt}"
					// セパレータが存在しない場合は
					else
					{
						// サブフォーマットを取得します。
						sub_fmt = { begin.tail, end.head };
					}

					// インデックスの位置にある引数を文字列化して結果に追加します。
					result += sub_format(arg_index, sub_fmt, args...);

					// 次の引数を使用対象にします。
					current_arg_index++;

					// 現在の検索対象位置を終了位置末尾に変更します。
					current_pos = end.tail;
				}
				// "{//..."
				// エスケープされている場合は
				else
				{
					// "{/"を結果に追加します。
					result += utils.keyword.begin;

					// 現在の検索対象位置を開始位置末尾に変更します。
					current_pos = begin.tail + 1;
				}
			}

			// 現在の検索位置から最後までを結果に追加します。
			result += { current_pos, fmt_end };

			return result;
		}
	};

	//
	// std::string型用のサブフォーマッタを作成して返します。
	//
	template <typename T, typename... Args>
	_NODISCARD constexpr auto sfs(const T& value, Args&&... args)
	{
		return make_sub_formatter_s()(value, args...);
	}

	//
	// std::wstring型用のサブフォーマッタを作成して返します。
	//
	template <typename T, typename... Args>
	_NODISCARD constexpr auto sfw(const T& value, Args&&... args)
	{
		return make_sub_formatter_w()(value, args...);
	}

	//
	// デフォルトのフォーマッタを使用して
	// フォーマット化された文字列を返します。
	//
	template <typename... Args>
	_NODISCARD auto format(const std::string& fmt, Args&&... args) -> std::string
	{
		return sprintfmt::Formatter<std::string, make_sub_formatter_s>::format(fmt, args...);
	}

	//
	// デフォルトのフォーマッタを使用して
	// フォーマット化されたワイド文字列を返します。
	//
	template <typename... Args>
	_NODISCARD auto format(const std::wstring& fmt, Args&&... args) -> std::wstring
	{
		return sprintfmt::Formatter<std::wstring, make_sub_formatter_w>::format(fmt, args...);
	}
}
