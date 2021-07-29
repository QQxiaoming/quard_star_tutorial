(: Name: Axes063 :)
(: Description: Absolute path '//child::' with specified element name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//child::south)
