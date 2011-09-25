/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/
#ifndef RECT_H
#define RECT_H

#include <algorithm>

template <typename T> 
struct PointT
{
  T x1;
  T y1;

  PointT() : x1(0), y1(0) {}
  PointT(const T &xx, const T &yy) : x1(xx), y1(yy) {}
  PointT(const PointT &p) : x1(p.x1), y1(p.y1) {}

  T x() const {return this->x1;}
  T y() const {return this->y1;}

  PointT& operator = (const PointT &p) {x1 = p.x1; y1 = p.y1; return *this;}

  PointT operator + (const PointT &p) const {return PointT(x1 + p.x1, y1 + p.y1);}
  PointT operator - (const PointT &p) const {return PointT(x1 - p.x1, y1 - p.y1);}
  bool operator == (const PointT &p) const {return (x1 == p.x1 && y1 == p.y1);}
  bool operator < (const PointT &p) const {return (x1 < p.x1 ? true : (x1 == p.x1 ? y1 < p.y1 : false));}
};

template <typename T> 
struct SizeT
{
  T w;
  T h;

  SizeT(const T &ww, const T &hh) : w(ww), h(hh) {}
  SizeT(const SizeT &p) : h(p.w), h(p.h) {}
};


template <typename T>
class Rect
{
public:
    Rect() { x1 = y1 = 0; x2 = y2 = -1; }
    Rect(const PointT<T> &topleft, const PointT<T> &bottomright);
    Rect(const PointT<T> &topleft, const SizeT<T> &size);
    Rect(T left, T top, T width, T height);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    T left() const;
    T top() const;
    T right() const;
    T bottom() const;
    Rect normalized() const;

    T x() const;
    T y() const;
    void setLeft(T pos);
    void setTop(T pos);
    void setRight(T pos);
    void setBottom(T pos);
    void setX(T x);
    void setY(T y);

    void setTopLeft(const PointT<T> &p);
    void setBottomRight(const PointT<T> &p);
    void setTopRight(const PointT<T> &p);
    void setBottomLeft(const PointT<T> &p);

    PointT<T> topLeft() const;
    PointT<T> bottomRight() const;
    PointT<T> topRight() const;
    PointT<T> bottomLeft() const;
    PointT<T> center() const;

    void moveLeft(T pos);
    void moveTop(T pos);
    void moveRight(T pos);
    void moveBottom(T pos);
    void moveTopLeft(const PointT<T> &p);
    void moveBottomRight(const PointT<T> &p);
    void moveTopRight(const PointT<T> &p);
    void moveBottomLeft(const PointT<T> &p);
    void moveCenter(const PointT<T> &p);

    inline void translate(T dx, T dy);
    inline void translate(const PointT<T> &p);
    inline Rect translated(T dx, T dy) const;
    inline Rect translated(const PointT<T> &p) const;

    void moveTo(T x, T t);
    void moveTo(const PointT<T> &p);

    void setRect(T x, T y, T w, T h);
    inline void getRect(T *x, T *y, T *w, T *h) const;

    void setCoords(T x1, T y1, T x2, T y2);
    inline void getCoords(T *x1, T *y1, T *x2, T *y2) const;

    inline void adjust(T x1, T y1, T x2, T y2);
    inline Rect adjusted(T x1, T y1, T x2, T y2) const;

    SizeT<T> size() const;
    T width() const;
    T height() const;
    void setWidth(T w);
    void setHeight(T h);
    void setSize(const SizeT<T> &s);

    Rect operator|(const Rect &r) const;
    Rect operator&(const Rect &r) const;
    Rect& operator|=(const Rect &r);
    Rect& operator&=(const Rect &r);

