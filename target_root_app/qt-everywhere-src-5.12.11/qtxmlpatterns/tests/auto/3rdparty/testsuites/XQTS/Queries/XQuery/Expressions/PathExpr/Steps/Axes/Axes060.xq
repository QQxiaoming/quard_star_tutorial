(: Name: Axes060 :)
(: Description: Absolute path '/descendant-or-self::' with specified element name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant-or-self::south)
