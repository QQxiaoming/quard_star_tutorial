(: Name: Axes064 :)
(: Description: Test '//child::node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//child::node())
