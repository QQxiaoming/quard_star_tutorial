(: Name: Axes066 :)
(: Description: Absolute path with element name after // implies child axis. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//south)