    bool contains(const PointT<T> &p, bool proper=false) const;
    bool contains(T x, T y) const; // inline methods, _don't_ merge these
    bool contains(T x, T y, bool proper) const;
    bool contains(const Rect &r, bool proper = false) const;
    Rect unite(const Rect &r) const;  // ### Qt 5: make QT4_SUPPORT
    Rect united(const Rect &other) const;
    Rect intersect(const Rect &r) const;  // ### Qt 5: make QT4_SUPPORT
    Rect intersected(const Rect &other) const;
    bool intersects(const Rect &r) const;

private:
    T x1;
    T y1;
    T x2;
    T y2;
};

/*****************************************************************************
  Rect inline member functions
 *****************************************************************************/

template <typename T> inline Rect<T>::Rect(T aleft, T atop, T awidth, T aheight)
{
    x1 = aleft;
    y1 = atop;
    x2 = (aleft + awidth - 1);
    y2 = (atop + aheight - 1);
}

template <typename T> inline Rect<T>::Rect(const PointT<T> &atopLeft, const PointT<T> &abottomRight)
{
    x1 = atopLeft.x();
    y1 = atopLeft.y();
    x2 = abottomRight.x();
    y2 = abottomRight.y();
}

template <typename T> inline Rect<T>::Rect(const PointT<T> &atopLeft, const SizeT<T> &asize)
{
    x1 = atopLeft.x();
    y1 = atopLeft.y();
    x2 = (x1+asize.width() - 1);
    y2 = (y1+asize.height() - 1);
}

template <typename T> inline bool Rect<T>::isNull() const
{ return x2 == x1 - 1 && y2 == y1 - 1; }

template <typename T> inline bool Rect<T>::isEmpty() const
{ return x1 > x2 || y1 > y2; }

template <typename T> inline bool Rect<T>::isValid() const
{ return x1 <= x2 && y1 <= y2; }

template <typename T> inline T Rect<T>::left() const
{ return x1; }

template <typename T> inline T Rect<T>::top() const
{ return y1; }

template <typename T> inline T Rect<T>::right() const
{ return x2; }

template <typename T> inline T Rect<T>::bottom() const
{ return y2; }

template <typename T> inline T Rect<T>::x() const
{ return x1; }

template <typename T> inline T Rect<T>::y() const
{ return y1; }

template <typename T> inline void Rect<T>::setLeft(T pos)
{ x1 = pos; }

template <typename T> inline void Rect<T>::setTop(T pos)
{ y1 = pos; }

template <typename T> inline void Rect<T>::setRight(T pos)
{ x2 = pos; }

template <typename T> inline void Rect<T>::setBottom(T pos)
{ y2 = pos; }

template <typename T> inline void Rect<T>::setTopLeft(const PointT<T> &p)
{ x1 = p.x(); y1 = p.y(); }

template <typename T> inline void Rect<T>::setBottomRight(const PointT<T> &p)
{ x2 = p.x(); y2 = p.y(); }

template <typename T> inline void Rect<T>::setTopRight(const PointT<T> &p)
{ x2 = p.x(); y1 = p.y(); }

template <typename T> inline void Rect<T>::setBottomLeft(const PointT<T> &p)
{ x1 = p.x(); y2 = p.y(); }

template <typename T> inline void Rect<T>::setX(T ax)
{ x1 = ax; }

template <typename T> inline void Rect<T>::setY(T ay)
{ y1 = ay; }

template <typename T> inline PointT<T> Rect<T>::topLeft() const
{ return PointT<T>(x1, y1); }

template <typename T> inline PointT<T> Rect<T>::bottomRight() const
{ return PointT<T>(x2, y2); }

template <typename T> inline PointT<T> Rect<T>::topRight() const
{ return PointT<T>(x2, y1); }

template <typename T> inline PointT<T> Rect<T>::bottomLeft() const
{ return PointT<T>(x1, y2); }

template <typename T> inline PointT<T> Rect<T>::center() const
{ return PointT<T>((x1+x2)/2, (y1+y2)/2); }

template <typename T> inline T Rect<T>::width() const
{ return  x2 - x1 + 1; }

