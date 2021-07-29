(: Name: fn-nilled-3 :)
(: Description: Evaluation of nilled function with argument set to an element node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled($input-context1/works[1]/employee[2])