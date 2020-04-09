class ColorRange
{

public:
    int *redRange;
    int *greenRange;
    int *blueRange;

    ColorRange(int _redRange[2], int _greenRange[2], int _blueRange[2])
    {
        redRange = _redRange;
        greenRange = _greenRange;
        blueRange = _blueRange;
    }

    bool inRange(int v, int range[2])
    {
        if (range[0] <= v && v <= range[1])
        {
            return true;
        }
        return false;
    }

    bool match(Color c)
    {
        if (inRange(c.r, redRange) && inRange(c.g, greenRange) && inRange(c.b, blueRange))
        {
            return true;
        }
        return false;
    }
};

class Color
{
public:
    int r, g, b;
    Color(int r, int g, int b)
    {
        r = r;
        g = g;
        b = b;
    }
};