template <typename T> inline T Rect<T>::height() const
{ return  y2 - y1 + 1; }

template <typename T> inline SizeT<T> Rect<T>::size() const
{ return SizeT<T>(width(), height()); }

template <typename T> inline void Rect<T>::translate(T dx, T dy)
{
    x1 += dx;
    y1 += dy;
    x2 += dx;
    y2 += dy;
}

template <typename T> inline void Rect<T>::translate(const PointT<T> &p)
{
    x1 += p.x();
    y1 += p.y();
    x2 += p.x();
    y2 += p.y();
}

template <typename T> inline Rect<T> Rect<T>::translated(T dx, T dy) const
{ return Rect(PointT<T>(x1 + dx, y1 + dy), PointT<T>(x2 + dx, y2 + dy)); }

template <typename T> inline Rect<T> Rect<T>::translated(const PointT<T> &p) const
{ return QRect(PointT<T>(x1 + p.x(), y1 + p.y()), PointT<T>(x2 + p.x(), y2 + p.y())); }

template <typename T> inline void Rect<T>::moveTo(T ax, T ay)
{
    x2 += ax - x1;
    y2 += ay - y1;
    x1 = ax;
    y1 = ay;
}

template <typename T> inline void Rect<T>::moveTo(const PointT<T> &p)
{
    x2 += p.x() - x1;
    y2 += p.y() - y1;
    x1 = p.x();
    y1 = p.y();
}

template <typename T> inline void Rect<T>::moveLeft(T pos)
{ x2 += (pos - x1); x1 = pos; }

template <typename T> inline void Rect<T>::moveTop(T pos)
{ y2 += (pos - y1); y1 = pos; }

template <typename T> inline void Rect<T>::moveRight(T pos)
{
    x1 += (pos - x2);
    x2 = pos;
}

template <typename T> inline void Rect<T>::moveBottom(T pos)
{
    y1 += (pos - y2);
    y2 = pos;
}

template <typename T> inline void Rect<T>::moveTopLeft(const PointT<T> &p)
{
    moveLeft(p.x());
    moveTop(p.y());
}

template <typename T> inline void Rect<T>::moveBottomRight(const PointT<T> &p)
{
    moveRight(p.x());
    moveBottom(p.y());
}

template <typename T> inline void Rect<T>::moveTopRight(const PointT<T> &p)
{
    moveRight(p.x());
    moveTop(p.y());
}

template <typename T> inline void Rect<T>::moveBottomLeft(const PointT<T> &p)
{
    moveLeft(p.x());
    moveBottom(p.y());
}

template <typename T> inline void Rect<T>::getRect(T *ax, T *ay, T *aw, T *ah) const
{
    *ax = x1;
    *ay = y1;
    *aw = x2 - x1 + 1;
    *ah = y2 - y1 + 1;
}

template <typename T> inline void Rect<T>::setRect(T ax, T ay, T aw, T ah)
{
    x1 = ax;
    y1 = ay;
    x2 = (ax + aw - 1);
    y2 = (ay + ah - 1);
}

template <typename T> inline void Rect<T>::getCoords(T *xp1, T *yp1, T *xp2, T *yp2) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

template <typename T> inline void Rect<T>::setCoords(T xp1, T yp1, T xp2, T yp2)
{
    x1 = xp1;
    y1 = yp1;
    x2 = xp2;
    y2 = yp2;
}

template <typename T> inline Rect<T> Rect<T>::adjusted(T xp1, T yp1, T xp2, T yp2) const
{ return Rect(PointT<T>(x1 + xp1, y1 + yp1), PointT<T>(x2 + xp2, y2 + yp2)); }

template <typename T> inline void Rect<T>::adjust(T dx1, T dy1, T dx2, T dy2)
{
    x1 += dx1;
    y1 += dy1;
    x2 += dx2;
    y2 += dy2;
}

template <typename T> inline void Rect<T>::setWidth(T w)
{ x2 = (x1 + w - 1); }

