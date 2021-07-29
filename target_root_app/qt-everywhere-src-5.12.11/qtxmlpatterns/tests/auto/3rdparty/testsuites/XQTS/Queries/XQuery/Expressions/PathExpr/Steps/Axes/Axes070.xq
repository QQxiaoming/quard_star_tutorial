(: Name: Axes070 :)
(: Description: Absolute path '//@*' has abbreviated syntax. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//@*)
