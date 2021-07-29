(: Name: fn-nilled-6 :)
(: Description: Evaluation of nilled function with argument set to an element node with xsi:nill = false. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="false"/>)