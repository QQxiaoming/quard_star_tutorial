(: Name: Axes071 :)
(: Description: Absolute path '//@' with specified name. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//@mark)
