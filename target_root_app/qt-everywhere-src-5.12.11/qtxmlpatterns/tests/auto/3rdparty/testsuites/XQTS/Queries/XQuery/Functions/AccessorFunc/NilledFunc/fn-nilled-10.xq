(: Name: fn-nilled-10 :)
(: Description: Evaluation of nilled function as argument to fn:not function.  returns true. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(fn:nilled(<shoe xsi:nil="{fn:false()}"/>))