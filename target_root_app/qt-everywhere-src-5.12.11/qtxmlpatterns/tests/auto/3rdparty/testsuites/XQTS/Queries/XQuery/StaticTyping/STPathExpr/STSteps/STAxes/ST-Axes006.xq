(: Name: ST-Axes006 :)
(: Description: self::name from a text node gets nothing, because node kind differs :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/text()/self::center)
