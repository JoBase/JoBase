import shapely.geometry

def point_collides_with_shape(x, y, shape):
    point = shapely.geometry.Point(x, y)
    shape = shapely.geometry.Polygon(shape)

    return shape.contains(point)

def shape_collides_with_shape(shape1, shape2):
    shape1 = shapely.geometry.Polygon(shape1)
    shape2 = shapely.geometry.Polygon(shape2)
    
    return shape1.intersects(shape2)
    
def point_collides_with_point(x1, y1, x2, y2):
    return x1 is x2 and y1 is y2