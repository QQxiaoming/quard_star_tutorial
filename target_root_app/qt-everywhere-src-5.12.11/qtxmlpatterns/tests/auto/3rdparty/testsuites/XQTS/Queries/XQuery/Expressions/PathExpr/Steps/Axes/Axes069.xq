(: Name: Axes069 :)
(: Description: Absolute path '//attribute::' with specified name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//attribute::mark)
