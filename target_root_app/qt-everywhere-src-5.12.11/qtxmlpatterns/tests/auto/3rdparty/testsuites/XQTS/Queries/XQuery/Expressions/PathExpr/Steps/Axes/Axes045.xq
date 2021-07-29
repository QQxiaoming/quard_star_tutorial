(: Name: Axes045 :)
(: Description: Absolute path '/child::' with element name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/child::far-north)
