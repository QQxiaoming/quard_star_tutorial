(: Name: Axes057 :)
(: Description: Absolute path '/descendant::' with specified element name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant::south)