template <typename T> inline void Rect<T>::setHeight(T h)
{ y2 = (y1 + h - 1); }

template <typename T> inline void Rect<T>::setSize(const SizeT<T> &s)
{
    x2 = (s.width()  + x1 - 1);
    y2 = (s.height() + y1 - 1);
}

template <typename T> inline bool Rect<T>::contains(T ax, T ay, bool aproper) const
{
    return contains(PointT<T>(ax, ay), aproper);
}

template <typename T> inline bool Rect<T>::contains(T ax, T ay) const
{
    return contains(PointT<T>(ax, ay), false);
}

template <typename T> inline Rect<T>& Rect<T>::operator|=(const Rect<T> &r)
{
    *this = *this | r;
    return *this;
}

template <typename T> inline Rect<T>& Rect<T>::operator&=(const Rect<T> &r)
{
    *this = *this & r;
    return *this;
}

template <typename T> inline Rect<T> Rect<T>::intersect(const Rect<T> &r) const
{
    return *this & r;
}

template <typename T> inline Rect<T> Rect<T>::intersected(const Rect <T>&other) const
{
    return intersect(other);
}

template <typename T> inline Rect<T> Rect<T>::unite(const Rect<T> &r) const
{
    return *this | r;
}

template <typename T> inline Rect<T> Rect<T>::united(const Rect<T> &r) const
{
     return unite(r);
}

template <typename T> inline bool operator==(const Rect<T> &r1, const Rect<T> &r2)
{
    return r1.x1==r2.x1 && r1.x2==r2.x2 && r1.y1==r2.y1 && r1.y2==r2.y2;
}

template <typename T> inline bool operator!=(const Rect<T> &r1, const Rect<T> &r2)
{
    return r1.x1!=r2.x1 || r1.x2!=r2.x2 || r1.y1!=r2.y1 || r1.y2!=r2.y2;
}


template <typename T>
Rect<T> Rect<T>::normalized() const
{
    Rect r;
    if (x2 < x1 - 1) {                                // swap bad x values
        r.x1 = x2;
        r.x2 = x1;
    } else {
        r.x1 = x1;
        r.x2 = x2;
    }
    if (y2 < y1 - 1) {                                // swap bad y values
        r.y1 = y2;
        r.y2 = y1;
    } else {
        r.y1 = y1;
        r.y2 = y2;
    }
    return r;
}


template <typename T>
void Rect<T>::moveCenter(const PointT<T> &p)
{
    T w = x2 - x1;
    T h = y2 - y1;
    x1 = p.x() - w/2;
    y1 = p.y() - h/2;
    x2 = x1 + w;
    y2 = y1 + h;
}


template <typename T>
bool Rect<T>::contains(const PointT<T> &p, bool proper) const
{
    T l, r;
    if (x2 < x1 - 1) {
        l = x2;
        r = x1;
    } else {
        l = x1;
        r = x2;
    }
    if (proper) {
        if (p.x() <= l || p.x() >= r)
            return false;
    } else {
        if (p.x() < l || p.x() > r)
            return false;
    }
    int t, b;
    if (y2 < y1 - 1) {
        t = y2;
        b = y1;
    } else {
        t = y1;
        b = y2;
    }
    if (proper) {
        if (p.y() <= t || p.y() >= b)
            return false;
    } else {
        if (p.y() < t || p.y() > b)
            return false;
    }
    return true;
}


