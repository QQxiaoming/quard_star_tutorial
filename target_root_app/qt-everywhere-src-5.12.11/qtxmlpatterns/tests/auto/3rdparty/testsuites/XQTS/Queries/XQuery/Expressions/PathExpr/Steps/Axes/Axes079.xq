(: Name: Axes079 :)
(: Description: Test implied child axis in '//node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//node())
