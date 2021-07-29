(: Name: fn-id-dtd-20 :)
(: Description: Evaluation of fn:id for which $arg uses the xs:string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id(xs:string("id1"), $input-context1/IDS[1])