(: Name: fn-nilled-18 :)
(: Description: Evaluation of nilled function used as argument to xs:boolean function. Returns false :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:boolean(fn:nilled(<shoe xsi:nil="{fn:false()}"/>))