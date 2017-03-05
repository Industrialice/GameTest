#ifndef __SHADER_HPP__
#define __SHADER_HPP__

// TODO: this code is a failure

namespace GameTest
{
	inline constexpr ui32 FormUniformType( const ui16 major, const ui16 minor )
	{
		return (major << 16) | minor;
	}

	class ShaderUniform
	{
		string _name;

	public:
		enum class Type : ui32 
		{ 
			Tex2D = FormUniformType( 0, 0 ),
			Tex3D = FormUniformType( 1, 0 ),
			TexCube = FormUniformType( 2, 0 ), 
			Vec4F = FormUniformType( 3, 3 ),
			Vec3F = FormUniformType( 3, 2 ),
			Vec2F = FormUniformType( 3, 1 ),
			Float = FormUniformType( 3, 0 ),
			Vec4I = FormUniformType( 4, 3 ),
			Vec3I = FormUniformType( 4, 2 ),
			Vec2I = FormUniformType( 4, 1 ),
			Int = FormUniformType( 4, 0 ),
			Vec4UI = FormUniformType( 5, 3 ),
			Vec3UI = FormUniformType( 5, 2 ),
			Vec2UI = FormUniformType( 5, 1 ),
			UInt = FormUniformType( 5, 0 ),
			Boolean = FormUniformType( 6, 0 )
		};

		const string &Name() const
		{
			return _name;
		}

		template < typename StrType > void Name( StrType &&name )
		{
			_name = std::move( name );
		}

		virtual Type UnderlyingType() const = 0;
	};

	std::pair < ui16, ui16 > DissectUniformType( ShaderUniform::Type type )
	{
		ui16 major = (ui16)((ui32)type >> 16);
		ui16 minor = (ui16)type;
		return std::make_pair( major, minor );
	}

	class Shader
	{
	protected:
		struct UniformsPair
		{
			std::unique_ptr < ShaderUniform > defaultUniform;
			std::unique_ptr < ShaderUniform > currentUniform;
		};

		CVec < UniformsPair, void > _uniforms;
		string _name;

	public:
		const ShaderUniform *UniformGet( const char *name ) const
		{
			auto it = std::find_if( _uniforms.begin(), _uniforms.end(), [&name]( const UniformsPair &up ) { return up.defaultUniform->Name() == name; } );
			if( it != _uniforms.end() )
			{
				return it->currentUniform.get();
			}
			return nullptr;
		}
		
		const ShaderUniform *UniformGet( ui32 index ) const
		{
			if( index < _uniforms.Size() )
			{
				return _uniforms[ index ].currentUniform.get();
			}
			return nullptr;
		}
		
		bool UniformSet( const ShaderUniform &uniform )
		{
			auto it = std::find_if( _uniforms.begin(), _uniforms.end(), [&uniform]( const UniformsPair &up ) { return up.defaultUniform->Name() == uniform.Name(); } );
			if( it != _uniforms.end() )
			{
				if( &uniform != it->currentUniform.get() )
				{
					/*auto targetType = DissectUniformType( it->currentUniform->UnderlyingType() );
					auto sourceType = DissectUniformType( uniform.UnderlyingType() );

					if( targetType.first != sourceType.first )
					{
						return false;
					}

					if( targetType.second != sourceType.second )
					{
						return false;
					}*/

					if( uniform.UnderlyingType() != it->currentUniform->UnderlyingType() )
					{
						return false;
					}

					*it->currentUniform = uniform;
				}
				return true;
			}
			return false;
		}
		
		ui32 UniformsCount() const
		{
			return _uniforms.Size();
		}
	};

	struct ShaderUniformTex2D final : public ShaderUniform
	{
		std::shared_ptr < Texture > texture;
		vec2 scale { 1, 1 };
		vec2 offset { 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Tex2D;
		}
	};

	struct ShaderUniformTex3D final : public ShaderUniform
	{
		std::shared_ptr < Texture > texture;

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Tex3D;
		}
	};

	struct ShaderUniformTexCube final : public ShaderUniform
	{
		std::shared_ptr < Texture > texture;

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::TexCube;
		}
	};

	struct ShaderUniformVec4F final : public ShaderUniform
	{
		vec4 value { 0, 0, 0, 1 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec4F;
		}
	};

	struct ShaderUniformVec3F final : public ShaderUniform
	{
		vec3 value { 0, 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec3F;
		}
	};

	struct ShaderUniformVec2F final : public ShaderUniform
	{
		vec2 value { 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec2F;
		}
	};

	struct ShaderUniformFloat final : public ShaderUniform
	{
		f32 value { 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Float;
		}
	};

	struct ShaderUniformVec4I final : public ShaderUniform
	{
		i32 value[ 4 ] { 0, 0, 0, 1 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec4I;
		}
	};

	struct ShaderUniformVec3I final : public ShaderUniform
	{
		i32 value[ 3 ] { 0, 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec3I;
		}
	};

	struct ShaderUniformVec2I final : public ShaderUniform
	{
		i32 value[ 2 ] { 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec2I;
		}
	};

	struct ShaderUniformInt final : public ShaderUniform
	{
		i32 value { 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Int;
		}
	};

	struct ShaderUniformVec4UI final : public ShaderUniform
	{
		ui32 value[ 4 ] { 0, 0, 0, 1 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec4UI;
		}
	};

	struct ShaderUniformVec3UI final : public ShaderUniform
	{
		ui32 value[ 3 ] { 0, 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec3UI;
		}
	};

	struct ShaderUniformVec2UI final : public ShaderUniform
	{
		ui32 value[ 2 ] { 0, 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Vec2UI;
		}
	};

	struct ShaderUniformUInt final : public ShaderUniform
	{
		ui32 value { 0 };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::UInt;
		}
	};

	struct ShaderUniformBoolean final : public ShaderUniform
	{
		bool value { false };

		virtual Type UnderlyingType() const override
		{
			return ShaderUniform::Type::Boolean;
		}
	};
}

#endif