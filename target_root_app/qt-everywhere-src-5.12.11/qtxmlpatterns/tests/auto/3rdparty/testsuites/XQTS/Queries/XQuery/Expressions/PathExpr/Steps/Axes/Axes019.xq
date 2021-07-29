(: Name: Axes019 :)
(: Description: Test abbreviated '..' syntax from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/..)