template <typename T>
bool Rect<T>::contains(const Rect<T> &r, bool proper) const
{
    if (isNull() || r.isNull())
        return false;

    T l1 = x1;
    T r1 = x1;
    if (x2 - x1 + 1 < 0)
        l1 = x2;
    else
        r1 = x2;

    T l2 = r.x1;
    T r2 = r.x1;
    if (r.x2 - r.x1 + 1 < 0)
        l2 = r.x2;
    else
        r2 = r.x2;

    if (proper) {
        if (l2 <= l1 || r2 >= r1)
            return false;
    } else {
        if (l2 < l1 || r2 > r1)
            return false;
    }

    T t1 = y1;
    T b1 = y1;
    if (y2 - y1 + 1 < 0)
        t1 = y2;
    else
        b1 = y2;

    T t2 = r.y1;
    T b2 = r.y1;
    if (r.y2 - r.y1 + 1 < 0)
        t2 = r.y2;
    else
        b2 = r.y2;

    if (proper) {
        if (t2 <= t1 || b2 >= b1)
            return false;
    } else {
        if (t2 < t1 || b2 > b1)
            return false;
    }

    return true;
}


template <typename T>
Rect<T> Rect<T>::operator|(const Rect<T> &r) const
{
    if (isNull())
        return r;
    if (r.isNull())
        return *this;

    T l1 = x1;
    T r1 = x1;
    if (x2 - x1 + 1 < 0)
        l1 = x2;
    else
        r1 = x2;

    T l2 = r.x1;
    T r2 = r.x1;
    if (r.x2 - r.x1 + 1 < 0)
        l2 = r.x2;
    else
        r2 = r.x2;

    T t1 = y1;
    T b1 = y1;
    if (y2 - y1 + 1 < 0)
        t1 = y2;
    else
        b1 = y2;

    T t2 = r.y1;
    T b2 = r.y1;
    if (r.y2 - r.y1 + 1 < 0)
        t2 = r.y2;
    else
        b2 = r.y2;

    Rect tmp;
    tmp.x1 = std::min(l1, l2);
    tmp.x2 = std::max(r1, r2);
    tmp.y1 = std::min(t1, t2);
    tmp.y2 = std::max(b1, b2);
    return tmp;
}


template <typename T>
Rect<T> Rect<T>::operator&(const Rect<T> &r) const
{
    if (isNull() || r.isNull())
        return Rect();

    T l1 = x1;
    T r1 = x1;
    if (x2 - x1 + 1 < 0)
        l1 = x2;
    else
        r1 = x2;

    T l2 = r.x1;
    T r2 = r.x1;
    if (r.x2 - r.x1 + 1 < 0)
        l2 = r.x2;
    else
        r2 = r.x2;

    if (l1 > r2 || l2 > r1)
        return Rect();

    T t1 = y1;
    T b1 = y1;
    if (y2 - y1 + 1 < 0)
        t1 = y2;
    else
        b1 = y2;

    T t2 = r.y1;
    T b2 = r.y1;
    if (r.y2 - r.y1 + 1 < 0)
        t2 = r.y2;
    else
        b2 = r.y2;

    if (t1 > b2 || t2 > b1)
        return Rect();

    Rect tmp;
    tmp.x1 = std::max(l1, l2);
    tmp.x2 = std::min(r1, r2);
    tmp.y1 = std::max(t1, t2);
    tmp.y2 = std::min(b1, b2);
    return tmp;
}


template <typename T>
bool Rect<T>::intersects(const Rect<T> &r) const
{
    if (isNull() || r.isNull())
        return false;

    T l1 = x1;
    T r1 = x1;
    if (x2 - x1 + 1 < 0)
        l1 = x2;
    else
        r1 = x2;

    T l2 = r.x1;
    T r2 = r.x1;
    if (r.x2 - r.x1 + 1 < 0)
        l2 = r.x2;
    else
        r2 = r.x2;

    if (l1 > r2 || l2 > r1)
        return false;

    T t1 = y1;
    T b1 = y1;
    if (y2 - y1 + 1 < 0)
        t1 = y2;
    else
        b1 = y2;

    T t2 = r.y1;
    T b2 = r.y1;
    if (r.y2 - r.y1 + 1 < 0)
        t2 = r.y2;
    else
        b2 = r.y2;

    if (t1 > b2 || t2 > b1)
        return false;

    return true;
}


#endif // RECT_H
