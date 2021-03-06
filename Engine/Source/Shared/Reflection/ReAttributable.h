#ifndef __REFLECTION_ATTRIBUTABLE_H__
#define __REFLECTION_ATTRIBUTABLE_H__

#include "Reflection/ReAttribute.h"

#include <deque>

//////////////////////////////////////////////////////////////////////////
// ReAttributable
class ReAttributable:
    public RePrimitive
{
public:
    REFLECTION_DECLARE_DERIVED( ReAttributable, RePrimitive );

public:
    ReAttributable();
    virtual ~ReAttributable();

	/**
	 * Add attribute.
	 */
	ReAttributable& addAttribute( ReAttribute* Attribute );

	/**
	 * Get attribute.
	 */
	ReAttribute* getAttribute( const ReClass* Class ) const;

	/**
	 * Get attribute.
	 */
	template< class _Ty >
	_Ty* getAttribute() const
	{
		return static_cast< _Ty* >( getAttribute( _Ty::StaticGetClass() ) );
	}

private:
	std::deque< ReAttribute* > Attributes_;
};

#endif // __REFLECTION_ATTRIBUTE_H__
