(: Name: fn-id-dtd-21 :)
(: Description: Evaluation of fn:id for which $arg uses the fn:string-join function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:string-join(("id","1"),""), $input-context1/IDS[1])