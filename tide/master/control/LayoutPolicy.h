#ifndef LAYOUTPOLICY_H
#define LAYOUTPOLICY_H

class LayoutPolicy
{
public:
    //TODO demander à raphaël l'utilité des constructeur pour une classe abstraite
    LayoutPolicy( const DisplayGroup& group );

    /** @return the focused coordinates for the window. */
    virtual QRectF getFocusedCoord( const ContentWindow& window ) const=0;

    /** Update the focused coordinates for the set of windows. */
    virtual void updateFocusedCoord( const ContentWindowSet& windows ) const=0;

protected:
    const DisplayGroup& _group;

};

#endif
