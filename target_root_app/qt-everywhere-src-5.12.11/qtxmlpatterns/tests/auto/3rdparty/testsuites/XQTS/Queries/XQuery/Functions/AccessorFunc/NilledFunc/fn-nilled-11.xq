(: Name: fn-nilled-11 :)
(: Description: Evaluation of nilled function as argument to fn:not function.  returns false. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(fn:nilled(<shoe xsi:nil="{fn:true()}"/>))